if (BUILD_TESTING)
    add_subdirectory(signal)
    add_subdirectory(seh)

    file(GLOB hdrs "*.h")
    file(GLOB srcs "*.cpp")
    list(APPEND testrunner_SOURCES ${hdrs} ${srcs})

    add_executable(testrunner ${testrunner_SOURCES})
    target_link_libraries(testrunner cppcheck-core tinyxml2 picojson simplecpp frontend cli)
    if (WIN32 AND NOT BORLAND)
        if(NOT MINGW)
            target_link_libraries(testrunner Shlwapi.lib)
        else()
            target_link_libraries(testrunner shlwapi)
        endif()
    endif()
    target_link_libraries(testrunner ${CMAKE_THREAD_LIBS_INIT})
    if (CMAKE_CXX_COMPILER_ID MATCHES "GNU")
        # (void) in ASSERT_THROW* macros might trigger this
        target_compile_options_safe(testrunner -Wno-useless-cast)
    endif()
    if (CMAKE_CXX_COMPILER_ID MATCHES "Clang")
        # $ is used in dinit() designated initialization helper
        target_compile_options_safe(testrunner -Wno-dollar-in-identifier-extension)
    endif()

    if (NOT CMAKE_DISABLE_PRECOMPILE_HEADERS)
        target_precompile_headers(testrunner PRIVATE precompiled.h)
    endif()

    add_dependencies(testrunner copy_cfg)
    add_dependencies(testrunner copy_addons)
    add_dependencies(testrunner copy_platforms)
    if (NOT DISABLE_DMAKE)
        add_dependencies(testrunner run-dmake)
    endif()

    if (LIBXML2_XMLLINT_EXECUTABLE)
        # TODO: run the CMake implementation of the tests
        # TODO: get rid of the copy
        add_custom_target(checkcfg ${CMAKE_COMMAND} -E copy $<TARGET_FILE:cppcheck> ${CMAKE_SOURCE_DIR}
                COMMAND ${CMAKE_CURRENT_SOURCE_DIR}/cfg/runtests.sh
                DEPENDS cppcheck validateCFG)
    endif()

    if (REGISTER_TESTS)
        # CMAKE_MATCH_<n> usage for if (MATCHES) requires CMake 3.9

        find_package(Threads REQUIRED)
        include(ProcessorCount)
        ProcessorCount(N)
        set(CTEST_PARALLEL_LEVEL ${N} CACHE STRING "CTest parallel level")
        # the macos-* runners are sporadically much slower than other runners so chose a high timeout
        set(CTEST_TIMEOUT 130 CACHE STRING "CTest timeout")
        add_custom_target(check ${CMAKE_CTEST_COMMAND} --output-on-failure -j ${CTEST_PARALLEL_LEVEL} -C ${CMAKE_CFG_INTDIR} --timeout ${CTEST_TIMEOUT}
                DEPENDS testrunner cppcheck)

        set(SKIP_TESTS "" CACHE STRING "A list of tests to skip")

        function(add_fixture NAME)
            if (${NAME} IN_LIST SKIP_TESTS)
            elseif(TEST ${NAME})
            else()
                add_test(NAME ${NAME} COMMAND $<TARGET_FILE:testrunner> ${NAME} WORKING_DIRECTORY ${CMAKE_RUNTIME_OUTPUT_DIRECTORY})
            endif()
        endfunction()

        foreach(SRC ${srcs})
            file(STRINGS ${SRC} FIXTURE_LINE REGEX "^REGISTER_TEST\\([a-zA-z0-9]+\\)$")
            foreach(_fixture_line ${FIXTURE_LINE})
                if(_fixture_line MATCHES "^REGISTER_TEST\\(([a-zA-z0-9]+)\\)$")
                    add_fixture(${CMAKE_MATCH_1})
                endif()
            endforeach()
        endforeach()

        function(add_cfg CFG_TEST)
            set(oneValueArgs PLATFORM NAME)
            set(multiValueArgs ADD_LIBRARY)

            cmake_parse_arguments(PARSE "" "${oneValueArgs}" "${multiValueArgs}" ${ARGN})
            get_filename_component(LIBRARY ${CFG_TEST} NAME_WE)
            # TODO: get rid of this
            if(PARSE_ADD_LIBRARY)
                string(REPLACE ";" "," ADD_LIBRARY "${PARSE_ADD_LIBRARY}")
                set(LIBRARY "${ADD_LIBRARY},${LIBRARY}")
            endif()
            set(PLATFORM unix64)
            if(PARSE_PLATFORM)
                set(PLATFORM ${PARSE_PLATFORM})
            endif()
            if(PARSE_NAME)
                set(TEST_NAME ${PARSE_NAME})
            else()
                string(MAKE_C_IDENTIFIER ${CFG_TEST} TEST_NAME)
            endif()
            if ("cfg-${TEST_NAME}" IN_LIST SKIP_TESTS)
            else()
                # TODO: add syntax check
                add_test(NAME cfg-${TEST_NAME}
                        COMMAND $<TARGET_FILE:cppcheck>
                        --library=${LIBRARY}
                        --check-library
                        --check-level=exhaustive
                        --platform=${PLATFORM}
                        --enable=style,information
                        --inconclusive
                        --force
                        --error-exitcode=1
                        --inline-suppr
                        --debug-warnings
                        --suppress=checkersReport
                        ${CMAKE_CURRENT_SOURCE_DIR}/cfg/${CFG_TEST}
                )
            endif()
        endfunction()
        # TODO: glob this
        add_cfg(boost.cpp)
        add_cfg(bsd.c)
        add_cfg(cairo.c)
        add_cfg(cppunit.cpp)
        add_cfg(emscripten.cpp)
        # TODO: posix needs to specified first or it has a different mmap() config
        # TODO: get rid of posix dependency
        add_cfg(gnu.c ADD_LIBRARY posix)
        add_cfg(googletest.cpp)
        add_cfg(gtk.c)
        add_cfg(kde.cpp ADD_LIBRARY qt)
        add_cfg(libcurl.c)
        add_cfg(libsigc++.cpp)
        add_cfg(lua.c)
        add_cfg(mfc.cpp)
        add_cfg(opencv2.cpp)
        add_cfg(openmp.c)
        add_cfg(openssl.c)
        add_cfg(posix.c)
        add_cfg(python.c)
        add_cfg(qt.cpp)
        add_cfg(selinux.c)
        add_cfg(sqlite3.c)
        add_cfg(std.c)
        add_cfg(std.cpp)
        add_cfg(windows.cpp NAME windows32A PLATFORM win32A)
        add_cfg(windows.cpp NAME windows32W PLATFORM win32W)
        add_cfg(windows.cpp NAME windows64 PLATFORM win64)
        add_cfg(wxwidgets.cpp)

        function(fixture_cost NAME COST)
            if(TEST ${NAME})
                set_tests_properties(${NAME} PROPERTIES COST ${COST})
            endif()
        endfunction()

        # Set cost of the more expensive tests to help improve parallel scheduling
        # of tests
        #
        # To collect data to update this list remove "<cmake-build-folder>/Testing/Temporary/CTestCostData.txt",
        # disable the fixture_cost() statements below and run a Debug build with "ctest -j1" several times.
        # Afterwards run it with "ctest -j11" and immediately cancel the run and update the list accordingly to the
        # first eleven tests chosen.
        #
        # NOTE: The TestProcessExecutor* tests are not the slowest but they invoke processes which max out the system
        # and negatively impact the run-time of the other tests
        if (TRUE)
            fixture_cost(TestProcessExecutorFiles 2.00)
            fixture_cost(TestProcessExecutorFS 2.00)
            fixture_cost(TestCondition 1.30)
            fixture_cost(TestStl 1.30)
            fixture_cost(TestTokenizer 1.30)
            fixture_cost(cfg-std_c 1.20)
            fixture_cost(cfg-std_cpp 1.20)
            fixture_cost(TestAutoVariables 1.20)
            fixture_cost(TestUninitVar 1.20)
            fixture_cost(TestSimplifyTemplate 1.15)
            fixture_cost(TestIO 1.15)
            fixture_cost(TestBufferOverrun 1.15)
            fixture_cost(TestClass 1.15)
            fixture_cost(TestNullPointer 1.15)
            fixture_cost(TestSuppressions 1.15)
            fixture_cost(TestLeakAutoVar 1.10)
            fixture_cost(TestSingleExecutorFS 1.10)
            fixture_cost(TestLeakAutoVarRecursiveCountLimit 1.05)
            fixture_cost(TestThreadExecutorFS 1.05)
            fixture_cost(TestThreadExecutorFiles 1.05)
            fixture_cost(TestSymbolDatabase 1.05)
            fixture_cost(TestSingleExecutorFiles 1.05)
            fixture_cost(TestOther 1.05)
            fixture_cost(TestValueFlow 1.05)
            fixture_cost(cfg-wxwidgets_cpp 1.05)
            fixture_cost(cfg-posix_c 1.05)
            fixture_cost(cfg-windows32A 1.03)
            fixture_cost(cfg-windows32W 1.03)
            fixture_cost(cfg-windows64 1.03)
            fixture_cost(TestUnusedVar 1.03)
        endif()
    endif()
endif()
