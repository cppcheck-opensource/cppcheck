
add_executable(dmake EXCLUDE_FROM_ALL
        dmake.cpp
)
target_link_libraries(dmake cppcheck-core cli simplecpp)
if (WIN32 AND NOT BORLAND)
    if(NOT MINGW)
        target_link_libraries(dmake Shlwapi.lib)
    else()
        target_link_libraries(dmake shlwapi)
    endif()
endif()

add_custom_target(run-dmake $<TARGET_FILE:dmake>
        WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
        DEPENDS dmake)
