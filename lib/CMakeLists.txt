file(GLOB_RECURSE hdrs "*.h")
file(GLOB_RECURSE srcs "*.cpp")

# move the slowest files to the top
set(srcs_slow "${CMAKE_CURRENT_SOURCE_DIR}/valueflow.cpp" "${CMAKE_CURRENT_SOURCE_DIR}/tokenize.cpp" "${CMAKE_CURRENT_SOURCE_DIR}/symboldatabase.cpp")
list(REMOVE_ITEM srcs ${srcs_slow})
list(INSERT srcs 0 ${srcs_slow})

function(build_src output filename)
    get_filename_component(file ${filename} NAME)
    set(outfile ${CMAKE_CURRENT_BINARY_DIR}/build/mc_${file})
    set(${output} ${${output}} ${outfile} PARENT_SCOPE)
    if (USE_MATCHCOMPILER_OPT STREQUAL "Verify")
        set(verify_option "--verify")
    else()
        set(verify_option "")
    endif()
    add_custom_command(
        OUTPUT ${outfile}
        COMMAND ${Python_EXECUTABLE} "${PROJECT_SOURCE_DIR}/tools/matchcompiler.py"
                --read-dir="${CMAKE_CURRENT_SOURCE_DIR}" 
                --prefix="mc_" 
                --line 
                ${verify_option} 
                ${file}
        DEPENDS ${file} 
        DEPENDS ${PROJECT_SOURCE_DIR}/tools/matchcompiler.py
    )
    set_source_files_properties(${outfile} PROPERTIES GENERATED TRUE)
endfunction()

if (NOT USE_MATCHCOMPILER_OPT STREQUAL "Off")
    set(srcs_build "")
    foreach(file ${srcs})
        build_src(srcs_build ${file})
    endforeach()

    set(srcs_lib ${srcs_build})
else()
    set(srcs_lib ${srcs})
endif()

if(BUILD_SHARED_LIBS)
    add_library(cppcheck-core ${srcs_lib} ${hdrs})
else()
    # A OBJECT library is used because the auto-registration doesn't work with static libraries
    add_library(cppcheck-core OBJECT ${srcs_lib} ${hdrs})
endif()
if (BUILD_SHARED_LIBS AND MSVC)
    target_sources(cppcheck-core PRIVATE version.rc)
endif()

target_dll_compile_definitions(cppcheck-core EXPORT CPPCHECKLIB_EXPORT IMPORT CPPCHECKLIB_IMPORT)

target_include_directories(cppcheck-core PUBLIC .)
target_link_libraries(cppcheck-core PRIVATE tinyxml2 simplecpp picojson)
if (HAVE_RULES)
    if(USE_PCRE2)
        target_link_libraries(cppcheck-core PRIVATE ${PCRE2_LIBRARY})
    endif()
    if(NOT DISABLE_PCRE1)
        target_link_libraries(cppcheck-core PRIVATE ${PCRE_LIBRARY})
    endif()
endif()

if (HAVE_RULES)
    if(USE_PCRE2)
        target_include_directories(cppcheck-core SYSTEM PRIVATE ${PCRE2_INCLUDE})
    endif()
    if(NOT DISABLE_PCRE1)
        target_include_directories(cppcheck-core SYSTEM PRIVATE ${PCRE_INCLUDE})
    endif()
endif()
if (Boost_FOUND)
    target_include_directories(cppcheck-core SYSTEM PUBLIC ${Boost_INCLUDE_DIRS})
endif()

if (NOT CMAKE_DISABLE_PRECOMPILE_HEADERS)
    target_precompile_headers(cppcheck-core PRIVATE precompiled.h)
endif()
