<?xml version="1.0" encoding="utf-8"?>
<!DOCTYPE TS>
<TS version="2.1" language="de_DE">
<context>
    <name>About</name>
    <message>
        <location filename="about.ui" line="14"/>
        <source>About Cppcheck</source>
        <translation>Über Cppcheck</translation>
    </message>
    <message>
        <location filename="about.ui" line="64"/>
        <source>Version %1</source>
        <translation>Version %1</translation>
    </message>
    <message>
        <location filename="about.ui" line="71"/>
        <source>Cppcheck - A tool for static C/C++ code analysis.</source>
        <translation>Cppcheck - Ein Werkzeug zur statischen C/C++-Code-Analyse.</translation>
    </message>
    <message>
        <location filename="about.ui" line="81"/>
        <source>Copyright © 2007-%1 Cppcheck team.</source>
        <oldsource>Copyright © 2007-2021 Cppcheck team.</oldsource>
        <translation>Copyright © 2007-%1 Cppcheck-Team.</translation>
    </message>
    <message>
        <location filename="about.ui" line="91"/>
        <source>This program is licensed under the terms
of the GNU General Public License version 3</source>
        <translation>Dieses Programm ist unter den Bedingungen
der GNU General Public License Version 3 lizenziert</translation>
    </message>
    <message>
        <location filename="about.ui" line="102"/>
        <source>Visit Cppcheck homepage at %1</source>
        <translation>Besuchen Sie die Cppcheck-Homepage unter %1</translation>
    </message>
    <message>
        <location filename="about.ui" line="115"/>
        <source>&lt;html&gt;&lt;head/&gt;&lt;body&gt;&lt;p&gt;Many thanks to these libraries that we use:&lt;/p&gt;&lt;ul style=&quot;margin-top: 0px; margin-bottom: 0px; margin-left: 0px; margin-right: 0px; -qt-list-indent: 1;&quot;&gt;&lt;li style=&quot; margin-top:12px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;&quot;&gt;PCRE&lt;/li&gt;&lt;li style=&quot; margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;&quot;&gt;PicoJSON&lt;/li&gt;&lt;li style=&quot; margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;&quot;&gt;Qt&lt;/li&gt;&lt;li style=&quot; margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;&quot;&gt;TinyXML2&lt;/li&gt;&lt;li style=&quot; margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;&quot;&gt;Boost&lt;/li&gt;&lt;/ul&gt;&lt;/body&gt;&lt;/html&gt;</source>
        <oldsource>&lt;html&gt;&lt;head/&gt;&lt;body&gt;&lt;p&gt;Many thanks to these libraries that we use:&lt;/p&gt;&lt;ul style=&quot;margin-top: 0px; margin-bottom: 0px; margin-left: 0px; margin-right: 0px; -qt-list-indent: 1;&quot;&gt;&lt;li style=&quot; margin-top:12px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;&quot;&gt;pcre&lt;/li&gt;&lt;li style=&quot; margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;&quot;&gt;picojson&lt;/li&gt;&lt;li style=&quot; margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;&quot;&gt;qt&lt;/li&gt;&lt;li style=&quot; margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;&quot;&gt;tinyxml2&lt;/li&gt;&lt;/ul&gt;&lt;/body&gt;&lt;/html&gt;</oldsource>
        <translation>&lt;html&gt;&lt;head/&gt;&lt;body&gt;&lt;p&gt;Vielen Dank den Bibliotheken die wir nutzen:&lt;/p&gt;&lt;ul style=&quot;margin-top: 0px; margin-bottom: 0px; margin-left: 0px; margin-right: 0px; -qt-list-indent: 1;&quot;&gt;&lt;li style=&quot; margin-top:12px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;&quot;&gt;PCRE&lt;/li&gt;&lt;li style=&quot; margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;&quot;&gt;PicoJSON&lt;/li&gt;&lt;li style=&quot; margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;&quot;&gt;Qt&lt;/li&gt;&lt;li style=&quot; margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;&quot;&gt;TinyXML2&lt;/li&gt;&lt;li style=&quot; margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;&quot;&gt;Boost&lt;/li&gt;&lt;/ul&gt;&lt;/body&gt;&lt;/html&gt;</translation>
    </message>
</context>
<context>
    <name>ApplicationDialog</name>
    <message>
        <location filename="applicationdialog.ui" line="23"/>
        <source>Add an application</source>
        <translation>Anwendung hinzufügen</translation>
    </message>
    <message>
        <location filename="applicationdialog.ui" line="41"/>
        <source>Here you can add an application that can open error files. Specify a name for the application, the application executable and command line parameters for the application.

The following texts in parameters are replaced with appropriate values when application is executed:
(file) - Filename containing the error
(line) - Line number containing the error
(message) - Error message
(severity) - Error severity

Example opening a file with Kate and make Kate scroll to the correct line:
Executable: kate
Parameters: -l(line) (file)</source>
        <translation>Hier können Sie Anwendungen hinzufügen, die Codedateien öffnen können. Geben Sie den Namen der Anwendung, deren ausführbare Datei und Kommandozeilenparameter für die Ausführung, an.

Die folgenden Texte in Parametern werden durch die passenden Werte ersetzt, wenn die Anwendung ausgeführt wird:
(file) - Name der Datei, die den Fehler enthält
(line) - Zeile, die den Fehler enthält
(message) - Fehlermeldung
(severity) - Schweregrad des Fehlers

Beispiel: Öffnen einer Datei mit Kate, automatisch zur korrekten Zeile scrollen:
Ausführbare Datei: kate
Parameter: -l(line) (file)
</translation>
    </message>
    <message>
        <location filename="applicationdialog.ui" line="76"/>
        <source>&amp;Name:</source>
        <translation>&amp;Name:</translation>
    </message>
    <message>
        <location filename="applicationdialog.ui" line="86"/>
        <source>&amp;Executable:</source>
        <translation>&amp;Ausführbare Datei:</translation>
    </message>
    <message>
        <location filename="applicationdialog.ui" line="96"/>
        <source>&amp;Parameters:</source>
        <translation>&amp;Parameter:</translation>
    </message>
    <message>
        <location filename="applicationdialog.ui" line="138"/>
        <source>Browse</source>
        <translation>Suchen</translation>
    </message>
    <message>
        <location filename="applicationdialog.cpp" line="65"/>
        <source>Executable files (*.exe);;All files(*.*)</source>
        <translation>Ausführbare Dateien (*.exe);;Alle Dateien(*.*)</translation>
    </message>
    <message>
        <location filename="applicationdialog.cpp" line="68"/>
        <source>Select viewer application</source>
        <translation>Anzeigeanwendung auswählen</translation>
    </message>
    <message>
        <location filename="applicationdialog.cpp" line="83"/>
        <source>Cppcheck</source>
        <translation>Cppcheck</translation>
    </message>
    <message>
        <location filename="applicationdialog.cpp" line="84"/>
        <source>You must specify a name, a path and optionally parameters for the application!</source>
        <translation>Sie müssen einen Namen, einen Pfad und ggf. Parameter für die Anwendung angeben!</translation>
    </message>
</context>
<context>
    <name>ComplianceReportDialog</name>
    <message>
        <location filename="compliancereportdialog.ui" line="14"/>
        <source>Compliance Report</source>
        <translation>Compliance-Report</translation>
    </message>
    <message>
        <location filename="compliancereportdialog.ui" line="29"/>
        <source>Project name</source>
        <translation>Projektname</translation>
    </message>
    <message>
        <location filename="compliancereportdialog.ui" line="22"/>
        <source>Project version</source>
        <translation>Projekt-Version</translation>
    </message>
    <message>
        <location filename="compliancereportdialog.ui" line="42"/>
        <source>Coding Standard</source>
        <translation>Programmierstandards</translation>
    </message>
    <message>
        <location filename="compliancereportdialog.ui" line="50"/>
        <source>Misra C</source>
        <translation>Misra C</translation>
    </message>
    <message>
        <location filename="compliancereportdialog.ui" line="55"/>
        <source>Cert C</source>
        <translation>Cert C</translation>
    </message>
    <message>
        <location filename="compliancereportdialog.ui" line="60"/>
        <source>Cert C++</source>
        <translation>Cert C++</translation>
    </message>
    <message>
        <location filename="compliancereportdialog.ui" line="70"/>
        <source>List of files with md5 checksums</source>
        <translation>Datei-Listen mit MD5-Prüfsummen</translation>
    </message>
    <message>
        <location filename="compliancereportdialog.cpp" line="133"/>
        <source>Compliance report</source>
        <translation>Compliance-Report</translation>
    </message>
    <message>
        <location filename="compliancereportdialog.cpp" line="135"/>
        <source>HTML files (*.html)</source>
        <translation>HTML-Dateien (*.html)</translation>
    </message>
    <message>
        <location filename="compliancereportdialog.cpp" line="176"/>
        <location filename="compliancereportdialog.cpp" line="239"/>
        <source>Save compliance report</source>
        <translation>Compliance-Report speichern</translation>
    </message>
    <message>
        <location filename="compliancereportdialog.cpp" line="177"/>
        <source>Failed to import &apos;%1&apos; (%2), can not show files in compliance report</source>
        <translation>Import von &apos;%1&apos; (%2) fehlgeschlagen, Compliance-Report kann nicht angezeigt werden</translation>
    </message>
</context>
<context>
    <name>FileViewDialog</name>
    <message>
        <location filename="fileviewdialog.cpp" line="53"/>
        <source>Could not find the file: %1</source>
        <translation>Konnte die Datei nicht finden: %1</translation>
    </message>
    <message>
        <location filename="fileviewdialog.cpp" line="57"/>
        <location filename="fileviewdialog.cpp" line="71"/>
        <source>Cppcheck</source>
        <translation>Cppcheck</translation>
    </message>
    <message>
        <location filename="fileviewdialog.cpp" line="67"/>
        <source>Could not read the file: %1</source>
        <translation>Konnte die Datei nicht lesen: %1</translation>
    </message>
</context>
<context>
    <name>HelpDialog</name>
    <message>
        <location filename="helpdialog.ui" line="14"/>
        <source>Cppcheck GUI help</source>
        <translation>Cppcheck GUI Hilfe</translation>
    </message>
    <message>
        <location filename="helpdialog.ui" line="34"/>
        <source>Contents</source>
        <translation>Inhalt</translation>
    </message>
    <message>
        <location filename="helpdialog.ui" line="44"/>
        <source>Index</source>
        <translation>Index</translation>
    </message>
    <message>
        <location filename="helpdialog.cpp" line="84"/>
        <source>Helpfile &apos;%1&apos; was not found</source>
        <translation>Hilfedatei &apos;%1&apos; nicht gefunden</translation>
    </message>
    <message>
        <location filename="helpdialog.cpp" line="86"/>
        <source>Cppcheck</source>
        <translation>Cppcheck</translation>
    </message>
</context>
<context>
    <name>LibraryAddFunctionDialog</name>
    <message>
        <location filename="libraryaddfunctiondialog.ui" line="23"/>
        <source>Add function</source>
        <translation>Funktion hinzufügen</translation>
    </message>
    <message>
        <location filename="libraryaddfunctiondialog.ui" line="34"/>
        <source>Function name(s)</source>
        <translation>Funktionsname(n)</translation>
    </message>
    <message>
        <location filename="libraryaddfunctiondialog.ui" line="44"/>
        <source>Number of arguments</source>
        <translation>Anzahl Argumente</translation>
    </message>
