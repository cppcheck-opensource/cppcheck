include(CheckCXXCompilerFlag)

# compile options, compile definitions and link options for all targets
# directory-level commands like add_compile_options() are not recommended so these are collected in lists and
# applied to the targets with cppcheck_apply_global_options()
set(CPPCHECK_COMPILE_OPTIONS)
set(CPPCHECK_COMPILE_DEFINITIONS)
set(CPPCHECK_LINK_OPTIONS)

function(add_compile_options_safe FLAG)
    string(MAKE_C_IDENTIFIER "HAS_CXX_FLAG${FLAG}" mangled_flag)
    check_cxx_compiler_flag(${FLAG} ${mangled_flag})
    if(${mangled_flag})
        set(CPPCHECK_COMPILE_OPTIONS ${CPPCHECK_COMPILE_OPTIONS} ${FLAG} PARENT_SCOPE)
    endif()
endfunction()

# applies CPPCHECK_COMPILE_OPTIONS, CPPCHECK_COMPILE_DEFINITIONS and CPPCHECK_LINK_OPTIONS to all targets which are
# built in DIR and its subdirectories - they are prepended so they come before the target-specific ones like they
# did with the directory-level commands
function(cppcheck_apply_global_options DIR)
    get_property(targets DIRECTORY ${DIR} PROPERTY BUILDSYSTEM_TARGETS)
    foreach(target ${targets})
        get_target_property(type ${target} TYPE)
        if(type STREQUAL "INTERFACE_LIBRARY" OR type STREQUAL "UTILITY")
            continue()
        endif()
        foreach(prop COMPILE_OPTIONS COMPILE_DEFINITIONS LINK_OPTIONS)
            get_target_property(values ${target} ${prop})
            if(NOT values)
                set(values)
            endif()
            set_property(TARGET ${target} PROPERTY ${prop} ${CPPCHECK_${prop}} ${values})
        endforeach()
    endforeach()

    get_property(subdirs DIRECTORY ${DIR} PROPERTY SUBDIRECTORIES)
    foreach(subdir ${subdirs})
        cppcheck_apply_global_options(${subdir})
    endforeach()
endfunction()

function(target_compile_options_safe TARGET FLAG)
    string(MAKE_C_IDENTIFIER "HAS_CXX_FLAG${FLAG}" mangled_flag)
    check_cxx_compiler_flag(${FLAG} ${mangled_flag})
    if(${mangled_flag})
        target_compile_options(${TARGET} PRIVATE ${FLAG})
    endif()
endfunction()

function(target_externals_include_directories TARGET)
    if(EXTERNALS_AS_SYSTEM)
        target_include_directories(${TARGET} SYSTEM ${ARGN})
    else()
        target_include_directories(${TARGET} ${ARGN})
    endif()
endfunction()

function(target_dll_compile_definitions TARGET)
    set(options)
    set(oneValueArgs IMPORT EXPORT)
    set(multiValueArgs)

    cmake_parse_arguments(PARSE "${options}" "${oneValueArgs}" "${multiValueArgs}" ${ARGN})
    if(PARSE_UNPARSED_ARGUMENTS)
        message(
            FATAL_ERROR "Unknown keywords given to target_dll_compile_definitions(): \"${PARSE_UNPARSED_ARGUMENTS}\"")
    endif()


    if (BUILD_SHARED_LIBS AND MSVC)
        if(PARSE_EXPORT)
            target_compile_definitions(${TARGET} PRIVATE ${PARSE_EXPORT})
        endif()
        if(PARSE_IMPORT)
            target_compile_definitions(${TARGET} INTERFACE ${PARSE_IMPORT})
        endif()
    endif()
endfunction()

if(CMAKE_CXX_COMPILER_ID MATCHES "Clang")
    list(APPEND CPPCHECK_COMPILE_OPTIONS -Weverything)
endif()

if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU" OR CMAKE_CXX_COMPILER_ID MATCHES "Clang")
    if(CMAKE_BUILD_TYPE STREQUAL "Release")
        # "Release" uses -O3 by default
        list(APPEND CPPCHECK_COMPILE_OPTIONS -O2)
    endif()
    if(CMAKE_COMPILE_WARNING_AS_ERROR)
        list(APPEND CPPCHECK_COMPILE_OPTIONS -Werror)
    endif()
    list(APPEND CPPCHECK_COMPILE_OPTIONS -pedantic) # TODO: is this implied by -Weverything?
endif()

