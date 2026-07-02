# TODO: bail out when Python_EXECUTABLE is not set

if(NOT CMAKE_DISABLE_PRECOMPILE_HEADERS)
    # clang-tidy and clang need to have the same version when precompiled headers are being used
    if(CMAKE_CXX_COMPILER_ID MATCHES "Clang")
        string(REGEX MATCHALL "[0-9]+" _clang_ver_parts "${CMAKE_CXX_COMPILER_VERSION}")
        LIST(GET _clang_ver_parts 0 _clang_major)
        set(RUN_CLANG_TIDY_NAMES "run-clang-tidy-${_clang_major}")
        message(STATUS "Clang and clang-tidy version need to match when precompiled headers are enabled - limiting search to '${RUN_CLANG_TIDY_NAMES}'")
    else()
        message(STATUS "Cannot use non-Clang compiler with clang-tidy when precompiled headers are enabled - skipping 'run-clang-tidy'/'run-clang-tidy-csa' target generation")
    endif()
else()
    set(RUN_CLANG_TIDY_NAMES run-clang-tidy run-clang-tidy-23 run-clang-tidy-22 run-clang-tidy-21 run-clang-tidy-20 run-clang-tidy-19 run-clang-tidy-18 run-clang-tidy-17 run-clang-tidy-16 run-clang-tidy-15 run-clang-tidy-14 run-clang-tidy-13 run-clang-tidy-12 run-clang-tidy-11 run-clang-tidy-10 run-clang-tidy-9 run-clang-tidy-8)
endif()

if(RUN_CLANG_TIDY_NAMES)
    find_program(RUN_CLANG_TIDY NAMES ${RUN_CLANG_TIDY_NAMES})
    message(STATUS "RUN_CLANG_TIDY=${RUN_CLANG_TIDY}")
    if(RUN_CLANG_TIDY)
        include(ProcessorCount)
        ProcessorCount(NPROC)
        if(NPROC EQUAL 0)
            message(FATAL_ERROR "could not get processor count")
        endif()
        message(STATUS "NPROC=${NPROC}")

        if(CMAKE_CXX_COMPILER_VERSION VERSION_GREATER_EQUAL 22)
            set(CLANG_TIDY_CONFIG "-enable-check-profile")
        else()
            set(CLANG_TIDY_CONFIG "")
        endif()

        # most of these are disabled because they are too noisy in our code
        # clang-analyzer-core.CallAndMessage
        # clang-analyzer-core.NonNullParamChecker
        # clang-analyzer-cplusplus.NewDeleteLeaks
        # clang-analyzer-cplusplus.NewDelete
        # clang-analyzer-core.NullDereference
        # clang-analyzer-unix.Stream
        # clang-analyzer-alpha.clone.CloneChecker
        # clang-analyzer-alpha.webkit.* - we are not interested in these
        set(CLANG_TIDY_CSA_CONFIG "-config={InheritParentConfig: true, Checks: '-*,clang-analyzer-*,-clang-analyzer-core.CallAndMessage,-clang-analyzer-core.NonNullParamChecker,-clang-analyzer-cplusplus.NewDeleteLeaks,-clang-analyzer-cplusplus.NewDelete,-clang-analyzer-core.NullDereference,-clang-analyzer-unix.Stream,-clang-analyzer-alpha.clone.CloneChecker,-clang-analyzer-alpha.webkit.*'}")
        if (ENABLE_CSA_ALPHA)
            set(CLANG_TIDY_CSA_ALPHA_OPTS "-allow-enabling-alpha-checkers" "-extra-arg=-Xclang" "-extra-arg=-analyzer-config" "-extra-arg=-Xclang" "-extra-arg=aggressive-binary-operation-simplification=true")
        else()
            set(CLANG_TIDY_CSA_ALPHA_OPTS "")
        endif()

        # TODO: exclude moc_*.cpp
        # TODO: exclude mocs_compilation.cpp
        add_custom_target(run-clang-tidy
                ${Python_EXECUTABLE} ${RUN_CLANG_TIDY} -p=${CMAKE_BINARY_DIR} -j ${NPROC} -quiet ${CLANG_TIDY_CONFIG}
                USES_TERMINAL
                VERBATIM)
        add_custom_target(run-clang-tidy-csa
                ${Python_EXECUTABLE} ${RUN_CLANG_TIDY} -p=${CMAKE_BINARY_DIR} -j ${NPROC} -quiet ${CLANG_TIDY_CONFIG} ${CLANG_TIDY_CSA_ALPHA_OPTS} ${CLANG_TIDY_CSA_CONFIG}
                USES_TERMINAL
                VERBATIM)
        if(BUILD_GUI)
            add_dependencies(run-clang-tidy gui-build-deps)
            add_dependencies(run-clang-tidy-csa gui-build-deps)
            if(BUILD_TRIAGE)
                add_dependencies(run-clang-tidy triage-build-ui-deps)
                add_dependencies(run-clang-tidy-csa triage-build-ui-deps)
            endif()
        endif()
    endif()
endif()
