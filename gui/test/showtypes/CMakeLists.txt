qt_wrap_cpp(test-showtypes_SRC testshowtypes.h)
add_custom_target(build-testshowtypes-deps SOURCES ${test-showtypes_SRC})
add_dependencies(gui-build-deps build-testshowtypes-deps)
add_executable(test-showtypes
        ${test-showtypes_SRC}
        testshowtypes.cpp
        ${CMAKE_SOURCE_DIR}/gui/showtypes.cpp
        )
target_include_directories(test-showtypes PRIVATE ${CMAKE_SOURCE_DIR}/gui)
target_link_libraries(test-showtypes ${QT_CORE_LIB} ${QT_TEST_LIB} cppcheck-core)

if (CMAKE_CXX_COMPILER_ID MATCHES "Clang")
    if(QT_VERSION VERSION_GREATER_EQUAL "6.9.0")
        # caused by Qt generated moc code starting with 6.9.0 - see https://bugreports.qt.io/browse/QTBUG-135638
        target_compile_options_safe(test-showtypes -Wno-ctad-maybe-unsupported)
    endif()
endif()

if (REGISTER_GUI_TESTS)
    add_test(NAME test-showtypes COMMAND $<TARGET_FILE:test-showtypes>)
endif()

add_dependencies(gui-tests test-showtypes)