if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
    list(APPEND CPPCHECK_COMPILE_OPTIONS -Wall)
    list(APPEND CPPCHECK_COMPILE_OPTIONS -Wextra)
    list(APPEND CPPCHECK_COMPILE_OPTIONS -Wcast-qual)                # Cast for removing type qualifiers
    list(APPEND CPPCHECK_COMPILE_OPTIONS -Wfloat-equal)              # Floating values used in equality comparisons
    list(APPEND CPPCHECK_COMPILE_OPTIONS -Wmissing-declarations)     # If a global function is defined without a previous declaration
    list(APPEND CPPCHECK_COMPILE_OPTIONS -Wmissing-format-attribute) #
    list(APPEND CPPCHECK_COMPILE_OPTIONS -Wno-long-long)
    list(APPEND CPPCHECK_COMPILE_OPTIONS -Wpacked)                   #
    list(APPEND CPPCHECK_COMPILE_OPTIONS -Wredundant-decls)          # if anything is declared more than once in the same scope
    list(APPEND CPPCHECK_COMPILE_OPTIONS -Wundef)
    list(APPEND CPPCHECK_COMPILE_OPTIONS -Wno-sign-compare)
    list(APPEND CPPCHECK_COMPILE_OPTIONS -Wno-multichar)
    list(APPEND CPPCHECK_COMPILE_OPTIONS -Woverloaded-virtual)       # when a function declaration hides virtual functions from a base class

    # TODO: evaluate
    #list(APPEND CPPCHECK_COMPILE_OPTIONS -Wconversion)  # danmar: gives fp. for instance: unsigned int sizeof_pointer = sizeof(void *);
    #list(APPEND CPPCHECK_COMPILE_OPTIONS -Wlogical-op) # doesn't work on older GCC
    #list(APPEND CPPCHECK_COMPILE_OPTIONS -Wsign-conversion) # too many warnings
    #list(APPEND CPPCHECK_COMPILE_OPTIONS -Wunreachable-code) # some GCC versions report lots of warnings
    #list(APPEND CPPCHECK_COMPILE_OPTIONS -Wsign-promo)

    # use pipes instead of temporary files - greatly reduces I/O usage
    list(APPEND CPPCHECK_COMPILE_OPTIONS -pipe)

    list(APPEND CPPCHECK_COMPILE_OPTIONS -Wsuggest-attribute=noreturn)
    add_compile_options_safe(-Wuseless-cast)
    # add_compile_options_safe(-Wsuggest-attribute=returns_nonnull) # reports the warning even if the attribute is set

    # TODO: evaluate
    #add_compile_options_safe(-Wduplicated-branches)
    #add_compile_options_safe(-Wduplicated-cond)
    #add_compile_options_safe(-Wformat=2)
    #add_compile_options_safe(-Wformat-overflow=2)
    #add_compile_options_safe(-Wformat-signedness)
    #add_compile_options_safe(-Wnull-dereference)
    #add_compile_options_safe(-Wnrvo)
    #add_compile_options_safe(-Wimplicit-fallthrough=5)
    #add_compile_options_safe(-Wmissing-include-dirs)
    #add_compile_options_safe(-Wunused)
    #add_compile_options_safe(-Wunused-const-variable)
    #add_compile_options_safe(-Wuninitialized)
    #add_compile_options_safe(-Wsuggest-attribute=pure)
    #add_compile_options_safe(-Wsuggest-attribute=const)
    #add_compile_options_safe(-Wunused-macros)
    #add_compile_options_safe(-Wpedantic)