</context>
<context>
    <name>LibraryDialog</name>
    <message>
        <location filename="librarydialog.ui" line="14"/>
        <source>Library Editor</source>
        <translation>Bibliothekseditor</translation>
    </message>
    <message>
        <location filename="librarydialog.ui" line="22"/>
        <source>Open</source>
        <translation>Öffnen</translation>
    </message>
    <message>
        <location filename="librarydialog.ui" line="29"/>
        <source>Save</source>
        <translation>Speichern</translation>
    </message>
    <message>
        <location filename="librarydialog.ui" line="36"/>
        <source>Save as</source>
        <translation>Speichern unter</translation>
    </message>
    <message>
        <location filename="librarydialog.ui" line="62"/>
        <source>Functions</source>
        <translation>Funktionen</translation>
    </message>
    <message>
        <location filename="librarydialog.ui" line="71"/>
        <source>Sort</source>
        <translation>Sortiere</translation>
    </message>
    <message>
        <location filename="librarydialog.ui" line="111"/>
        <source>Add</source>
        <translation>Hinzufügen</translation>
    </message>
    <message>
        <location filename="librarydialog.ui" line="131"/>
        <source>Filter:</source>
        <translation>Filter:</translation>
    </message>
    <message>
        <location filename="librarydialog.ui" line="164"/>
        <source>Comments</source>
        <translation>Kommentare</translation>
    </message>
    <message>
        <location filename="librarydialog.ui" line="204"/>
        <source>noreturn</source>
        <translation>Zurückkehrend</translation>
    </message>
    <message>
        <location filename="librarydialog.ui" line="212"/>
        <source>False</source>
        <translation>Ja</translation>
    </message>
    <message>
        <location filename="librarydialog.ui" line="217"/>
        <source>True</source>
        <translation>Nein</translation>
    </message>
    <message>
        <location filename="librarydialog.ui" line="222"/>
        <source>Unknown</source>
        <translation>Unbekannt</translation>
    </message>
    <message>
        <location filename="librarydialog.ui" line="232"/>
        <source>return value must be used</source>
        <translation>Rückgabewert muss genutzt werden</translation>
    </message>
    <message>
        <location filename="librarydialog.ui" line="239"/>
        <source>ignore function in leaks checking</source>
        <translation>Ignoriere Funktion in Speicherleck-Prüfung</translation>
    </message>
    <message>
        <location filename="librarydialog.ui" line="246"/>
        <source>Arguments</source>
        <translation>Argumente</translation>
    </message>
    <message>
        <location filename="librarydialog.ui" line="258"/>
        <source>Edit</source>
        <translation>Bearbeiten</translation>
    </message>
    <message>
        <location filename="librarydialog.cpp" line="97"/>
        <location filename="librarydialog.cpp" line="169"/>
        <source>Library files (*.cfg)</source>
        <translation>Bibliotheksdateien (*.cfg)</translation>
    </message>
    <message>
        <location filename="librarydialog.cpp" line="99"/>
        <source>Open library file</source>
        <translation>Bibliothek öffnen</translation>
    </message>
    <message>
        <location filename="librarydialog.cpp" line="110"/>
        <location filename="librarydialog.cpp" line="122"/>
        <location filename="librarydialog.cpp" line="159"/>
        <source>Cppcheck</source>
        <translation>Cppcheck</translation>
    </message>
    <message>
        <location filename="librarydialog.cpp" line="111"/>
        <source>Cannot open file %1.</source>
        <translation>Datei %1 kann nicht geöffnet werden.</translation>
    </message>
    <message>
        <location filename="librarydialog.cpp" line="123"/>
        <source>Failed to load %1. %2.</source>
        <translation>%1 kann nicht geladen werden. %2.</translation>
    </message>
    <message>
        <location filename="librarydialog.cpp" line="160"/>
        <source>Cannot save file %1.</source>
        <translation>Datei %1 kann nicht gespeichert werden.</translation>
    </message>
    <message>
        <location filename="librarydialog.cpp" line="172"/>
        <source>Save the library as</source>
        <translation>Speichere Bibliothek unter</translation>
    </message>
</context>
<context>
    <name>LibraryEditArgDialog</name>
    <message>
        <location filename="libraryeditargdialog.ui" line="14"/>
        <source>Edit argument</source>
        <translation>Argument bearbeiten</translation>
    </message>
    <message>
        <location filename="libraryeditargdialog.ui" line="20"/>
        <source>&lt;html&gt;&lt;head/&gt;&lt;body&gt;
&lt;p&gt;Is bool value allowed? For instance result from comparison or from &apos;!&apos; operator.&lt;/p&gt;
&lt;p&gt;Typically, set this if the argument is a pointer, size, etc.&lt;/p&gt;
&lt;p&gt;Example:&lt;/p&gt;
&lt;pre&gt;    memcmp(x, y, i == 123);   // last argument should not have a bool value&lt;/pre&gt;
&lt;/body&gt;&lt;/html&gt;</source>
        <translation>&lt;html&gt;&lt;head/&gt;&lt;body&gt;
&lt;p&gt;Ist ein boolscher Wert, beispielsweise das Ergebnis eines Vergleichsoperators, oder von &apos;!&apos; zulässig?&lt;/p&gt;
&lt;p&gt;Diese Option wird typischerweise gesetzt, wenn das Argument ein Zeiger, eine Größe, etc. ist.&lt;/p&gt;
&lt;p&gt;Beispiel:&lt;/p&gt;
&lt;pre&gt;    memcmp(x, y, i == 123);   // Das letzte Argument sollte kein boolscher Wert sein.&lt;/pre&gt;
&lt;/body&gt;&lt;/html&gt;</translation>
    </message>
    <message>
        <location filename="libraryeditargdialog.ui" line="28"/>
        <source>Not bool</source>
        <translation>Nicht boolsch</translation>
    </message>
    <message>
        <location filename="libraryeditargdialog.ui" line="35"/>
        <source>&lt;html&gt;&lt;head/&gt;&lt;body&gt;
&lt;p&gt;Is a null parameter value allowed?&lt;/p&gt;
&lt;p&gt;Typically this should be used on any pointer parameter that does not allow null.&lt;/p&gt;
&lt;p&gt;Example:&lt;/p&gt;
&lt;pre&gt;    strcpy(x,y); // neither x or y is allowed to be null.&lt;/pre&gt;
&lt;/body&gt;&lt;/html&gt;</source>
        <translation>&lt;html&gt;&lt;head/&gt;&lt;body&gt;
&lt;p&gt;Ist die Übergabe von Null zulässig?&lt;/p&gt;
&lt;p&gt;Dies wird typischerweise für Funktionen mit Zeigern als Parameter genutzt, die nicht Null sein dürfen.&lt;/p&gt;
&lt;p&gt;Beispiel:&lt;/p&gt;
&lt;pre&gt;    strcpy(x,y); // Weder x noch y dürfen ein Nullzeiger sein.&lt;/pre&gt;
&lt;/body&gt;&lt;/html&gt;</translation>
    </message>
    <message>
        <location filename="libraryeditargdialog.ui" line="43"/>
        <source>Not null</source>
        <translation>Nicht Null</translation>
    </message>
    <message>
        <location filename="libraryeditargdialog.ui" line="50"/>
        <source>Not uninit</source>
        <translation>Nicht uninitialisiert</translation>
    </message>
    <message>
        <location filename="libraryeditargdialog.ui" line="57"/>
        <source>String</source>
        <translation>String</translation>
    </message>
    <message>
        <location filename="libraryeditargdialog.ui" line="70"/>
        <source>Format string</source>
        <translation>Formatstring</translation>
    </message>
    <message>
        <location filename="libraryeditargdialog.ui" line="92"/>
        <source>Min size of buffer</source>
        <translation>Minimale Puffergröße</translation>
    </message>
    <message>
        <location filename="libraryeditargdialog.ui" line="101"/>
        <location filename="libraryeditargdialog.ui" line="203"/>
        <source>Type</source>
        <translation>Typ</translation>
    </message>
    <message>
        <location filename="libraryeditargdialog.ui" line="109"/>
        <location filename="libraryeditargdialog.ui" line="214"/>
        <source>None</source>
        <translation>Keine</translation>
    </message>
    <message>
        <location filename="libraryeditargdialog.ui" line="114"/>
        <location filename="libraryeditargdialog.ui" line="219"/>
        <source>argvalue</source>
        <translation>Argumentwert</translation>
    </message>
    <message>
        <location filename="libraryeditargdialog.ui" line="119"/>
        <location filename="libraryeditargdialog.ui" line="224"/>
        <source>mul</source>
        <translation>Multiplikation</translation>
    </message>
    <message>
        <location filename="libraryeditargdialog.ui" line="124"/>
        <location filename="libraryeditargdialog.ui" line="229"/>
        <source>strlen</source>
        <translation>strlen</translation>
    </message>
    <message>
        <location filename="libraryeditargdialog.ui" line="132"/>
        <location filename="libraryeditargdialog.ui" line="237"/>
        <source>Arg</source>
        <translation>Argument 1</translation>
    </message>
    <message>
        <location filename="libraryeditargdialog.ui" line="159"/>
        <location filename="libraryeditargdialog.ui" line="264"/>
        <source>Arg2</source>
        <translation>Argument 2</translation>
    </message>
    <message>
        <location filename="libraryeditargdialog.ui" line="194"/>
        <source>and</source>
        <translation>und</translation>
    </message>
    <message>
        <location filename="libraryeditargdialog.ui" line="310"/>
        <source>Valid values</source>
        <translation>Zulässige Werte</translation>
    </message>
