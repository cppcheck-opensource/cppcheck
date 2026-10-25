Feature: Results list

  Background:
    Given the application is started with no project loaded

  @id:t852206f1
  Scenario: Severity buttons toggle
    When I click each of the toolbar buttons Error, Warning, Style, Portability, Performance, Information, Cppcheck and Clang once
    Then every button can be clicked and changes between checked and unchecked
    When I click each of them once more
    Then all buttons are in the state they had at the start

  @id:td7decf1a @project:basic-project
  Scenario: Filter the results
    Given the project file {project:basic-project}/basic.cppcheck is opened from the File menu
    And the analysis is done
    When I type "arrayIndex" in the Filter field
    Then only the arrayIndexOutOfBounds error in b.c is shown
    When I clear the Filter field
    Then all errors are shown again

  @id:t2af9d498 @project:basic-project
  Scenario: Expand all
    Given the project file {project:basic-project}/basic.cppcheck is opened from the File menu
    And the analysis is done
    When I open View > Collapse All
    And I open View > Expand All
    Then all files are expanded and show their errors
    And the errors in a.c, util.h and b.c are visible

  @id:tec898362 @project:basic-project
  Scenario: Collapse all
    Given the project file {project:basic-project}/basic.cppcheck is opened from the File menu
    And the analysis is done
    When I open View > Expand All
    And I open View > Collapse All
    Then all files are collapsed
    And the file entries are shown and their errors are not

  @id:t4c027507 @project:basic-project
  Scenario: Context menu of a file entry
    Given the project file {project:basic-project}/basic.cppcheck is opened from the File menu
    And the analysis is done
    When I right click on the file a.c in the results (not on one of its errors)
    Then the context menu of the file entry has the entries Recheck and Open containing folder
    But it has none of the entries Hide, Hide all with ID or Suppress selected id, they only apply to a single error

  @id:taacc3841 @project:basic-project
  Scenario: Hide one error
    Given the project file {project:basic-project}/basic.cppcheck is opened from the File menu
    And the analysis is done
    When I expand a.c in the results
    And I right click on the first nullPointer error (line 6) and click Hide
    Then that error is gone
    And the other errors are still shown, including the second nullPointer error in a.c (line 12)

  @id:t2937b690 @project:basic-project
  Scenario: Hide all errors with the same ID
    Given the project file {project:basic-project}/basic.cppcheck is opened from the File menu
    And the analysis is done
    When I expand a.c in the results
    And I right click on a nullPointer error and click Hide all with ID
    Then all nullPointer errors are gone, in a.c and in util.h
    And the arrayIndexOutOfBounds error in b.c is still shown

  @id:t313463d3 @project:basic-project
  Scenario: Show hidden errors again
    Given the project file {project:basic-project}/basic.cppcheck is opened from the File menu
    And the analysis is done
    When I expand a.c in the results
    And I right click on the first nullPointer error and click Hide all with id
    Then the a.c results are hidden
    When I open View > Show Hidden
    Then the error that was hidden is shown again

  @id:t90bf32e9 @project:basic-project
  Scenario: Suppress selected ID
    Given the project file {project:basic-project}/basic.cppcheck is opened from the File menu
    And the analysis is done
    When I expand a.c in the results
    And I right click on a nullPointer error and click Suppress selected id
    And I open File > Edit Project File and go to Warning options
    Then no nullPointer error is shown in the results
    And the id nullPointer is listed under Suppressions in the project file dialog

  @id:t389a5f3a @project:basic-project
  Scenario: Clear results
    Given the project file {project:basic-project}/basic.cppcheck is opened from the File menu
    And the analysis is done
    When I click Clear results
    Then the results are cleared: the result tree is empty
