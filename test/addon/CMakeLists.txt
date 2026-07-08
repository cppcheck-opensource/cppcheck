if (NOT Python_Interpreter_FOUND)
    message(WARNING "Python interpreter not found - skipping addon tests.")
    return()
endif()

set(VENV_DIR ${CMAKE_CURRENT_BINARY_DIR}/venv)
if (WIN32)
    set(VENV_PYTHON ${VENV_DIR}/Scripts/python.exe)
else()
    set(VENV_PYTHON ${VENV_DIR}/bin/python)
endif()

# fixture: create a virtual environment and install the python dependencies into it
add_test(NAME addon-venv-create
        COMMAND ${Python_EXECUTABLE} -m venv ${VENV_DIR})
set_tests_properties(addon-venv-create PROPERTIES
        FIXTURES_SETUP addon-venv-dir)

add_test(NAME addon-venv-install
        COMMAND ${VENV_PYTHON} -m pip install -r ${CMAKE_CURRENT_SOURCE_DIR}/requirements.txt)
set_tests_properties(addon-venv-install PROPERTIES
        FIXTURES_REQUIRED addon-venv-dir
        FIXTURES_SETUP addon-venv
        TIMEOUT 300)

add_test(NAME addon-cppcheckdata
        COMMAND ${VENV_PYTHON} -m pytest --cppcheck-binary=$<TARGET_FILE:cppcheck> ${CMAKE_CURRENT_SOURCE_DIR})
set_tests_properties(addon-cppcheckdata PROPERTIES
        FIXTURES_REQUIRED addon-venv)
