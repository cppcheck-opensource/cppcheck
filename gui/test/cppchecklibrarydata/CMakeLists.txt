qt_wrap_cpp(test-cppchecklibrarydata_SRC testcppchecklibrarydata.h)
QT_ADD_RESOURCES(test-cppchecklibrarydata_resources "resources.qrc")
add_custom_target(build-cppchecklibrarydata-deps SOURCES ${test-cppchecklibrarydata_SRC} ${test-cppchecklibrarydata_resources})
add_dependencies(gui-build-deps build-cppchecklibrarydata-deps)
add_executable(test-cppchecklibrarydata
        ${test-cppchecklibrarydata_SRC}
        ${test-cppchecklibrarydata_resources}
        testcppchecklibrarydata.cpp
        ${CMAKE_SOURCE_DIR}/gui/cppchecklibrarydata.cpp
        )
target_include_directories(test-cppchecklibrarydata PRIVATE ${CMAKE_SOURCE_DIR}/lib ${CMAKE_SOURCE_DIR}/gui)
target_compile_definitions(test-cppchecklibrarydata PRIVATE SRCDIR="${CMAKE_CURRENT_SOURCE_DIR}")
target_link_libraries(test-cppchecklibrarydata ${QT_CORE_LIB} ${QT_TEST_LIB})

if (CMAKE_CXX_COMPILER_ID MATCHES "Clang")
    if(QT_VERSION VERSION_GREATER_EQUAL "6.9.0")
        # caused by Qt generated moc code starting with 6.9.0 - see https://bugreports.qt.io/browse/QTBUG-135638
        target_compile_options_safe(test-cppchecklibrarydata -Wno-ctad-maybe-unsupported)
    endif()
endif()

if (REGISTER_GUI_TESTS)
    add_test(NAME test-cppchecklibrarydata COMMAND $<TARGET_FILE:test-cppchecklibrarydata>)
endif()

add_dependencies(gui-tests test-cppchecklibrarydata)