</context>
<context>
    <name>MainWindow</name>
    <message>
        <location filename="mainwindow.ui" line="26"/>
        <location filename="mainwindow.ui" line="688"/>
        <location filename="mainwindow.cpp" line="466"/>
        <location filename="mainwindow.cpp" line="670"/>
        <location filename="mainwindow.cpp" line="755"/>
        <location filename="mainwindow.cpp" line="860"/>
        <location filename="mainwindow.cpp" line="882"/>
        <location filename="mainwindow.cpp" line="1467"/>
        <location filename="mainwindow.cpp" line="1596"/>
        <location filename="mainwindow.cpp" line="1937"/>
        <location filename="mainwindow.cpp" line="1945"/>
        <location filename="mainwindow.cpp" line="1999"/>
        <location filename="mainwindow.cpp" line="2008"/>
        <location filename="mainwindow.cpp" line="2080"/>
        <location filename="mainwindow.cpp" line="2168"/>
        <source>Cppcheck</source>
        <translation>Cppcheck</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="267"/>
        <source>Standard</source>
        <translation>Standard</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="132"/>
        <source>&amp;File</source>
        <translation>&amp;Datei</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="152"/>
        <source>&amp;View</source>
        <translation>&amp;Ansicht</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="156"/>
        <source>&amp;Toolbars</source>
        <translation>&amp;Symbolleisten</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="210"/>
        <source>A&amp;nalyze</source>
        <translation>A&amp;nalysieren</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="214"/>
        <source>C++ standard</source>
        <translation>C++-Standard</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="226"/>
        <source>&amp;C standard</source>
        <translation>&amp;C-Standard</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="254"/>
        <source>&amp;Edit</source>
        <translation>&amp;Bearbeiten</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="315"/>
        <source>&amp;License...</source>
        <translation>&amp;Lizenz...</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="320"/>
        <source>A&amp;uthors...</source>
        <translation>&amp;Autoren...</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="329"/>
        <source>&amp;About...</source>
        <translation>Ü&amp;ber...</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="334"/>
        <source>&amp;Files...</source>
        <translation>&amp;Dateien...</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="337"/>
        <location filename="mainwindow.ui" line="340"/>
        <source>Analyze files</source>
        <translation>Analysiere Dateien</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="343"/>
        <source>Ctrl+F</source>
        <translation>Strg+F</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="352"/>
        <source>&amp;Directory...</source>
        <translation>&amp;Verzeichnis...</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="355"/>
        <location filename="mainwindow.ui" line="358"/>
        <source>Analyze directory</source>
        <translation>Analysiere Verzeichnis</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="361"/>
        <source>Ctrl+D</source>
        <translation>Strg+D</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="373"/>
        <source>Ctrl+R</source>
        <translation>Strg+R</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="391"/>
        <source>&amp;Stop</source>
        <translation>&amp;Stoppen</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="394"/>
        <location filename="mainwindow.ui" line="397"/>
        <source>Stop analysis</source>
        <translation>Analyse abbrechen</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="400"/>
        <source>Esc</source>
        <translation>Esc</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="409"/>
        <source>&amp;Save results to file...</source>
        <translation>&amp;Ergebnisse in Datei speichern...</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="412"/>
        <source>Ctrl+S</source>
        <translation>Strg+S</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="417"/>
        <source>&amp;Quit</source>
        <translation>&amp;Beenden</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="429"/>
        <source>&amp;Clear results</source>
        <translation>Ergebnisse &amp;löschen</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="438"/>
        <source>&amp;Preferences</source>
        <translation>&amp;Einstellungen</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="471"/>
        <location filename="mainwindow.ui" line="474"/>
        <location filename="mainwindow.cpp" line="2394"/>
        <source>Show errors</source>
        <translation>Zeige Fehler</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="617"/>
        <location filename="mainwindow.ui" line="620"/>
        <location filename="mainwindow.cpp" line="2395"/>
        <source>Show warnings</source>
        <translation>Zeige Warnungen</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="635"/>
        <location filename="mainwindow.ui" line="638"/>
        <source>Show performance warnings</source>
        <translation>Zeige Performance-Warnungen</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="646"/>
        <source>Show &amp;hidden</source>
        <translation>Zeige &amp;versteckte</translation>
    </message>
    <message>
        <location filename="mainwindow.cpp" line="986"/>
        <location filename="mainwindow.cpp" line="1024"/>
        <source>Information</source>
        <translation>Information</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="661"/>
        <source>Show information messages</source>
        <translation>Zeige Informationsmeldungen</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="676"/>
        <source>Show portability warnings</source>
        <translation>Zeige Portabilitätswarnungen</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="691"/>
        <source>Show Cppcheck results</source>
        <translation>Zeige Cppcheck-Ergebnisse</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="703"/>
        <source>Clang</source>
        <translation>Clang</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="706"/>
        <source>Show Clang results</source>
        <translation>Zeige Clang-Ergebnisse</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="714"/>
        <source>&amp;Filter</source>
        <translation>&amp;Filter</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="717"/>
        <source>Filter results</source>
        <translation>Gefilterte Ergebnisse</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="733"/>
        <source>Windows 32-bit ANSI</source>
        <translation>Windows 32-bit, ANSI</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="741"/>
        <source>Windows 32-bit Unicode</source>
        <translation>Windows 32-bit, Unicode</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="749"/>
        <source>Unix 32-bit</source>
        <translation>Unix 32-bit</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="757"/>
        <source>Unix 64-bit</source>
        <translation>Unix 64-bit</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="765"/>
        <source>Windows 64-bit</source>
        <translation>Windows 64-bit</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="859"/>
        <source>&amp;Print...</source>
        <translation>Drucken...</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="862"/>
        <source>Print the Current Report</source>
        <translation>Aktuellen Bericht ausdrucken</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="867"/>
        <source>Print Pre&amp;view...</source>
        <translation>Druckvorschau</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="870"/>
        <source>Open a Print Preview Dialog for the Current Results</source>
        <translation>Druckvorschaudialog für aktuelle Ergebnisse öffnen</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="878"/>
        <source>Open library editor</source>
        <translation>Bibliothekseditor öffnen</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="479"/>
        <source>&amp;Check all</source>
        <translation>Alle &amp;auswählen</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="84"/>
        <source>Checking for updates</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="100"/>
        <source>Hide</source>
        <translation type="unfinished">Verstecken</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="164"/>
        <location filename="mainwindow.ui" line="980"/>
        <source>Report</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="304"/>
        <source>Filter</source>
        <translation>Filter</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="370"/>
        <source>&amp;Reanalyze modified files</source>
        <translation>Veränderte Dateien neu analysieren</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="382"/>
        <source>Reanal&amp;yze all files</source>
        <translation>Alle Dateien erneut anal&amp;ysieren</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="420"/>
        <source>Ctrl+Q</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="450"/>
        <source>Style war&amp;nings</source>
        <translation>Stilwar&amp;nungen</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="468"/>
        <source>E&amp;rrors</source>
        <translation>F&amp;ehler</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="484"/>
        <source>&amp;Uncheck all</source>
        <translation>Alle a&amp;bwählen</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="489"/>
        <source>Collapse &amp;all</source>
        <translation>Alle &amp;reduzieren</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="494"/>
        <source>&amp;Expand all</source>
        <translation>Alle &amp;erweitern</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="502"/>
        <source>&amp;Standard</source>
        <translation>&amp;Standard</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="505"/>
        <source>Standard items</source>
        <translation>Standardeinträge</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="521"/>
        <source>Toolbar</source>
        <translation>Symbolleiste</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="529"/>
        <source>&amp;Categories</source>
        <translation>&amp;Kategorien</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="532"/>
        <source>Error categories</source>
        <translation>Fehler-Kategorien</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="537"/>
        <source>&amp;Open XML...</source>
        <translation>Öffne &amp;XML...</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="546"/>
        <source>Open P&amp;roject File...</source>
        <translation>Pr&amp;ojektdatei öffnen...</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="549"/>
        <source>Ctrl+Shift+O</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="558"/>
        <source>Sh&amp;ow Scratchpad...</source>
        <translation>&amp;Zeige Schmierzettel...</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="563"/>
        <source>&amp;New Project File...</source>
        <translation>&amp;Neue Projektdatei...</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="566"/>
        <source>Ctrl+Shift+N</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="571"/>
        <source>&amp;Log View</source>
        <translation>&amp;Loganzeige</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="574"/>
        <source>Log View</source>
        <translation>Loganzeige</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="582"/>
        <source>C&amp;lose Project File</source>
        <translation>Projektdatei &amp;schließen</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="590"/>
        <source>&amp;Edit Project File...</source>
        <translation>Projektdatei &amp;bearbeiten...</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="602"/>
        <source>&amp;Statistics</source>
        <translation>&amp;Statistik</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="614"/>
        <source>&amp;Warnings</source>
        <translation>&amp;Warnungen</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="632"/>
        <source>Per&amp;formance warnings</source>
        <translation>Per&amp;formance-Warnungen</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="658"/>
        <source>&amp;Information</source>
        <translation>&amp;Information</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="673"/>
        <source>&amp;Portability</source>
        <translation>&amp;Portabilität</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="773"/>
        <source>P&amp;latforms</source>
        <translation>P&amp;lattformen</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="787"/>
        <source>C++&amp;11</source>
        <translation>C++&amp;11</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="798"/>
        <source>C&amp;99</source>
        <translation>C&amp;99</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="806"/>
        <source>&amp;Posix</source>
        <translation>Posix</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="814"/>
        <source>C&amp;11</source>
        <translation>C&amp;11</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="846"/>
        <source>&amp;C89</source>
        <translation>&amp;C89</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="854"/>
        <source>&amp;C++03</source>
        <translation>&amp;C++03</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="875"/>
        <source>&amp;Library Editor...</source>
        <translation>&amp;Bibliothekseditor</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="886"/>
        <source>&amp;Auto-detect language</source>
        <translation>Sprache &amp;automatisch erkennen</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="894"/>
        <source>&amp;Enforce C++</source>
        <translation>C++ &amp;erzwingen</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="902"/>
        <source>E&amp;nforce C</source>
        <translation>C e&amp;rzwingen</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="913"/>
        <source>C++14</source>
        <translation>C++14</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="921"/>
        <source>Reanalyze and check library</source>
        <translation>Neu analysieren und Bibliothek prüfen</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="929"/>
        <source>Check configuration (defines, includes)</source>
        <translation>Prüfe Konfiguration (Definitionen, Includes)</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="937"/>
        <source>C++17</source>
        <translation>C++17</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="948"/>
        <source>C++20</source>
        <translation>C++20</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="975"/>
        <source>Compliance report...</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="988"/>
        <source>Normal</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="996"/>
        <source>Misra C</source>
        <translation>Misra C</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="1004"/>
        <source>Misra C++ 2008</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="1012"/>
        <source>Cert C</source>
        <translation>Cert C</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="1020"/>
        <source>Cert C++</source>
        <translation>Cert C++</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="1028"/>
        <source>Misra C++ 2023</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="1036"/>
        <source>Autosar</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="1041"/>
        <source>EULA...</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="1046"/>
        <source>Thread Details</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="1049"/>
        <source>Show thread details</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="510"/>
        <source>&amp;Contents</source>
        <translation>&amp;Inhalte</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="285"/>
        <source>Categories</source>
        <translation>Kategorien</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="453"/>
        <location filename="mainwindow.ui" line="456"/>
        <source>Show style warnings</source>
        <translation>Zeige Stilwarnungen</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="513"/>
        <source>Open the help contents</source>
        <translation>Öffnet die Hilfe-Inhalte</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="516"/>
        <source>F1</source>
        <translation>F1</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="199"/>
        <source>&amp;Help</source>
        <translation>&amp;Hilfe</translation>
    </message>
    <message>
        <location filename="mainwindow.cpp" line="157"/>
        <location filename="mainwindow.cpp" line="1782"/>
        <source>Quick Filter:</source>
        <translation>Schnellfilter:</translation>
    </message>
    <message>
        <location filename="mainwindow.cpp" line="831"/>
        <source>Select configuration</source>
        <translation>Konfiguration wählen</translation>
    </message>
    <message>
        <location filename="mainwindow.cpp" line="861"/>
        <source>Found project file: %1

Do you want to load this project file instead?</source>
        <translation>Gefundene Projektdatei: %1

Möchten Sie stattdessen diese öffnen?</translation>
    </message>
    <message>
        <location filename="mainwindow.cpp" line="995"/>
        <source>File not found</source>
        <translation>Datei nicht gefunden</translation>
    </message>
    <message>
        <location filename="mainwindow.cpp" line="998"/>
        <source>Bad XML</source>
        <translation>Fehlerhaftes XML</translation>
    </message>
    <message>
        <location filename="mainwindow.cpp" line="1001"/>
        <source>Missing attribute</source>
        <translation>Fehlendes Attribut</translation>
    </message>
    <message>
        <location filename="mainwindow.cpp" line="1004"/>
        <source>Bad attribute value</source>
        <translation>Falscher Attributwert</translation>
    </message>
    <message>
        <location filename="mainwindow.cpp" line="1010"/>
        <source>Duplicate platform type</source>
        <translation>Plattformtyp doppelt</translation>
    </message>
    <message>
        <location filename="mainwindow.cpp" line="1013"/>
        <source>Platform type redefined</source>
        <translation>Plattformtyp neu definiert</translation>
    </message>
    <message>
        <location filename="mainwindow.cpp" line="1016"/>
        <source>Duplicate define</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="mainwindow.cpp" line="1024"/>
        <source>Failed to load the selected library &apos;%1&apos;.
%2</source>
        <translation>Laden der ausgewählten Bibliothek &apos;%1&apos; schlug fehl.
%2</translation>
    </message>
    <message>
        <location filename="mainwindow.cpp" line="1035"/>
        <source>File not found: &apos;%1&apos;</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="mainwindow.cpp" line="1058"/>
        <source>Failed to load/setup addon %1: %2</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="mainwindow.cpp" line="1081"/>
        <source>Failed to load %1. Your Cppcheck installation is broken. You can use --data-dir=&lt;directory&gt; at the command line to specify where this file is located. Please note that --data-dir is supposed to be used by installation scripts and therefore the GUI does not start when it is used, all that happens is that the setting is configured.

Analysis is aborted.</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="mainwindow.cpp" line="1091"/>
        <source>Failed to load %1 - %2

Analysis is aborted.</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="mainwindow.cpp" line="1103"/>
        <location filename="mainwindow.cpp" line="1198"/>
        <source>%1

Analysis is aborted.</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="mainwindow.cpp" line="1648"/>
        <source>License</source>
        <translation>Lizenz</translation>
    </message>
    <message>
        <location filename="mainwindow.cpp" line="1655"/>
        <source>Authors</source>
        <translation>Autoren</translation>
    </message>
    <message>
        <location filename="mainwindow.cpp" line="1675"/>
        <source>Save the report file</source>
        <translation>Speichert die Berichtdatei</translation>
    </message>
    <message>
        <location filename="mainwindow.cpp" line="1483"/>
        <location filename="mainwindow.cpp" line="1682"/>
        <source>XML files (*.xml)</source>
        <translation>XML-Dateien (*.xml)</translation>
    </message>
    <message>
        <location filename="mainwindow.cpp" line="461"/>
        <source>There was a problem with loading the editor application settings.

This is probably because the settings were changed between the Cppcheck versions. Please check (and fix) the editor application settings, otherwise the editor program might not start correctly.</source>
        <translation>Beim Laden der Editor-Anwendungseinstellungen trat ein Problem auf.

Dies wurde vermutlich durch einen Wechsel der Cppcheck-Version hervorgerufen. Bitte prüfen (und korrigieren) Sie die Einstellungen, andernfalls könnte die Editor-Anwendung nicht korrekt starten.</translation>
    </message>
    <message>
        <location filename="mainwindow.cpp" line="756"/>
        <source>You must close the project file before selecting new files or directories!</source>
        <translation>Sie müssen die Projektdatei schließen, bevor Sie neue Dateien oder Verzeichnisse auswählen!</translation>
    </message>
    <message>
        <location filename="mainwindow.cpp" line="986"/>
        <source>The library &apos;%1&apos; contains unknown elements:
%2</source>
        <translation>Die Bibliothek &apos;%1&apos; enthält unbekannte Elemente:
