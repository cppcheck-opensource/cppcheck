Feature: Open and create projects

  Background:
    Given the application is started with no project loaded

  @id:tf9fc8bf6 @project:basic-project
  Scenario: Open a project file
    When I open the project file {project:basic-project}/basic.cppcheck with the open project file entry in the File menu
    Then the project opens
    And the title of the main window names basic.cppcheck
    And the analysis results are shown: nullPointer errors in a.c and util.h, arrayIndexOutOfBounds in b.c

  @id:t6bb11424 @project:switch-project
  Scenario: Open a second project
    TODO: with and without build dir
    Given the project file {project:switch-project}/one/p1.cppcheck is opened
    And the analysis is done
    And the results show a nullPointer error in a.c
    When I open the project file {project:switch-project}/two/p2.cppcheck
    And I wait until the analysis is done
    Then the results of the first project (nullPointer in a.c) are gone
    And the results of the second project (arrayIndexOutOfBounds in b.c) are shown

  @id:t176fd17f @project:switch-project
  Scenario: Open a recent project
    Given the project file {project:switch-project}/one/p1.cppcheck is opened and analysed
    And the project file {project:switch-project}/two/p2.cppcheck is opened and analysed
    When I open the File menu
    And I click p1.cppcheck in the list of recent projects
    Then the project p1.cppcheck is opened
    And the title of the main window names p1.cppcheck, not p2.cppcheck or another file

  @id:t1e899c1c @project:basic-project
  Scenario: New project file
    When I open File > New Project File
    And I enter the file name {project:basic-project}/OSgui.cppcheck
    And I add the folder {project:basic-project}/src as a path to check in the project file dialog
    And I save the project file without selecting an addon
    And I wait until the analysis is done
    Then the project file OSgui.cppcheck is saved without any addon in it
    And the analysis is run
    And the results are shown: nullPointer errors in a.c, arrayIndexOutOfBounds in b.c
    And util.h is not checked, the new project has no include path to it

  @id:t202b9515 @project:basic-project
  Scenario Outline: New project file with an addon
    When I open File > New Project File
    And I enter the file name {project:basic-project}/OSgui.cppcheck
    And I add the folder {project:basic-project}/src as a path to check in the project file dialog
    And I tick the checkbox of the addon whose name contains "<addon>" on the Addons tab (Misra C, Y2038 or Thread safety)
    And I save the project file
    And I wait until the analysis is done
    Then the project file OSgui.cppcheck is saved with an addon entry that contains "<addon>" (misra, y2038 or threadsafety)
    And the analysis is run and results are shown
    And none of the results is an internalError about the addon

    Examples:
      | addon  |
      | misra  |
      | y2038  |
      | thread |

  @id:tf6da2cfd @project:basic-project @edition:opensource
  Scenario: Add the Misra addon to a project
    Given the project file {project:basic-project}/basic.cppcheck is opened from the File menu
    And the analysis is done
    When I open File > Edit Project File
    And I select MISRA C 2012 on the Addons tab
    And I save the project file
    And I wait until the analysis is done
    Then basic.cppcheck has the addon misra in it
    And the analysis is run again
