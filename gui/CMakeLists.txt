if (BUILD_GUI)

    # disable all clang-tidy checks for Qt generated files
    file(WRITE "${CMAKE_CURRENT_BINARY_DIR}/.clang-tidy"
"---
Checks:          '-*,misc-definitions-in-headers'
WarningsAsErrors: '*'
CheckOptions:
  - { key: HeaderFileExtensions,          value: 'x' }
")

    file(GLOB hdrs "*.h")
    file(GLOB srcs "*.cpp")
    file(GLOB uis  "*.ui")
    file(GLOB tss  "*.ts")
    QT_WRAP_UI(uis_hdrs ${uis})
    QT_ADD_RESOURCES(resources "gui.qrc")
    # TODO: passing "-no-obsolete" here breaks the translations
    QT_CREATE_TRANSLATION(qms ${CMAKE_CURRENT_SOURCE_DIR} ${tss})
    list(APPEND cppcheck-gui-deps ${hdrs} ${uis_hdrs} ${resources} ${qms})
    add_custom_target(gui-build-deps SOURCES ${cppcheck-gui-deps})

    list(APPEND cppcheck-gui_SOURCES ${srcs})
    if (WIN32)
        list(APPEND cppcheck-gui_SOURCES cppcheck-gui.rc)
    endif()

    add_executable(cppcheck-gui ${cppcheck-gui-deps} ${cppcheck-gui_SOURCES})
    target_link_libraries(cppcheck-gui cppcheck-core simplecpp tinyxml2 picojson frontend)

    set_target_properties(cppcheck-gui PROPERTIES AUTOMOC ON)
    set_target_properties(cppcheck-gui PROPERTIES WIN32_EXECUTABLE ON)
    if (NOT CMAKE_DISABLE_PRECOMPILE_HEADERS)
        target_precompile_headers(cppcheck-gui PRIVATE precompiled.h)
    endif()
    if (HAVE_RULES)
        target_link_libraries(cppcheck-gui ${PCRE_LIBRARY})
    endif()
    target_link_libraries(cppcheck-gui ${QT_CORE_LIB} ${QT_GUI_LIB} ${QT_WIDGETS_LIB} ${QT_PRINTSUPPORT_LIB} ${QT_HELP_LIB} ${QT_NETWORK_LIB})
    if(WITH_QCHART)
        target_link_libraries(cppcheck-gui ${QT_CHARTS_LIB})
    endif()
    if (CMAKE_CXX_COMPILER_ID MATCHES "Clang")
        if(QT_VERSION VERSION_LESS "6.4.0")
            # caused by Qt generated moc code - see https://bugreports.qt.io/browse/QTBUG-100915
            target_compile_options_safe(cppcheck-gui -Wno-redundant-parens)
        endif()
        if(QT_VERSION VERSION_GREATER_EQUAL "6.9.0")
            # caused by Qt generated moc code starting with 6.9.0 - see https://bugreports.qt.io/browse/QTBUG-135638
            target_compile_options_safe(cppcheck-gui -Wno-ctad-maybe-unsupported)
        endif()
    elseif (CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
        # caused by mocs
        target_compile_options_safe(cppcheck-gui -Wno-useless-cast)
    endif()
    if(QT_VERSION VERSION_GREATER_EQUAL "6.9.2")
        # QBrush fails to compile before 6.9.0 - see https://bugreports.qt.io/browse/QTBUG-134038
        # QtCharts/qxyseries.h fails to compile in 6.9.0 and 6.9.1 - see https://bugreports.qt.io/browse/QTBUG-135637
        #target_compile_definitions(cppcheck-gui PRIVATE -DQT_NO_QPAIR)
    endif()
    target_compile_definitions(cppcheck-gui PRIVATE -DQT_NO_FOREACH)

    target_compile_definitions(cppcheck-gui PRIVATE $<$<NOT:$<CONFIG:Debug>>:QT_NO_DEBUG>)
    target_compile_definitions(cppcheck-gui PRIVATE $<$<NOT:$<CONFIG:Debug>>:QT_NO_DEBUG_OUTPUT>)
    target_compile_definitions(cppcheck-gui PRIVATE $<$<NOT:$<CONFIG:Debug>>:QT_NO_WARNING_OUTPUT>)
    target_compile_definitions(cppcheck-gui PRIVATE $<$<CONFIG:Debug>:QT_DEBUG>)

    if (QHELPGENERATOR)
        # TODO: generate in CMAKE_BINARY_DIR folder
        add_custom_target(online-help.qhc ${QHELPGENERATOR} ${CMAKE_CURRENT_SOURCE_DIR}/help/online-help.qhcp -o ${CMAKE_CURRENT_SOURCE_DIR}/help/online-help.qhc)
        add_dependencies(cppcheck-gui online-help.qhc)
    endif()

    install(TARGETS cppcheck-gui RUNTIME DESTINATION ${CMAKE_INSTALL_FULL_BINDIR} COMPONENT applications)
    install(FILES ${qms} DESTINATION ${CMAKE_INSTALL_FULL_BINDIR} COMPONENT applications)

    install(FILES cppcheck-gui.desktop DESTINATION ${CMAKE_INSTALL_DATAROOTDIR}/applications)

    # icons
    install(FILES cppcheck-gui.svg DESTINATION ${CMAKE_INSTALL_DATADIR}/icons/hicolor/scalable/apps)
    install(FILES cppcheck-gui.png DESTINATION ${CMAKE_INSTALL_DATADIR}/icons/hicolor/64x64/apps)

    add_dependencies(cppcheck-gui copy_cfg)
    add_dependencies(cppcheck-gui copy_addons)
    add_dependencies(cppcheck-gui copy_platforms)
    if (NOT DISABLE_DMAKE)
        add_dependencies(cppcheck-gui run-dmake)
    endif()

    if (BUILD_TESTING)
        add_subdirectory(test)
    endif()
endif()
