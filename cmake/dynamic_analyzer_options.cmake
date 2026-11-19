if(ANALYZE_MEMORY)
    list(APPEND CPPCHECK_COMPILE_OPTIONS -fsanitize=memory)
    list(APPEND CPPCHECK_COMPILE_OPTIONS -fsanitize-memory-track-origins=2)
    list(APPEND CPPCHECK_COMPILE_OPTIONS -fno-omit-frame-pointer)
    list(APPEND CPPCHECK_COMPILE_OPTIONS -fno-optimize-sibling-calls)

    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -fsanitize=memory")
elseif(ANALYZE_ADDRESS)
    list(APPEND CPPCHECK_COMPILE_OPTIONS -fsanitize=address)
    list(APPEND CPPCHECK_COMPILE_OPTIONS -fno-omit-frame-pointer)
    list(APPEND CPPCHECK_COMPILE_OPTIONS -fno-optimize-sibling-calls)

    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -fsanitize=address")
elseif(ANALYZE_THREAD)
    list(APPEND CPPCHECK_COMPILE_OPTIONS -fsanitize=thread)

    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -fsanitize=thread")
endif()

if(ANALYZE_UNDEFINED)
    # TODO: enable signed-integer-overflow
    list(APPEND CPPCHECK_COMPILE_OPTIONS -fsanitize=undefined -fno-sanitize=signed-integer-overflow)
    if(CMAKE_CXX_COMPILER_ID MATCHES "Clang")
        list(APPEND CPPCHECK_COMPILE_OPTIONS -fsanitize=nullability)
    endif()
    list(APPEND CPPCHECK_COMPILE_OPTIONS -fno-sanitize-recover=all)
    list(APPEND CPPCHECK_COMPILE_OPTIONS -fno-omit-frame-pointer)

    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -fsanitize=undefined -fno-sanitize=signed-integer-overflow")
    if(CMAKE_CXX_COMPILER_ID MATCHES "Clang")
        set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -fsanitize=nullability")
    endif()
endif()

if(ANALYZE_TYPE)
    list(APPEND CPPCHECK_COMPILE_OPTIONS -fsanitize=type)

    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -fsanitize=type")
endif()
