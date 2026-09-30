# warn about platforms which are not actively maintained (i.e. no CI integration)

# TODO: we actually need to consider the target (i.e. handle cross-compilation cases)

message(STATUS "${CMAKE_SYSTEM_PROCESSOR}")

# TODO: before more granular about UNIX - CMAKE_HOST_LINUX was only added in CMake 3.25
if(NOT CMAKE_HOST_APPLE AND
        NOT CMAKE_HOST_UNIX AND
        NOT CMAKE_HOST_WIN32)
    message(WARNING "Builds for your host system (${CMAKE_HOST_SYSTEM}) are not actively maintained.")
    set(_unsupported TRUE)
endif()

if(NOT _unsupported)
    # AMD64 for Windows
    # x86_64 for Linux
    # we do support macOS ARM via macos-14 in the CI
    if(NOT CMAKE_SYSTEM_PROCESSOR STREQUAL "x86_64" AND
            NOT CMAKE_SYSTEM_PROCESSOR STREQUAL "AMD64" AND
            NOT CMAKE_HOST_APPLE)
        message(WARNING "Builds for your processor (${CMAKE_SYSTEM_PROCESSOR}) are not actively maintained.")
    endif()
    set(_unsupported TRUE)
endif()

if(NOT _unsupported)
    # we do support test 32-bit builds
    if(CMAKE_GENERATOR_PLATFORM AND NOT CMAKE_GENERATOR_PLATFORM STREQUAL "x64")
        message(WARNING "Builds for your platform (${CMAKE_GENERATOR_PLATFORM}) are not actively maintained.")
    endif()
    set(_unsupported TRUE)
endif()