add_executable(test-sehwrapper
    test-sehwrapper.cpp
    ${PROJECT_SOURCE_DIR}/cli/sehwrapper.cpp)
target_include_directories(test-sehwrapper PRIVATE ${PROJECT_SOURCE_DIR}/cli ${PROJECT_SOURCE_DIR}/lib)