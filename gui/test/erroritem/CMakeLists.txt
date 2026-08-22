qt_wrap_cpp(test-erroritem_SRC testerroritem.h)
add_custom_target(build-testerroritem-deps SOURCES ${test-erroritem_SRC})
add_dependencies(gui-build-deps build-testerroritem-deps)
add_executable(test-erroritem
        ${test-erroritem_SRC}
        testerroritem.cpp
        ${CMAKE_SOURCE_DIR}/gui/erroritem.cpp
        )
target_include_directories(test-erroritem PRIVATE ${CMAKE_SOURCE_DIR}/gui)
target_link_libraries(test-erroritem ${QT_CORE_LIB} ${QT_TEST_LIB} cppcheck-core)

if (CMAKE_CXX_COMPILER_ID MATCHES "Clang")
    if(QT_VERSION VERSION_GREATER_EQUAL "6.9.0")
        # caused by Qt generated moc code starting with 6.9.0 - see https://bugreports.qt.io/browse/QTBUG-135638
        target_compile_options_safe(test-erroritem -Wno-ctad-maybe-unsupported)
    endif()
endif()

if (REGISTER_GUI_TESTS)
    add_test(NAME test-erroritem COMMAND $<TARGET_FILE:test-erroritem>)
endif()

add_dependencies(gui-tests test-erroritem)
