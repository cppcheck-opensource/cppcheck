/*
 * Cppcheck - A tool for static C/C++ code analysis
 * Copyright (C) 2007-2025 Cppcheck team.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "sehwrapper.h"

#ifdef USE_WINDOWS_SEH

#include "utils.h"

#include <windows.h>
#include <dbghelp.h>
#include <tchar.h>

static FILE* sehOutput = stdout;

void set_seh_output(FILE* f)
{
    sehOutput = f;
}

namespace {
    const ULONG maxnamelength = 512;
    struct IMAGEHLP_SYMBOL64_EXT : public IMAGEHLP_SYMBOL64 {
        TCHAR nameExt[maxnamelength]; // actually no need to worry about character encoding here
    };
    typedef BOOL (WINAPI *fpStackWalk64)(DWORD, HANDLE, HANDLE, LPSTACKFRAME64, PVOID, PREAD_PROCESS_MEMORY_ROUTINE64, PFUNCTION_TABLE_ACCESS_ROUTINE64, PGET_MODULE_BASE_ROUTINE64, PTRANSLATE_ADDRESS_ROUTINE64);
    fpStackWalk64 pStackWalk64;
    typedef DWORD64 (WINAPI *fpSymGetModuleBase64)(HANDLE, DWORD64);
    fpSymGetModuleBase64 pSymGetModuleBase64;
    typedef BOOL (WINAPI *fpSymGetSymFromAddr64)(HANDLE, DWORD64, PDWORD64, PIMAGEHLP_SYMBOL64);
    fpSymGetSymFromAddr64 pSymGetSymFromAddr64;
    typedef BOOL (WINAPI *fpSymGetLineFromAddr64)(HANDLE, DWORD64, PDWORD, PIMAGEHLP_LINE64);
    fpSymGetLineFromAddr64 pSymGetLineFromAddr64;
    typedef DWORD (WINAPI *fpUnDecorateSymbolName)(const TCHAR*, PTSTR, DWORD, DWORD);
    fpUnDecorateSymbolName pUnDecorateSymbolName;
    typedef PVOID (WINAPI *fpSymFunctionTableAccess64)(HANDLE, DWORD64);
    fpSymFunctionTableAccess64 pSymFunctionTableAccess64;
    typedef BOOL (WINAPI *fpSymInitialize)(HANDLE, PCSTR, BOOL);
    fpSymInitialize pSymInitialize;

    HMODULE hLibDbgHelp;
// avoid explicit dependency on Dbghelp.dll
    bool loadDbgHelp()
    {
        hLibDbgHelp = ::LoadLibraryW(L"Dbghelp.dll");
        if (!hLibDbgHelp)
            return false;
        pStackWalk64 = (fpStackWalk64) ::GetProcAddress(hLibDbgHelp, "StackWalk64");
        pSymGetModuleBase64 = (fpSymGetModuleBase64) ::GetProcAddress(hLibDbgHelp, "SymGetModuleBase64");
        pSymGetSymFromAddr64 = (fpSymGetSymFromAddr64) ::GetProcAddress(hLibDbgHelp, "SymGetSymFromAddr64");
        pSymGetLineFromAddr64 = (fpSymGetLineFromAddr64)::GetProcAddress(hLibDbgHelp, "SymGetLineFromAddr64");
        pSymFunctionTableAccess64 = (fpSymFunctionTableAccess64)::GetProcAddress(hLibDbgHelp, "SymFunctionTableAccess64");
        pSymInitialize = (fpSymInitialize) ::GetProcAddress(hLibDbgHelp, "SymInitialize");
        pUnDecorateSymbolName = (fpUnDecorateSymbolName)::GetProcAddress(hLibDbgHelp, "UnDecorateSymbolName");
        return true;
    }


    void printCallstack(FILE* outputFile, PEXCEPTION_POINTERS ex)
    {
        if (!loadDbgHelp())
            return;
        const HANDLE hProcess   = GetCurrentProcess();
        const HANDLE hThread    = GetCurrentThread();
        pSymInitialize(
            hProcess,
            nullptr,
            TRUE
            );
        CONTEXT context = *(ex->ContextRecord);
        STACKFRAME64 stack= {0};
        stack.AddrPC.Mode    = AddrModeFlat;
        stack.AddrStack.Mode = AddrModeFlat;
        stack.AddrFrame.Mode = AddrModeFlat;
#if defined(_M_IX86)
        stack.AddrPC.Offset    = context.Eip;
        stack.AddrStack.Offset = context.Esp;
        stack.AddrFrame.Offset = context.Ebp;
#elif defined(_M_AMD64)
        stack.AddrPC.Offset    = context.Rip;
        stack.AddrStack.Offset = context.Rsp;
        stack.AddrFrame.Offset = context.Rsp;
#elif defined(_M_ARM64)
        stack.AddrPC.Offset    = context.Pc;
        stack.AddrStack.Offset = context.Sp;
        stack.AddrFrame.Offset = context.Fp;
#else
#error Platform not supported!
#endif
        IMAGEHLP_SYMBOL64_EXT symbol;
        symbol.SizeOfStruct  = sizeof(IMAGEHLP_SYMBOL64);
        symbol.MaxNameLength = maxnamelength;
        DWORD64 displacement   = 0;
        int beyond_main=-1; // emergency exit, see below
        for (ULONG frame = 0; ; frame++) {
            BOOL result = pStackWalk64
                          (
#if defined(_M_IX86)
                IMAGE_FILE_MACHINE_I386,
#elif defined(_M_AMD64)
                IMAGE_FILE_MACHINE_AMD64,
#elif defined(_M_ARM64)
                IMAGE_FILE_MACHINE_ARM64,
#endif
                hProcess,
                hThread,
                &stack,
                &context,
                nullptr,
                pSymFunctionTableAccess64,
                pSymGetModuleBase64,
                nullptr
                          );
            if (!result)  // official end...
                break;
            pSymGetSymFromAddr64(hProcess, (ULONG64)stack.AddrPC.Offset, &displacement, &symbol);
            TCHAR undname[maxnamelength]= {0};
            pUnDecorateSymbolName((const TCHAR*)symbol.Name, (PTSTR)undname, (DWORD)getArrayLength(undname), UNDNAME_COMPLETE);
            if (beyond_main>=0)
                ++beyond_main;
            if (_tcscmp(undname, _T("main"))==0)
                beyond_main=0;
            fprintf(outputFile,
                    "%lu. 0x%08I64X in ",
                    frame, (ULONG64)stack.AddrPC.Offset);
            fputs((const char *)undname, outputFile);
            fputc('\n', outputFile);
            if (0==stack.AddrReturn.Offset || beyond_main>2) // StackWalk64() sometimes doesn't reach any end...
                break;
        }

        FreeLibrary(hLibDbgHelp);
        hLibDbgHelp=nullptr;
    }

    void writeMemoryErrorDetails(FILE* outputFile, PEXCEPTION_POINTERS ex, const char* description)
    {
        fputs(description, outputFile);
        fprintf(outputFile, " (instruction: 0x%p) ", ex->ExceptionRecord->ExceptionAddress);
        // Using %p for ULONG_PTR later on, so it must have size identical to size of pointer
        // This is not the universally portable solution but good enough for Win32/64
        C_ASSERT(sizeof(void*) == sizeof(ex->ExceptionRecord->ExceptionInformation[1]));
        switch (ex->ExceptionRecord->ExceptionInformation[0]) {
        case 0:
            fprintf(outputFile, "reading from 0x%p",
                    reinterpret_cast<void*>(ex->ExceptionRecord->ExceptionInformation[1]));
            break;
        case 1:
            fprintf(outputFile, "writing to 0x%p",
                    reinterpret_cast<void*>(ex->ExceptionRecord->ExceptionInformation[1]));
            break;
        case 8:
            fprintf(outputFile, "data execution prevention at 0x%p",
                    reinterpret_cast<void*>(ex->ExceptionRecord->ExceptionInformation[1]));
            break;
        default:
            break;
        }
    }

    /*
     * Any evaluation of the exception needs to be done here!
     */
    int filterException(FILE *outputFile, int code, PEXCEPTION_POINTERS ex)
    {
        fputs("Internal error: ", outputFile);
        switch (ex->ExceptionRecord->ExceptionCode) {
        case EXCEPTION_ACCESS_VIOLATION:
            writeMemoryErrorDetails(outputFile, ex, "Access violation");
            break;
        case EXCEPTION_ARRAY_BOUNDS_EXCEEDED:
            fputs("Out of array bounds", outputFile);
            break;
        case EXCEPTION_BREAKPOINT:
            fputs("Breakpoint", outputFile);
            break;
        case EXCEPTION_DATATYPE_MISALIGNMENT:
            fputs("Misaligned data", outputFile);
            break;
        case EXCEPTION_FLT_DENORMAL_OPERAND:
            fputs("Denormalized floating-point value", outputFile);
            break;
        case EXCEPTION_FLT_DIVIDE_BY_ZERO:
            fputs("Floating-point divide-by-zero", outputFile);
            break;
        case EXCEPTION_FLT_INEXACT_RESULT:
            fputs("Inexact floating-point value", outputFile);
            break;
        case EXCEPTION_FLT_INVALID_OPERATION:
            fputs("Invalid floating-point operation", outputFile);
            break;
        case EXCEPTION_FLT_OVERFLOW:
            fputs("Floating-point overflow", outputFile);
            break;
        case EXCEPTION_FLT_STACK_CHECK:
            fputs("Floating-point stack overflow", outputFile);
            break;
        case EXCEPTION_FLT_UNDERFLOW:
            fputs("Floating-point underflow", outputFile);
            break;
        case EXCEPTION_GUARD_PAGE:
            fputs("Page-guard access", outputFile);
            break;
        case EXCEPTION_ILLEGAL_INSTRUCTION:
            fputs("Illegal instruction", outputFile);
            break;
        case EXCEPTION_IN_PAGE_ERROR:
            writeMemoryErrorDetails(outputFile, ex, "Invalid page access");
            break;
        case EXCEPTION_INT_DIVIDE_BY_ZERO:
            fputs("Integer divide-by-zero", outputFile);
            break;
        case EXCEPTION_INT_OVERFLOW:
            fputs("Integer overflow", outputFile);
            break;
        case EXCEPTION_INVALID_DISPOSITION:
            fputs("Invalid exception dispatcher", outputFile);
            break;
        case EXCEPTION_INVALID_HANDLE:
            fputs("Invalid handle", outputFile);
            break;
        case EXCEPTION_NONCONTINUABLE_EXCEPTION:
            fputs("Non-continuable exception", outputFile);
            break;
        case EXCEPTION_PRIV_INSTRUCTION:
            fputs("Invalid instruction", outputFile);
            break;
        case EXCEPTION_SINGLE_STEP:
            fputs("Single instruction step", outputFile);
            break;
        case EXCEPTION_STACK_OVERFLOW:
            fputs("Stack overflow", outputFile);
            break;
        default:
            fprintf(outputFile, "Unknown exception (%d)\n",
                    code);
            break;
        }
        fputc('\n', outputFile);
        printCallstack(outputFile, ex);
        fputs("Please report this to the cppcheck developers!\n", outputFile);
        fflush(outputFile);
        return EXCEPTION_EXECUTE_HANDLER;
    }
}

namespace internal
{
    int filter_seh_exeception(int code, void* ex)
    {
        return filterException(sehOutput, code, static_cast<PEXCEPTION_POINTERS>(ex));
    }
}

#endif
