if(ANALYZE_MEMORY)
    add_compile_options(-fsanitize=memory)
    add_compile_options(-fsanitize-memory-track-origins=2)
    add_compile_options(-fno-omit-frame-pointer)
    add_compile_options(-fno-optimize-sibling-calls)

    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -fsanitize=memory")
elseif(ANALYZE_ADDRESS)
    add_compile_options(-fsanitize=address)
    add_compile_options(-fno-omit-frame-pointer)
    add_compile_options(-fno-optimize-sibling-calls)

    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -fsanitize=address")
elseif(ANALYZE_THREAD)
    add_compile_options(-fsanitize=thread)

    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -fsanitize=thread")
endif()

if(ANALYZE_UNDEFINED)
    # TODO: enable signed-integer-overflow
    add_compile_options(-fsanitize=undefined -fno-sanitize=signed-integer-overflow)
    if(CMAKE_CXX_COMPILER_ID MATCHES "Clang")
        add_compile_options(-fsanitize=nullability)
    endif()
    add_compile_options(-fno-sanitize-recover=all)
    add_compile_options(-fno-omit-frame-pointer)

    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -fsanitize=undefined -fno-sanitize=signed-integer-overflow")
    if(CMAKE_CXX_COMPILER_ID MATCHES "Clang")
        set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -fsanitize=nullability")
    endif()
endif()

if(ANALYZE_TYPE)
    add_compile_options(-fsanitize=type)

    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -fsanitize=type")
endif()
