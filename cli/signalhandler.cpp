/*
 * Cppcheck - A tool for static C/C++ code analysis
 * Copyright (C) 2007-2026 Cppcheck team.
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

#if defined(__CYGWIN__)
#define _BSD_SOURCE // required to have siginfo_t, SIGSTKSZ, kill(), sigaction and others
#endif

#include "signalhandler.h"

#if defined(USE_UNIX_SIGNAL_HANDLING)

#ifdef USE_UNIX_BACKTRACE_SUPPORT
#include "stacktrace.h"
#endif

#include <csignal>
#include <cstdio>
#include <cstdlib>
#include <cstring>
//#include <features.h> // __USE_DYNAMIC_STACK_SIZE
#include <map>
#include <string>
#include <sys/types.h>
#include <unistd.h>
#include <utility>

#if defined(__linux__) && defined(REG_ERR)
#include <sys/syscall.h>
#endif

#if defined(__APPLE__)
#   define _XOPEN_SOURCE // ucontext.h APIs can only be used on Mac OSX >= 10.7 if _XOPEN_SOURCE is defined
#   include <ucontext.h>

#   undef _XOPEN_SOURCE
#elif !defined(__OpenBSD__) && !defined(__HAIKU__)
#   include <ucontext.h>
#endif

// TODO: __USE_DYNAMIC_STACK_SIZE is dependent on the features.h include and not a built-in compiler define, so it might be problematic to depend on it
#ifdef __USE_DYNAMIC_STACK_SIZE
static constexpr size_t MYSTACKSIZE = (16*1024)+32768; // wild guess about a reasonable buffer
#else
static constexpr size_t MYSTACKSIZE = (16*1024)+SIGSTKSZ; // wild guess about a reasonable buffer
#endif
static char mytstack[MYSTACKSIZE]= {0}; // alternative stack for signal handler
static bool bStackBelowHeap=false; // lame attempt to locate heap vs. stack address space. See CppCheckExecutor::check_wrapper()
static FILE* signalOutput = stdout; // TODO: get rid of this

/**
 * \param[in] ptr address to be examined.
 * \return true if address is supposed to be on stack (contrary to heap). If ptr is 0 false will be returned.
 * If unknown better return false.
 */
static bool IsAddressOnStack(const void* ptr)
{
    if (nullptr==ptr)
        return false;
    char a;
    if (bStackBelowHeap)
        return ptr < &a;
    return ptr > &a;
}

/* (declare this list here, so it may be used in signal handlers in addition to main())
 * A list of signals available in ISO C
 * Check out http://pubs.opengroup.org/onlinepubs/009695399/basedefs/signal.h.html
 * For now we only want to detect abnormal behaviour for a few selected signals:
 */

#define DECLARE_SIGNAL(x) std::make_pair(x, #x)
using Signalmap_t = std::map<int, std::string>;
static const Signalmap_t listofsignals = {
    DECLARE_SIGNAL(SIGABRT),
    DECLARE_SIGNAL(SIGBUS),
    DECLARE_SIGNAL(SIGFPE),
    DECLARE_SIGNAL(SIGILL),
    DECLARE_SIGNAL(SIGINT),
    DECLARE_SIGNAL(SIGQUIT),
    DECLARE_SIGNAL(SIGSEGV),
    DECLARE_SIGNAL(SIGSYS),
    // don't care: SIGTERM
    DECLARE_SIGNAL(SIGUSR1),
    //DECLARE_SIGNAL(SIGUSR2) no usage currently
};
#undef DECLARE_SIGNAL
/*
 * Entry pointer for signal handlers
 * It uses functions which are not safe to be called from a signal handler,
 * (http://pubs.opengroup.org/onlinepubs/9699919799/functions/V2_chap02.html#tag_15_04 has a whitelist)
 * but when ending up here something went terribly wrong anyway.
 * And all which is left is just printing some information and terminate.
 */
