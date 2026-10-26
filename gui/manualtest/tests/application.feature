Feature: Application, menus and help

  Background:
    Given the application is started with no project loaded

  @id:t41f9102b
  Scenario: All menus open
    When I open each of the menus File, Edit, View, Analyze and Help in turn
    And I look at its entries and close it with Escape
    Then each of the menus File, Edit, View, Analyze and Help opens
    And each menu shows at least one entry

  @id:t1cc2be0f
  Scenario: Show and hide toolbars
    Given all entries of View > Toolbar are checked
    When I uncheck the entries of View > Toolbar one at a time
    Then a toolbar that is unchecked is no longer shown in the main window
    And the toolbars that are still checked are shown

  @id:t5fdeee58 @edition:opensource
  Scenario: Same version in the title and in About
    Given the title of the main window is "Cppcheck <version>"
    When I note the version in the title
    And I open Help > About
    Then the About dialog shows the same version as the title of the main window

  @id:t90321d9c
  Scenario: Help contents
    When I open Help > Contents
    Then a help window opens
    And the help window has contents, it is not empty

  @id:tc059b730
  Scenario: Other Help menu entries
    When I open each of the other entries of the Help menu in turn (About and the rest)
    And I close each window with Escape or its Close button
    Then every entry opens a dialog or window with text in it
    And none of them is empty

  @id:t5bf8f6ae @ticket:13982
  Scenario: Scratch pad
    When I open View > Scratch Pad
    And I enter this code, one line each:
      """
      int main(void)
      {
          int *p = 0;
          return *p;
      }
      """
    And I click Check
    Then the code is analysed
    And a nullPointer error is shown
    And the application does not crash

  @id:t1a2285d8
  Scenario: Quit from the File menu
    When I open File > Quit
    Then the application closes
