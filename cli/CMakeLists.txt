file(GLOB hdrs "*.h")
file(GLOB srcs "*.cpp")
file(GLOB mainfile "main.cpp")
list(REMOVE_ITEM srcs ${mainfile})

add_library(cli ${hdrs} ${srcs})
target_include_directories(cli PUBLIC .)
target_link_libraries(cli PRIVATE cppcheck-core frontend tinyxml2 simplecpp picojson)
if (NOT CMAKE_DISABLE_PRECOMPILE_HEADERS)
    target_precompile_headers(cli PRIVATE precompiled.h)
endif()

if (CMAKE_CXX_COMPILER_ID MATCHES "Clang" AND CMAKE_CXX_COMPILER_VERSION VERSION_EQUAL 13)
    # false positive warning in Clang 13 - caused by FD_ZERO macro
    set_source_files_properties(processexecutor.cpp PROPERTIES COMPILE_FLAGS -Wno-reserved-identifier)
endif()

if (BUILD_CLI)
    list(APPEND cppcheck_SOURCES ${hdrs} ${mainfile})
    if (WIN32)
        list(APPEND cppcheck_SOURCES version.rc)
    endif()

    add_executable(cppcheck ${cppcheck_SOURCES})
    target_link_libraries(cppcheck cppcheck-core cli tinyxml2 simplecpp)
    if (WIN32 AND NOT BORLAND)
        if(NOT MINGW)
            target_link_libraries(cppcheck Shlwapi.lib)
        else()
            target_link_libraries(cppcheck shlwapi)
        endif()
    endif()
    target_link_libraries(cppcheck ${CMAKE_THREAD_LIBS_INIT})

    add_dependencies(cppcheck copy_cfg)
    add_dependencies(cppcheck copy_addons)
    add_dependencies(cppcheck copy_platforms)
    if (NOT DISABLE_DMAKE)
        add_dependencies(cppcheck run-dmake)
    endif()

    install(TARGETS cppcheck
        RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
        COMPONENT applications)

    install(PROGRAMS ${CMAKE_SOURCE_DIR}/htmlreport/cppcheck-htmlreport
        DESTINATION ${CMAKE_INSTALL_BINDIR}
        COMPONENT applications)

    # TODO: leverage CMAKE_INSTALL_DATAROOTDIR (share)?

    install(FILES ${addons_py}
       DESTINATION ${FILESDIR_INSTALL}/addons
       COMPONENT headers)

    install(FILES ${addons_json}
       DESTINATION ${FILESDIR_INSTALL}/addons
       COMPONENT headers)

    install(FILES ${cfgs}
       DESTINATION ${FILESDIR_INSTALL}/cfg
       COMPONENT headers)

    install(FILES ${platforms}
       DESTINATION ${FILESDIR_INSTALL}/platforms
       COMPONENT headers)

    # TODO: install manpage into CMAKE_INSTALL_MANDIR
    # TODO: install documentation into CMAKE_INSTALL_DOCDIR?

endif()
