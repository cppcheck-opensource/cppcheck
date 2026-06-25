qt_wrap_cpp(test-filelist_SRC testfilelist.h)
add_custom_target(build-testfilelist-deps SOURCES ${test-filelist_SRC})
add_dependencies(gui-build-deps build-testfilelist-deps)
add_executable(test-filelist
        ${test-filelist_SRC}
        testfilelist.cpp
        ${CMAKE_SOURCE_DIR}/gui/filelist.cpp
        )
target_include_directories(test-filelist PRIVATE ${CMAKE_SOURCE_DIR}/gui)
target_compile_definitions(test-filelist PRIVATE SRCDIR="${CMAKE_CURRENT_SOURCE_DIR}")
target_link_libraries(test-filelist ${QT_CORE_LIB} ${QT_TEST_LIB} cppcheck-core simplecpp)

if (CMAKE_CXX_COMPILER_ID MATCHES "Clang")
    if(QT_VERSION VERSION_GREATER_EQUAL "6.9.0")
        # caused by Qt generated moc code starting with 6.9.0 - see https://bugreports.qt.io/browse/QTBUG-135638
        target_compile_options_safe(test-filelist -Wno-ctad-maybe-unsupported)
    endif()
endif()

if (REGISTER_GUI_TESTS)
    add_test(NAME test-filelist COMMAND $<TARGET_FILE:test-filelist>)
endif()

add_dependencies(gui-tests test-filelist)
