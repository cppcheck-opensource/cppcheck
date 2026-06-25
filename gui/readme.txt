Cppcheck GUI
============
This is a GUI for cppcheck. It allows selecting folder or set of files to check
with cppcheck and shows list of found errors.

Running
-------
You need Qt5 libraries installed in your system. Packages/files to install
depends on your operating system:
- Windows: download Qt from http://www.qt.io/download/
- Linux: install Qt using your package manager, look for packages having Qt
  in their name, e.g. for Ubuntu install libqt5core5a, libqt5gui5, libqt5widgets5 
  and libqt5printsupport5.

Compiling
---------
Windows:
- The easy ways are:
-- download Qt SDK from http://www.qt.io/download/ and use
   QtCreator to build the GUI.
-- Download precompiled libraries for your platform and use your preferred
   IDE/environment to build GUI. Be careful to download the correct version of
   library for your compiler!
- The harder way is to download Qt sources and build Qt. Compiling Qt alone may
  take over 4 hours!

Linux:
- Install Qt development packages (make sure qmake -tool gets installed!). The
  names depend on distribution, but e.g. for Ubuntu the needed packages are:
  * qt5-default 

After you have needed libraries and tools installed you need to run CMake with
`-DBUILD_GUI=On`.

On Windows, you have to either call qtvars.bat in Qt folder or use the Qt command
line prompt shortcut added in the start menu by Qt installation.

Tests
-----
There are tests for the GUI in gui/test -directory. Each test is in own subdirectory
and builds own binary. Test is run by simple running that binary. The binary also
has several options to select tests etc. You can get the help by running
"binaryname -help" -command.

Translations
------------
The GUI is translated to several languages. Qt comes with two tools to update
and compile the translations. lupdate updates translations files from the code
and lrelease compiles translation files use with the executable.

The translations are automatically updated and compiled within CMake.