static void CppcheckSignalHandler(int signo, siginfo_t * info, void * context) // cppcheck-suppress constParameterCallback - info can be const
{
    int type = -1;
    pid_t killid;
    // TODO: separate these two defines
#if defined(__linux__) && defined(REG_ERR)
    const auto* const uc = reinterpret_cast<const ucontext_t*>(context);
    killid = static_cast<pid_t>(syscall(SYS_gettid));
    if (uc) {
        type = static_cast<int>(uc->uc_mcontext.gregs[REG_ERR]) & 2;
    }
#else
    (void)context;
    killid = getpid();
#endif

    const auto it = listofsignals.find(signo);
    const char * const signame = (it==listofsignals.end()) ? "unknown" : it->second.c_str();
    bool unexpectedSignal=true; // unexpected indicates program failure
    bool terminate=true; // exit process/thread
    const bool isAddressOnStack = IsAddressOnStack(info->si_addr);
    FILE * const output = signalOutput;
    switch (signo) {
    case SIGABRT:
        fputs("Internal error: cppcheck received signal ", output);
        fputs(signame, output);
        fputs(
#ifdef NDEBUG
            " - abort\n",
#else
            " - abort or assertion\n",
#endif
            output);
        break;
    case SIGBUS:
        fputs("Internal error: cppcheck received signal ", output);
        fputs(signame, output);
        switch (info->si_code) {
        case BUS_ADRALN:         // invalid address alignment
            fputs(" - BUS_ADRALN", output);
            break;
        case BUS_ADRERR:         // nonexistent physical address
            fputs(" - BUS_ADRERR", output);
            break;
        case BUS_OBJERR:         // object-specific hardware error
            fputs(" - BUS_OBJERR", output);
            break;
#ifdef BUS_MCEERR_AR
        case BUS_MCEERR_AR:             // Hardware memory error consumed on a machine check;
            fputs(" - BUS_MCEERR_AR", output);
            break;
#endif
#ifdef BUS_MCEERR_AO
        case BUS_MCEERR_AO:             // Hardware memory error detected in process but not consumed
            fputs(" - BUS_MCEERR_AO", output);
            break;
#endif
        default:
            break;
        }
        fprintf(output, " (at 0x%lx).\n",
                reinterpret_cast<unsigned long>(info->si_addr));
        break;
    case SIGFPE:
        fputs("Internal error: cppcheck received signal ", output);
        fputs(signame, output);
        switch (info->si_code) {
        case FPE_INTDIV:         //     integer divide by zero
            fputs(" - FPE_INTDIV", output);
            break;
        case FPE_INTOVF:         //     integer overflow
            fputs(" - FPE_INTOVF", output);
            break;
        case FPE_FLTDIV:         //     floating-point divide by zero
            fputs(" - FPE_FLTDIV", output);
            break;
        case FPE_FLTOVF:         //     floating-point overflow
            fputs(" - FPE_FLTOVF", output);
            break;
        case FPE_FLTUND:         //     floating-point underflow
            fputs(" - FPE_FLTUND", output);
            break;
        case FPE_FLTRES:         //     floating-point inexact result
            fputs(" - FPE_FLTRES", output);
            break;
        case FPE_FLTINV:         //     floating-point invalid operation
            fputs(" - FPE_FLTINV", output);
            break;
        case FPE_FLTSUB:         //     subscript out of range
            fputs(" - FPE_FLTSUB", output);
            break;
        default:
            break;
        }
        fprintf(output, " (at 0x%lx).\n",
                reinterpret_cast<unsigned long>(info->si_addr));
        break;
    case SIGILL:
        fputs("Internal error: cppcheck received signal ", output);
        fputs(signame, output);
        switch (info->si_code) {
        case ILL_ILLOPC:         //     illegal opcode
            fputs(" - ILL_ILLOPC", output);
            break;
        case ILL_ILLOPN:         //    illegal operand
            fputs(" - ILL_ILLOPN", output);
            break;
        case ILL_ILLADR:         //    illegal addressing mode
            fputs(" - ILL_ILLADR", output);
            break;
        case ILL_ILLTRP:         //    illegal trap
            fputs(" - ILL_ILLTRP", output);
            break;
        case ILL_PRVOPC:         //    privileged opcode
            fputs(" - ILL_PRVOPC", output);
            break;
        case ILL_PRVREG:         //    privileged register
            fputs(" - ILL_PRVREG", output);
            break;
        case ILL_COPROC:         //    coprocessor error
            fputs(" - ILL_COPROC", output);
            break;
        case ILL_BADSTK:         //    internal stack error
            fputs(" - ILL_BADSTK", output);
            break;
        default:
            break;
        }
        fprintf(output, " (at 0x%lx).%s\n",
                reinterpret_cast<unsigned long>(info->si_addr),
                isAddressOnStack ? " Stackoverflow?" : "");
        break;
    case SIGINT:
        unexpectedSignal=false;     // legal usage: interrupt application via CTRL-C
        fputs("cppcheck received signal ", output);
        fputs(signame, output);
        fputs(".\n", output);
        break;
    case SIGSEGV:
        fputs("Internal error: cppcheck received signal ", output);
        fputs(signame, output);
        switch (info->si_code) {
        case SEGV_MAPERR:         //    address not mapped to object
            fputs(" - SEGV_MAPERR", output);
            break;
        case SEGV_ACCERR:         //    invalid permissions for mapped object
            fputs(" - SEGV_ACCERR", output);
            break;
        default:
            break;
        }
        fprintf(output, " (%sat 0x%lx).%s\n",
                // cppcheck-suppress knownConditionTrueFalse ; FP
                (type==-1)? "" :
                (type==0) ? "reading " : "writing ",
                reinterpret_cast<unsigned long>(info->si_addr),
                isAddressOnStack ? " Stackoverflow?" : ""
                );
        break;
    case SIGUSR1:
        fputs("cppcheck received signal ", output);
        fputs(signame, output);
        fputs(".\n", output);
        terminate=false;
        break;
    default:
        fputs("Internal error: cppcheck received signal ", output);
        fputs(signame, output);
        fputs(".\n", output);
        break;
    }
#ifdef USE_UNIX_BACKTRACE_SUPPORT
    // flush otherwise the trace might be printed earlier
    fflush(output);
    print_stacktrace(output, 1, true, -1, true);
#endif
    if (unexpectedSignal) {
        fputs("\nPlease report this to the cppcheck developers!\n", output);
    }
    fflush(output);

    if (terminate) {
        // now let things proceed, shutdown and hopefully dump core for post-mortem analysis
        struct sigaction act;
        memset(&act, 0, sizeof(act));
        act.sa_handler=SIG_DFL;
        sigaction(signo, &act, nullptr);
        kill(killid, signo);
    }
}

void register_signal_handler(FILE * const output)
{
    signalOutput = output;

    // determine stack vs. heap
    char stackVariable;
    char *heapVariable=static_cast<char*>(malloc(1));
    bStackBelowHeap = &stackVariable < heapVariable;
    free(heapVariable);

    // set up alternative stack for signal handler
    stack_t segv_stack;
    segv_stack.ss_sp = mytstack;
    segv_stack.ss_flags = 0;
    segv_stack.ss_size = MYSTACKSIZE;
    if (sigaltstack(&segv_stack, nullptr) != 0) {
        // TODO: log errno
        fputs("could not set alternate signal stack context.\n", output);
        std::exit(EXIT_FAILURE);
    }

    // install signal handler
    struct sigaction act;
    memset(&act, 0, sizeof(act));
    act.sa_flags=SA_SIGINFO|SA_ONSTACK;
    act.sa_sigaction=CppcheckSignalHandler;
    for (auto sig=listofsignals.cbegin(); sig!=listofsignals.cend(); ++sig) {
        sigaction(sig->first, &act, nullptr);
    }
}

#endif