elseif(CMAKE_CXX_COMPILER_ID MATCHES "Clang")
    if(CMAKE_CXX_COMPILER_VERSION VERSION_GREATER_EQUAL 14)
        # TODO: verify this regression still exists in clang-15
        if(CMAKE_BUILD_TYPE STREQUAL "Release" OR CMAKE_BUILD_TYPE STREQUAL "RelWithDebInfo")
            # work around performance regression - see https://github.com/llvm/llvm-project/issues/53555
            check_cxx_compiler_flag("-mllvm -inline-deferral" _has_mllvm_inline_deferral)
            if(_has_mllvm_inline_deferral)
                list(APPEND CPPCHECK_COMPILE_OPTIONS -mllvm -inline-deferral)
            endif()
        endif()

        # use force DWARF 4 debug format since not all tools might be able to handle DWARF 5 yet - e.g. valgrind on ubuntu 20.04
        list(APPEND CPPCHECK_COMPILE_OPTIONS -gdwarf-4)
    endif()

    if (CMAKE_CXX_COMPILER_ID STREQUAL "AppleClang")
        list(APPEND CPPCHECK_COMPILE_OPTIONS -Wno-poison-system-directories)
    endif()

    if(USE_LIBCXX)
        list(APPEND CPPCHECK_COMPILE_OPTIONS -stdlib=libc++)
        set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -lc++")
    endif()

    # TODO: fix and enable these warnings - or move to suppression list below
    add_compile_options_safe(-Wno-sign-conversion)
    add_compile_options_safe(-Wno-shadow-field-in-constructor)
    add_compile_options_safe(-Wno-implicit-int-conversion)
    add_compile_options_safe(-Wno-double-promotion)
    add_compile_options_safe(-Wno-shadow-field)
    add_compile_options_safe(-Wno-shadow-uncaptured-local)
    add_compile_options_safe(-Wno-implicit-float-conversion)
    add_compile_options_safe(-Wno-switch-enum)
    add_compile_options_safe(-Wno-date-time)
    list(APPEND CPPCHECK_COMPILE_OPTIONS -Wno-disabled-macro-expansion)
    list(APPEND CPPCHECK_COMPILE_OPTIONS -Wno-sign-compare)
    add_compile_options_safe(-Wno-ms-bitfield-padding) # TODO: fix this

    # these cannot be fixed properly without adopting later C++ standards
    add_compile_options_safe(-Wno-unsafe-buffer-usage)
    add_compile_options_safe(-Wno-global-constructors)
    add_compile_options_safe(-Wno-exit-time-destructors)

    # can only be partially addressed
    list(APPEND CPPCHECK_COMPILE_OPTIONS -Wno-padded)
    add_compile_options_safe(-Wno-nrvo)

    # no need for C++98 compatibility
    list(APPEND CPPCHECK_COMPILE_OPTIONS -Wno-c++98-compat)
    list(APPEND CPPCHECK_COMPILE_OPTIONS -Wno-c++98-compat-pedantic)

    # only needs to be addressed to work around issues in older compilers
    add_compile_options_safe(-Wno-return-std-move-in-c++11)

    # this is reported even when it is unnecessary i.e. -Wswitch-enum warnings have been mitigated
    add_compile_options_safe(-Wno-switch-default)

    # warnings we are currently not interested in
    list(APPEND CPPCHECK_COMPILE_OPTIONS -Wno-four-char-constants)
    list(APPEND CPPCHECK_COMPILE_OPTIONS -Wno-weak-vtables)
    list(APPEND CPPCHECK_COMPILE_OPTIONS -Wno-multichar)

    # FIXME: only reported when running clang-tidy
    # /home/runner/work/cppcheck/cppcheck/tools/triage/mainwindow.cpp:19:10: error: multiple candidates for header 'mainwindow.h' found; directory '/home/runner/work/cppcheck/cppcheck/tools/triage' chosen, ignoring others including '/home/runner/work/cppcheck/cppcheck/gui' [clang-diagnostic-shadow-header]
    # 19 | #include "mainwindow.h"
    # |          ^
    add_compile_options_safe(-Wno-shadow-header)

    if(ENABLE_COVERAGE OR ENABLE_COVERAGE_XML)
      message(FATAL_ERROR "Do not use clang to generate code coverage. Use GCC instead.")
    endif()
endif()