%2</translation>
    </message>
    <message>
        <location filename="mainwindow.cpp" line="1007"/>
        <source>Unsupported format</source>
        <translation>Nicht unterstütztes Format</translation>
    </message>
    <message>
        <location filename="mainwindow.cpp" line="1019"/>
        <source>Unknown element</source>
        <translation>Unbekanntes Element</translation>
    </message>
    <message>
        <location filename="mainwindow.cpp" line="1019"/>
        <source>Unknown element</source>
        <oldsource>Unknown issue</oldsource>
        <translation type="unfinished">Unbekannter Fehler</translation>
    </message>
    <message>
        <location filename="mainwindow.cpp" line="1081"/>
        <location filename="mainwindow.cpp" line="1091"/>
        <location filename="mainwindow.cpp" line="1103"/>
        <location filename="mainwindow.cpp" line="1198"/>
        <source>Error</source>
        <translation>Fehler</translation>
    </message>
    <message>
        <source>Failed to load %1. Your Cppcheck installation is broken. You can use --data-dir=&lt;directory&gt; at the command line to specify where this file is located. Please note that --data-dir is supposed to be used by installation scripts and therefore the GUI does not start when it is used, all that happens is that the setting is configured.</source>
        <translation type="vanished">Laden von %1 fehlgeschlagen. Ihre Cppcheck-Installation ist defekt. Sie können --data-dir=&lt;Verzeichnis&gt; als Kommandozeilenparameter verwenden, um anzugeben, wo die Datei sich befindet. Bitte beachten Sie, dass --data-dir in Installationsroutinen genutzt werden soll, und die GUI bei dessen Nutzung nicht startet, sondern die Einstellungen konfiguriert.</translation>
    </message>
    <message>
        <location filename="mainwindow.cpp" line="1485"/>
        <source>Open the report file</source>
        <translation>Berichtdatei öffnen</translation>
    </message>
    <message>
        <location filename="mainwindow.cpp" line="1686"/>
        <source>Text files (*.txt)</source>
        <translation>Textdateien (*.txt)</translation>
    </message>
    <message>
        <location filename="mainwindow.cpp" line="1690"/>
        <source>CSV files (*.csv)</source>
        <translation>CSV-Dateien (*.csv)</translation>
    </message>
    <message>
        <location filename="mainwindow.cpp" line="1820"/>
        <source>Project files (*.cppcheck);;All files(*.*)</source>
        <translation>Projektdateien (*.cppcheck);;Alle Dateien(*.*)</translation>
    </message>
    <message>
        <location filename="mainwindow.cpp" line="1822"/>
        <source>Select Project File</source>
        <translation>Projektdatei auswählen</translation>
    </message>
    <message>
        <location filename="mainwindow.cpp" line="242"/>
        <location filename="mainwindow.cpp" line="1784"/>
        <location filename="mainwindow.cpp" line="1850"/>
        <location filename="mainwindow.cpp" line="2048"/>
        <source>Project:</source>
        <translation>Projekt:</translation>
    </message>
    <message>
        <location filename="mainwindow.cpp" line="671"/>
        <source>No suitable files found to analyze!</source>
        <translation>Keine passenden Dateien für Analyse gefunden!</translation>
    </message>
    <message>
        <location filename="mainwindow.cpp" line="770"/>
        <source>C/C++ Source</source>
        <translation>C/C++-Quellcode</translation>
    </message>
    <message>
        <location filename="mainwindow.cpp" line="771"/>
        <source>Compile database</source>
        <translation>Compilerdatenbank</translation>
    </message>
    <message>
        <location filename="mainwindow.cpp" line="772"/>
        <source>Visual Studio</source>
        <translation>Visual Studio</translation>
    </message>
    <message>
        <location filename="mainwindow.cpp" line="773"/>
        <source>Borland C++ Builder 6</source>
        <translation>Borland C++-Builder 6</translation>
    </message>
    <message>
        <location filename="mainwindow.cpp" line="776"/>
        <source>Select files to analyze</source>
        <translation>Dateien für Analyse auswählen</translation>
    </message>
    <message>
        <location filename="mainwindow.cpp" line="791"/>
        <source>Select directory to analyze</source>
        <translation>Verzeichnis für Analyse auswählen</translation>
    </message>
    <message>
        <location filename="mainwindow.cpp" line="831"/>
        <source>Select the configuration that will be analyzed</source>
        <translation>Zu analysierende Konfiguration auswählen</translation>
    </message>
    <message>
        <location filename="mainwindow.cpp" line="883"/>
        <source>Found project files from the directory.

Do you want to proceed analysis without using any of these project files?</source>
        <translation>Projektdateien im Verzeichnis gefunden.
        
Wollen sie fortfahren, ohne diese Projektdateien zu nutzen?</translation>
    </message>
    <message>
        <location filename="mainwindow.cpp" line="1468"/>
        <source>Current results will be cleared.

Opening a new XML file will clear current results.
Do you want to proceed?</source>
        <translation>Aktuelle Ergebnisse werden gelöscht.

Eine neue XML-Datei zu öffnen wird die aktuellen Ergebnisse löschen
Möchten sie fortfahren?</translation>
    </message>
    <message>
        <location filename="mainwindow.cpp" line="1592"/>
        <source>Analyzer is running.

Do you want to stop the analysis and exit Cppcheck?</source>
        <translation>Analyse läuft.
        
Wollen sie die Analyse abbrechen und Cppcheck beenden?</translation>
    </message>
    <message>
        <location filename="mainwindow.cpp" line="1634"/>
        <source>About</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="mainwindow.cpp" line="1673"/>
        <source>XML files (*.xml);;Text files (*.txt);;CSV files (*.csv)</source>
        <translation>XML-Dateien (*.xml);;Textdateien (*.txt);;CSV-Dateien (*.csv)</translation>
    </message>
    <message>
        <location filename="mainwindow.cpp" line="1713"/>
        <source>Cannot generate a compliance report right now, an analysis must finish successfully. Try to reanalyze the code and ensure there are no critical errors.</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="mainwindow.cpp" line="1938"/>
        <source>Build dir &apos;%1&apos; does not exist, create it?</source>
        <translation>Erstellungsverzeichnis &apos;%1&apos; existiert nicht. Erstellen?</translation>
    </message>
    <message>
        <location filename="mainwindow.cpp" line="1946"/>
        <source>To check the project using addons, you need a build directory.</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="mainwindow.cpp" line="1978"/>
        <source>Failed to open file</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="mainwindow.cpp" line="1981"/>
        <source>Unknown project file format</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="mainwindow.cpp" line="1984"/>
        <source>Failed to import project file</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="mainwindow.cpp" line="2000"/>
        <source>Failed to import &apos;%1&apos;: %2

Analysis is stopped.</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="mainwindow.cpp" line="2009"/>
        <source>Failed to import &apos;%1&apos; (%2), analysis is stopped</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="mainwindow.cpp" line="2367"/>
        <source>Show Mandatory</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="mainwindow.cpp" line="2368"/>
        <source>Show Required</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="mainwindow.cpp" line="2369"/>
        <source>Show Advisory</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="mainwindow.cpp" line="2370"/>
        <source>Show Document</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="mainwindow.cpp" line="2387"/>
        <source>Show L1</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="mainwindow.cpp" line="2388"/>
        <source>Show L2</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="mainwindow.cpp" line="2389"/>
        <source>Show L3</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="mainwindow.cpp" line="2396"/>
        <source>Show style</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="mainwindow.cpp" line="2397"/>
        <source>Show portability</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="mainwindow.cpp" line="2398"/>
        <source>Show performance</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="mainwindow.cpp" line="2399"/>
        <source>Show information</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Failed to import &apos;%1&apos;, analysis is stopped</source>
        <translation type="vanished">Import von &apos;%1&apos; fehlgeschlagen; Analyse wurde abgebrochen.</translation>
    </message>
    <message>
        <location filename="mainwindow.cpp" line="2033"/>
        <source>Project files (*.cppcheck)</source>
        <translation>Projektdateien (*.cppcheck)</translation>
    </message>
    <message>
        <location filename="mainwindow.cpp" line="2035"/>
        <source>Select Project Filename</source>
        <translation>Projektnamen auswählen</translation>
    </message>
    <message>
        <location filename="mainwindow.cpp" line="2081"/>
        <source>No project file loaded</source>
        <translation>Keine Projektdatei geladen</translation>
    </message>
    <message>
        <location filename="mainwindow.cpp" line="2163"/>
        <source>The project file

%1

 could not be found!

Do you want to remove the file from the recently used projects -list?</source>
        <translation>Die Projektdatei

%1

 konnte nicht gefunden werden!

Möchten Sie die Datei von der Liste der zuletzt benutzten Projekte entfernen?</translation>
    </message>
    <message>
        <location filename="mainwindow.cpp" line="2325"/>
        <source>Install</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="mainwindow.cpp" line="2329"/>
        <source>New version available: %1. %2</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="main.cpp" line="100"/>
        <source>Cppcheck GUI.

Syntax:
    cppcheck-gui [OPTIONS] [files or paths]

Options:
    -h, --help              Print this help
    -p &lt;file&gt;               Open given project file and start checking it
    -l &lt;file&gt;               Open given results xml file
    -d &lt;directory&gt;          Specify the directory that was checked to generate the results xml specified with -l
    -v, --version           Show program version
    --data-dir=&lt;directory&gt;  This option is for installation scripts so they can configure the directory where
                            datafiles are located (translations, cfg). The GUI is not started when this option
                            is used.</source>
        <translation>
          Cppcheck GUI.

          Syntax:
          cppcheck-gui [OPTIONEN] [Dateien oder Pfade]

          Options:
          -h, --help              Gibt diese Hilfeinformationen aus
          -p &lt;file&gt;               Öffnet das angegebene Projekt und beginnt die Prüfung
          -l &lt;file&gt;               Öffnet die angegebene XML-Ergebnisdatei
          -d &lt;directory&gt;          Gibt das Verzeichnis an, das geprüft wurde, um das unter -l angegebene XML-Ergebnis zu erzeugen
          -v, --version           Zeigt Programmversion an
          --data-dir=&lt;directory&gt;  Gibt das Verzeichnis an, unter dem sich die Konfigurationsdateien für die GUI (Übersetzungen, Cfg) befinden. Die GUI startet bei Nutzung dieser Option nicht.
        </translation>
    </message>
    <message>
        <location filename="main.cpp" line="115"/>
        <source>Cppcheck GUI - Command line parameters</source>
        <translation>Cppcheck GUI - Kommandozeilenparameter</translation>
    </message>
</context>
<context>
    <name>NewSuppressionDialog</name>
    <message>
        <location filename="newsuppressiondialog.ui" line="17"/>
        <source>New suppression</source>
        <translation>Neue Fehlerunterdrückung</translation>
    </message>
    <message>
        <location filename="newsuppressiondialog.ui" line="25"/>
        <source>Error ID</source>
        <translation>Fehler-ID</translation>
    </message>
    <message>
        <location filename="newsuppressiondialog.ui" line="32"/>
        <source>File name</source>
        <translation>Dateiname</translation>
    </message>
    <message>
        <location filename="newsuppressiondialog.ui" line="42"/>
        <source>Line number</source>
        <translation>Zeilennummer</translation>
    </message>
    <message>
        <location filename="newsuppressiondialog.ui" line="52"/>
        <source>Symbol name</source>
        <translation>Symbolname</translation>
    </message>
    <message>
        <location filename="newsuppressiondialog.cpp" line="83"/>
        <source>Edit suppression</source>
        <translation>Fehlerunterdrückung bearbeiten</translation>
    </message>
</context>
<context>
    <name>Platforms</name>
    <message>
        <location filename="platforms.cpp" line="38"/>
        <source>Native</source>
        <translation>Nativ</translation>
    </message>
    <message>
        <location filename="platforms.cpp" line="39"/>
        <source>Unix 32-bit</source>
        <translation>Unix 32-bit</translation>
    </message>
    <message>
        <location filename="platforms.cpp" line="40"/>
        <source>Unix 64-bit</source>
        <translation>Unix 64-bit</translation>
    </message>
    <message>
        <location filename="platforms.cpp" line="41"/>
        <source>Windows 32-bit ANSI</source>
        <translation>Windows 32-bit, ANSI</translation>
    </message>
    <message>
        <location filename="platforms.cpp" line="42"/>
        <source>Windows 32-bit Unicode</source>
        <translation>Windows 32-bit, Unicode</translation>
    </message>
    <message>
        <location filename="platforms.cpp" line="43"/>
        <source>Windows 64-bit</source>
        <translation>Windows 64-bit</translation>
    </message>
