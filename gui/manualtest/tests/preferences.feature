Feature: Preferences

  Background:
    Given the application is started with no project loaded

  @id:t232a9783
  Scenario: Default number of threads
    When I open Edit > Preferences
    And I look at the General tab
    Then the number of threads is 1

  @id:tf5b5a7e1
  Scenario: Number of threads cannot be 0
    Given Edit > Preferences is open on the General tab
    When I type 0 in the number of threads field
    And I close the dialog with OK
    And I open Edit > Preferences again
    Then the field has not accepted 0
    And the number of threads is at least 1, it is never 0

  @id:t841cca6c @project:basic-project
  Scenario: Analysis with 100 threads
    Given the number of threads is set to 100 in Edit > Preferences and the dialog is closed with OK
    When I open the project file {project:basic-project}/basic.cppcheck from the File menu
    And I wait until the analysis is done
    Then the application does not crash
    And the analysis is done and the results are shown

  @id:t28ce8afe @project:embedded @edition:opensource @disabled
  Scenario Outline: Misra rule text file
    Given Edit > Preferences is open on the Addons tab
    When I select the rule text file {project:embedded}/<rulefile>
    And I close the dialog with OK
    And I open the project file {project:embedded}/embedded.cppcheck
    And I wait until the analysis is done
    Then the Misra error for the unused parameter has the rule text from the file (TEST RULE TEXT: unused parameters are not allowed)
    And there are no internalError results

    Examples:
      | rulefile              |
      | rules.txt             |
      | rules with spaces.txt |