if(MSVC)
    # General
    list(APPEND CPPCHECK_COMPILE_OPTIONS /W4) # Warning Level
    list(APPEND CPPCHECK_COMPILE_OPTIONS /Zi) # Debug Information Format - Program Database
    if(CMAKE_COMPILE_WARNING_AS_ERROR)
        list(APPEND CPPCHECK_COMPILE_OPTIONS /WX) # Treat Warning As Errors
    endif()
    list(APPEND CPPCHECK_COMPILE_OPTIONS /MP) # Multi-processor Compilation

    # Advanced
    # Character Set - Use Unicode Character Set
    # No Whole Program Optimization

    # C/C++ - Optimization
    list(APPEND CPPCHECK_COMPILE_OPTIONS $<$<NOT:$<CONFIG:Debug>>:/O2>) # Optimization - Maximum Optimization (Favor Speed)
    list(APPEND CPPCHECK_COMPILE_OPTIONS $<$<NOT:$<CONFIG:Debug>>:/Ob2>) # Inline Function Expansion - Any Suitable
    list(APPEND CPPCHECK_COMPILE_OPTIONS $<$<NOT:$<CONFIG:Debug>>:/Oi>) # Enable Intrinsic Functions
    list(APPEND CPPCHECK_COMPILE_OPTIONS $<$<NOT:$<CONFIG:Debug>>:/Ot>) # Favor fast code
    list(APPEND CPPCHECK_COMPILE_OPTIONS $<$<NOT:$<CONFIG:Debug>>:/Oy>) # Omit Frame Pointers
    list(APPEND CPPCHECK_COMPILE_OPTIONS $<$<CONFIG:Debug>:/Od>) # Optimization - Disabled

    # C/C++ - Code Generation
    list(APPEND CPPCHECK_COMPILE_OPTIONS $<$<NOT:$<CONFIG:Debug>>:/GF>) # Enable String Pooling
    list(APPEND CPPCHECK_COMPILE_OPTIONS $<$<NOT:$<CONFIG:Debug>>:/MD>) # Runtime Library - Multi-threaded DLL
    list(APPEND CPPCHECK_COMPILE_OPTIONS $<$<NOT:$<CONFIG:Debug>>:/GS->) # Disable Security Check
    list(APPEND CPPCHECK_COMPILE_OPTIONS $<$<NOT:$<CONFIG:Debug>>:/Gy>) # Enable Function-Level Linking
    list(APPEND CPPCHECK_COMPILE_OPTIONS $<$<CONFIG:Debug>:/MDd>) # Runtime Library - Multi-threaded Debug DLL
    list(APPEND CPPCHECK_COMPILE_OPTIONS $<$<CONFIG:Debug>:/GS>) # Enable Security Check

    # C/C++ - Language
    list(APPEND CPPCHECK_COMPILE_OPTIONS /Zc:rvalueCast) # Enforce type conversion rules
    #list(APPEND CPPCHECK_COMPILE_OPTIONS /std:c++14) # C++ Language Standard - ISO C++14 Standard

    # C/C++ - Browse Information
    # Enable Browse Information - No

    # C/C++ - Advanced
    list(APPEND CPPCHECK_COMPILE_OPTIONS /wd4018) # warning C4018: '>': signed/unsigned mismatch
    list(APPEND CPPCHECK_COMPILE_OPTIONS /wd4127) # warning C4127: conditional expression is constant
    list(APPEND CPPCHECK_COMPILE_OPTIONS /wd4146) # warning C4146: unary minus operator applied to unsigned type, result still unsigned
    list(APPEND CPPCHECK_COMPILE_OPTIONS /wd4244) # warning C4244: 'initializing': conversion from 'int' to 'char', possible loss of data
    list(APPEND CPPCHECK_COMPILE_OPTIONS /wd4251) # warning C4251: 'x': class 'y' needs to have dll-interface to be used by clients of struct 'u'
    # Clang: -Wshorten-64-to-32 -Wimplicit-int-conversion
    list(APPEND CPPCHECK_COMPILE_OPTIONS /wd4267) # warning C4267: 'return': conversion from 'size_t' to 'int', possible loss of data
    list(APPEND CPPCHECK_COMPILE_OPTIONS /wd4389) # warning C4389: '==': signed/unsigned mismatch
    list(APPEND CPPCHECK_COMPILE_OPTIONS /wd4701) # warning C4701: potentially uninitialized local variable 'err' used
    list(APPEND CPPCHECK_COMPILE_OPTIONS /wd4706) # warning C4706: assignment within conditional expression
    list(APPEND CPPCHECK_COMPILE_OPTIONS /wd4800) # warning C4800: 'const SymbolDatabase *' : forcing value to bool 'true' or 'false' (performance warning)
    list(APPEND CPPCHECK_COMPILE_OPTIONS /wd4805) # warning C4805: '==' : unsafe mix of type 'bool' and type 'long long' in operation

    # C/C++ - All Options
    list(APPEND CPPCHECK_COMPILE_OPTIONS /Zc:throwingNew /Zc:__cplusplus) # Additional Options

    # Linker - General
    list(APPEND CPPCHECK_LINK_OPTIONS $<$<CONFIG:Debug>:/INCREMENTAL>) # Enable Incremental Linking - Yes

    list(APPEND CPPCHECK_LINK_OPTIONS /NOLOGO) # Suppress Startup Banner - Yes
    # Ignore Import Library - Yes

    # Linker - Debugging
    list(APPEND CPPCHECK_LINK_OPTIONS /DEBUG) # Generate Debug Information

    # Linker - System
    # Stack Reserve Size - 8000000
    # Stack Commit Size - 8000000
    list(APPEND CPPCHECK_LINK_OPTIONS /LARGEADDRESSAWARE) # Enable Large Addresses - Yes

    # Linker - Optimization
    list(APPEND CPPCHECK_LINK_OPTIONS /OPT:REF) # References - Yes
    list(APPEND CPPCHECK_LINK_OPTIONS /OPT:ICF) # Enable COMDAT Folding - Yes

    # Linker - Advanced
    list(APPEND CPPCHECK_LINK_OPTIONS $<$<NOT:$<CONFIG:Debug>>:/RELEASE>) # Set Checksum - Yes
endif()

# TODO: check if this can be enabled again - also done in Makefile
if(CMAKE_SYSTEM_NAME STREQUAL "Linux" AND
    CMAKE_CXX_COMPILER_ID MATCHES "Clang")

    list(APPEND CPPCHECK_COMPILE_OPTIONS -U_GLIBCXX_DEBUG)
endif()

if(MSVC)
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} /STACK:8000000")
endif()

if(CYGWIN)
    # TODO: this is a linker flag - not a compiler flag
    list(APPEND CPPCHECK_COMPILE_OPTIONS -Wl,--stack,8388608)
endif()

include(cmake/dynamic_analyzer_options.cmake)
