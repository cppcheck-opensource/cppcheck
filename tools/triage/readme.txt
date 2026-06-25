triage tool
This tool lets you comfortably look at Cppcheck analysis results for daca packages. It automatically
downloads the package, extracts it and jumps to the corresponding source code for a Cppcheck
message.

triage uses "wget" and "tar"
On Linux the tool can be directly run since the programs should be installed.
On Windows something like Cygwin is necessary and the directory containing the executables must be
in the PATH environment variable (for example "C:\cygwin\bin").

Usage:
After triage has been started you have to load daca results from a file via the "Load from file"
button or from the clipboard via the "Load from clipboard" button.
The file or clipboard text must contain the package URL line beginning with "ftp://" and the
Cppcheck messages.
When the results data has been parsed successfully you can see a list of Cppcheck messages directly
beneath the "Load ..." buttons. Double-click any entry to let the tool show the source code and jump
to and mark the corresponding line. If the package is not found it is downloaded and extracted
automatically. So after the first double-click it is normal that it takes some time until the
source code is shown.
