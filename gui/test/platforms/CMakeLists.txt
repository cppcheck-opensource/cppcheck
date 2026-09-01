qt_wrap_cpp(test-platforms_SRC
        testplatforms.h
        ${CMAKE_SOURCE_DIR}/gui/platforms.h
        )
add_custom_target(build-testplatforms-deps SOURCES ${test-platforms_SRC})
add_dependencies(gui-build-deps build-testplatforms-deps)
add_executable(test-platforms
        ${test-platforms_SRC}
        testplatforms.cpp
        ${CMAKE_SOURCE_DIR}/gui/platforms.cpp
        )
target_include_directories(test-platforms PRIVATE ${CMAKE_SOURCE_DIR}/gui)
target_link_libraries(test-platforms ${QT_CORE_LIB} ${QT_TEST_LIB} cppcheck-core)

if (CMAKE_CXX_COMPILER_ID MATCHES "Clang")
    if(QT_VERSION VERSION_GREATER_EQUAL "6.9.0")
        # caused by Qt generated moc code starting with 6.9.0 - see https://bugreports.qt.io/browse/QTBUG-135638
        target_compile_options_safe(test-platforms -Wno-ctad-maybe-unsupported)
    endif()
endif()

if (REGISTER_GUI_TESTS)
    add_test(NAME test-platforms COMMAND $<TARGET_FILE:test-platforms>)
endif()

add_dependencies(gui-tests test-platforms)