</context>
<context>
    <name>ProjectFile</name>
    <message>
        <location filename="projectfile.ui" line="14"/>
        <source>Project File</source>
        <translation>Projektdatei</translation>
    </message>
    <message>
        <location filename="projectfile.ui" line="24"/>
        <source>Paths and Defines</source>
        <translation>Pfade und Definitionen</translation>
    </message>
    <message>
        <location filename="projectfile.ui" line="30"/>
        <source>Import Project (Visual studio / compile database/ Borland C++ Builder 6)</source>
        <translation>Importiere Projekt (Visual Studio / Compile-Datenbank / Borland C++-Builder 6)</translation>
    </message>
    <message>
        <location filename="projectfile.ui" line="231"/>
        <source>Defines must be separated by a semicolon. Example: DEF1;DEF2=5;DEF3=int</source>
        <translation>Definitionen müssen mit einem Semikolon getrennt werden. Beispiel: DEF1;DEF2=5;DEF3=int</translation>
    </message>
    <message>
        <location filename="projectfile.ui" line="389"/>
        <source>Note: Put your own custom .cfg files in the same folder as the project file. You should see them above.</source>
        <translation>Hinweis: Legen Sie eigene .cfg-Dateien in den Ordner der Projektdatei. Dann sollten sie oben sichtbar werden.</translation>
    </message>
    <message>
        <source>MISRA C 2012</source>
        <translation type="vanished">MISRA C 2012</translation>
    </message>
    <message>
        <location filename="projectfile.ui" line="876"/>
        <source>MISRA rule texts</source>
        <translation>MISRA-Regeltexte</translation>
    </message>
    <message>
        <location filename="projectfile.ui" line="883"/>
        <source>&lt;html&gt;&lt;head/&gt;&lt;body&gt;&lt;p&gt;Copy/paste the text from Appendix A &amp;quot;Summary of guidelines&amp;quot; from the MISRA C 2012 pdf to a text file.&lt;/p&gt;&lt;/body&gt;&lt;/html&gt;</source>
        <translation>&lt;html&gt;&lt;head/&gt;&lt;body&gt;&lt;p&gt;Text aus Anhang A &amp;quot;Summary of guidelines&amp;quot; aus der MISRA-C-2012-PDF in eine Textdatei einfügen.&lt;/p&gt;&lt;/body&gt;&lt;/html&gt;</translation>
    </message>
    <message>
        <location filename="projectfile.ui" line="890"/>
        <source>...</source>
        <translation>...</translation>
    </message>
    <message>
        <location filename="projectfile.ui" line="73"/>
        <source>&lt;html&gt;&lt;head/&gt;&lt;body&gt;&lt;p&gt;You have a choice:&lt;/p&gt;&lt;p&gt; * Analyze all Debug and Release configurations&lt;/p&gt;&lt;p&gt; * Only analyze the first matching Debug configuration&lt;/p&gt;&lt;p&gt;&lt;br/&gt;&lt;/p&gt;&lt;/body&gt;&lt;/html&gt;</source>
        <translation>&lt;html&gt;&lt;head/&gt;&lt;body&gt;&lt;p&gt;Sie haben die Auswahl:&lt;/p&gt;&lt;p&gt; * Alle Debug- und Release-Konfigurationen analysieren&lt;/p&gt;&lt;p&gt; * Nur die erste passende Debug-Konfiguration analysieren&lt;/p&gt;&lt;p&gt;&lt;br/&gt;&lt;/p&gt;&lt;/body&gt;&lt;/html&gt;</translation>
    </message>
    <message>
        <location filename="projectfile.ui" line="60"/>
        <location filename="projectfile.ui" line="422"/>
        <location filename="projectfile.ui" line="616"/>
        <source>Browse...</source>
        <translation>Durchsuchen...</translation>
    </message>
    <message>
        <location filename="projectfile.ui" line="76"/>
        <source>Analyze all Visual Studio configurations</source>
        <translation>Alle Visual-Studio-Konfigurationen analysieren</translation>
    </message>
    <message>
        <location filename="projectfile.ui" line="113"/>
        <source>Selected VS Configurations</source>
        <translation>Ausgewählte VS-Konfiguration</translation>
    </message>
    <message>
        <location filename="projectfile.ui" line="147"/>
        <source>Paths:</source>
        <translation>Pfade:</translation>
    </message>
    <message>
        <location filename="projectfile.ui" line="181"/>
        <location filename="projectfile.ui" line="296"/>
        <source>Add...</source>
        <translation>Hinzufügen...</translation>
    </message>
    <message>
        <location filename="projectfile.ui" line="188"/>
        <location filename="projectfile.ui" line="303"/>
        <location filename="projectfile.ui" line="690"/>
        <source>Edit</source>
        <translation>Bearbeiten</translation>
    </message>
    <message>
        <location filename="projectfile.ui" line="195"/>
        <location filename="projectfile.ui" line="310"/>
        <location filename="projectfile.ui" line="697"/>
        <location filename="projectfile.ui" line="740"/>
        <source>Remove</source>
        <translation>Entfernen</translation>
    </message>
    <message>
        <location filename="projectfile.ui" line="242"/>
        <source>Undefines:</source>
        <translation>Un-Definitionen:</translation>
    </message>
    <message>
        <location filename="projectfile.ui" line="252"/>
        <source>Undefines must be separated by a semicolon. Example: UNDEF1;UNDEF2;UNDEF3</source>
        <translation>Un-Definitionen müssen Semikolon-getrennt sein. Beispiel: UNDEF1;UNDEF2;UNDEF3</translation>
    </message>
    <message>
        <location filename="projectfile.ui" line="265"/>
        <source>Include Paths:</source>
        <translation>Includepfade:</translation>
    </message>
    <message>
        <location filename="projectfile.ui" line="330"/>
        <source>Up</source>
        <translation>Auf</translation>
    </message>
    <message>
        <location filename="projectfile.ui" line="337"/>
        <source>Down</source>
        <translation>Ab</translation>
    </message>
    <message>
        <location filename="projectfile.ui" line="368"/>
        <source>Platform</source>
        <translation>Plattform</translation>
    </message>
    <message>
        <location filename="projectfile.ui" line="403"/>
        <location filename="projectfile.ui" line="488"/>
        <source>Analysis</source>
        <translation>Analyse</translation>
    </message>
    <message>
        <location filename="projectfile.ui" line="415"/>
        <source>This is a workfolder that Cppcheck will use for various purposes.</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="projectfile.ui" line="448"/>
        <source>Clang (experimental)</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="projectfile.ui" line="458"/>
        <source>Check level</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="projectfile.ui" line="464"/>
        <source>Reduced -- meant for usage where developer wants results directly. Limited and faster analysis with fewer results.</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="projectfile.ui" line="471"/>
        <source>Normal -- meant for normal analysis in CI. Analysis should finish in reasonable time.</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="projectfile.ui" line="478"/>
        <source>Exhaustive -- meant for nightly builds etc. Analysis time can be longer (10x slower than compilation is OK).</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="projectfile.ui" line="494"/>
        <source>If you want to design your classes to be as flexible and robust as possible then the public interface must be very robust. Cppcheck will asumme that arguments can take *any* value.</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="projectfile.ui" line="529"/>
        <source>Check code in unused templates (should be ON normally, however in theory you can safely ignore warnings in unused templates)</source>
        <oldsource>Check code in unused templates  (slower and less accurate analysis)</oldsource>
        <translation>Prüfe Code in ungenutzten Templates (normalerweise AN, theoretisch können Warnungen in ungenutzten Templates gefahrlos ignoriert werden)</translation>
    </message>
    <message>
        <location filename="projectfile.ui" line="541"/>
        <source>Max CTU depth</source>
        <translation>Maximale CTU-Tiefe</translation>
    </message>
    <message>
        <location filename="projectfile.ui" line="572"/>
        <source>Max recursion in template instantiation</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="projectfile.ui" line="607"/>
        <source>Premium License</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="projectfile.ui" line="627"/>
        <source>Warning options</source>
        <translation>Warnoptionen</translation>
    </message>
    <message>
        <location filename="projectfile.ui" line="633"/>
        <source>Root path:</source>
        <translation>Wurzelverzeichnis:</translation>
    </message>
    <message>
        <location filename="projectfile.ui" line="639"/>
        <source>Filepaths in warnings will be relative to this path</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="projectfile.ui" line="649"/>
        <source>Warning tags (separated by semicolon)</source>
        <translation>Warnungs-Tags (Semikolon-getrennt)</translation>
    </message>
    <message>
        <location filename="projectfile.ui" line="765"/>
        <source>Enable inline suppressions</source>
        <translation>Inline-Fehlerunterdrückung aktivieren</translation>
    </message>
    <message>
        <location filename="projectfile.ui" line="851"/>
        <source>2025</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="projectfile.ui" line="901"/>
        <source>Misra C++</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="projectfile.ui" line="909"/>
        <source>2008</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="projectfile.ui" line="937"/>
        <source>Cert C</source>
        <translation>Cert C</translation>
    </message>
    <message>
        <location filename="projectfile.ui" line="946"/>
        <source>CERT-INT35-C:  int precision (if size equals precision, you can leave empty)</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="projectfile.ui" line="965"/>
        <source>Autosar</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="projectfile.ui" line="972"/>
        <source>Safety profiles (defined in C++ core guidelines)</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="projectfile.ui" line="988"/>
        <source>Bug hunting</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="projectfile.ui" line="998"/>
        <source>External tools</source>
        <translation>Externe Werkzeuge</translation>
    </message>
    <message>
        <location filename="projectfile.ui" line="409"/>
        <source>Cppcheck build dir (whole program analysis, incremental analysis, statistics, etc)</source>
        <translation>Cppcheck-Arbeitsverzeichnis (Vollständige Programmanalyse, inkrementelle Analyse, Statistiken, etc.)</translation>
    </message>
    <message>
        <location filename="projectfile.ui" line="362"/>
        <source>Types and Functions</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="projectfile.ui" line="380"/>
        <source>Libraries</source>
        <translation>Bibliotheken</translation>
    </message>
    <message>
        <location filename="projectfile.ui" line="432"/>
        <source>Parser</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="projectfile.ui" line="438"/>
        <source>Cppcheck (built in)</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="projectfile.ui" line="497"/>
        <source>Check that each class has a safe public interface</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="projectfile.ui" line="513"/>
        <source>Limit analysis</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="projectfile.ui" line="519"/>
        <source>Check code in headers  (should be ON normally. if you want a limited quick analysis then turn this OFF)</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="projectfile.ui" line="655"/>
        <source>If tags are added, you will be able to right click on warnings and set one of these tags. You can manually categorize warnings.</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="projectfile.ui" line="665"/>
        <source>Exclude source files</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="projectfile.ui" line="676"/>
        <source>Exclude folder...</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="projectfile.ui" line="683"/>
        <source>Exclude file...</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="projectfile.ui" line="722"/>
        <source>Suppressions</source>
        <translation>Fehlerunterdrückungen</translation>
    </message>
    <message>
        <location filename="projectfile.ui" line="733"/>
        <source>Add</source>
        <translation>Hinzufügen</translation>
    </message>
    <message>
        <location filename="projectfile.ui" line="786"/>
        <location filename="projectfile.ui" line="792"/>
        <source>Addons</source>
        <translation>Add-Ons</translation>
    </message>
    <message>
        <location filename="projectfile.ui" line="798"/>
        <source>Note: Addons require &lt;a href=&quot;https://www.python.org/&quot;&gt;Python&lt;/a&gt; being installed.</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="projectfile.ui" line="808"/>
        <source>Y2038</source>
        <translation>Y2038</translation>
    </message>
    <message>
        <location filename="projectfile.ui" line="815"/>
        <source>Thread safety</source>
        <translation>Threadsicherheit</translation>
    </message>
    <message>
        <location filename="projectfile.ui" line="825"/>
        <source>Coding standards</source>
        <translation>Programmierstandards</translation>
    </message>
    <message>
        <location filename="projectfile.ui" line="833"/>
        <source>Misra C</source>
        <translation>Misra C</translation>
    </message>
    <message>
        <location filename="projectfile.ui" line="841"/>
        <source>2012</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="projectfile.ui" line="846"/>
        <location filename="projectfile.ui" line="914"/>
        <source>2023</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="projectfile.ui" line="958"/>
        <source>Cert C++</source>
        <translation>Cert C++</translation>
    </message>
    <message>
        <location filename="projectfile.ui" line="982"/>
        <source>Bug hunting (Premium)</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="projectfile.ui" line="1011"/>
        <source>Clang analyzer</source>
        <translation>Clang-Analyzer</translation>
    </message>
    <message>
        <location filename="projectfile.ui" line="1004"/>
        <source>Clang-tidy</source>
        <translation>Clang-Tidy</translation>
    </message>
    <message>
        <location filename="projectfile.ui" line="221"/>
        <source>Defines:</source>
        <translation>Definitionen:</translation>
    </message>
