qt_wrap_cpp(test-translationhandler_SRC testtranslationhandler.h ${CMAKE_SOURCE_DIR}/gui/translationhandler.h)
add_custom_target(build-translationhandler-deps SOURCES ${test-translationhandler_SRC})
add_dependencies(gui-build-deps build-translationhandler-deps)
add_executable(test-translationhandler
        ${test-translationhandler_SRC}
        testtranslationhandler.cpp
        ${CMAKE_SOURCE_DIR}/gui/common.cpp
        ${CMAKE_SOURCE_DIR}/gui/translationhandler.cpp
        )
target_include_directories(test-translationhandler PRIVATE ${CMAKE_SOURCE_DIR}/gui ${CMAKE_SOURCE_DIR}/lib)
target_link_libraries(test-translationhandler ${QT_CORE_LIB} ${QT_WIDGETS_LIB} ${QT_TEST_LIB})

if (CMAKE_CXX_COMPILER_ID MATCHES "Clang")
    if(QT_VERSION VERSION_GREATER_EQUAL "6.9.0")
        # caused by Qt generated moc code starting with 6.9.0 - see https://bugreports.qt.io/browse/QTBUG-135638
        target_compile_options_safe(test-translationhandler -Wno-ctad-maybe-unsupported)
    endif()
endif()

if (REGISTER_GUI_TESTS)
    add_test(NAME test-translationhandler COMMAND $<TARGET_FILE:test-translationhandler> -platform offscreen)
endif()

add_dependencies(gui-tests test-translationhandler)