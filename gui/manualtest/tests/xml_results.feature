Feature: XML results

  Background:
    Given the application is started with no project loaded

  @id:tc1fc409e @project:basic-project
  Scenario Outline: Save results to a file
    Given the project file {project:basic-project}/basic.cppcheck is opened from the File menu
    And the analysis is done
    When I open File > Save results to file
    And I choose the file type for <format> files
    And I save the results as results.<format> in the work directory {work}
    Then the file results.<format> is saved in the <format> format
    And it has the nullPointer error in a.c ("Null pointer dereference")
    And it has the arrayIndexOutOfBounds error in b.c ("out of bounds")

    Examples:
      | format |
      | xml    |
      | txt    |
      | csv    |

  @id:tcdb051d4 @project:xml-results
  Scenario: Open XML results
    When I open File > Open XML
    And I select {project:xml-results}/results.xml
    Then the XML results are opened
    And the nullPointer error in a.c is shown

  @id:t8d9a2076 @ticket:13984 @project:xml-results
  Scenario: No Recheck in XML results
    Given the XML results {project:xml-results}/results.xml are opened with File > Open XML
    When I right click on the file a.c in the results
    Then the context menu of the file has no Recheck entry and no Open containing folder entry, XML results can not be analysed again
    And the application does not crash

  @id:t053b7c9c @project:xml-results @project:basic-project
  Scenario: Reanalyze enabled after XML results
    Given the XML results {project:xml-results}/results.xml are opened with File > Open XML
    Then the button Reanalyze all files is disabled
    And if I rightclick on a.c in the results I get a popup
    And there is a option "Recheck 1 file(s)" in the popup that is disabled
    When I open the project file {project:basic-project}/basic.cppcheck
    And I wait until the analysis is done
    Then the button Reanalyze all files is enabled and can be clicked