</context>
<context>
    <name>ProjectFileDialog</name>
    <message>
        <location filename="projectfiledialog.cpp" line="138"/>
        <source>Project file: %1</source>
        <translation>Projektdatei: %1</translation>
    </message>
    <message>
        <location filename="projectfiledialog.cpp" line="590"/>
        <source>Select Cppcheck build dir</source>
        <translation>Wähle Cppcheck-Erstellungsverzeichnis</translation>
    </message>
    <message>
        <location filename="projectfiledialog.cpp" line="851"/>
        <source>Select include directory</source>
        <translation>Wähle Include-Verzeichnisse</translation>
    </message>
    <message>
        <location filename="projectfiledialog.cpp" line="831"/>
        <source>Select a directory to check</source>
        <translation>Wähle zu prüfendes Verzeichnis</translation>
    </message>
    <message>
        <location filename="projectfiledialog.cpp" line="396"/>
        <source>Note: Open source Cppcheck does not fully implement Misra C 2012</source>
        <translation>Hinweis: Open-Source Cppcheck implementiert Misra C 2012 nicht vollständig</translation>
    </message>
    <message>
        <location filename="projectfiledialog.cpp" line="461"/>
        <source>Clang-tidy (not found)</source>
        <translation>Clang-tidy (nicht gefunden)</translation>
    </message>
    <message>
        <location filename="projectfiledialog.cpp" line="629"/>
        <source>Visual Studio</source>
        <translation>Visual Studio</translation>
    </message>
    <message>
        <location filename="projectfiledialog.cpp" line="630"/>
        <source>Compile database</source>
        <translation>Compilerdatenbank</translation>
    </message>
    <message>
        <location filename="projectfiledialog.cpp" line="631"/>
        <source>Borland C++ Builder 6</source>
        <translation>Borland C++-Builder 6</translation>
    </message>
    <message>
        <location filename="projectfiledialog.cpp" line="632"/>
        <source>Import Project</source>
        <translation>Projekt importieren</translation>
    </message>
    <message>
        <location filename="projectfiledialog.cpp" line="871"/>
        <source>Select directory to ignore</source>
        <translation>Wähle zu ignorierendes Verzeichnis</translation>
    </message>
    <message>
        <location filename="projectfiledialog.cpp" line="879"/>
        <source>Source files</source>
        <translation>Quelltext-Dateien</translation>
    </message>
    <message>
        <location filename="projectfiledialog.cpp" line="880"/>
        <source>All files</source>
        <translation>Alle Dateien</translation>
    </message>
    <message>
        <location filename="projectfiledialog.cpp" line="881"/>
        <source>Exclude file</source>
        <translation>Datei ausschließen</translation>
    </message>
    <message>
        <location filename="projectfiledialog.cpp" line="964"/>
        <source>Select MISRA rule texts file</source>
        <translation>Wähle MISRA-Regeltext-Datei</translation>
    </message>
    <message>
        <location filename="projectfiledialog.cpp" line="966"/>
        <source>MISRA rule texts file (%1)</source>
        <translation>MISRA-Regeltext-Datei (%1)</translation>
    </message>
</context>
<context>
    <name>QObject</name>
    <message>
        <location filename="translationhandler.cpp" line="91"/>
        <source>Unknown language specified!</source>
        <translation>Unbekannte Sprache angegeben!</translation>
    </message>
    <message>
        <location filename="translationhandler.cpp" line="117"/>
        <source>Language file %1 not found!</source>
        <translation>Sprachdatei %1 nicht gefunden!</translation>
    </message>
    <message>
        <location filename="translationhandler.cpp" line="122"/>
        <source>Failed to load translation for language %1 from file %2</source>
        <translation>Die Übersetzungen der Sprache %1 konnten nicht aus der Datei %2 geladen werden</translation>
    </message>
    <message>
        <location filename="cppchecklibrarydata.cpp" line="38"/>
        <source>line %1: Unhandled element %2</source>
        <translation>Zeile %1: Nicht behandeltes Element %2</translation>
    </message>
    <message>
        <location filename="cppchecklibrarydata.cpp" line="43"/>
        <source>line %1: Mandatory attribute &apos;%2&apos; missing in &apos;%3&apos;</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="projectfiledialog.cpp" line="284"/>
        <source> (Not found)</source>
        <translation> (nicht gefunden)</translation>
    </message>
    <message>
        <location filename="codeeditstylecontrols.cpp" line="76"/>
        <source>Thin</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="codeeditstylecontrols.cpp" line="78"/>
        <source>ExtraLight</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="codeeditstylecontrols.cpp" line="80"/>
        <source>Light</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="codeeditstylecontrols.cpp" line="82"/>
        <source>Normal</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="codeeditstylecontrols.cpp" line="84"/>
        <source>Medium</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="codeeditstylecontrols.cpp" line="86"/>
        <source>DemiBold</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="codeeditstylecontrols.cpp" line="88"/>
        <source>Bold</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="codeeditstylecontrols.cpp" line="90"/>
        <source>ExtraBold</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="codeeditstylecontrols.cpp" line="92"/>
        <source>Black</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="codeeditstyledialog.cpp" line="80"/>
        <source>Editor Foreground Color</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="codeeditstyledialog.cpp" line="83"/>
        <source>Editor Background Color</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="codeeditstyledialog.cpp" line="86"/>
        <source>Highlight Background Color</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="codeeditstyledialog.cpp" line="89"/>
        <source>Line Number Foreground Color</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="codeeditstyledialog.cpp" line="92"/>
        <source>Line Number Background Color</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="codeeditstyledialog.cpp" line="95"/>
        <source>Keyword Foreground Color</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="codeeditstyledialog.cpp" line="98"/>
        <source>Keyword Font Weight</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="codeeditstyledialog.cpp" line="101"/>
        <source>Class Foreground Color</source>
        <translation>Klassen-Vordergrundfarbe</translation>
    </message>
    <message>
        <location filename="codeeditstyledialog.cpp" line="104"/>
        <source>Class Font Weight</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="codeeditstyledialog.cpp" line="107"/>
        <source>Quote Foreground Color</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="codeeditstyledialog.cpp" line="110"/>
        <source>Quote Font Weight</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="codeeditstyledialog.cpp" line="113"/>
        <source>Comment Foreground Color</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="codeeditstyledialog.cpp" line="116"/>
        <source>Comment Font Weight</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="codeeditstyledialog.cpp" line="119"/>
        <source>Symbol Foreground Color</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="codeeditstyledialog.cpp" line="122"/>
        <source>Symbol Background Color</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="codeeditstyledialog.cpp" line="125"/>
        <source>Symbol Font Weight</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="codeeditstyledialog.cpp" line="141"/>
        <source>Set to Default Light</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="codeeditstyledialog.cpp" line="143"/>
        <source>Set to Default Dark</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="resultstree.cpp" line="113"/>
        <source>File</source>
        <translation type="unfinished">Datei</translation>
    </message>
    <message>
        <location filename="resultstree.cpp" line="114"/>
        <source>Line</source>
        <translation type="unfinished">Zeile</translation>
    </message>
    <message>
        <location filename="resultstree.cpp" line="115"/>
        <source>Severity</source>
        <translation type="unfinished">Schweregrad</translation>
    </message>
    <message>
        <location filename="resultstree.cpp" line="116"/>
        <source>Classification</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="resultstree.cpp" line="117"/>
        <source>Level</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="resultstree.cpp" line="118"/>
        <source>Inconclusive</source>
        <translation type="unfinished">Unklar</translation>
    </message>
    <message>
        <location filename="resultstree.cpp" line="119"/>
        <source>Summary</source>
        <translation type="unfinished">Zusammenfassung</translation>
    </message>
    <message>
        <location filename="resultstree.cpp" line="120"/>
        <source>Id</source>
        <translation type="unfinished">Id</translation>
    </message>
    <message>
        <location filename="resultstree.cpp" line="121"/>
        <source>Guideline</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="resultstree.cpp" line="122"/>
        <source>Rule</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="resultstree.cpp" line="123"/>
        <source>Since date</source>
        <translation type="unfinished">Seit Datum</translation>
    </message>
    <message>
        <location filename="resultstree.cpp" line="124"/>
        <source>Tags</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="resultstree.cpp" line="125"/>
        <source>CWE</source>
        <translation type="unfinished"></translation>
    </message>
</context>
<context>
    <name>QPlatformTheme</name>
    <message>
        <location filename="translationhandler.cpp" line="41"/>
        <source>OK</source>
        <translation>OK</translation>
    </message>
    <message>
        <location filename="translationhandler.cpp" line="42"/>
        <source>Cancel</source>
        <translation>Abbrechen</translation>
    </message>
    <message>
        <location filename="translationhandler.cpp" line="43"/>
        <source>Close</source>
        <translation>Schließen</translation>
    </message>
    <message>
        <location filename="translationhandler.cpp" line="44"/>
        <source>Save</source>
        <translation>Speichern</translation>
    </message>
</context>
<context>
    <name>ResultsTree</name>
    <message>
        <source>File</source>
        <translation type="vanished">Datei</translation>
    </message>
    <message>
        <source>Severity</source>
        <translation type="vanished">Schweregrad</translation>
    </message>
    <message>
        <source>Line</source>
        <translation type="vanished">Zeile</translation>
    </message>
    <message>
        <source>Summary</source>
        <translation type="vanished">Zusammenfassung</translation>
    </message>
    <message>
        <source>Undefined file</source>
        <translation type="vanished">Undefinierte Datei</translation>
    </message>
    <message>
        <location filename="resultstree.cpp" line="668"/>
        <source>Copy</source>
        <translation>Kopieren</translation>
    </message>
    <message>
        <location filename="resultstree.cpp" line="836"/>
        <source>Could not find file:</source>
        <translation>Kann Datei nicht finden:</translation>
    </message>
    <message>
        <location filename="resultstree.cpp" line="840"/>
        <source>Please select the folder &apos;%1&apos;</source>
        <translation>Bitte wählen Sie den Ordner &apos;%1&apos;</translation>
    </message>
    <message>
        <location filename="resultstree.cpp" line="841"/>
        <source>Select Directory &apos;%1&apos;</source>
        <translation>Wähle Verzeichnis &apos;%1&apos;</translation>
    </message>
    <message>
        <location filename="resultstree.cpp" line="843"/>
        <source>Please select the directory where file is located.</source>
        <translation>Bitte wählen Sie das Verzeichnis, in dem sich die Datei befindet.</translation>
    </message>
    <message>
        <location filename="resultstree.cpp" line="373"/>
        <source>debug</source>
        <translation>Debug</translation>
    </message>
    <message>
        <location filename="resultstree.cpp" line="307"/>
        <source>note</source>
        <translation>Anmerkung</translation>
    </message>
    <message>
        <location filename="resultstree.cpp" line="670"/>
        <source>Hide all with id</source>
        <translation>Verstecke alle mit gleicher ID</translation>
    </message>
    <message>
        <location filename="resultstree.cpp" line="692"/>
        <source>Suppress selected id(s)</source>
        <translation>Ausgewählte ID(s) unterdrücken</translation>
    </message>
    <message>
        <location filename="resultstree.cpp" line="671"/>
        <source>Open containing folder</source>
        <translation>Übergeordneten Ordner öffnen</translation>
    </message>
    <message>
        <location filename="resultstree.cpp" line="376"/>
        <source>internal</source>
        <translation>Intern</translation>
    </message>
    <message>
        <location filename="resultstree.cpp" line="667"/>
        <source>Recheck %1 file(s)</source>
        <translation>Erneut %1 Datei(en) prüfen</translation>
    </message>
    <message>
        <location filename="resultstree.cpp" line="669"/>
        <source>Hide %1 result(s)</source>
        <translation>Verstecke %1 Ergebnis(se)</translation>
    </message>
    <message>
        <location filename="resultstree.cpp" line="712"/>
        <source>Tag</source>
        <translation>Tag</translation>
    </message>
    <message>
        <location filename="resultstree.cpp" line="714"/>
        <source>No tag</source>
        <translation>Kein Tag</translation>
    </message>
    <message>
        <location filename="resultstree.cpp" line="744"/>
        <location filename="resultstree.cpp" line="758"/>
        <source>Cppcheck</source>
        <translation>Cppcheck</translation>
    </message>
    <message>
        <location filename="resultstree.cpp" line="745"/>
        <source>No editor application configured.

Configure the editor application for Cppcheck in preferences/Applications.</source>
        <translation>Keine Editor-Anwendung eingestellt.

Konfigurieren Sie diese unter Einstellungen/Anwendungen.</translation>
    </message>
    <message>
        <location filename="resultstree.cpp" line="759"/>
        <source>No default editor application selected.

Please select the default editor application in preferences/Applications.</source>
        <translation>Keine Standard-Editor-Anwendung eingestellt.

 Bitte wählen Sie eine Standardanwendung unter Einstellungen/Anwendungen.</translation>
    </message>
    <message>
        <location filename="resultstree.cpp" line="779"/>
        <source>Could not find the file!</source>
        <translation>Datei konnte nicht gefunden werden!</translation>
    </message>
    <message>
        <location filename="resultstree.cpp" line="822"/>
        <source>Could not start %1

Please check the application path and parameters are correct.</source>
        <translation>%1 konnte nicht gestartet werden.

Bitte überprüfen Sie ob der Pfad und die Parameter der Anwendung richtig eingestellt sind.</translation>
    </message>
    <message>
        <location filename="resultstree.cpp" line="844"/>
        <source>Select Directory</source>
        <translation>Wähle Verzeichnis</translation>
    </message>
    <message>
        <location filename="resultstree.cpp" line="355"/>
        <source>style</source>
        <translation>Stil</translation>
    </message>
    <message>
        <location filename="resultstree.cpp" line="358"/>
        <source>error</source>
        <translation>Fehler</translation>
    </message>
    <message>
        <location filename="resultstree.cpp" line="361"/>
        <source>warning</source>
        <translation>Warnung</translation>
    </message>
    <message>
        <location filename="resultstree.cpp" line="364"/>
        <source>performance</source>
        <translation>Performance</translation>
    </message>
    <message>
        <location filename="resultstree.cpp" line="367"/>
        <source>portability</source>
        <translation>Portabilität</translation>
    </message>
    <message>
        <location filename="resultstree.cpp" line="370"/>
        <source>information</source>
        <translation>Information</translation>
    </message>
