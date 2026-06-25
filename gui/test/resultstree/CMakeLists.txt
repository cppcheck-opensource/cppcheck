qt_wrap_cpp(test-resultstree_SRC
        testresultstree.h
        ${CMAKE_SOURCE_DIR}/gui/resultstree.h
        ${CMAKE_SOURCE_DIR}/gui/resultitem.h
        ${CMAKE_SOURCE_DIR}/gui/applicationlist.h
        ${CMAKE_SOURCE_DIR}/gui/common.h
        ${CMAKE_SOURCE_DIR}/gui/projectfile.h
        ${CMAKE_SOURCE_DIR}/gui/threadhandler.h
        ${CMAKE_SOURCE_DIR}/gui/threadresult.h
)
add_custom_target(build-resultstree-deps SOURCES ${test-resultstree_SRC})
add_dependencies(gui-build-deps build-resultstree-deps)
add_executable(test-resultstree
        ${test-resultstree_SRC}
        testresultstree.cpp
        ${CMAKE_SOURCE_DIR}/gui/resultstree.cpp
        ${CMAKE_SOURCE_DIR}/gui/resultitem.cpp
        ${CMAKE_SOURCE_DIR}/gui/common.cpp
        ${CMAKE_SOURCE_DIR}/gui/erroritem.cpp
        ${CMAKE_SOURCE_DIR}/gui/showtypes.cpp
        ${CMAKE_SOURCE_DIR}/gui/report.cpp
        ${CMAKE_SOURCE_DIR}/gui/xmlreportv2.cpp
        )
target_include_directories(test-resultstree PRIVATE ${CMAKE_SOURCE_DIR}/gui)
target_link_libraries(test-resultstree cppcheck-core simplecpp tinyxml2)
if (HAVE_RULES)
    target_link_libraries(test-resultstree ${PCRE_LIBRARY})
    target_include_directories(test-resultstree SYSTEM PRIVATE ${PCRE_INCLUDE})
endif()
target_compile_definitions(test-resultstree PRIVATE SRCDIR="${CMAKE_CURRENT_SOURCE_DIR}")
target_link_libraries(test-resultstree ${QT_CORE_LIB} ${QT_GUI_LIB} ${QT_WIDGETS_LIB} ${QT_TEST_LIB})

if (CMAKE_CXX_COMPILER_ID MATCHES "Clang")
    if(QT_VERSION VERSION_GREATER_EQUAL "6.9.0")
        # caused by Qt generated moc code starting with 6.9.0 - see https://bugreports.qt.io/browse/QTBUG-135638
        target_compile_options_safe(test-resultstree -Wno-ctad-maybe-unsupported)
    endif()
    # caused by mocks
    target_compile_options_safe(test-resultstree -Wno-missing-noreturn)
elseif (CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
    # caused by mocs
    target_compile_options_safe(test-resultstree -Wno-useless-cast)
    # caused by mocks
    target_compile_options_safe(test-resultstree -Wno-suggest-attribute=noreturn)
endif()

if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
    # caused by mocks
    target_compile_options_safe(test-resultstree -Wno-suggest-attribute=noreturn)
endif()

if (REGISTER_GUI_TESTS)
    # TODO: might crash - see #13223
    #add_test(NAME test-resultstree COMMAND $<TARGET_FILE:test-resultstree> -platform offscreen)
endif()

add_dependencies(gui-tests test-resultstree)
