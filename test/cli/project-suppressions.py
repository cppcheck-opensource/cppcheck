
# python -m pytest project-suppressions.py

from testutils import create_gui_project_file, assert_cppcheck

def test_cli_and_project_suppressions(tmp_path):
    # Uninitvar suppressed in project file
    suppressions = [{ 'id': 'uninitvar' }]
    project_path = tmp_path / 'project.cppcheck'
    create_gui_project_file(project_path, root_path=str(tmp_path), suppressions=suppressions)

    # Uninitvar suppressed on command line before import
    args = ['--suppress=uninitvar', f'--project={project_path}']
    out_exp = [
        "cppcheck: error: suppression 'uninitvar' already exists",
        f"cppcheck: error: failed to load project '{project_path}'. An error occurred."
    ]
    assert_cppcheck(args, ec_exp=1, out_exp=out_exp)

def test_multiple_cli_and_project_suppressions(tmp_path):
    # Uninitvar and unreadVariable suppressed in project file
    suppressions = [{ 'id': 'uninitvar' }, { 'id': 'unreadVariable' },]
    project_path = tmp_path / 'project.cppcheck'
    create_gui_project_file(project_path, root_path=str(tmp_path), suppressions=suppressions)

    # Uninitvar and unreadVariable suppressed on command line before import
    args = ['--suppress=uninitvar', '--suppress=unreadVariable', f'--project={project_path}']
    out_exp = [
        "cppcheck: error: suppression 'uninitvar' already exists",
        "cppcheck: error: suppression 'unreadVariable' already exists",
        f"cppcheck: error: failed to load project '{project_path}'. An error occurred."
    ]
    assert_cppcheck(args, ec_exp=1, out_exp=out_exp)
