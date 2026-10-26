Feature: Analysis and recheck

  Background:
    Given the application is started with no project loaded

  @id:te65b1fa8 @project:basic-project
  Scenario: Reanalyze all files
    Given the project file {project:basic-project}/basic.cppcheck is opened from the File menu
    And the analysis is done
    When I change {project:basic-project}/src/a.c and {project:basic-project}/src/b.c so that they only contain ";"
    And I click Reanalyze all files
    And I wait until the analysis is done
    Then there are no results for a.c
    And there are no results for b.c

  @id:tc707b85b @ticket:14893 @project:recheck
  Scenario: Recheck a changed file
    Given the project file {project:recheck}/p.cppcheck is opened
    And the analysis is done
    And the results show a nullPointer error in a.c and an arrayIndexOutOfBounds error in b.c
    When I copy {project:recheck}/a_v2.c over {project:recheck}/a.c (a_v2.c has an uninitvar error instead)
    And I right click on the file a.c in the results and click Recheck
    Then the results for a.c are refreshed
    And the nullPointer error in a.c is gone
    And the uninitvar error from the changed a.c is shown
    And the arrayIndexOutOfBounds error in b.c is still shown, unchanged

  @id:tc550387d @project:basic-project
  Scenario: Recheck a header file
    Given the project file {project:basic-project}/basic.cppcheck is opened from the File menu
    And the analysis is done
    When I right click on the file util.h in the results
    Then the context menu of util.h has a Recheck entry
    When I click Recheck
    And I wait until the analysis is done
    Then the analysis is run again for the files that include util.h
    And the nullPointer error in util.h is shown again

  @id:ta6f3b40b @project:syntax-error
  Scenario Outline: Analyze a broken source file
    Given the project file {project:syntax-error}/<case>.cppcheck checks <case>.c, a broken or unusual source file, and ok.c, which has a nullPointer error
    When I open the project file {project:syntax-error}/<case>.cppcheck
    And I wait until the analysis is done
    Then the analysis finishes
    And the application does not crash
    And the nullPointer error in ok.c is shown

    Examples:
      | case             |
      | braces           |
      | empty            |
      | binary           |
      | latin1           |
      | long_line        |
      | unclosed_nesting |