</context>
<context>
    <name>ResultsView</name>
    <message>
        <location filename="resultsview.cpp" line="255"/>
        <source>Print Report</source>
        <translation>Bericht drucken</translation>
    </message>
    <message>
        <location filename="resultsview.cpp" line="274"/>
        <source>No errors found, nothing to print.</source>
        <translation>Keine Funde, nichts zu drucken.</translation>
    </message>
    <message>
        <location filename="resultsview.cpp" line="326"/>
        <source>%p% (%1 of %2 files checked)</source>
        <translation>%p% (%1 von %2 Dateien geprüft)</translation>
    </message>
    <message>
        <location filename="resultsview.cpp" line="350"/>
        <location filename="resultsview.cpp" line="361"/>
        <source>Cppcheck</source>
        <translation>Cppcheck</translation>
    </message>
    <message>
        <location filename="resultsview.cpp" line="351"/>
        <source>No errors found.</source>
        <translation>Keine Fehler gefunden.</translation>
    </message>
    <message>
        <location filename="resultsview.cpp" line="358"/>
        <source>Errors were found, but they are configured to be hidden.
To toggle what kind of errors are shown, open view menu.</source>
        <translation>Es wurden Fehler gefunden, aber sie sind so konfiguriert, ausgeblendet zu werden.
Legen Sie unter dem Menü Ansicht fest, welche Arten von Fehlern angezeigt werden sollen.</translation>
    </message>
    <message>
        <location filename="resultsview.cpp" line="402"/>
        <location filename="resultsview.cpp" line="421"/>
        <source>Failed to read the report.</source>
        <translation>Lesen des Berichts fehlgeschlagen.</translation>
    </message>
    <message>
        <location filename="resultsview.cpp" line="409"/>
        <source>XML format version 1 is no longer supported.</source>
        <translation>XML-Format-Version 1 wird nicht länger unterstützt.</translation>
    </message>
    <message>
        <location filename="resultsview.cpp" line="460"/>
        <source>First included by</source>
        <translation>Zuerst inkludiert von</translation>
    </message>
    <message>
        <location filename="resultsview.cpp" line="465"/>
        <source>Id</source>
        <translation>Id</translation>
    </message>
    <message>
        <location filename="resultsview.cpp" line="539"/>
        <source>Clear Log</source>
        <translation>Protokoll leeren</translation>
    </message>
    <message>
        <location filename="resultsview.cpp" line="540"/>
        <source>Copy this Log entry</source>
        <translation>Diesen Protokolleintrag kopieren</translation>
    </message>
    <message>
        <location filename="resultsview.cpp" line="541"/>
        <source>Copy complete Log</source>
        <translation>Gesamtes Protokoll kopieren</translation>
    </message>
    <message>
        <location filename="resultsview.cpp" line="549"/>
        <source>Analysis was stopped</source>
        <translation>Analyse wurde gestoppt</translation>
    </message>
    <message>
        <location filename="resultsview.cpp" line="563"/>
        <source>There was a critical error with id &apos;%1&apos;</source>
        <translation>Kritischer Fehler mit ID &apos;%1&apos; aufgetreten</translation>
    </message>
    <message>
        <location filename="resultsview.cpp" line="565"/>
        <source>when checking %1</source>
        <translation>beim Prüfen von %1</translation>
    </message>
    <message>
        <location filename="resultsview.cpp" line="567"/>
        <source>when checking a file</source>
        <translation>beim Prüfen einer Datei</translation>
    </message>
    <message>
        <location filename="resultsview.cpp" line="568"/>
        <source>Analysis was aborted.</source>
        <translation>Analyse abgebrochen.</translation>
    </message>
    <message>
        <location filename="resultsview.cpp" line="237"/>
        <location filename="resultsview.cpp" line="245"/>
        <source>Failed to save the report.</source>
        <translation>Der Bericht konnte nicht speichern werden.</translation>
    </message>
    <message>
        <location filename="resultsview.ui" line="26"/>
        <source>Results</source>
        <translation>Berichte</translation>
    </message>
    <message>
        <location filename="resultsview.ui" line="60"/>
        <source>Critical errors</source>
        <translation>Kritische Fehler</translation>
    </message>
    <message>
        <location filename="resultsview.ui" line="92"/>
        <source>Analysis Log</source>
        <translation>Analyseprotokoll</translation>
    </message>
    <message>
        <location filename="resultsview.ui" line="114"/>
        <source>Warning Details</source>
        <translation>Warnungs-Details</translation>
    </message>
</context>
<context>
    <name>ScratchPad</name>
    <message>
        <location filename="scratchpad.ui" line="14"/>
        <source>Scratchpad</source>
        <translation>Schmierzettel</translation>
    </message>
    <message>
        <location filename="scratchpad.ui" line="20"/>
        <source>Copy or write some C/C++ code here:</source>
        <translation>Kopieren oder schreiben Sie C/C++-Code hierher:</translation>
    </message>
    <message>
        <location filename="scratchpad.ui" line="37"/>
        <source>Optionally enter a filename (mainly for automatic language detection) and click on &quot;Check&quot;:</source>
        <translation>Optional einen Dateinamen (hauptsächlich für automatische Spracherkennung) eingeben und auf &quot;Prüfe&quot; klicken:</translation>
    </message>
    <message>
        <location filename="scratchpad.ui" line="71"/>
        <source>filename</source>
        <translation>Dateiname</translation>
    </message>
    <message>
        <location filename="scratchpad.ui" line="78"/>
        <source>Check</source>
        <translation>Prüfe</translation>
    </message>
</context>
<context>
    <name>Settings</name>
    <message>
        <location filename="settings.ui" line="14"/>
        <source>Preferences</source>
        <translation>Einstellungen</translation>
    </message>
    <message>
        <location filename="settings.ui" line="24"/>
        <source>General</source>
        <translation>Allgemein</translation>
    </message>
    <message>
        <location filename="settings.ui" line="196"/>
        <source>Add...</source>
        <translation>Hinzufügen...</translation>
    </message>
    <message>
        <location filename="settings.ui" line="41"/>
        <source>Number of threads: </source>
        <translation>Anzahl der Threads: </translation>
    </message>
    <message>
        <source>Ideal count:</source>
        <translation type="vanished">Ideale Anzahl:</translation>
    </message>
    <message>
        <location filename="settings.ui" line="108"/>
        <source>Force checking all #ifdef configurations</source>
        <translation>Erzwinge Prüfung aller #ifdef-Konfigurationen</translation>
    </message>
    <message>
        <location filename="settings.ui" line="115"/>
        <source>Show full path of files</source>
        <translation>Vollständigen Dateipfad anzeigen</translation>
    </message>
    <message>
        <location filename="settings.ui" line="122"/>
        <source>Show &quot;No errors found&quot; message when no errors found</source>
        <translation>&quot;Keine Fehler gefunden&quot;-Meldung anzeigen, wenn keine Fehler gefunden werden</translation>
    </message>
    <message>
        <location filename="settings.ui" line="129"/>
        <source>Display error Id in column &quot;Id&quot;</source>
        <translation>Zeige Meldungs-Id in Spalte &quot;Id&quot;</translation>
    </message>
    <message>
        <location filename="settings.ui" line="136"/>
        <source>Enable inline suppressions</source>
        <translation>Inline-Fehlerunterdrückung aktivieren</translation>
    </message>
    <message>
        <location filename="settings.ui" line="143"/>
        <source>Check for inconclusive errors also</source>
        <translation>Auch nach unklaren Fehlern suchen</translation>
    </message>
    <message>
        <location filename="settings.ui" line="150"/>
        <source>Show statistics on check completion</source>
        <translation>Zeige Statistiken nach Prüfungsabschluss</translation>
    </message>
    <message>
        <location filename="settings.ui" line="157"/>
        <source>Check for updates</source>
        <translation>Auf Updates prüfen</translation>
    </message>
    <message>
        <location filename="settings.ui" line="177"/>
        <source>Show internal warnings in log</source>
        <translation>Interne Warnungen im Log anzeigen</translation>
    </message>
    <message>
        <location filename="settings.ui" line="288"/>
        <source>Addons</source>
        <translation>Add-Ons</translation>
    </message>
    <message>
        <location filename="settings.ui" line="294"/>
        <source>Python binary (leave this empty to use python in the PATH)</source>
        <translation>Python-Binärdatei (Python aus PATH wird genutzt, wenn leer)</translation>
    </message>
    <message>
        <location filename="settings.ui" line="305"/>
        <location filename="settings.ui" line="346"/>
        <location filename="settings.ui" line="391"/>
        <source>...</source>
        <translation>...</translation>
    </message>
    <message>
        <location filename="settings.ui" line="324"/>
        <source>MISRA addon</source>
        <translation>MISRA-Addon</translation>
    </message>
    <message>
        <location filename="settings.ui" line="332"/>
        <source>MISRA rule texts file</source>
        <translation>MISRA-Regeltext-Datei</translation>
    </message>
    <message>
        <location filename="settings.ui" line="339"/>
        <source>&lt;html&gt;&lt;head/&gt;&lt;body&gt;&lt;p&gt;Copy/paste the text from Appendix A &amp;quot;Summary of guidelines&amp;quot; from the MISRA C 2012 pdf to a text file.&lt;/p&gt;&lt;/body&gt;&lt;/html&gt;</source>
        <translation>&lt;html&gt;&lt;head/&gt;&lt;body&gt;&lt;p&gt;Text aus Anhang A &amp;quot;Summary of guidelines&amp;quot; aus der MISRA-C-2012-PDF in eine Textdatei einfügen.&lt;/p&gt;&lt;/body&gt;&lt;/html&gt;</translation>
    </message>
    <message>
        <location filename="settings.ui" line="372"/>
        <source>Clang</source>
        <translation>Clang</translation>
    </message>
    <message>
        <location filename="settings.ui" line="378"/>
        <source>Clang path (leave empty to use system PATH)</source>
        <translation>Clang-Verzeichnis (PATH wird genutzt, wenn leer)</translation>
    </message>
    <message>
        <location filename="settings.ui" line="401"/>
        <source>Visual Studio headers</source>
        <translation>Visual-Studio-Header</translation>
    </message>
    <message>
        <location filename="settings.ui" line="407"/>
        <source>&lt;html&gt;&lt;head/&gt;&lt;body&gt;&lt;p&gt;Paths to Visual Studio headers, separated by semicolon &apos;;&apos;.&lt;/p&gt;&lt;p&gt;You can open a Visual Studio command prompt, write &amp;quot;SET INCLUDE&amp;quot;. Then copy/paste the paths.&lt;/p&gt;&lt;/body&gt;&lt;/html&gt;</source>
        <translation>&lt;html&gt;&lt;head/&gt;&lt;body&gt;&lt;p&gt;Pfade zu Visual-Studio-Headern, Semikolon-getrennt.&lt;/p&gt;&lt;p&gt;Sie können eine Visual-Studio-Kommandozeile öffnen, &amp;quot;SET INCLUDE&amp;quot; eingeben und dann die Pfade hier reinkopieren.&lt;/p&gt;&lt;/body&gt;&lt;/html&gt;</translation>
    </message>
    <message>
        <location filename="settings.ui" line="437"/>
        <source>Code Editor</source>
        <translation>Code-Editor</translation>
    </message>
    <message>
        <location filename="settings.ui" line="443"/>
        <source>Code Editor Style</source>
        <translation>Code-Editor-Stil</translation>
    </message>
    <message>
        <location filename="settings.ui" line="449"/>
        <source>System Style</source>
        <translation>Systemstil</translation>
    </message>
    <message>
        <location filename="settings.ui" line="456"/>
        <source>Default Light Style</source>
        <translation>Heller Standardstil</translation>
    </message>
    <message>
        <location filename="settings.ui" line="463"/>
        <source>Default Dark Style</source>
        <translation>Dunkler Standardstil</translation>
    </message>
    <message>
        <location filename="settings.ui" line="472"/>
        <source>Custom</source>
        <translation>Benutzerdefiniert</translation>
    </message>
    <message>
        <location filename="settings.ui" line="210"/>
        <source>Remove</source>
        <translation>Entfernen</translation>
    </message>
    <message>
        <location filename="settings.ui" line="79"/>
        <source>Max count:</source>
        <translation>Maximalzahl:</translation>
    </message>
    <message>
        <location filename="settings.ui" line="185"/>
        <source>Applications</source>
        <translation>Anwendungen</translation>
    </message>
    <message>
        <location filename="settings.ui" line="203"/>
        <location filename="settings.ui" line="479"/>
        <source>Edit...</source>
        <translation>Bearbeiten...</translation>
    </message>
    <message>
        <location filename="settings.ui" line="217"/>
        <source>Set as default</source>
        <translation>Als Standard festlegen</translation>
    </message>
    <message>
        <location filename="settings.ui" line="240"/>
        <source>Reports</source>
        <translation>Berichte</translation>
    </message>
    <message>
        <location filename="settings.ui" line="246"/>
        <source>Save all errors when creating report</source>
        <translation>Alle Fehler beim Erstellen von Berichten speichern</translation>
    </message>
    <message>
        <location filename="settings.ui" line="253"/>
        <source>Save full path to files in reports</source>
        <translation>Vollständigen Dateipfad in Berichten speichern</translation>
    </message>
    <message>
        <location filename="settings.ui" line="274"/>
        <source>Language</source>
        <translation>Sprache</translation>
    </message>
