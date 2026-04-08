if(MSVC)
    # add_compile_definitions() requires CMake 3.12

    # Visual Studio only sets _DEBUG
    add_compile_definitions($<$<CONFIG:Debug>:DEBUG>)

    add_definitions(-D_CRT_SECURE_NO_WARNINGS)
    add_definitions(-DWIN32_LEAN_MEAN)
endif()

# libstdc++-specific flags
if(CMAKE_CXX_COMPILER_ID MATCHES "GNU" OR (NOT USE_LIBCXX AND CMAKE_CXX_COMPILER_ID MATCHES "Clang"))
    if(CPPCHK_GLIBCXX_DEBUG AND CMAKE_BUILD_TYPE STREQUAL "Debug")
        add_definitions(-D_GLIBCXX_DEBUG)
    endif()
endif()

# TODO: what about clang-cl?
# libc++-specific flags - AppleClang only has libc++
if ((USE_LIBCXX AND CMAKE_CXX_COMPILER_ID MATCHES "Clang") OR CMAKE_CXX_COMPILER_ID STREQUAL "AppleClang")
    if(CPPCHK_GLIBCXX_DEBUG AND CMAKE_BUILD_TYPE STREQUAL "Debug")
        # TODO: determine proper version for AppleClang - current value is based on the oldest version avaialble in CI
        if((CMAKE_CXX_COMPILER_ID STREQUALS "Clang" AND CMAKE_CXX_COMPILER_VERSION VERSION_GREATER_EQUAL 18) OR
           (CMAKE_CXX_COMPILER_ID STREQUAL "AppleClang" AND CMAKE_CXX_COMPILER_VERSION VERSION_GREATER_EQUAL 17))
            add_definitions(-D_LIBCPP_HARDENING_MODE=_LIBCPP_HARDENING_MODE_DEBUG)
        else()
            add_definitions(-D_LIBCPP_ENABLE_ASSERTIONS=1)
        endif()
        # TODO: also add _LIBCPP_ENABLE_THREAD_SAFETY_ANNOTATIONS?
    else()
        # TODO: check if this can be enabled again for Clang - also done in Makefile
        add_definitions(-D_GLIBCXX_DEBUG)
    endif()

    add_definitions(-D_LIBCPP_REMOVE_TRANSITIVE_INCLUDES)
endif()

if(HAVE_RULES)
    add_definitions(-DHAVE_RULES)
    if(USE_PCRE2)
        add_definitions(-DHAVE_PCRE2)
    endif()
    if(NOT DISABLE_PCRE1)
        add_definitions(-DHAVE_PCRE)
    endif()
endif()

if(Boost_FOUND)
    add_definitions(-DHAVE_BOOST)
    if(USE_BOOST_INT128)
        add_definitions(-DHAVE_BOOST_INT128)
    endif()
endif()

if(ENABLE_CHECK_INTERNAL)
    add_definitions(-DCHECK_INTERNAL)
endif()

if(DISALLOW_THREAD_EXECUTOR)
    add_definitions(-DDISALLOW_THREAD_EXECUTOR)
endif()

if(DISALLOW_PROCESS_EXECUTOR)
    add_definitions(-DDISALLOW_PROCESS_EXECUTOR)
endif()

if(NO_UNIX_SIGNAL_HANDLING)
    add_definitions(-DNO_UNIX_SIGNAL_HANDLING)
endif()

if(NO_UNIX_BACKTRACE_SUPPORT)
    add_definitions(-DNO_UNIX_BACKTRACE_SUPPORT)
endif()

if(NO_WINDOWS_SEH)
    add_definitions(-DNO_WINDOWS_SEH)
endif()

if(NOT MSVC)
    add_definitions(-DHAVE_EXECINFO_H=${HAVE_EXECINFO_H})
endif()

if(FILESDIR_DEF)
    file(TO_CMAKE_PATH "${FILESDIR_DEF}" _filesdir)
    add_definitions(-DFILESDIR="${_filesdir}")
endif()
