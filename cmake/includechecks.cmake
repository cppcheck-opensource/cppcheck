include(CheckIncludeFileCXX)

if(NOT MSVC)
    check_include_file_cxx(execinfo.h HAVE_EXECINFO_H)
    if(NOT HAVE_EXECINFO_H)
        set(HAVE_EXECINFO_H 0)
    endif()
endif()