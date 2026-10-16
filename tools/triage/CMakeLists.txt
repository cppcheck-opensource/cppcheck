if (BUILD_GUI AND BUILD_TRIAGE)
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

    add_custom_target(triage-build-ui-deps SOURCES ${hdrs} ${uis})
    add_executable(
        triage
        ${hdrs}
        ${srcs}
        ${uis_hdrs}
        ${PROJECT_SOURCE_DIR}/gui/codeeditorstyle.cpp
        ${PROJECT_SOURCE_DIR}/gui/codeeditor.cpp)
    set_target_properties(triage PROPERTIES AUTOMOC ON)
    set_target_properties(triage PROPERTIES AUTOUIC ON)
    set_target_properties(triage PROPERTIES WIN32_EXECUTABLE ON)
    target_include_directories(triage PRIVATE ${PROJECT_SOURCE_DIR}/lib/ ${PROJECT_SOURCE_DIR}/gui/)
    target_link_libraries(triage ${QT_CORE_LIB} ${QT_GUI_LIB} ${QT_WIDGETS_LIB})
    if (CMAKE_CXX_COMPILER_ID MATCHES "Clang")
        if(QT_VERSION VERSION_LESS "6.4.0")
            # caused by Qt generated moc code - see https://bugreports.qt.io/browse/QTBUG-100915
            target_compile_options_safe(triage -Wno-redundant-parens)
        endif()
    endif()
    if(QT_VERSION VERSION_GREATER_EQUAL "6.9.0")
        # QBrush fails to compile before 6.9.0 - see https://bugreports.qt.io/browse/QTBUG-134038
        target_compile_definitions(triage PRIVATE -DQT_NO_QPAIR)
    endif()
    if(QT_VERSION VERSION_GREATER_EQUAL "6.9.0")
        # caused by Qt generated moc code starting with 6.9.0 - see https://bugreports.qt.io/browse/QTBUG-135638
        target_compile_options_safe(triage -Wno-ctad-maybe-unsupported)
    endif()
    target_compile_definitions(triage PRIVATE -DQT_NO_FOREACH)

    target_compile_definitions(triage PRIVATE $<$<NOT:$<CONFIG:Debug>>:QT_NO_DEBUG>)
    target_compile_definitions(triage PRIVATE $<$<NOT:$<CONFIG:Debug>>:QT_NO_DEBUG_OUTPUT>)
    target_compile_definitions(triage PRIVATE $<$<NOT:$<CONFIG:Debug>>:QT_NO_WARNING_OUTPUT>)
    target_compile_definitions(triage PRIVATE $<$<CONFIG:Debug>:QT_DEBUG>)
endif()
