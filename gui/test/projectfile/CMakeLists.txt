qt_wrap_cpp(test-projectfile_SRC testprojectfile.h ${CMAKE_SOURCE_DIR}/gui/projectfile.h)
add_custom_target(build-projectfile-deps SOURCES ${test-projectfile_SRC})
add_dependencies(gui-build-deps build-projectfile-deps)
add_executable(test-projectfile
        ${test-projectfile_SRC}
        testprojectfile.cpp
        ${CMAKE_SOURCE_DIR}/gui/common.cpp
        ${CMAKE_SOURCE_DIR}/gui/projectfile.cpp
        )
target_include_directories(test-projectfile PRIVATE ${CMAKE_SOURCE_DIR}/gui ${CMAKE_SOURCE_DIR}/lib)
target_compile_definitions(test-projectfile PRIVATE SRCDIR="${CMAKE_CURRENT_SOURCE_DIR}")
target_link_libraries(test-projectfile ${QT_CORE_LIB} ${QT_TEST_LIB})

if (CMAKE_CXX_COMPILER_ID MATCHES "Clang")
    if(QT_VERSION VERSION_GREATER_EQUAL "6.9.0")
        # caused by Qt generated moc code starting with 6.9.0 - see https://bugreports.qt.io/browse/QTBUG-135638
        target_compile_options_safe(test-projectfile -Wno-ctad-maybe-unsupported)
    endif()
endif()

if (REGISTER_GUI_TESTS)
    add_test(NAME test-projectfile COMMAND $<TARGET_FILE:test-projectfile>)
endif()

add_dependencies(gui-tests test-projectfile)