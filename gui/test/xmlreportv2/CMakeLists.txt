qt_wrap_cpp(test-xmlreportv2_SRC testxmlreportv2.h)
add_custom_target(build-xmlreportv2-deps SOURCES ${test-xmlreportv2_SRC})
add_dependencies(gui-build-deps build-xmlreportv2-deps)
add_executable(test-xmlreportv2
        ${test-xmlreportv2_SRC}
        testxmlreportv2.cpp
        ${CMAKE_SOURCE_DIR}/gui/erroritem.cpp
        ${CMAKE_SOURCE_DIR}/gui/report.cpp
        ${CMAKE_SOURCE_DIR}/gui/xmlreport.cpp
        ${CMAKE_SOURCE_DIR}/gui/xmlreportv2.cpp
        )
target_include_directories(test-xmlreportv2 PRIVATE ${CMAKE_SOURCE_DIR}/gui)
target_compile_definitions(test-xmlreportv2 PRIVATE SRCDIR="${CMAKE_CURRENT_SOURCE_DIR}")
target_link_libraries(test-xmlreportv2 ${QT_CORE_LIB} ${QT_TEST_LIB} cppcheck-core)
if (CMAKE_CXX_COMPILER_ID MATCHES "Clang")
    if(QT_VERSION VERSION_GREATER_EQUAL "6.9.0")
        # caused by Qt generated moc code starting with 6.9.0 - see https://bugreports.qt.io/browse/QTBUG-135638
        target_compile_options_safe(test-xmlreportv2 -Wno-ctad-maybe-unsupported)
    endif()
endif()

if (REGISTER_GUI_TESTS)
    add_test(NAME test-xmlreportv2 COMMAND $<TARGET_FILE:test-xmlreportv2>)
endif()

add_dependencies(gui-tests test-xmlreportv2)