</context>
<context>
    <name>SettingsDialog</name>
    <message>
        <source>N/A</source>
        <translation type="vanished">kA</translation>
    </message>
    <message>
        <location filename="settingsdialog.cpp" line="232"/>
        <source>The executable file &quot;%1&quot; is not available</source>
        <translation>Die ausführbare Datei &quot;%1&quot; ist nicht verfügbar</translation>
    </message>
    <message>
        <location filename="settingsdialog.cpp" line="243"/>
        <source>Add a new application</source>
        <translation>Neue Anwendung hinzufügen</translation>
    </message>
    <message>
        <location filename="settingsdialog.cpp" line="273"/>
        <source>Modify an application</source>
        <translation>Anwendung ändern</translation>
    </message>
    <message>
        <location filename="settingsdialog.cpp" line="278"/>
        <source> [Default]</source>
        <translation> [Standard]</translation>
    </message>
    <message>
        <location filename="settingsdialog.cpp" line="303"/>
        <source>[Default]</source>
        <translation>[Standard]</translation>
    </message>
    <message>
        <location filename="settingsdialog.cpp" line="358"/>
        <source>Select python binary</source>
        <translation>Python-Binärdatei auswählen</translation>
    </message>
    <message>
        <location filename="settingsdialog.cpp" line="365"/>
        <source>Select MISRA File</source>
        <translation>Wähle MISRA-Datei</translation>
    </message>
    <message>
        <location filename="settingsdialog.cpp" line="396"/>
        <source>Select clang path</source>
        <translation>Clang-Verzeichnis auswählen</translation>
    </message>
</context>
<context>
    <name>StatsDialog</name>
    <message>
        <location filename="statsdialog.ui" line="14"/>
        <location filename="statsdialog.ui" line="248"/>
        <location filename="statsdialog.cpp" line="183"/>
        <location filename="statsdialog.cpp" line="230"/>
        <source>Statistics</source>
        <translation>Statistik</translation>
    </message>
    <message>
        <location filename="statsdialog.ui" line="27"/>
        <location filename="statsdialog.cpp" line="221"/>
        <source>Project</source>
        <translation>Projekt</translation>
    </message>
    <message>
        <location filename="statsdialog.ui" line="33"/>
        <source>Project:</source>
        <translation>Projekt:</translation>
    </message>
    <message>
        <location filename="statsdialog.ui" line="53"/>
        <source>Paths:</source>
        <translation>Pfade:</translation>
    </message>
    <message>
        <location filename="statsdialog.ui" line="85"/>
        <source>Include paths:</source>
        <translation>Include-Pfade:</translation>
    </message>
    <message>
        <location filename="statsdialog.ui" line="108"/>
        <source>Defines:</source>
        <translation>Definitionen:</translation>
    </message>
    <message>
        <location filename="statsdialog.ui" line="131"/>
        <source>Undefines:</source>
        <translation>Un-Definitionen:</translation>
    </message>
    <message>
        <location filename="statsdialog.ui" line="165"/>
        <location filename="statsdialog.cpp" line="226"/>
        <source>Previous Scan</source>
        <translation>Vorherige Prüfung</translation>
    </message>
    <message>
        <location filename="statsdialog.ui" line="171"/>
        <source>Path Selected:</source>
        <translation>Ausgewählte Pfade:</translation>
    </message>
    <message>
        <location filename="statsdialog.ui" line="181"/>
        <source>Number of Files Scanned:</source>
        <translation>Anzahl der geprüften Dateien:</translation>
    </message>
    <message>
        <location filename="statsdialog.ui" line="201"/>
        <source>Scan Duration:</source>
        <translation>Prüfungsdauer:</translation>
    </message>
    <message>
        <location filename="statsdialog.ui" line="254"/>
        <source>Errors:</source>
        <translation>Fehler:</translation>
    </message>
    <message>
        <location filename="statsdialog.ui" line="271"/>
        <source>Warnings:</source>
        <translation>Warnungen:</translation>
    </message>
    <message>
        <location filename="statsdialog.ui" line="288"/>
        <source>Stylistic warnings:</source>
        <translation>Stilwarnungen:</translation>
    </message>
    <message>
        <location filename="statsdialog.ui" line="305"/>
        <source>Portability warnings:</source>
        <translation>Portabilitätswarnungen:</translation>
    </message>
    <message>
        <location filename="statsdialog.ui" line="322"/>
        <source>Performance issues:</source>
        <translation>Performance-Probleme:</translation>
    </message>
    <message>
        <location filename="statsdialog.ui" line="339"/>
        <source>Information messages:</source>
        <translation>Informationsmeldungen:</translation>
    </message>
    <message>
        <location filename="statsdialog.ui" line="356"/>
        <source>Active checkers:</source>
        <translation>Aktive Checker:</translation>
    </message>
    <message>
        <location filename="statsdialog.ui" line="374"/>
        <source>Checkers</source>
        <translation>Checker</translation>
    </message>
    <message>
        <location filename="statsdialog.ui" line="399"/>
        <source>History</source>
        <translation>Verlauf</translation>
    </message>
    <message>
        <location filename="statsdialog.ui" line="405"/>
        <source>File:</source>
        <translation>Datei:</translation>
    </message>
    <message>
        <location filename="statsdialog.ui" line="443"/>
        <source>Copy to Clipboard</source>
        <translation>In die Zwischenablage kopieren</translation>
    </message>
    <message>
        <location filename="statsdialog.ui" line="450"/>
        <source>Pdf Export</source>
        <translation>PDF-Export</translation>
    </message>
    <message>
        <location filename="statsdialog.cpp" line="159"/>
        <source>1 day</source>
        <translation>1 Tag</translation>
    </message>
    <message>
        <location filename="statsdialog.cpp" line="159"/>
        <source>%1 days</source>
        <translation>%1 Tage</translation>
    </message>
    <message>
        <location filename="statsdialog.cpp" line="161"/>
        <source>1 hour</source>
        <translation>1 Stunde</translation>
    </message>
    <message>
        <location filename="statsdialog.cpp" line="161"/>
        <source>%1 hours</source>
        <translation>%1 Stunden</translation>
    </message>
    <message>
        <location filename="statsdialog.cpp" line="163"/>
        <source>1 minute</source>
        <translation>1 Minute</translation>
    </message>
    <message>
        <location filename="statsdialog.cpp" line="163"/>
        <source>%1 minutes</source>
        <translation>%1 Minuten</translation>
    </message>
    <message>
        <location filename="statsdialog.cpp" line="165"/>
        <source>1 second</source>
        <translation>1 Sekunde</translation>
    </message>
    <message>
        <location filename="statsdialog.cpp" line="165"/>
        <source>%1 seconds</source>
        <translation>%1 Sekunden</translation>
    </message>
    <message>
        <location filename="statsdialog.cpp" line="169"/>
        <source>0.%1 seconds</source>
        <translation>0,%1 Sekunden</translation>
    </message>
    <message>
        <location filename="statsdialog.cpp" line="171"/>
        <source> and </source>
        <translation> und </translation>
    </message>
    <message>
        <location filename="statsdialog.cpp" line="198"/>
        <source>Export PDF</source>
        <translation>Exportiere PDF</translation>
    </message>
    <message>
        <location filename="statsdialog.cpp" line="220"/>
        <source>Project Settings</source>
        <translation>Projekteinstellungen</translation>
    </message>
    <message>
        <location filename="statsdialog.cpp" line="222"/>
        <source>Paths</source>
        <translation>Pfade</translation>
    </message>
    <message>
        <location filename="statsdialog.cpp" line="223"/>
        <source>Include paths</source>
        <translation>Include-Pfade</translation>
    </message>
    <message>
        <location filename="statsdialog.cpp" line="224"/>
        <source>Defines</source>
        <translation>Definitionen</translation>
    </message>
    <message>
        <location filename="statsdialog.cpp" line="225"/>
        <source>Undefines</source>
        <translation>Un-Definitionen</translation>
    </message>
    <message>
        <location filename="statsdialog.cpp" line="227"/>
        <source>Path selected</source>
        <translation>Gewählte Pfade</translation>
    </message>
    <message>
        <location filename="statsdialog.cpp" line="228"/>
        <source>Number of files scanned</source>
        <translation>Anzahl geprüfter Dateien</translation>
    </message>
    <message>
        <location filename="statsdialog.cpp" line="229"/>
        <source>Scan duration</source>
        <translation>Prüfungsdauer</translation>
    </message>
    <message>
        <location filename="statsdialog.cpp" line="185"/>
        <location filename="statsdialog.cpp" line="231"/>
        <source>Errors</source>
        <translation>Fehler</translation>
    </message>
    <message>
        <location filename="statsdialog.cpp" line="112"/>
        <source>File: </source>
        <translation>Datei: </translation>
    </message>
    <message>
        <location filename="statsdialog.cpp" line="112"/>
        <source>No cppcheck build dir</source>
        <translation>Kein Cppcheck-Analyseverzeichnis</translation>
    </message>
    <message>
        <location filename="statsdialog.cpp" line="187"/>
        <location filename="statsdialog.cpp" line="232"/>
        <source>Warnings</source>
        <translation>Warnungen</translation>
    </message>
    <message>
        <location filename="statsdialog.cpp" line="189"/>
        <location filename="statsdialog.cpp" line="233"/>
        <source>Style warnings</source>
        <translation>Stilwarnungen</translation>
    </message>
    <message>
        <location filename="statsdialog.cpp" line="191"/>
        <location filename="statsdialog.cpp" line="234"/>
        <source>Portability warnings</source>
        <translation>Portabilitätswarnungen</translation>
    </message>
    <message>
        <location filename="statsdialog.cpp" line="193"/>
        <location filename="statsdialog.cpp" line="235"/>
        <source>Performance warnings</source>
        <translation>Performance-Warnungen</translation>
    </message>
    <message>
        <location filename="statsdialog.cpp" line="195"/>
        <location filename="statsdialog.cpp" line="236"/>
        <source>Information messages</source>
        <translation>Informationsmeldungen</translation>
    </message>
</context>
<context>
    <name>ThreadDetails</name>
    <message>
        <location filename="threaddetails.ui" line="14"/>
        <source>Thread Details</source>
        <translation type="unfinished"></translation>
    </message>
</context>
<context>
    <name>ThreadResult</name>
    <message>
        <location filename="threadresult.cpp" line="47"/>
        <source>%1 of %2 files checked</source>
        <translation>%1 von %2 Dateien geprüft</translation>
    </message>
</context>
<context>
    <name>TranslationHandler</name>
    <message>
        <location filename="translationhandler.cpp" line="130"/>
        <source>Failed to change the user interface language:

%1

The user interface language has been reset to English. Open the Preferences-dialog to select any of the available languages.</source>
        <translation>Wechsel der Sprache der Benutzeroberfläche fehlgeschlagen:

%1

Die Sprache wurde auf Englisch zurückgesetzt. Öffnen Sie den Einstellungen-Dialog um eine verfügbare Sprache auszuwählen.</translation>
    </message>
    <message>
        <location filename="translationhandler.cpp" line="136"/>
        <source>Cppcheck</source>
        <translation>Cppcheck</translation>
    </message>
</context>
<context>
    <name>TxtReport</name>
    <message>
        <location filename="txtreport.cpp" line="74"/>
        <source>inconclusive</source>
        <translation>unklar</translation>
    </message>
</context>
<context>
    <name>toFilterString</name>
    <message>
        <location filename="common.cpp" line="58"/>
        <source>All supported files (%1)</source>
        <translation>Alle unterstützten Dateien (%1)</translation>
    </message>
    <message>
        <location filename="common.cpp" line="63"/>
        <source>All files (%1)</source>
        <translation>Alle Dateien (%1)</translation>
    </message>
</context>
</TS>
