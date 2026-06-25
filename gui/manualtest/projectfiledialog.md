
# Project file dialog

Some manual testing in the project file dialog interface


## Test: Platform file pic8.xml

Ticket: #14489

EXPECTED: In the project file dialog it should be possible to select xml files i.e. pic8.xml

TODO: can this test be automated


## Test: Custom cfg file

Ticket: #14672

1. Copy addons/avr.cfg to a file "aa.cfg" in same folder as a cppcheck GUI project file

EXPECTED: It should not be possible to activate "aa.cfg" in the project file dialog, it should appear in alphabetical order.


## Test: Misra C checkbox

Ticket: #14488

Matrix: Use both open source and premium

1. Load project file in trac ticket 14488
1. Goto "Edit project"
1. Goto "Addons" tab

EXPECTED: The misra c checkbox should be checked

TODO: can this test be automated

