Feature: Project file dialog

  Background:
    Given the application is started with no project loaded

  @id:tcf7ddb76 @ticket:14983 @project:relative-paths
  Scenario: Relative paths in the saved project
    Given a new project file is created and the project file dialog is open
    When I import the project {project:relative-paths}/compile_commands.json (Paths and defines, Import project)
    And I add the include directory {project:relative-paths}/inc
    And I exclude the file {project:relative-paths}/skip/b.c
    And I save the project file as {project:relative-paths}/rel.cppcheck
    Then in the saved rel.cppcheck the import project is written relative to the project file: compile_commands.json
    And the include directory is written relative to the project file: inc
    And the excluded file is written relative to the project file: skip/b.c
    And the XML contains no absolute path to the project folder

  @id:t5afff002 @ticket:14489
  Scenario: Select the platform pic8.xml
    Given a new project file is created and the project file dialog is open
    When I open the Platform drop-down list
    And I select pic8.xml in the list
    Then pic8.xml is in the Platform list
    And pic8.xml is the platform that the dialog shows

  @id:t91490557 @ticket:14672 @project:custom-cfg
  Scenario: Custom cfg file in the library list
    Given the folder of {project:custom-cfg}/p.cppcheck also holds aa.cfg, a copy of cfg/avr.cfg
    When I open the project file {project:custom-cfg}/p.cppcheck
    And I open the project file dialog (edit the project file)
    And I look at the library list (Libraries)
    Then aa is in the library list
    And aa comes before avr, the list is in alphabetical order

  @id:t0ec0680b @ticket:14488 @project:embedded @edition:opensource
  Scenario: Misra checkbox of a loaded project
    When I open the project file {project:embedded}/embedded.cppcheck
    And I open the project file dialog (edit the project file)
    And I go to the Addons tab
    Then the Misra C 2012 checkbox is checked
