qt_wrap_cpp(test-codeeditorstyle_SRC testcodeeditorstyle.h)
add_custom_target(build-testcodeeditorstyle-deps SOURCES ${test-codeeditorstyle_SRC})
add_dependencies(gui-build-deps build-testcodeeditorstyle-deps)
add_executable(test-codeeditorstyle
        ${test-codeeditorstyle_SRC}
        testcodeeditorstyle.cpp
        ${CMAKE_SOURCE_DIR}/gui/codeeditorstyle.cpp
        )
target_include_directories(test-codeeditorstyle PRIVATE ${CMAKE_SOURCE_DIR}/gui)
target_link_libraries(test-codeeditorstyle ${QT_CORE_LIB} ${QT_GUI_LIB} ${QT_TEST_LIB})

if (CMAKE_CXX_COMPILER_ID MATCHES "Clang")
    if(QT_VERSION VERSION_GREATER_EQUAL "6.9.0")
        # caused by Qt generated moc code starting with 6.9.0 - see https://bugreports.qt.io/browse/QTBUG-135638
        target_compile_options_safe(test-codeeditorstyle -Wno-ctad-maybe-unsupported)
    endif()
endif()

if (REGISTER_GUI_TESTS)
    add_test(NAME test-codeeditorstyle COMMAND $<TARGET_FILE:test-codeeditorstyle> -platform offscreen)
endif()

add_dependencies(gui-tests test-codeeditorstyle)
