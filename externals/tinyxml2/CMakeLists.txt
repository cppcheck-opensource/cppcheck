file(GLOB hdrs "*.h")
file(GLOB srcs "*.cpp")

add_library(tinyxml2 ${srcs} ${hdrs})
target_dll_compile_definitions(tinyxml2 EXPORT TINYXML2_EXPORT IMPORT TINYXML2_IMPORT)

# TODO: needs to be fixed upstream
if (CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
    target_compile_options(tinyxml2 PRIVATE -Wno-suggest-attribute=format)
    target_compile_options(tinyxml2 PRIVATE -Wno-useless-cast)
endif()
if (CMAKE_CXX_COMPILER_ID MATCHES "Clang")
    target_compile_options_safe(tinyxml2 -Wno-implicit-fallthrough)
    target_compile_options_safe(tinyxml2 -Wno-suggest-destructor-override)
    target_compile_options_safe(tinyxml2 -Wno-zero-as-null-pointer-constant)
    target_compile_options_safe(tinyxml2 -Wno-format-nonliteral)
    target_compile_options_safe(tinyxml2 -Wno-inconsistent-missing-destructor-override)
    target_compile_options_safe(tinyxml2 -Wno-sign-conversion)
    target_compile_options_safe(tinyxml2 -Wno-double-promotion)
endif()
if(CYGWIN)
    target_compile_definitions(-D_LARGEFILE_SOURCE)  # required for fseeko() and ftello()
endif()

target_externals_include_directories(tinyxml2 PUBLIC .)

