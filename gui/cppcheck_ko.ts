<?xml version="1.0" encoding="utf-8"?>
<!DOCTYPE TS>
<TS version="2.1" language="ko_KR">
<context>
    <name>About</name>
    <message>
        <location filename="about.ui" line="14"/>
        <location filename="build/Skrivbord-Felsu00f6k/temp/ui_about.h" line="130"/>
        <source>About Cppcheck</source>
        <translation>Cppcheck 정보</translation>
    </message>
    <message>
        <location filename="about.ui" line="64"/>
        <location filename="build/Skrivbord-Felsu00f6k/temp/ui_about.h" line="132"/>
        <source>Version %1</source>
        <translation>버전 %1</translation>
    </message>
    <message>
        <location filename="about.ui" line="71"/>
        <location filename="build/Skrivbord-Felsu00f6k/temp/ui_about.h" line="133"/>
        <source>Cppcheck - A tool for static C/C++ code analysis.</source>
        <translation>Cppcheck - 정적 C/C++ 코드 분석 도구.</translation>
    </message>
    <message>
        <location filename="about.ui" line="91"/>
        <location filename="build/Skrivbord-Felsu00f6k/temp/ui_about.h" line="135"/>
        <source>This program is licensed under the terms
of the GNU General Public License version 3</source>
        <translation>이 프로그램은 GNU General Public License version 3을
준수합니다</translation>
    </message>
    <message>
        <location filename="about.ui" line="102"/>
        <location filename="build/Skrivbord-Felsu00f6k/temp/ui_about.h" line="137"/>
        <source>Visit Cppcheck homepage at %1</source>
        <translation>Cppcheck 홈페이지(%1)를 방문해보세요</translation>
    </message>
    <message>
        <location filename="about.ui" line="115"/>
        <location filename="build/Skrivbord-Felsu00f6k/temp/ui_about.h" line="138"/>
        <source>&lt;html&gt;&lt;head/&gt;&lt;body&gt;&lt;p&gt;Many thanks to these libraries that we use:&lt;/p&gt;&lt;ul style=&quot;margin-top: 0px; margin-bottom: 0px; margin-left: 0px; margin-right: 0px; -qt-list-indent: 1;&quot;&gt;&lt;li style=&quot; margin-top:12px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;&quot;&gt;PCRE&lt;/li&gt;&lt;li style=&quot; margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;&quot;&gt;PicoJSON&lt;/li&gt;&lt;li style=&quot; margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;&quot;&gt;Qt&lt;/li&gt;&lt;li style=&quot; margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;&quot;&gt;TinyXML2&lt;/li&gt;&lt;li style=&quot; margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;&quot;&gt;Boost&lt;/li&gt;&lt;/ul&gt;&lt;/body&gt;&lt;/html&gt;</source>
        <oldsource>&lt;html&gt;&lt;head/&gt;&lt;body&gt;&lt;p&gt;Many thanks to these libraries that we use:&lt;/p&gt;&lt;ul style=&quot;margin-top: 0px; margin-bottom: 0px; margin-left: 0px; margin-right: 0px; -qt-list-indent: 1;&quot;&gt;&lt;li style=&quot; margin-top:12px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;&quot;&gt;pcre&lt;/li&gt;&lt;li style=&quot; margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;&quot;&gt;picojson&lt;/li&gt;&lt;li style=&quot; margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;&quot;&gt;qt&lt;/li&gt;&lt;li style=&quot; margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;&quot;&gt;tinyxml2&lt;/li&gt;&lt;/ul&gt;&lt;/body&gt;&lt;/html&gt;</oldsource>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="about.ui" line="81"/>
        <location filename="build/Skrivbord-Felsu00f6k/temp/ui_about.h" line="134"/>
        <source>Copyright © 2007-%1 Cppcheck team.</source>
        <oldsource>Copyright © 2007-2021 Cppcheck team.</oldsource>
        <translation type="unfinished"></translation>
    </message>
</context>
<context>
    <name>ApplicationDialog</name>
    <message>
        <location filename="applicationdialog.ui" line="23"/>
        <location filename="build/Skrivbord-Felsu00f6k/temp/ui_applicationdialog.h" line="161"/>
        <source>Add an application</source>
        <translation>응용 프로그램 추가</translation>
    </message>
    <message>
        <location filename="applicationdialog.ui" line="41"/>
        <location filename="build/Skrivbord-Felsu00f6k/temp/ui_applicationdialog.h" line="162"/>
        <source>Here you can add an application that can open error files. Specify a name for the application, the application executable and command line parameters for the application.

The following texts in parameters are replaced with appropriate values when application is executed:
(file) - Filename containing the error
(line) - Line number containing the error
(message) - Error message
(severity) - Error severity

Example opening a file with Kate and make Kate scroll to the correct line:
Executable: kate
Parameters: -l(line) (file)</source>
        <translation>에러 파일을 열 응용 프로그램을 추가할 수 있습니다. 응용 프로그램의 이름, 실행 파일, 명령행 인자를 입력하세요.

인자 중에서 아래와 같은 텍스트는 응용 프로그램 실행 시 해당 값으로 대치됩니다:
(file) - 에러를 포함한 파일이름
(line) - 에러를 포함한 행번호
(message) - 에러 메시지
(severity) - 에러 종류

Kate로 파일을 열고, 해당 행으로 이동하는 예제:
실행파일: kate
인자: -l(line) (file)</translation>
    </message>
    <message>
        <location filename="applicationdialog.ui" line="76"/>
        <location filename="build/Skrivbord-Felsu00f6k/temp/ui_applicationdialog.h" line="173"/>
        <source>&amp;Name:</source>
        <translation>이름(&amp;N):</translation>
    </message>
    <message>
        <location filename="applicationdialog.ui" line="86"/>
        <location filename="build/Skrivbord-Felsu00f6k/temp/ui_applicationdialog.h" line="174"/>
        <source>&amp;Executable:</source>
        <translation>실행 파일(&amp;E):</translation>
    </message>
    <message>
        <location filename="applicationdialog.ui" line="96"/>
        <location filename="build/Skrivbord-Felsu00f6k/temp/ui_applicationdialog.h" line="175"/>
        <source>&amp;Parameters:</source>
        <translation>명령행 인자(&amp;P):</translation>
    </message>
    <message>
        <location filename="applicationdialog.ui" line="138"/>
        <location filename="build/Skrivbord-Felsu00f6k/temp/ui_applicationdialog.h" line="176"/>
        <source>Browse</source>
        <translation>찾기</translation>
    </message>
    <message>
        <location filename="applicationdialog.cpp" line="65"/>
        <source>Executable files (*.exe);;All files(*.*)</source>
        <translation>실행 파일(*.exe);;모든 파일(*.*)</translation>
    </message>
    <message>
        <location filename="applicationdialog.cpp" line="68"/>
        <source>Select viewer application</source>
        <translation>뷰어 프로그램 선택</translation>
    </message>
    <message>
        <source>Cppcheck</source>
        <translation type="vanished">Cppcheck</translation>
    </message>
    <message>
        <location filename="applicationdialog.cpp" line="84"/>
        <source>You must specify a name, a path and optionally parameters for the application!</source>
        <translation type="unfinished"></translation>
    </message>
</context>
<context>
    <name>FileViewDialog</name>
    <message>
        <location filename="fileviewdialog.cpp" line="53"/>
        <source>Could not find the file: %1</source>
        <translation>파일 찾기 실패: %1</translation>
    </message>
    <message>
        <source>Cppcheck</source>
        <translation type="vanished">Cppcheck</translation>
    </message>
    <message>
        <location filename="fileviewdialog.cpp" line="67"/>
        <source>Could not read the file: %1</source>
        <translation>파일 읽기 실패: %1</translation>
    </message>
</context>
<context>
    <name>HelpDialog</name>
    <message>
        <location filename="helpdialog.ui" line="14"/>
        <location filename="build/Skrivbord-Felsu00f6k/temp/ui_helpdialog.h" line="88"/>
        <source>Cppcheck GUI help</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="helpdialog.ui" line="34"/>
        <location filename="build/Skrivbord-Felsu00f6k/temp/ui_helpdialog.h" line="89"/>
        <source>Contents</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="helpdialog.ui" line="44"/>
        <location filename="build/Skrivbord-Felsu00f6k/temp/ui_helpdialog.h" line="90"/>
        <source>Index</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="helpdialog.cpp" line="85"/>
        <source>Helpfile &apos;%1&apos; was not found</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Cppcheck</source>
        <translation type="obsolete">Cppcheck</translation>
    </message>
</context>
<context>
    <name>LibraryAddFunctionDialog</name>
    <message>
        <location filename="libraryaddfunctiondialog.ui" line="23"/>
        <location filename="build/Skrivbord-Felsu00f6k/temp/ui_libraryaddfunctiondialog.h" line="105"/>
        <source>Add function</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="libraryaddfunctiondialog.ui" line="34"/>
        <location filename="build/Skrivbord-Felsu00f6k/temp/ui_libraryaddfunctiondialog.h" line="106"/>
        <source>Function name(s)</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="libraryaddfunctiondialog.ui" line="44"/>
        <location filename="build/Skrivbord-Felsu00f6k/temp/ui_libraryaddfunctiondialog.h" line="107"/>
        <source>Number of arguments</source>
        <translation type="unfinished"></translation>
    </message>
</context>
<context>
    <name>LibraryDialog</name>
    <message>
        <location filename="librarydialog.ui" line="14"/>
        <location filename="build/Skrivbord-Felsu00f6k/temp/ui_librarydialog.h" line="275"/>
        <source>Library Editor</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="librarydialog.ui" line="22"/>
        <location filename="build/Skrivbord-Felsu00f6k/temp/ui_librarydialog.h" line="276"/>
        <source>Open</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="librarydialog.ui" line="29"/>
        <location filename="build/Skrivbord-Felsu00f6k/temp/ui_librarydialog.h" line="277"/>
        <source>Save</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="librarydialog.ui" line="62"/>
        <location filename="build/Skrivbord-Felsu00f6k/temp/ui_librarydialog.h" line="279"/>
        <source>Functions</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="librarydialog.ui" line="111"/>
        <location filename="build/Skrivbord-Felsu00f6k/temp/ui_librarydialog.h" line="281"/>
        <source>Add</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="librarydialog.ui" line="204"/>
        <location filename="build/Skrivbord-Felsu00f6k/temp/ui_librarydialog.h" line="284"/>
        <source>noreturn</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="librarydialog.ui" line="212"/>
        <location filename="build/Skrivbord-Felsu00f6k/temp/ui_librarydialog.h" line="285"/>
        <source>False</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="librarydialog.ui" line="217"/>
        <location filename="build/Skrivbord-Felsu00f6k/temp/ui_librarydialog.h" line="286"/>
        <source>True</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="librarydialog.ui" line="222"/>
        <location filename="build/Skrivbord-Felsu00f6k/temp/ui_librarydialog.h" line="287"/>
        <source>Unknown</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="librarydialog.ui" line="232"/>
        <location filename="build/Skrivbord-Felsu00f6k/temp/ui_librarydialog.h" line="289"/>
        <source>return value must be used</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="librarydialog.ui" line="239"/>
        <location filename="build/Skrivbord-Felsu00f6k/temp/ui_librarydialog.h" line="290"/>
        <source>ignore function in leaks checking</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="librarydialog.ui" line="246"/>
        <location filename="build/Skrivbord-Felsu00f6k/temp/ui_librarydialog.h" line="291"/>
        <source>Arguments</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="librarydialog.ui" line="258"/>
        <location filename="build/Skrivbord-Felsu00f6k/temp/ui_librarydialog.h" line="292"/>
        <source>Edit</source>
        <translation type="unfinished">편집</translation>
    </message>
    <message>
        <location filename="librarydialog.cpp" line="97"/>
        <location filename="librarydialog.cpp" line="169"/>
        <source>Library files (*.cfg)</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="librarydialog.cpp" line="99"/>
        <source>Open library file</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="librarydialog.ui" line="71"/>
        <location filename="build/Skrivbord-Felsu00f6k/temp/ui_librarydialog.h" line="280"/>
        <source>Sort</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="librarydialog.ui" line="131"/>
        <location filename="build/Skrivbord-Felsu00f6k/temp/ui_librarydialog.h" line="282"/>
        <source>Filter:</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="librarydialog.ui" line="164"/>
        <location filename="build/Skrivbord-Felsu00f6k/temp/ui_librarydialog.h" line="283"/>
        <source>Comments</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="librarydialog.ui" line="36"/>
        <location filename="build/Skrivbord-Felsu00f6k/temp/ui_librarydialog.h" line="278"/>
        <source>Save as</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Cppcheck</source>
        <translation type="obsolete">Cppcheck</translation>
    </message>
    <message>
        <location filename="librarydialog.cpp" line="172"/>
        <source>Save the library as</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="librarydialog.cpp" line="123"/>
        <source>Failed to load %1. %2.</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="librarydialog.cpp" line="111"/>
        <source>Cannot open file %1.</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="librarydialog.cpp" line="160"/>
        <source>Cannot save file %1.</source>
        <translation type="unfinished"></translation>
    </message>
</context>
<context>
    <name>LibraryEditArgDialog</name>
    <message>
        <location filename="libraryeditargdialog.ui" line="14"/>
        <location filename="build/Skrivbord-Felsu00f6k/temp/ui_libraryeditargdialog.h" line="284"/>
        <source>Edit argument</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="libraryeditargdialog.ui" line="20"/>
        <location filename="build/Skrivbord-Felsu00f6k/temp/ui_libraryeditargdialog.h" line="286"/>
        <source>&lt;html&gt;&lt;head/&gt;&lt;body&gt;
&lt;p&gt;Is bool value allowed? For instance result from comparison or from &apos;!&apos; operator.&lt;/p&gt;
&lt;p&gt;Typically, set this if the argument is a pointer, size, etc.&lt;/p&gt;
&lt;p&gt;Example:&lt;/p&gt;
&lt;pre&gt;    memcmp(x, y, i == 123);   // last argument should not have a bool value&lt;/pre&gt;
&lt;/body&gt;&lt;/html&gt;</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="libraryeditargdialog.ui" line="28"/>
        <location filename="build/Skrivbord-Felsu00f6k/temp/ui_libraryeditargdialog.h" line="293"/>
        <source>Not bool</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="libraryeditargdialog.ui" line="35"/>
        <location filename="build/Skrivbord-Felsu00f6k/temp/ui_libraryeditargdialog.h" line="295"/>
        <source>&lt;html&gt;&lt;head/&gt;&lt;body&gt;
&lt;p&gt;Is a null parameter value allowed?&lt;/p&gt;
&lt;p&gt;Typically this should be used on any pointer parameter that does not allow null.&lt;/p&gt;
&lt;p&gt;Example:&lt;/p&gt;
&lt;pre&gt;    strcpy(x,y); // neither x or y is allowed to be null.&lt;/pre&gt;
&lt;/body&gt;&lt;/html&gt;</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="libraryeditargdialog.ui" line="43"/>
        <location filename="build/Skrivbord-Felsu00f6k/temp/ui_libraryeditargdialog.h" line="302"/>
        <source>Not null</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="libraryeditargdialog.ui" line="50"/>
        <location filename="build/Skrivbord-Felsu00f6k/temp/ui_libraryeditargdialog.h" line="303"/>
        <source>Not uninit</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="libraryeditargdialog.ui" line="57"/>
        <location filename="build/Skrivbord-Felsu00f6k/temp/ui_libraryeditargdialog.h" line="304"/>
        <source>String</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="libraryeditargdialog.ui" line="70"/>
        <location filename="build/Skrivbord-Felsu00f6k/temp/ui_libraryeditargdialog.h" line="305"/>
        <source>Format string</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="libraryeditargdialog.ui" line="92"/>
        <location filename="build/Skrivbord-Felsu00f6k/temp/ui_libraryeditargdialog.h" line="306"/>
        <source>Min size of buffer</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="libraryeditargdialog.ui" line="101"/>
        <location filename="libraryeditargdialog.ui" line="203"/>
        <location filename="build/Skrivbord-Felsu00f6k/temp/ui_libraryeditargdialog.h" line="307"/>
        <location filename="build/Skrivbord-Felsu00f6k/temp/ui_libraryeditargdialog.h" line="316"/>
        <source>Type</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="libraryeditargdialog.ui" line="109"/>
        <location filename="libraryeditargdialog.ui" line="214"/>
        <location filename="build/Skrivbord-Felsu00f6k/temp/ui_libraryeditargdialog.h" line="308"/>
        <location filename="build/Skrivbord-Felsu00f6k/temp/ui_libraryeditargdialog.h" line="317"/>
        <source>None</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="libraryeditargdialog.ui" line="114"/>
        <location filename="libraryeditargdialog.ui" line="219"/>
        <location filename="build/Skrivbord-Felsu00f6k/temp/ui_libraryeditargdialog.h" line="309"/>
        <location filename="build/Skrivbord-Felsu00f6k/temp/ui_libraryeditargdialog.h" line="318"/>
        <source>argvalue</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="libraryeditargdialog.ui" line="119"/>
        <location filename="libraryeditargdialog.ui" line="224"/>
        <location filename="build/Skrivbord-Felsu00f6k/temp/ui_libraryeditargdialog.h" line="310"/>
        <location filename="build/Skrivbord-Felsu00f6k/temp/ui_libraryeditargdialog.h" line="319"/>
        <source>mul</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="libraryeditargdialog.ui" line="124"/>
        <location filename="libraryeditargdialog.ui" line="229"/>
        <location filename="build/Skrivbord-Felsu00f6k/temp/ui_libraryeditargdialog.h" line="311"/>
        <location filename="build/Skrivbord-Felsu00f6k/temp/ui_libraryeditargdialog.h" line="320"/>
        <source>strlen</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="libraryeditargdialog.ui" line="132"/>
        <location filename="libraryeditargdialog.ui" line="237"/>
        <location filename="build/Skrivbord-Felsu00f6k/temp/ui_libraryeditargdialog.h" line="313"/>
        <location filename="build/Skrivbord-Felsu00f6k/temp/ui_libraryeditargdialog.h" line="322"/>
        <source>Arg</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="libraryeditargdialog.ui" line="159"/>
        <location filename="libraryeditargdialog.ui" line="264"/>
        <location filename="build/Skrivbord-Felsu00f6k/temp/ui_libraryeditargdialog.h" line="314"/>
        <location filename="build/Skrivbord-Felsu00f6k/temp/ui_libraryeditargdialog.h" line="323"/>
        <source>Arg2</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="libraryeditargdialog.ui" line="194"/>
        <location filename="build/Skrivbord-Felsu00f6k/temp/ui_libraryeditargdialog.h" line="315"/>
        <source>and</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="libraryeditargdialog.ui" line="310"/>
        <location filename="build/Skrivbord-Felsu00f6k/temp/ui_libraryeditargdialog.h" line="324"/>
        <source>Valid values</source>
        <translation type="unfinished"></translation>
    </message>
</context>
<context>
    <name>MainWindow</name>
    <message>
        <source>Cppcheck</source>
        <translation type="vanished">Cppcheck</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="84"/>
        <location filename="build/Skrivbord-Felsu00f6k/temp/ui_mainwindow.h" line="738"/>
        <source>Checking for updates</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="100"/>
        <location filename="build/Skrivbord-Felsu00f6k/temp/ui_mainwindow.h" line="739"/>
        <source>Hide</source>
        <translation type="unfinished">숨기기</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="132"/>
        <location filename="build/Skrivbord-Felsu00f6k/temp/ui_mainwindow.h" line="740"/>
        <source>&amp;File</source>
        <translation>파일(&amp;F)</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="151"/>
        <location filename="build/Skrivbord-Felsu00f6k/temp/ui_mainwindow.h" line="741"/>
        <source>&amp;View</source>
        <translation>보기(&amp;V)</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="155"/>
        <location filename="build/Skrivbord-Felsu00f6k/temp/ui_mainwindow.h" line="742"/>
        <source>&amp;Toolbars</source>
        <translation>도구바(&amp;T)</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="163"/>
        <location filename="mainwindow.ui" line="979"/>
        <location filename="build/Skrivbord-Felsu00f6k/temp/ui_mainwindow.h" line="730"/>
        <location filename="build/Skrivbord-Felsu00f6k/temp/ui_mainwindow.h" line="743"/>
        <source>Report</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="198"/>
        <location filename="build/Skrivbord-Felsu00f6k/temp/ui_mainwindow.h" line="744"/>
        <source>&amp;Help</source>
        <translation>도움말(&amp;H)</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="253"/>
        <location filename="build/Skrivbord-Felsu00f6k/temp/ui_mainwindow.h" line="748"/>
        <source>&amp;Edit</source>
        <translation>편집(&amp;E)</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="266"/>
        <location filename="build/Skrivbord-Felsu00f6k/temp/ui_mainwindow.h" line="749"/>
        <source>Standard</source>
        <translation>표준 도구</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="284"/>
        <location filename="build/Skrivbord-Felsu00f6k/temp/ui_mainwindow.h" line="750"/>
        <source>Categories</source>
        <translation>분류 도구</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="303"/>
        <location filename="build/Skrivbord-Felsu00f6k/temp/ui_mainwindow.h" line="751"/>
        <source>Filter</source>
        <translation>필터 도구</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="314"/>
        <location filename="build/Skrivbord-Felsu00f6k/temp/ui_mainwindow.h" line="578"/>
        <source>&amp;License...</source>
        <translation>저작권(&amp;L)...</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="319"/>
        <location filename="build/Skrivbord-Felsu00f6k/temp/ui_mainwindow.h" line="579"/>
        <source>A&amp;uthors...</source>
        <translation>제작자(&amp;u)...</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="328"/>
        <location filename="build/Skrivbord-Felsu00f6k/temp/ui_mainwindow.h" line="580"/>
        <source>&amp;About...</source>
        <translation>정보(&amp;A)...</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="333"/>
        <location filename="build/Skrivbord-Felsu00f6k/temp/ui_mainwindow.h" line="581"/>
        <source>&amp;Files...</source>
        <translation>파일(&amp;F)...</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="342"/>
        <location filename="build/Skrivbord-Felsu00f6k/temp/ui_mainwindow.h" line="587"/>
        <source>Ctrl+F</source>
        <translation>Ctrl+F</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="351"/>
        <location filename="build/Skrivbord-Felsu00f6k/temp/ui_mainwindow.h" line="589"/>
        <source>&amp;Directory...</source>
        <translation>디렉토리(&amp;D)...</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="360"/>
        <location filename="build/Skrivbord-Felsu00f6k/temp/ui_mainwindow.h" line="595"/>
        <source>Ctrl+D</source>
        <translation>Ctrl+D</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="372"/>
        <location filename="build/Skrivbord-Felsu00f6k/temp/ui_mainwindow.h" line="599"/>
        <source>Ctrl+R</source>
        <translation>Ctrl+R</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="390"/>
        <location filename="build/Skrivbord-Felsu00f6k/temp/ui_mainwindow.h" line="602"/>
        <source>&amp;Stop</source>
        <translation>중지(&amp;S)</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="399"/>
        <location filename="build/Skrivbord-Felsu00f6k/temp/ui_mainwindow.h" line="608"/>
        <source>Esc</source>
        <translation>Esc</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="408"/>
        <location filename="build/Skrivbord-Felsu00f6k/temp/ui_mainwindow.h" line="610"/>
        <source>&amp;Save results to file...</source>
        <translation>결과를 파일에 저장(&amp;S)...</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="411"/>
        <location filename="build/Skrivbord-Felsu00f6k/temp/ui_mainwindow.h" line="612"/>
        <source>Ctrl+S</source>
        <translation>Ctrl+S</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="416"/>
        <location filename="build/Skrivbord-Felsu00f6k/temp/ui_mainwindow.h" line="614"/>
        <source>&amp;Quit</source>
        <translation>종료(&amp;Q)</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="428"/>
        <location filename="build/Skrivbord-Felsu00f6k/temp/ui_mainwindow.h" line="618"/>
        <source>&amp;Clear results</source>
        <translation>결과 지우기(&amp;C)</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="437"/>
        <location filename="build/Skrivbord-Felsu00f6k/temp/ui_mainwindow.h" line="619"/>
        <source>&amp;Preferences</source>
        <translation>설정(&amp;P)</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="452"/>
        <location filename="mainwindow.ui" line="455"/>
        <location filename="build/Skrivbord-Felsu00f6k/temp/ui_mainwindow.h" line="621"/>
        <location filename="build/Skrivbord-Felsu00f6k/temp/ui_mainwindow.h" line="623"/>
        <source>Show style warnings</source>
        <translation>스타일 경고 표시</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="470"/>
        <location filename="mainwindow.ui" line="473"/>
        <location filename="build/Skrivbord-Felsu00f6k/temp/ui_mainwindow.h" line="626"/>
        <location filename="build/Skrivbord-Felsu00f6k/temp/ui_mainwindow.h" line="628"/>
        <location filename="mainwindow.cpp" line="2408"/>
        <source>Show errors</source>
        <translation>애러 표시</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="478"/>
        <location filename="build/Skrivbord-Felsu00f6k/temp/ui_mainwindow.h" line="630"/>
        <source>&amp;Check all</source>
        <translation>전체 선택(&amp;C)</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="483"/>
        <location filename="build/Skrivbord-Felsu00f6k/temp/ui_mainwindow.h" line="631"/>
        <source>&amp;Uncheck all</source>
        <translation>전체 해제(&amp;U)</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="488"/>
        <location filename="build/Skrivbord-Felsu00f6k/temp/ui_mainwindow.h" line="632"/>
        <source>Collapse &amp;all</source>
        <translation>전체 접기(&amp;A)</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="493"/>
        <location filename="build/Skrivbord-Felsu00f6k/temp/ui_mainwindow.h" line="633"/>
        <source>&amp;Expand all</source>
        <translation>전체 펼치기(&amp;E)</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="501"/>
        <location filename="build/Skrivbord-Felsu00f6k/temp/ui_mainwindow.h" line="634"/>
        <source>&amp;Standard</source>
        <translation>표준 도구(&amp;S)</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="504"/>
        <location filename="build/Skrivbord-Felsu00f6k/temp/ui_mainwindow.h" line="636"/>
        <source>Standard items</source>
        <translation>표준 아이템</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="509"/>
        <location filename="build/Skrivbord-Felsu00f6k/temp/ui_mainwindow.h" line="638"/>
        <source>&amp;Contents</source>
        <translation>내용(&amp;C)</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="512"/>
        <location filename="build/Skrivbord-Felsu00f6k/temp/ui_mainwindow.h" line="640"/>
        <source>Open the help contents</source>
        <translation>도움말을 엽니다</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="515"/>
        <location filename="build/Skrivbord-Felsu00f6k/temp/ui_mainwindow.h" line="643"/>
        <source>F1</source>
        <translation>F1</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="520"/>
        <location filename="build/Skrivbord-Felsu00f6k/temp/ui_mainwindow.h" line="645"/>
        <source>Toolbar</source>
        <translation>도구바</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="528"/>
        <location filename="build/Skrivbord-Felsu00f6k/temp/ui_mainwindow.h" line="646"/>
        <source>&amp;Categories</source>
        <translation>분류 도구(&amp;C)</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="531"/>
        <location filename="build/Skrivbord-Felsu00f6k/temp/ui_mainwindow.h" line="648"/>
        <source>Error categories</source>
        <translation>에러 종류</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="536"/>
        <location filename="build/Skrivbord-Felsu00f6k/temp/ui_mainwindow.h" line="650"/>
        <source>&amp;Open XML...</source>
        <translation>XML 열기(&amp;O)...</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="545"/>
        <location filename="build/Skrivbord-Felsu00f6k/temp/ui_mainwindow.h" line="651"/>
        <source>Open P&amp;roject File...</source>
        <translation>프로젝트 파일 열기(&amp;R)...</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="562"/>
        <location filename="build/Skrivbord-Felsu00f6k/temp/ui_mainwindow.h" line="656"/>
        <source>&amp;New Project File...</source>
        <translation>새 프로젝트 파일(&amp;N)...</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="570"/>
        <location filename="build/Skrivbord-Felsu00f6k/temp/ui_mainwindow.h" line="660"/>
        <source>&amp;Log View</source>
        <translation>로그 보기(&amp;L)</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="573"/>
        <location filename="build/Skrivbord-Felsu00f6k/temp/ui_mainwindow.h" line="662"/>
        <source>Log View</source>
        <translation>로그 보기</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="581"/>
        <location filename="build/Skrivbord-Felsu00f6k/temp/ui_mainwindow.h" line="664"/>
        <source>C&amp;lose Project File</source>
        <translation>프로젝트 파일 닫기(&amp;L)</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="589"/>
        <location filename="build/Skrivbord-Felsu00f6k/temp/ui_mainwindow.h" line="665"/>
        <source>&amp;Edit Project File...</source>
        <translation>프로젝트 파일 편집(&amp;E)...</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="601"/>
        <location filename="build/Skrivbord-Felsu00f6k/temp/ui_mainwindow.h" line="666"/>
        <source>&amp;Statistics</source>
        <translation>통계 보기(&amp;S)</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="616"/>
        <location filename="mainwindow.ui" line="619"/>
        <location filename="build/Skrivbord-Felsu00f6k/temp/ui_mainwindow.h" line="668"/>
        <location filename="build/Skrivbord-Felsu00f6k/temp/ui_mainwindow.h" line="670"/>
        <location filename="mainwindow.cpp" line="2409"/>
        <source>Show warnings</source>
        <translation>경고 표시</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="634"/>
        <location filename="mainwindow.ui" line="637"/>
        <location filename="build/Skrivbord-Felsu00f6k/temp/ui_mainwindow.h" line="673"/>
        <location filename="build/Skrivbord-Felsu00f6k/temp/ui_mainwindow.h" line="675"/>
        <source>Show performance warnings</source>
        <translation>성능 경고 표시</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="645"/>
        <location filename="build/Skrivbord-Felsu00f6k/temp/ui_mainwindow.h" line="677"/>
        <source>Show &amp;hidden</source>
        <translation>숨기기 보기(&amp;H)</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="974"/>
        <location filename="build/Skrivbord-Felsu00f6k/temp/ui_mainwindow.h" line="729"/>
        <source>Compliance report...</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="987"/>
        <location filename="build/Skrivbord-Felsu00f6k/temp/ui_mainwindow.h" line="731"/>
        <source>Normal</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="1035"/>
        <location filename="build/Skrivbord-Felsu00f6k/temp/ui_mainwindow.h" line="732"/>
        <source>Autosar</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="1040"/>
        <location filename="build/Skrivbord-Felsu00f6k/temp/ui_mainwindow.h" line="733"/>
        <source>EULA...</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="1045"/>
        <location filename="build/Skrivbord-Felsu00f6k/temp/ui_mainwindow.h" line="734"/>
        <source>Thread Details</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="1048"/>
        <location filename="build/Skrivbord-Felsu00f6k/temp/ui_mainwindow.h" line="736"/>
        <source>Show thread details</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="mainwindow.cpp" line="1015"/>
        <location filename="mainwindow.cpp" line="1053"/>
        <source>Information</source>
        <translation>정보</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="660"/>
        <location filename="build/Skrivbord-Felsu00f6k/temp/ui_mainwindow.h" line="680"/>
        <source>Show information messages</source>
        <translation>정보 표시</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="675"/>
        <location filename="build/Skrivbord-Felsu00f6k/temp/ui_mainwindow.h" line="684"/>
        <source>Show portability warnings</source>
        <translation>이식성 경고 표시</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="713"/>
        <location filename="build/Skrivbord-Felsu00f6k/temp/ui_mainwindow.h" line="693"/>
        <source>&amp;Filter</source>
        <translation>필터 도구(&amp;F)</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="716"/>
        <location filename="build/Skrivbord-Felsu00f6k/temp/ui_mainwindow.h" line="695"/>
        <source>Filter results</source>
        <translation>필터링 결과</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="732"/>
        <location filename="build/Skrivbord-Felsu00f6k/temp/ui_mainwindow.h" line="697"/>
        <source>Windows 32-bit ANSI</source>
        <translation>Windows 32-bit ANSI</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="740"/>
        <location filename="build/Skrivbord-Felsu00f6k/temp/ui_mainwindow.h" line="698"/>
        <source>Windows 32-bit Unicode</source>
        <translation>Windows 32-bit Unicode</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="748"/>
        <location filename="build/Skrivbord-Felsu00f6k/temp/ui_mainwindow.h" line="699"/>
        <source>Unix 32-bit</source>
        <translation>Unix 32-bit</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="756"/>
        <location filename="build/Skrivbord-Felsu00f6k/temp/ui_mainwindow.h" line="700"/>
        <source>Unix 64-bit</source>
        <translation>Unix 64-bit</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="764"/>
        <location filename="build/Skrivbord-Felsu00f6k/temp/ui_mainwindow.h" line="701"/>
        <source>Windows 64-bit</source>
        <translation>Windows 64-bit</translation>
    </message>
    <message>
        <location filename="mainwindow.cpp" line="176"/>
        <location filename="mainwindow.cpp" line="1812"/>
        <source>Quick Filter:</source>
        <translation>빠른 필터:</translation>
    </message>
    <message>
        <location filename="mainwindow.cpp" line="477"/>
        <source>There was a problem with loading the editor application settings.

This is probably because the settings were changed between the Cppcheck versions. Please check (and fix) the editor application settings, otherwise the editor program might not start correctly.</source>
        <translation>편집기 설정을 불러오는데 문제가 있습니다.

Cppcheck 버전간 설정 방법 차이때문인 것으로 보입니다. 편집기 설정을 검사(및 수정)해주세요, 그렇지 않으면 편집기가 제대로 시작하지 않습니다.</translation>
    </message>
    <message>
        <location filename="mainwindow.cpp" line="601"/>
        <source>None of the selected files is part of the project, nothing to analyze.</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="mainwindow.cpp" line="784"/>
        <source>You must close the project file before selecting new files or directories!</source>
        <translation>새로운 파일이나 디렉토리를 선택하기 전에 프로젝트 파일을 닫으세요!</translation>
    </message>
    <message>
        <location filename="mainwindow.cpp" line="890"/>
        <source>Found project file: %1

Do you want to load this project file instead?</source>
        <translation>프로젝트 파일 존재: %1

이 프로젝트 파일을 불러오겠습니까?</translation>
    </message>
    <message>
        <location filename="mainwindow.cpp" line="1539"/>
        <location filename="mainwindow.cpp" line="1734"/>
        <source>XML files (*.xml)</source>
        <translation>XML 파일 (*.xml)</translation>
    </message>
    <message>
        <location filename="mainwindow.cpp" line="1541"/>
        <source>Open the report file</source>
        <translation>보고서 파일 열기</translation>
    </message>
    <message>
        <location filename="mainwindow.cpp" line="1700"/>
        <source>License</source>
        <translation>저작권</translation>
    </message>
    <message>
        <location filename="mainwindow.cpp" line="1707"/>
        <source>Authors</source>
        <translation>제작자</translation>
    </message>
    <message>
        <location filename="mainwindow.cpp" line="1727"/>
        <source>Save the report file</source>
        <translation>보고서 파일 저장</translation>
    </message>
    <message>
        <location filename="mainwindow.cpp" line="1738"/>
        <source>Text files (*.txt)</source>
        <translation>텍스트 파일 (*.txt)</translation>
    </message>
    <message>
        <location filename="mainwindow.cpp" line="1742"/>
        <source>CSV files (*.csv)</source>
        <translation>CSV 파일 (*.csv)</translation>
    </message>
    <message>
        <location filename="mainwindow.cpp" line="1850"/>
        <source>Project files (*.cppcheck);;All files(*.*)</source>
        <translation>프로젝트 파일 (*.cppcheck);;모든 파일(*.*)</translation>
    </message>
    <message>
        <location filename="mainwindow.cpp" line="1852"/>
        <source>Select Project File</source>
        <translation>프로젝트 파일 선택</translation>
    </message>
    <message>
        <location filename="mainwindow.cpp" line="1992"/>
        <source>Failed to open file</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="mainwindow.cpp" line="1995"/>
        <source>Unknown project file format</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="mainwindow.cpp" line="1998"/>
        <source>Failed to import project file</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="mainwindow.cpp" line="2014"/>
        <source>Failed to import &apos;%1&apos;: %2

Analysis is stopped.</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="mainwindow.cpp" line="2023"/>
        <source>Failed to import &apos;%1&apos; (%2), analysis is stopped</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="mainwindow.cpp" line="2339"/>
        <source>Install</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="mainwindow.cpp" line="2343"/>
        <source>New version available: %1. %2</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="mainwindow.cpp" line="2381"/>
        <source>Show Mandatory</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="mainwindow.cpp" line="2382"/>
        <source>Show Required</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="mainwindow.cpp" line="2383"/>
        <source>Show Advisory</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="mainwindow.cpp" line="2384"/>
        <source>Show Document</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="mainwindow.cpp" line="2401"/>
        <source>Show L1</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="mainwindow.cpp" line="2402"/>
        <source>Show L2</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="mainwindow.cpp" line="2403"/>
        <source>Show L3</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="mainwindow.cpp" line="2410"/>
        <source>Show style</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="mainwindow.cpp" line="2411"/>
        <source>Show portability</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="mainwindow.cpp" line="2412"/>
        <source>Show performance</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="mainwindow.cpp" line="2413"/>
        <source>Show information</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="mainwindow.cpp" line="260"/>
        <location filename="mainwindow.cpp" line="1814"/>
        <location filename="mainwindow.cpp" line="1880"/>
        <location filename="mainwindow.cpp" line="2062"/>
        <source>Project:</source>
        <translation>프로젝트:</translation>
    </message>
    <message>
        <location filename="mainwindow.cpp" line="1045"/>
        <source>Duplicate define</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="mainwindow.cpp" line="1064"/>
        <source>File not found: &apos;%1&apos;</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="mainwindow.cpp" line="1087"/>
        <source>Failed to load/setup addon %1: %2</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="mainwindow.cpp" line="1110"/>
        <source>Failed to load %1. Your Cppcheck installation is broken. You can use --data-dir=&lt;directory&gt; at the command line to specify where this file is located. Please note that --data-dir is supposed to be used by installation scripts and therefore the GUI does not start when it is used, all that happens is that the setting is configured.

Analysis is aborted.</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="mainwindow.cpp" line="1120"/>
        <source>Failed to load %1 - %2

Analysis is aborted.</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="mainwindow.cpp" line="1132"/>
        <location filename="mainwindow.cpp" line="1226"/>
        <source>%1

Analysis is aborted.</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="mainwindow.cpp" line="1686"/>
        <source>About</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="mainwindow.cpp" line="1959"/>
        <source>To check the project using addons, you need a build directory.</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="mainwindow.cpp" line="2049"/>
        <source>Select Project Filename</source>
        <translation>프로젝트 파일이름 선택</translation>
    </message>
    <message>
        <location filename="mainwindow.cpp" line="2095"/>
        <source>No project file loaded</source>
        <translation>프로젝트 파일 불러오기 실패</translation>
    </message>
    <message>
        <location filename="mainwindow.cpp" line="2177"/>
        <source>The project file

%1

 could not be found!

Do you want to remove the file from the recently used projects -list?</source>
        <translation>프로젝트 파일

%1

이 존재하지 않습니다!

최근 프로젝트 목록에서 파일을 제거하시겠습니까?</translation>
    </message>
    <message>
        <location filename="main.cpp" line="115"/>
        <source>Cppcheck GUI - Command line parameters</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="213"/>
        <location filename="build/Skrivbord-Felsu00f6k/temp/ui_mainwindow.h" line="746"/>
        <source>C++ standard</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="mainwindow.cpp" line="1110"/>
        <location filename="mainwindow.cpp" line="1120"/>
        <location filename="mainwindow.cpp" line="1132"/>
        <location filename="mainwindow.cpp" line="1226"/>
        <source>Error</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="mainwindow.cpp" line="1024"/>
        <source>File not found</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="mainwindow.cpp" line="1027"/>
        <source>Bad XML</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="mainwindow.cpp" line="1030"/>
        <source>Missing attribute</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="mainwindow.cpp" line="1033"/>
        <source>Bad attribute value</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="mainwindow.cpp" line="1053"/>
        <source>Failed to load the selected library &apos;%1&apos;.
%2</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="mainwindow.cpp" line="1036"/>
        <source>Unsupported format</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="mainwindow.cpp" line="1015"/>
        <source>The library &apos;%1&apos; contains unknown elements:
%2</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="mainwindow.cpp" line="1039"/>
        <source>Duplicate platform type</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="mainwindow.cpp" line="1042"/>
        <source>Platform type redefined</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="858"/>
        <location filename="build/Skrivbord-Felsu00f6k/temp/ui_mainwindow.h" line="709"/>
        <source>&amp;Print...</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="861"/>
        <location filename="build/Skrivbord-Felsu00f6k/temp/ui_mainwindow.h" line="711"/>
        <source>Print the Current Report</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="866"/>
        <location filename="build/Skrivbord-Felsu00f6k/temp/ui_mainwindow.h" line="713"/>
        <source>Print Pre&amp;view...</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="869"/>
        <location filename="build/Skrivbord-Felsu00f6k/temp/ui_mainwindow.h" line="715"/>
        <source>Open a Print Preview Dialog for the Current Results</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="877"/>
        <location filename="build/Skrivbord-Felsu00f6k/temp/ui_mainwindow.h" line="719"/>
        <source>Open library editor</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="mainwindow.cpp" line="1048"/>
        <source>Unknown element</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="mainwindow.cpp" line="860"/>
        <source>Select configuration</source>
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
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="mainwindow.cpp" line="1951"/>
        <source>Build dir &apos;%1&apos; does not exist, create it?</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="336"/>
        <location filename="mainwindow.ui" line="339"/>
        <location filename="build/Skrivbord-Felsu00f6k/temp/ui_mainwindow.h" line="582"/>
        <location filename="build/Skrivbord-Felsu00f6k/temp/ui_mainwindow.h" line="584"/>
        <source>Analyze files</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="354"/>
        <location filename="mainwindow.ui" line="357"/>
        <location filename="build/Skrivbord-Felsu00f6k/temp/ui_mainwindow.h" line="590"/>
        <location filename="build/Skrivbord-Felsu00f6k/temp/ui_mainwindow.h" line="592"/>
        <source>Analyze directory</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="369"/>
        <location filename="build/Skrivbord-Felsu00f6k/temp/ui_mainwindow.h" line="597"/>
        <source>&amp;Reanalyze modified files</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="393"/>
        <location filename="mainwindow.ui" line="396"/>
        <location filename="build/Skrivbord-Felsu00f6k/temp/ui_mainwindow.h" line="603"/>
        <location filename="build/Skrivbord-Felsu00f6k/temp/ui_mainwindow.h" line="605"/>
        <source>Stop analysis</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="mainwindow.cpp" line="1725"/>
        <source>XML files (*.xml);;Text files (*.txt);;CSV files (*.csv)</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="mainwindow.cpp" line="702"/>
        <source>No suitable files found to analyze!</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="mainwindow.cpp" line="804"/>
        <source>Select files to analyze</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="mainwindow.cpp" line="819"/>
        <source>Select directory to analyze</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="mainwindow.cpp" line="860"/>
        <source>Select the configuration that will be analyzed</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="mainwindow.cpp" line="912"/>
        <source>Found project files from the directory.

Do you want to proceed analysis without using any of these project files?</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="mainwindow.cpp" line="1644"/>
        <source>Analyzer is running.

Do you want to stop the analysis and exit Cppcheck?</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="209"/>
        <location filename="build/Skrivbord-Felsu00f6k/temp/ui_mainwindow.h" line="745"/>
        <source>A&amp;nalyze</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="225"/>
        <location filename="build/Skrivbord-Felsu00f6k/temp/ui_mainwindow.h" line="747"/>
        <source>&amp;C standard</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="381"/>
        <location filename="build/Skrivbord-Felsu00f6k/temp/ui_mainwindow.h" line="601"/>
        <source>Reanal&amp;yze all files</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="419"/>
        <location filename="build/Skrivbord-Felsu00f6k/temp/ui_mainwindow.h" line="616"/>
        <source>Ctrl+Q</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="449"/>
        <location filename="build/Skrivbord-Felsu00f6k/temp/ui_mainwindow.h" line="620"/>
        <source>Style war&amp;nings</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="467"/>
        <location filename="build/Skrivbord-Felsu00f6k/temp/ui_mainwindow.h" line="625"/>
        <source>E&amp;rrors</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="548"/>
        <location filename="build/Skrivbord-Felsu00f6k/temp/ui_mainwindow.h" line="653"/>
        <source>Ctrl+Shift+O</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="557"/>
        <location filename="build/Skrivbord-Felsu00f6k/temp/ui_mainwindow.h" line="655"/>
        <source>Sh&amp;ow Scratchpad...</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="565"/>
        <location filename="build/Skrivbord-Felsu00f6k/temp/ui_mainwindow.h" line="658"/>
        <source>Ctrl+Shift+N</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="613"/>
        <location filename="build/Skrivbord-Felsu00f6k/temp/ui_mainwindow.h" line="667"/>
        <source>&amp;Warnings</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="631"/>
        <location filename="build/Skrivbord-Felsu00f6k/temp/ui_mainwindow.h" line="672"/>
        <source>Per&amp;formance warnings</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="657"/>
        <location filename="build/Skrivbord-Felsu00f6k/temp/ui_mainwindow.h" line="678"/>
        <source>&amp;Information</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="672"/>
        <location filename="build/Skrivbord-Felsu00f6k/temp/ui_mainwindow.h" line="682"/>
        <source>&amp;Portability</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="690"/>
        <location filename="build/Skrivbord-Felsu00f6k/temp/ui_mainwindow.h" line="687"/>
        <source>Show Cppcheck results</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="702"/>
        <location filename="build/Skrivbord-Felsu00f6k/temp/ui_mainwindow.h" line="689"/>
        <source>Clang</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="705"/>
        <location filename="build/Skrivbord-Felsu00f6k/temp/ui_mainwindow.h" line="691"/>
        <source>Show Clang results</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="772"/>
        <location filename="build/Skrivbord-Felsu00f6k/temp/ui_mainwindow.h" line="702"/>
        <source>P&amp;latforms</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="786"/>
        <location filename="build/Skrivbord-Felsu00f6k/temp/ui_mainwindow.h" line="703"/>
        <source>C++&amp;11</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="797"/>
        <location filename="build/Skrivbord-Felsu00f6k/temp/ui_mainwindow.h" line="704"/>
        <source>C&amp;99</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="805"/>
        <location filename="build/Skrivbord-Felsu00f6k/temp/ui_mainwindow.h" line="705"/>
        <source>&amp;Posix</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="813"/>
        <location filename="build/Skrivbord-Felsu00f6k/temp/ui_mainwindow.h" line="706"/>
        <source>C&amp;11</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="845"/>
        <location filename="build/Skrivbord-Felsu00f6k/temp/ui_mainwindow.h" line="707"/>
        <source>&amp;C89</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="853"/>
        <location filename="build/Skrivbord-Felsu00f6k/temp/ui_mainwindow.h" line="708"/>
        <source>&amp;C++03</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="874"/>
        <location filename="build/Skrivbord-Felsu00f6k/temp/ui_mainwindow.h" line="717"/>
        <source>&amp;Library Editor...</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="885"/>
        <location filename="build/Skrivbord-Felsu00f6k/temp/ui_mainwindow.h" line="721"/>
        <source>&amp;Auto-detect language</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="893"/>
        <location filename="build/Skrivbord-Felsu00f6k/temp/ui_mainwindow.h" line="722"/>
        <source>&amp;Enforce C++</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="901"/>
        <location filename="build/Skrivbord-Felsu00f6k/temp/ui_mainwindow.h" line="723"/>
        <source>E&amp;nforce C</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="912"/>
        <location filename="build/Skrivbord-Felsu00f6k/temp/ui_mainwindow.h" line="724"/>
        <source>C++14</source>
        <translation type="unfinished">C++14</translation>
    </message>
    <message>
        <location filename="mainwindow.cpp" line="2047"/>
        <source>Project files (*.cppcheck)</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="920"/>
        <location filename="build/Skrivbord-Felsu00f6k/temp/ui_mainwindow.h" line="725"/>
        <source>Reanalyze and check library</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="928"/>
        <location filename="build/Skrivbord-Felsu00f6k/temp/ui_mainwindow.h" line="726"/>
        <source>Check configuration (defines, includes)</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="936"/>
        <location filename="build/Skrivbord-Felsu00f6k/temp/ui_mainwindow.h" line="727"/>
        <source>C++17</source>
        <translation type="unfinished">C++17</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="947"/>
        <location filename="build/Skrivbord-Felsu00f6k/temp/ui_mainwindow.h" line="728"/>
        <source>C++20</source>
        <translation type="unfinished">C++20</translation>
    </message>
    <message>
        <location filename="mainwindow.cpp" line="798"/>
        <source>C/C++ Source</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="mainwindow.cpp" line="799"/>
        <source>Compile database</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="mainwindow.cpp" line="1524"/>
        <source>Current results will be cleared.

Opening a new XML file will clear current results.
Do you want to proceed?</source>
        <translation type="unfinished"></translation>
    </message>
</context>
<context>
    <name>NewSuppressionDialog</name>
    <message>
        <location filename="newsuppressiondialog.ui" line="17"/>
        <location filename="build/Skrivbord-Felsu00f6k/temp/ui_newsuppressiondialog.h" line="114"/>
        <source>New suppression</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="newsuppressiondialog.ui" line="25"/>
        <location filename="build/Skrivbord-Felsu00f6k/temp/ui_newsuppressiondialog.h" line="115"/>
        <source>Error ID</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="newsuppressiondialog.ui" line="32"/>
        <location filename="build/Skrivbord-Felsu00f6k/temp/ui_newsuppressiondialog.h" line="116"/>
        <source>File name</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="newsuppressiondialog.ui" line="42"/>
        <location filename="build/Skrivbord-Felsu00f6k/temp/ui_newsuppressiondialog.h" line="117"/>
        <source>Line number</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="newsuppressiondialog.ui" line="52"/>
        <location filename="build/Skrivbord-Felsu00f6k/temp/ui_newsuppressiondialog.h" line="118"/>
        <source>Symbol name</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="newsuppressiondialog.cpp" line="83"/>
        <source>Edit suppression</source>
        <translation type="unfinished"></translation>
    </message>
</context>
<context>
    <name>Platforms</name>
    <message>
        <source>Unix 32-bit</source>
        <translation type="vanished">Unix 32-bit</translation>
    </message>
    <message>
        <source>Unix 64-bit</source>
        <translation type="vanished">Unix 64-bit</translation>
    </message>
    <message>
        <source>Windows 32-bit ANSI</source>
        <translation type="vanished">Windows 32-bit ANSI</translation>
    </message>
    <message>
        <source>Windows 32-bit Unicode</source>
        <translation type="vanished">Windows 32-bit Unicode</translation>
    </message>
    <message>
        <source>Windows 64-bit</source>
        <translation type="vanished">Windows 64-bit</translation>
    </message>
    <message>
        <location filename="platforms.cpp" line="38"/>
        <source>Native</source>
        <translation type="unfinished"></translation>
    </message>
</context>
<context>
    <name>ProjectFile</name>
    <message>
        <location filename="projectfile.ui" line="14"/>
        <location filename="build/Skrivbord-Felsu00f6k/temp/ui_projectfile.h" line="923"/>
        <source>Project File</source>
        <translation>프로젝트 파일</translation>
    </message>
    <message>
        <location filename="projectfile.ui" line="221"/>
        <location filename="build/Skrivbord-Felsu00f6k/temp/ui_projectfile.h" line="936"/>
        <source>Defines:</source>
        <translation>Defines:</translation>
    </message>
    <message>
        <location filename="projectfile.ui" line="147"/>
        <location filename="build/Skrivbord-Felsu00f6k/temp/ui_projectfile.h" line="932"/>
        <source>Paths:</source>
        <translation>경로:</translation>
    </message>
    <message>
        <location filename="projectfile.ui" line="181"/>
        <location filename="projectfile.ui" line="296"/>
        <location filename="build/Skrivbord-Felsu00f6k/temp/ui_projectfile.h" line="933"/>
        <location filename="build/Skrivbord-Felsu00f6k/temp/ui_projectfile.h" line="945"/>
        <source>Add...</source>
        <translation>추가...</translation>
    </message>
    <message>
        <location filename="projectfile.ui" line="188"/>
        <location filename="projectfile.ui" line="303"/>
        <location filename="projectfile.ui" line="699"/>
        <location filename="build/Skrivbord-Felsu00f6k/temp/ui_projectfile.h" line="934"/>
        <location filename="build/Skrivbord-Felsu00f6k/temp/ui_projectfile.h" line="946"/>
        <location filename="build/Skrivbord-Felsu00f6k/temp/ui_projectfile.h" line="994"/>
        <source>Edit</source>
        <translation>편집</translation>
    </message>
    <message>
        <location filename="projectfile.ui" line="195"/>
        <location filename="projectfile.ui" line="310"/>
        <location filename="projectfile.ui" line="706"/>
        <location filename="projectfile.ui" line="749"/>
        <location filename="build/Skrivbord-Felsu00f6k/temp/ui_projectfile.h" line="935"/>
        <location filename="build/Skrivbord-Felsu00f6k/temp/ui_projectfile.h" line="947"/>
        <location filename="build/Skrivbord-Felsu00f6k/temp/ui_projectfile.h" line="995"/>
        <location filename="build/Skrivbord-Felsu00f6k/temp/ui_projectfile.h" line="998"/>
        <source>Remove</source>
        <translation>제거</translation>
    </message>
    <message>
        <location filename="projectfile.ui" line="330"/>
        <location filename="build/Skrivbord-Felsu00f6k/temp/ui_projectfile.h" line="948"/>
        <source>Up</source>
        <translation>위로</translation>
    </message>
    <message>
        <location filename="projectfile.ui" line="337"/>
        <location filename="build/Skrivbord-Felsu00f6k/temp/ui_projectfile.h" line="949"/>
        <source>Down</source>
        <translation>아래로</translation>
    </message>
    <message>
        <location filename="projectfile.ui" line="731"/>
        <location filename="build/Skrivbord-Felsu00f6k/temp/ui_projectfile.h" line="996"/>
        <source>Suppressions</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="projectfile.ui" line="742"/>
        <location filename="build/Skrivbord-Felsu00f6k/temp/ui_projectfile.h" line="997"/>
        <source>Add</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="projectfile.ui" line="417"/>
        <location filename="build/Skrivbord-Felsu00f6k/temp/ui_projectfile.h" line="958"/>
        <source>Note: Put your own custom .cfg files in the same folder as the project file. You should see them above.</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="projectfile.ui" line="899"/>
        <location filename="build/Skrivbord-Felsu00f6k/temp/ui_projectfile.h" line="1015"/>
        <source>...</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="projectfile.ui" line="265"/>
        <location filename="build/Skrivbord-Felsu00f6k/temp/ui_projectfile.h" line="944"/>
        <source>Include Paths:</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="projectfile.ui" line="24"/>
        <location filename="build/Skrivbord-Felsu00f6k/temp/ui_projectfile.h" line="955"/>
        <source>Paths and Defines</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="projectfile.ui" line="73"/>
        <location filename="build/Skrivbord-Felsu00f6k/temp/ui_projectfile.h" line="928"/>
        <source>&lt;html&gt;&lt;head/&gt;&lt;body&gt;&lt;p&gt;You have a choice:&lt;/p&gt;&lt;p&gt; * Analyze all Debug and Release configurations&lt;/p&gt;&lt;p&gt; * Only analyze the first matching Debug configuration&lt;/p&gt;&lt;p&gt;&lt;br/&gt;&lt;/p&gt;&lt;/body&gt;&lt;/html&gt;</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="projectfile.ui" line="76"/>
        <location filename="build/Skrivbord-Felsu00f6k/temp/ui_projectfile.h" line="930"/>
        <source>Analyze all Visual Studio configurations</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="projectfile.ui" line="642"/>
        <location filename="build/Skrivbord-Felsu00f6k/temp/ui_projectfile.h" line="983"/>
        <source>Root path:</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="projectfile.ui" line="658"/>
        <location filename="build/Skrivbord-Felsu00f6k/temp/ui_projectfile.h" line="987"/>
        <source>Warning tags (separated by semicolon)</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="projectfile.ui" line="437"/>
        <location filename="build/Skrivbord-Felsu00f6k/temp/ui_projectfile.h" line="960"/>
        <source>Cppcheck build dir (whole program analysis, incremental analysis, statistics, etc)</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="projectfile.ui" line="113"/>
        <location filename="build/Skrivbord-Felsu00f6k/temp/ui_projectfile.h" line="931"/>
        <source>Selected VS Configurations</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="projectfile.ui" line="350"/>
        <location filename="build/Skrivbord-Felsu00f6k/temp/ui_projectfile.h" line="950"/>
        <source>Include file</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="projectfile.ui" line="360"/>
        <location filename="build/Skrivbord-Felsu00f6k/temp/ui_projectfile.h" line="952"/>
        <source>&lt;html&gt;&lt;head/&gt;&lt;body&gt;&lt;p&gt;&lt;span style=&quot; font-family:&apos;monospace&apos;; color:#000000; background-color:#ffffff;&quot;&gt;Force inclusion of a header file&lt;/span&gt;&lt;/p&gt;&lt;/body&gt;&lt;/html&gt;</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="projectfile.ui" line="367"/>
        <location filename="build/Skrivbord-Felsu00f6k/temp/ui_projectfile.h" line="954"/>
        <source>Browse..</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="projectfile.ui" line="390"/>
        <location filename="build/Skrivbord-Felsu00f6k/temp/ui_projectfile.h" line="959"/>
        <source>Types and Functions</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="projectfile.ui" line="408"/>
        <location filename="build/Skrivbord-Felsu00f6k/temp/ui_projectfile.h" line="957"/>
        <source>Libraries</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="projectfile.ui" line="460"/>
        <location filename="build/Skrivbord-Felsu00f6k/temp/ui_projectfile.h" line="965"/>
        <source>Parser</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="projectfile.ui" line="466"/>
        <location filename="build/Skrivbord-Felsu00f6k/temp/ui_projectfile.h" line="966"/>
        <source>Cppcheck (built in)</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="projectfile.ui" line="525"/>
        <location filename="build/Skrivbord-Felsu00f6k/temp/ui_projectfile.h" line="976"/>
        <source>Check that each class has a safe public interface</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="projectfile.ui" line="541"/>
        <location filename="build/Skrivbord-Felsu00f6k/temp/ui_projectfile.h" line="977"/>
        <source>Limit analysis</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="projectfile.ui" line="795"/>
        <location filename="projectfile.ui" line="801"/>
        <location filename="build/Skrivbord-Felsu00f6k/temp/ui_projectfile.h" line="1001"/>
        <location filename="build/Skrivbord-Felsu00f6k/temp/ui_projectfile.h" line="1028"/>
        <source>Addons</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="projectfile.ui" line="807"/>
        <location filename="build/Skrivbord-Felsu00f6k/temp/ui_projectfile.h" line="1002"/>
        <source>Note: Addons require &lt;a href=&quot;https://www.python.org/&quot;&gt;Python&lt;/a&gt; being installed.</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="projectfile.ui" line="817"/>
        <location filename="build/Skrivbord-Felsu00f6k/temp/ui_projectfile.h" line="1003"/>
        <source>Y2038</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="projectfile.ui" line="824"/>
        <location filename="build/Skrivbord-Felsu00f6k/temp/ui_projectfile.h" line="1004"/>
        <source>Thread safety</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="projectfile.ui" line="834"/>
        <location filename="build/Skrivbord-Felsu00f6k/temp/ui_projectfile.h" line="1005"/>
        <source>Coding standards</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="projectfile.ui" line="946"/>
        <location filename="build/Skrivbord-Felsu00f6k/temp/ui_projectfile.h" line="1020"/>
        <source>Cert C</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="projectfile.ui" line="955"/>
        <location filename="build/Skrivbord-Felsu00f6k/temp/ui_projectfile.h" line="1021"/>
        <source>CERT-INT35-C:  int precision (if size equals precision, you can leave empty)</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="projectfile.ui" line="974"/>
        <location filename="build/Skrivbord-Felsu00f6k/temp/ui_projectfile.h" line="1023"/>
        <source>Autosar</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="projectfile.ui" line="997"/>
        <location filename="build/Skrivbord-Felsu00f6k/temp/ui_projectfile.h" line="1026"/>
        <source>Bug hunting</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="projectfile.ui" line="60"/>
        <location filename="projectfile.ui" line="450"/>
        <location filename="build/Skrivbord-Felsu00f6k/temp/ui_projectfile.h" line="926"/>
        <location filename="build/Skrivbord-Felsu00f6k/temp/ui_projectfile.h" line="964"/>
        <source>Browse...</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="projectfile.ui" line="231"/>
        <location filename="build/Skrivbord-Felsu00f6k/temp/ui_projectfile.h" line="938"/>
        <source>Defines must be separated by a semicolon. Example: DEF1;DEF2=5;DEF3=int</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="projectfile.ui" line="396"/>
        <location filename="build/Skrivbord-Felsu00f6k/temp/ui_projectfile.h" line="956"/>
        <source>Platform</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="projectfile.ui" line="443"/>
        <location filename="build/Skrivbord-Felsu00f6k/temp/ui_projectfile.h" line="962"/>
        <source>This is a workfolder that Cppcheck will use for various purposes.</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="projectfile.ui" line="476"/>
        <location filename="build/Skrivbord-Felsu00f6k/temp/ui_projectfile.h" line="967"/>
        <source>Clang (experimental)</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="projectfile.ui" line="486"/>
        <location filename="build/Skrivbord-Felsu00f6k/temp/ui_projectfile.h" line="968"/>
        <source>Check level</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="projectfile.ui" line="492"/>
        <location filename="build/Skrivbord-Felsu00f6k/temp/ui_projectfile.h" line="969"/>
        <source>Reduced -- meant for usage where developer wants results directly. Limited and faster analysis with fewer results.</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="projectfile.ui" line="499"/>
        <location filename="build/Skrivbord-Felsu00f6k/temp/ui_projectfile.h" line="970"/>
        <source>Normal -- meant for normal analysis in CI. Analysis should finish in reasonable time.</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="projectfile.ui" line="506"/>
        <location filename="build/Skrivbord-Felsu00f6k/temp/ui_projectfile.h" line="971"/>
        <source>Exhaustive -- meant for nightly builds etc. Analysis time can be longer (10x slower than compilation is OK).</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="projectfile.ui" line="522"/>
        <location filename="build/Skrivbord-Felsu00f6k/temp/ui_projectfile.h" line="974"/>
        <source>If you want to design your classes to be as flexible and robust as possible then the public interface must be very robust. Cppcheck will asumme that arguments can take *any* value.</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="projectfile.ui" line="547"/>
        <location filename="build/Skrivbord-Felsu00f6k/temp/ui_projectfile.h" line="978"/>
        <source>Check code in headers  (should be ON normally. if you want a limited quick analysis then turn this OFF)</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="projectfile.ui" line="600"/>
        <location filename="build/Skrivbord-Felsu00f6k/temp/ui_projectfile.h" line="981"/>
        <source>Max recursion in template instantiation</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="projectfile.ui" line="636"/>
        <location filename="build/Skrivbord-Felsu00f6k/temp/ui_projectfile.h" line="1000"/>
        <source>Warning options</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="projectfile.ui" line="648"/>
        <location filename="build/Skrivbord-Felsu00f6k/temp/ui_projectfile.h" line="985"/>
        <source>Filepaths in warnings will be relative to this path</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="projectfile.ui" line="664"/>
        <location filename="build/Skrivbord-Felsu00f6k/temp/ui_projectfile.h" line="989"/>
        <source>If tags are added, you will be able to right click on warnings and set one of these tags. You can manually categorize warnings.</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="projectfile.ui" line="674"/>
        <location filename="build/Skrivbord-Felsu00f6k/temp/ui_projectfile.h" line="991"/>
        <source>Exclude source files</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="projectfile.ui" line="685"/>
        <location filename="build/Skrivbord-Felsu00f6k/temp/ui_projectfile.h" line="992"/>
        <source>Exclude folder...</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="projectfile.ui" line="692"/>
        <location filename="build/Skrivbord-Felsu00f6k/temp/ui_projectfile.h" line="993"/>
        <source>Exclude file...</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="projectfile.ui" line="774"/>
        <location filename="build/Skrivbord-Felsu00f6k/temp/ui_projectfile.h" line="999"/>
        <source>Enable inline suppressions</source>
        <translation type="unfinished">Inline suppression 사용</translation>
    </message>
    <message>
        <location filename="projectfile.ui" line="842"/>
        <location filename="build/Skrivbord-Felsu00f6k/temp/ui_projectfile.h" line="1006"/>
        <source>Misra C</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="projectfile.ui" line="850"/>
        <location filename="build/Skrivbord-Felsu00f6k/temp/ui_projectfile.h" line="1007"/>
        <source>2012</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="projectfile.ui" line="855"/>
        <location filename="projectfile.ui" line="923"/>
        <location filename="build/Skrivbord-Felsu00f6k/temp/ui_projectfile.h" line="1008"/>
        <location filename="build/Skrivbord-Felsu00f6k/temp/ui_projectfile.h" line="1018"/>
        <source>2023</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="projectfile.ui" line="860"/>
        <location filename="build/Skrivbord-Felsu00f6k/temp/ui_projectfile.h" line="1009"/>
        <source>2025</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="projectfile.ui" line="885"/>
        <location filename="build/Skrivbord-Felsu00f6k/temp/ui_projectfile.h" line="1011"/>
        <source>MISRA rule texts</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="projectfile.ui" line="892"/>
        <location filename="build/Skrivbord-Felsu00f6k/temp/ui_projectfile.h" line="1013"/>
        <source>&lt;html&gt;&lt;head/&gt;&lt;body&gt;&lt;p&gt;Copy/paste the text from Appendix A &amp;quot;Summary of guidelines&amp;quot; from the MISRA C 2012 pdf to a text file.&lt;/p&gt;&lt;/body&gt;&lt;/html&gt;</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="projectfile.ui" line="910"/>
        <location filename="build/Skrivbord-Felsu00f6k/temp/ui_projectfile.h" line="1016"/>
        <source>Misra C++</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="projectfile.ui" line="918"/>
        <location filename="build/Skrivbord-Felsu00f6k/temp/ui_projectfile.h" line="1017"/>
        <source>2008</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="projectfile.ui" line="967"/>
        <location filename="build/Skrivbord-Felsu00f6k/temp/ui_projectfile.h" line="1022"/>
        <source>Cert C++</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="projectfile.ui" line="981"/>
        <location filename="build/Skrivbord-Felsu00f6k/temp/ui_projectfile.h" line="1024"/>
        <source>Safety profiles (defined in C++ core guidelines)</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="projectfile.ui" line="991"/>
        <location filename="build/Skrivbord-Felsu00f6k/temp/ui_projectfile.h" line="1025"/>
        <source>Bug hunting (Premium)</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="projectfile.ui" line="1007"/>
        <location filename="build/Skrivbord-Felsu00f6k/temp/ui_projectfile.h" line="1027"/>
        <source>External tools</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="projectfile.ui" line="30"/>
        <location filename="build/Skrivbord-Felsu00f6k/temp/ui_projectfile.h" line="924"/>
        <source>Import Project (Visual studio / compile database/ Borland C++ Builder 6)</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="projectfile.ui" line="242"/>
        <location filename="build/Skrivbord-Felsu00f6k/temp/ui_projectfile.h" line="940"/>
        <source>Undefines:</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="projectfile.ui" line="252"/>
        <location filename="build/Skrivbord-Felsu00f6k/temp/ui_projectfile.h" line="942"/>
        <source>Undefines must be separated by a semicolon. Example: UNDEF1;UNDEF2;UNDEF3</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="projectfile.ui" line="431"/>
        <location filename="projectfile.ui" line="516"/>
        <location filename="build/Skrivbord-Felsu00f6k/temp/ui_projectfile.h" line="972"/>
        <location filename="build/Skrivbord-Felsu00f6k/temp/ui_projectfile.h" line="982"/>
        <source>Analysis</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="projectfile.ui" line="557"/>
        <location filename="build/Skrivbord-Felsu00f6k/temp/ui_projectfile.h" line="979"/>
        <source>Check code in unused templates (should be ON normally, however in theory you can safely ignore warnings in unused templates)</source>
        <oldsource>Check code in unused templates  (slower and less accurate analysis)</oldsource>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="projectfile.ui" line="569"/>
        <location filename="build/Skrivbord-Felsu00f6k/temp/ui_projectfile.h" line="980"/>
        <source>Max CTU depth</source>
        <translation type="unfinished"></translation>
    </message>
</context>
<context>
    <name>ProjectFileDialog</name>
    <message>
        <location filename="projectfiledialog.cpp" line="137"/>
        <source>Project file: %1</source>
        <translation>프로젝트 파일: %1</translation>
    </message>
    <message>
        <location filename="projectfiledialog.cpp" line="861"/>
        <source>Select include directory</source>
        <translation>Include 디렉토리 선택</translation>
    </message>
    <message>
        <location filename="projectfiledialog.cpp" line="841"/>
        <source>Select a directory to check</source>
        <translation>검사할 디렉토리 선택</translation>
    </message>
    <message>
        <location filename="projectfiledialog.cpp" line="881"/>
        <source>Select directory to ignore</source>
        <translation>무시할 디렉토리 선택</translation>
    </message>
    <message>
        <location filename="projectfiledialog.cpp" line="585"/>
        <source>Select Cppcheck build dir</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="projectfiledialog.cpp" line="627"/>
        <source>Import Project</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="projectfiledialog.cpp" line="460"/>
        <source>Clang-tidy (not found)</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="projectfiledialog.cpp" line="395"/>
        <source>Note: Open source Cppcheck does not fully implement Misra C 2012</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="projectfiledialog.cpp" line="889"/>
        <source>Source files</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="projectfiledialog.cpp" line="647"/>
        <location filename="projectfiledialog.cpp" line="890"/>
        <source>All files</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="projectfiledialog.cpp" line="646"/>
        <source>C/C++ header</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="projectfiledialog.cpp" line="648"/>
        <source>Include file</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="projectfiledialog.cpp" line="891"/>
        <source>Exclude file</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="projectfiledialog.cpp" line="976"/>
        <source>Select MISRA rule texts file</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="projectfiledialog.cpp" line="978"/>
        <source>MISRA rule texts file (%1)</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="projectfiledialog.cpp" line="625"/>
        <source>Compile database</source>
        <translation type="unfinished"></translation>
    </message>
</context>
<context>
    <name>QObject</name>
    <message>
        <location filename="translationhandler.cpp" line="91"/>
        <source>Unknown language specified!</source>
        <translation>알 수 없는 언어입니다!</translation>
    </message>
    <message>
        <location filename="translationhandler.cpp" line="117"/>
        <source>Language file %1 not found!</source>
        <translation>언어 파일(%1)이 없습니다!</translation>
    </message>
    <message>
        <location filename="translationhandler.cpp" line="122"/>
        <source>Failed to load translation for language %1 from file %2</source>
        <translation>파일(%2)로부터 언어(%1) 불러오기 실패</translation>
    </message>
    <message>
        <location filename="cppchecklibrarydata.cpp" line="38"/>
        <source>line %1: Unhandled element %2</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="cppchecklibrarydata.cpp" line="43"/>
        <source>line %1: Mandatory attribute &apos;%2&apos; missing in &apos;%3&apos;</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="projectfiledialog.cpp" line="282"/>
        <source> (Not found)</source>
        <translation type="unfinished"></translation>
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
        <location filename="codeeditstyledialog.cpp" line="101"/>
        <source>Class Foreground Color</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="resultstree.cpp" line="113"/>
        <source>File</source>
        <translation type="unfinished">파일</translation>
    </message>
    <message>
        <location filename="resultstree.cpp" line="114"/>
        <source>Line</source>
        <translation type="unfinished">행</translation>
    </message>
    <message>
        <location filename="resultstree.cpp" line="115"/>
        <source>Severity</source>
        <translation type="unfinished">분류</translation>
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
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="resultstree.cpp" line="119"/>
        <source>Summary</source>
        <translation type="unfinished">요약</translation>
    </message>
    <message>
        <location filename="resultstree.cpp" line="120"/>
        <source>Id</source>
        <translation type="unfinished"></translation>
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
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="resultstree.cpp" line="124"/>
        <source>Tags</source>
        <translation type="unfinished"></translation>
    </message>
</context>
<context>
    <name>QPlatformTheme</name>
    <message>
        <location filename="translationhandler.cpp" line="41"/>
        <source>OK</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="translationhandler.cpp" line="42"/>
        <source>Cancel</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="translationhandler.cpp" line="43"/>
        <source>Close</source>
        <translation type="unfinished">닫기</translation>
    </message>
    <message>
        <location filename="translationhandler.cpp" line="44"/>
        <source>Save</source>
        <translation type="unfinished"></translation>
    </message>
</context>
<context>
    <name>ResultsTree</name>
    <message>
        <source>File</source>
        <translation type="vanished">파일</translation>
    </message>
    <message>
        <source>Severity</source>
        <translation type="vanished">분류</translation>
    </message>
    <message>
        <source>Line</source>
        <translation type="vanished">행</translation>
    </message>
    <message>
        <source>Summary</source>
        <translation type="vanished">요약</translation>
    </message>
    <message>
        <source>Undefined file</source>
        <translation type="vanished">미정의된 파일</translation>
    </message>
    <message>
        <location filename="resultstree.cpp" line="355"/>
        <source>style</source>
        <translation>스타일</translation>
    </message>
    <message>
        <location filename="resultstree.cpp" line="358"/>
        <source>error</source>
        <translation>에러</translation>
    </message>
    <message>
        <location filename="resultstree.cpp" line="361"/>
        <source>warning</source>
        <translation>경고</translation>
    </message>
    <message>
        <location filename="resultstree.cpp" line="364"/>
        <source>performance</source>
        <translation>성능</translation>
    </message>
    <message>
        <location filename="resultstree.cpp" line="367"/>
        <source>portability</source>
        <translation>이식성</translation>
    </message>
    <message>
        <location filename="resultstree.cpp" line="370"/>
        <source>information</source>
        <translation>정보</translation>
    </message>
    <message>
        <location filename="resultstree.cpp" line="373"/>
        <source>debug</source>
        <translation>디버그</translation>
    </message>
    <message>
        <location filename="resultstree.cpp" line="376"/>
        <source>internal</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Hide</source>
        <translation type="vanished">숨기기</translation>
    </message>
    <message>
        <source>Cppcheck</source>
        <translation type="vanished">Cppcheck</translation>
    </message>
    <message>
        <location filename="resultstree.cpp" line="745"/>
        <source>No editor application configured.

Configure the editor application for Cppcheck in preferences/Applications.</source>
        <translation>편집기 미설정.

[설정 - 응용 프로그램]에서 편집기를 설정하세요.</translation>
    </message>
    <message>
        <location filename="resultstree.cpp" line="759"/>
        <source>No default editor application selected.

Please select the default editor application in preferences/Applications.</source>
        <translation>기본 편집기 미선택.

[설정 - 응용 프로그램]에서 기본 편집기를 선택하세요.</translation>
    </message>
    <message>
        <location filename="resultstree.cpp" line="779"/>
        <source>Could not find the file!</source>
        <translation>파일을 찾을 수 없습니다!</translation>
    </message>
    <message>
        <location filename="resultstree.cpp" line="822"/>
        <source>Could not start %1

Please check the application path and parameters are correct.</source>
        <translation>%1을 시잘할 수 없습니다

경로와 인자가 정확한지 확인하세요.</translation>
    </message>
    <message>
        <location filename="resultstree.cpp" line="844"/>
        <source>Select Directory</source>
        <translation>디렉토리 선택</translation>
    </message>
    <message>
        <location filename="resultstree.cpp" line="670"/>
        <source>Hide all with id</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="resultstree.cpp" line="671"/>
        <source>Open containing folder</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="resultstree.cpp" line="307"/>
        <source>note</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="resultstree.cpp" line="667"/>
        <source>Recheck %1 file(s)</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="resultstree.cpp" line="669"/>
        <source>Hide %1 result(s)</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="resultstree.cpp" line="692"/>
        <source>Suppress selected id(s)</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="resultstree.cpp" line="712"/>
        <source>Tag</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="resultstree.cpp" line="714"/>
        <source>No tag</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="resultstree.cpp" line="836"/>
        <source>Could not find file:</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="resultstree.cpp" line="840"/>
        <source>Please select the folder &apos;%1&apos;</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="resultstree.cpp" line="841"/>
        <source>Select Directory &apos;%1&apos;</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="resultstree.cpp" line="843"/>
        <source>Please select the directory where file is located.</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="resultstree.cpp" line="668"/>
        <source>Copy</source>
        <translation type="unfinished"></translation>
    </message>
</context>
<context>
    <name>ResultsView</name>
    <message>
        <location filename="resultsview.ui" line="26"/>
        <location filename="build/Skrivbord-Felsu00f6k/temp/ui_resultsview.h" line="147"/>
        <source>Results</source>
        <translation>결과</translation>
    </message>
    <message>
        <location filename="resultsview.cpp" line="236"/>
        <location filename="resultsview.cpp" line="244"/>
        <source>Failed to save the report.</source>
        <translation>결과 저장 실패.</translation>
    </message>
    <message>
        <location filename="resultsview.cpp" line="324"/>
        <source>%p% (%1 of %2 files checked)</source>
        <translation>%p% (%2 중 %1 파일 검사됨)</translation>
    </message>
    <message>
        <source>Cppcheck</source>
        <translation type="vanished">Cppcheck</translation>
    </message>
    <message>
        <location filename="resultsview.cpp" line="349"/>
        <source>No errors found.</source>
        <translation>에러가 발견되지 않았습니다.</translation>
    </message>
    <message>
        <location filename="resultsview.cpp" line="356"/>
        <source>Errors were found, but they are configured to be hidden.
To toggle what kind of errors are shown, open view menu.</source>
        <translation>에러가 발견되었지만, 감추도록 설정되어 있습니다.
에러 종류를 표시하도록 설정하려면, 보기 메뉴를 선택하세요.</translation>
    </message>
    <message>
        <location filename="resultsview.cpp" line="398"/>
        <location filename="resultsview.cpp" line="417"/>
        <source>Failed to read the report.</source>
        <translation>결과 불러오기 실패.</translation>
    </message>
    <message>
        <location filename="resultsview.cpp" line="545"/>
        <source>Analysis was stopped</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="resultsview.cpp" line="559"/>
        <source>There was a critical error with id &apos;%1&apos;</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="resultsview.cpp" line="561"/>
        <source>when checking %1</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="resultsview.cpp" line="563"/>
        <source>when checking a file</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="resultsview.cpp" line="564"/>
        <source>Analysis was aborted.</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="resultsview.cpp" line="461"/>
        <source>Id</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="resultsview.cpp" line="254"/>
        <source>Print Report</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="resultsview.cpp" line="273"/>
        <source>No errors found, nothing to print.</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="resultsview.cpp" line="456"/>
        <source>First included by</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="resultsview.cpp" line="405"/>
        <source>XML format version 1 is no longer supported.</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="resultsview.ui" line="60"/>
        <location filename="build/Skrivbord-Felsu00f6k/temp/ui_resultsview.h" line="148"/>
        <source>Critical errors</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="resultsview.ui" line="92"/>
        <location filename="build/Skrivbord-Felsu00f6k/temp/ui_resultsview.h" line="149"/>
        <source>Analysis Log</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="resultsview.ui" line="114"/>
        <location filename="build/Skrivbord-Felsu00f6k/temp/ui_resultsview.h" line="150"/>
        <source>Warning Details</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="resultsview.cpp" line="536"/>
        <source>Clear Log</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="resultsview.cpp" line="537"/>
        <source>Copy this Log entry</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="resultsview.cpp" line="538"/>
        <source>Copy complete Log</source>
        <translation type="unfinished"></translation>
    </message>
</context>
<context>
    <name>ScratchPad</name>
    <message>
        <location filename="scratchpad.ui" line="14"/>
        <location filename="build/Skrivbord-Felsu00f6k/temp/ui_scratchpad.h" line="107"/>
        <source>Scratchpad</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="scratchpad.ui" line="71"/>
        <location filename="build/Skrivbord-Felsu00f6k/temp/ui_scratchpad.h" line="110"/>
        <source>filename</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="scratchpad.ui" line="78"/>
        <location filename="build/Skrivbord-Felsu00f6k/temp/ui_scratchpad.h" line="111"/>
        <source>Check</source>
        <translation type="unfinished">검사</translation>
    </message>
    <message>
        <location filename="scratchpad.ui" line="20"/>
        <location filename="build/Skrivbord-Felsu00f6k/temp/ui_scratchpad.h" line="108"/>
        <source>Copy or write some C/C++ code here:</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="scratchpad.ui" line="37"/>
        <location filename="build/Skrivbord-Felsu00f6k/temp/ui_scratchpad.h" line="109"/>
        <source>Optionally enter a filename (mainly for automatic language detection) and click on &quot;Check&quot;:</source>
        <translation type="unfinished"></translation>
    </message>
</context>
<context>
    <name>Settings</name>
    <message>
        <location filename="settings.ui" line="14"/>
        <location filename="build/Skrivbord-Felsu00f6k/temp/ui_settings.h" line="490"/>
        <source>Preferences</source>
        <translation>설정</translation>
    </message>
    <message>
        <location filename="settings.ui" line="24"/>
        <location filename="build/Skrivbord-Felsu00f6k/temp/ui_settings.h" line="502"/>
        <source>General</source>
        <translation>일반</translation>
    </message>
    <message>
        <location filename="settings.ui" line="41"/>
        <location filename="build/Skrivbord-Felsu00f6k/temp/ui_settings.h" line="491"/>
        <source>Number of threads: </source>
        <translation>쓰레드 수: </translation>
    </message>
    <message>
        <source>Ideal count:</source>
        <translation type="vanished">최적 값:</translation>
    </message>
    <message>
        <location filename="settings.ui" line="108"/>
        <location filename="build/Skrivbord-Felsu00f6k/temp/ui_settings.h" line="493"/>
        <source>Force checking all #ifdef configurations</source>
        <translation>모든 #ifdef 설정을 강제로 검사</translation>
    </message>
    <message>
        <location filename="settings.ui" line="115"/>
        <location filename="build/Skrivbord-Felsu00f6k/temp/ui_settings.h" line="494"/>
        <source>Show full path of files</source>
        <translation>파일의 전체 경로 표시</translation>
    </message>
    <message>
        <location filename="settings.ui" line="122"/>
        <location filename="build/Skrivbord-Felsu00f6k/temp/ui_settings.h" line="495"/>
        <source>Show &quot;No errors found&quot; message when no errors found</source>
        <translation>에러가 발견되지 않는 경우 &quot;에러가 없습니다.&quot; 메시지 표시</translation>
    </message>
    <message>
        <location filename="settings.ui" line="136"/>
        <location filename="build/Skrivbord-Felsu00f6k/temp/ui_settings.h" line="497"/>
        <source>Enable inline suppressions</source>
        <translation>Inline suppression 사용</translation>
    </message>
    <message>
        <location filename="settings.ui" line="157"/>
        <location filename="build/Skrivbord-Felsu00f6k/temp/ui_settings.h" line="500"/>
        <source>Check for updates</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="settings.ui" line="196"/>
        <location filename="build/Skrivbord-Felsu00f6k/temp/ui_settings.h" line="503"/>
        <source>Add...</source>
        <translation>추가...</translation>
    </message>
    <message>
        <location filename="settings.ui" line="210"/>
        <location filename="build/Skrivbord-Felsu00f6k/temp/ui_settings.h" line="505"/>
        <source>Remove</source>
        <translation>제거</translation>
    </message>
    <message>
        <location filename="settings.ui" line="185"/>
        <location filename="build/Skrivbord-Felsu00f6k/temp/ui_settings.h" line="507"/>
        <source>Applications</source>
        <translation>응용 프로그램</translation>
    </message>
    <message>
        <location filename="settings.ui" line="203"/>
        <location filename="settings.ui" line="479"/>
        <location filename="build/Skrivbord-Felsu00f6k/temp/ui_settings.h" line="504"/>
        <location filename="build/Skrivbord-Felsu00f6k/temp/ui_settings.h" line="532"/>
        <source>Edit...</source>
        <translation>편집...</translation>
    </message>
    <message>
        <location filename="settings.ui" line="217"/>
        <location filename="build/Skrivbord-Felsu00f6k/temp/ui_settings.h" line="506"/>
        <source>Set as default</source>
        <translation>기본으로 지정</translation>
    </message>
    <message>
        <location filename="settings.ui" line="240"/>
        <location filename="build/Skrivbord-Felsu00f6k/temp/ui_settings.h" line="510"/>
        <source>Reports</source>
        <translation>보고서</translation>
    </message>
    <message>
        <location filename="settings.ui" line="246"/>
        <location filename="build/Skrivbord-Felsu00f6k/temp/ui_settings.h" line="508"/>
        <source>Save all errors when creating report</source>
        <translation>보고서 생성 시 모든 에러 저장</translation>
    </message>
    <message>
        <location filename="settings.ui" line="253"/>
        <location filename="build/Skrivbord-Felsu00f6k/temp/ui_settings.h" line="509"/>
        <source>Save full path to files in reports</source>
        <translation>보고서에 파일의 전체 경로 저장</translation>
    </message>
    <message>
        <location filename="settings.ui" line="274"/>
        <location filename="build/Skrivbord-Felsu00f6k/temp/ui_settings.h" line="511"/>
        <source>Language</source>
        <translation>언어</translation>
    </message>
    <message>
        <location filename="settings.ui" line="129"/>
        <location filename="build/Skrivbord-Felsu00f6k/temp/ui_settings.h" line="496"/>
        <source>Display error Id in column &quot;Id&quot;</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="settings.ui" line="143"/>
        <location filename="build/Skrivbord-Felsu00f6k/temp/ui_settings.h" line="498"/>
        <source>Check for inconclusive errors also</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="settings.ui" line="177"/>
        <location filename="build/Skrivbord-Felsu00f6k/temp/ui_settings.h" line="501"/>
        <source>Show internal warnings in log</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="settings.ui" line="150"/>
        <location filename="build/Skrivbord-Felsu00f6k/temp/ui_settings.h" line="499"/>
        <source>Show statistics on check completion</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="settings.ui" line="79"/>
        <location filename="build/Skrivbord-Felsu00f6k/temp/ui_settings.h" line="492"/>
        <source>Max count:</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="settings.ui" line="288"/>
        <location filename="build/Skrivbord-Felsu00f6k/temp/ui_settings.h" line="521"/>
        <source>Addons</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="settings.ui" line="294"/>
        <location filename="build/Skrivbord-Felsu00f6k/temp/ui_settings.h" line="512"/>
        <source>Python binary (leave this empty to use python in the PATH)</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="settings.ui" line="305"/>
        <location filename="settings.ui" line="346"/>
        <location filename="settings.ui" line="391"/>
        <location filename="build/Skrivbord-Felsu00f6k/temp/ui_settings.h" line="513"/>
        <location filename="build/Skrivbord-Felsu00f6k/temp/ui_settings.h" line="520"/>
        <location filename="build/Skrivbord-Felsu00f6k/temp/ui_settings.h" line="523"/>
        <source>...</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="settings.ui" line="372"/>
        <location filename="build/Skrivbord-Felsu00f6k/temp/ui_settings.h" line="526"/>
        <source>Clang</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="settings.ui" line="378"/>
        <location filename="build/Skrivbord-Felsu00f6k/temp/ui_settings.h" line="522"/>
        <source>Clang path (leave empty to use system PATH)</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="settings.ui" line="401"/>
        <location filename="build/Skrivbord-Felsu00f6k/temp/ui_settings.h" line="524"/>
        <source>Visual Studio headers</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="settings.ui" line="407"/>
        <location filename="build/Skrivbord-Felsu00f6k/temp/ui_settings.h" line="525"/>
        <source>&lt;html&gt;&lt;head/&gt;&lt;body&gt;&lt;p&gt;Paths to Visual Studio headers, separated by semicolon &apos;;&apos;.&lt;/p&gt;&lt;p&gt;You can open a Visual Studio command prompt, write &amp;quot;SET INCLUDE&amp;quot;. Then copy/paste the paths.&lt;/p&gt;&lt;/body&gt;&lt;/html&gt;</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="settings.ui" line="324"/>
        <location filename="build/Skrivbord-Felsu00f6k/temp/ui_settings.h" line="515"/>
        <source>MISRA addon</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="settings.ui" line="332"/>
        <location filename="build/Skrivbord-Felsu00f6k/temp/ui_settings.h" line="516"/>
        <source>MISRA rule texts file</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="settings.ui" line="339"/>
        <location filename="build/Skrivbord-Felsu00f6k/temp/ui_settings.h" line="518"/>
        <source>&lt;html&gt;&lt;head/&gt;&lt;body&gt;&lt;p&gt;Copy/paste the text from Appendix A &amp;quot;Summary of guidelines&amp;quot; from the MISRA C 2012 pdf to a text file.&lt;/p&gt;&lt;/body&gt;&lt;/html&gt;</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="settings.ui" line="437"/>
        <location filename="build/Skrivbord-Felsu00f6k/temp/ui_settings.h" line="533"/>
        <source>Code Editor</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="settings.ui" line="443"/>
        <location filename="build/Skrivbord-Felsu00f6k/temp/ui_settings.h" line="527"/>
        <source>Code Editor Style</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="settings.ui" line="456"/>
        <location filename="build/Skrivbord-Felsu00f6k/temp/ui_settings.h" line="529"/>
        <source>Default Light Style</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="settings.ui" line="463"/>
        <location filename="build/Skrivbord-Felsu00f6k/temp/ui_settings.h" line="530"/>
        <source>Default Dark Style</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="settings.ui" line="472"/>
        <location filename="build/Skrivbord-Felsu00f6k/temp/ui_settings.h" line="531"/>
        <source>Custom</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="settings.ui" line="449"/>
        <location filename="build/Skrivbord-Felsu00f6k/temp/ui_settings.h" line="528"/>
        <source>System Style</source>
        <translation type="unfinished"></translation>
    </message>
</context>
<context>
    <name>SettingsDialog</name>
    <message>
        <source>N/A</source>
        <translation type="vanished">N/A</translation>
    </message>
    <message>
        <location filename="settingsdialog.cpp" line="232"/>
        <source>The executable file &quot;%1&quot; is not available</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="settingsdialog.cpp" line="243"/>
        <source>Add a new application</source>
        <translation>새 응용 프로그램 추가</translation>
    </message>
    <message>
        <location filename="settingsdialog.cpp" line="273"/>
        <source>Modify an application</source>
        <translation>응용 프로그램 편집</translation>
    </message>
    <message>
        <location filename="settingsdialog.cpp" line="303"/>
        <source>[Default]</source>
        <translation>[기본]</translation>
    </message>
    <message>
        <location filename="settingsdialog.cpp" line="278"/>
        <source> [Default]</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="settingsdialog.cpp" line="358"/>
        <source>Select python binary</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="settingsdialog.cpp" line="396"/>
        <source>Select clang path</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="settingsdialog.cpp" line="365"/>
        <source>Select MISRA File</source>
        <translation type="unfinished"></translation>
    </message>
</context>
<context>
    <name>StatsDialog</name>
    <message>
        <location filename="statsdialog.ui" line="14"/>
        <location filename="statsdialog.ui" line="248"/>
        <location filename="build/Skrivbord-Felsu00f6k/temp/ui_statsdialog.h" line="395"/>
        <location filename="build/Skrivbord-Felsu00f6k/temp/ui_statsdialog.h" line="415"/>
        <location filename="statsdialog.cpp" line="184"/>
        <location filename="statsdialog.cpp" line="231"/>
        <source>Statistics</source>
        <translation>통계</translation>
    </message>
    <message>
        <location filename="statsdialog.ui" line="27"/>
        <location filename="build/Skrivbord-Felsu00f6k/temp/ui_statsdialog.h" line="401"/>
        <location filename="statsdialog.cpp" line="222"/>
        <source>Project</source>
        <translation>프로젝트</translation>
    </message>
    <message>
        <location filename="statsdialog.ui" line="33"/>
        <location filename="build/Skrivbord-Felsu00f6k/temp/ui_statsdialog.h" line="396"/>
        <source>Project:</source>
        <translation>프로젝트:</translation>
    </message>
    <message>
        <location filename="statsdialog.ui" line="53"/>
        <location filename="build/Skrivbord-Felsu00f6k/temp/ui_statsdialog.h" line="397"/>
        <source>Paths:</source>
        <translation>경로:</translation>
    </message>
    <message>
        <location filename="statsdialog.ui" line="85"/>
        <location filename="build/Skrivbord-Felsu00f6k/temp/ui_statsdialog.h" line="398"/>
        <source>Include paths:</source>
        <translation>Include 경로:</translation>
    </message>
    <message>
        <location filename="statsdialog.ui" line="108"/>
        <location filename="build/Skrivbord-Felsu00f6k/temp/ui_statsdialog.h" line="399"/>
        <source>Defines:</source>
        <translation>Defines:</translation>
    </message>
    <message>
        <location filename="statsdialog.ui" line="165"/>
        <location filename="build/Skrivbord-Felsu00f6k/temp/ui_statsdialog.h" line="407"/>
        <location filename="statsdialog.cpp" line="227"/>
        <source>Previous Scan</source>
        <translation>직전 검사</translation>
    </message>
    <message>
        <location filename="statsdialog.ui" line="171"/>
        <location filename="build/Skrivbord-Felsu00f6k/temp/ui_statsdialog.h" line="402"/>
        <source>Path Selected:</source>
        <translation>선택된 경로:</translation>
    </message>
    <message>
        <location filename="statsdialog.ui" line="181"/>
        <location filename="build/Skrivbord-Felsu00f6k/temp/ui_statsdialog.h" line="403"/>
        <source>Number of Files Scanned:</source>
        <translation>검사된 파일 수:</translation>
    </message>
    <message>
        <location filename="statsdialog.ui" line="201"/>
        <location filename="build/Skrivbord-Felsu00f6k/temp/ui_statsdialog.h" line="405"/>
        <source>Scan Duration:</source>
        <translation>검사 시간:</translation>
    </message>
    <message>
        <location filename="statsdialog.ui" line="254"/>
        <location filename="build/Skrivbord-Felsu00f6k/temp/ui_statsdialog.h" line="408"/>
        <source>Errors:</source>
        <translation>에러:</translation>
    </message>
    <message>
        <location filename="statsdialog.ui" line="271"/>
        <location filename="build/Skrivbord-Felsu00f6k/temp/ui_statsdialog.h" line="409"/>
        <source>Warnings:</source>
        <translation>경고:</translation>
    </message>
    <message>
        <location filename="statsdialog.ui" line="288"/>
        <location filename="build/Skrivbord-Felsu00f6k/temp/ui_statsdialog.h" line="410"/>
        <source>Stylistic warnings:</source>
        <translation>스타일 경고:</translation>
    </message>
    <message>
        <location filename="statsdialog.ui" line="305"/>
        <location filename="build/Skrivbord-Felsu00f6k/temp/ui_statsdialog.h" line="411"/>
        <source>Portability warnings:</source>
        <translation>이식성 경고:</translation>
    </message>
    <message>
        <location filename="statsdialog.ui" line="322"/>
        <location filename="build/Skrivbord-Felsu00f6k/temp/ui_statsdialog.h" line="412"/>
        <source>Performance issues:</source>
        <translation>성능 경고:</translation>
    </message>
    <message>
        <location filename="statsdialog.ui" line="339"/>
        <location filename="build/Skrivbord-Felsu00f6k/temp/ui_statsdialog.h" line="413"/>
        <source>Information messages:</source>
        <translation>정보 메시지:</translation>
    </message>
    <message>
        <location filename="statsdialog.ui" line="356"/>
        <location filename="build/Skrivbord-Felsu00f6k/temp/ui_statsdialog.h" line="414"/>
        <source>Active checkers:</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="statsdialog.ui" line="374"/>
        <location filename="build/Skrivbord-Felsu00f6k/temp/ui_statsdialog.h" line="416"/>
        <source>Checkers</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="statsdialog.ui" line="443"/>
        <location filename="build/Skrivbord-Felsu00f6k/temp/ui_statsdialog.h" line="419"/>
        <source>Copy to Clipboard</source>
        <translation>클립보드에 복사</translation>
    </message>
    <message>
        <location filename="statsdialog.cpp" line="160"/>
        <source>1 day</source>
        <translation>1일</translation>
    </message>
    <message>
        <location filename="statsdialog.cpp" line="160"/>
        <source>%1 days</source>
        <translation>%1일</translation>
    </message>
    <message>
        <location filename="statsdialog.cpp" line="162"/>
        <source>1 hour</source>
        <translation>1시간</translation>
    </message>
    <message>
        <location filename="statsdialog.cpp" line="162"/>
        <source>%1 hours</source>
        <translation>%1시간</translation>
    </message>
    <message>
        <location filename="statsdialog.cpp" line="164"/>
        <source>1 minute</source>
        <translation>1분</translation>
    </message>
    <message>
        <location filename="statsdialog.cpp" line="164"/>
        <source>%1 minutes</source>
        <translation>%1분</translation>
    </message>
    <message>
        <location filename="statsdialog.cpp" line="166"/>
        <source>1 second</source>
        <translation>1초</translation>
    </message>
    <message>
        <location filename="statsdialog.cpp" line="166"/>
        <source>%1 seconds</source>
        <translation>%1초</translation>
    </message>
    <message>
        <location filename="statsdialog.cpp" line="170"/>
        <source>0.%1 seconds</source>
        <translation>0.%1초</translation>
    </message>
    <message>
        <location filename="statsdialog.cpp" line="172"/>
        <source> and </source>
        <translation> 및 </translation>
    </message>
    <message>
        <location filename="statsdialog.cpp" line="221"/>
        <source>Project Settings</source>
        <translation>프로젝트 설정</translation>
    </message>
    <message>
        <location filename="statsdialog.cpp" line="223"/>
        <source>Paths</source>
        <translation>경로</translation>
    </message>
    <message>
        <location filename="statsdialog.cpp" line="224"/>
        <source>Include paths</source>
        <translation>Include 경로</translation>
    </message>
    <message>
        <location filename="statsdialog.cpp" line="225"/>
        <source>Defines</source>
        <translation>Defines</translation>
    </message>
    <message>
        <location filename="statsdialog.cpp" line="228"/>
        <source>Path selected</source>
        <translation>선택된 경로</translation>
    </message>
    <message>
        <location filename="statsdialog.cpp" line="229"/>
        <source>Number of files scanned</source>
        <translation>검사된 파일 수</translation>
    </message>
    <message>
        <location filename="statsdialog.cpp" line="230"/>
        <source>Scan duration</source>
        <translation>검사 시간</translation>
    </message>
    <message>
        <location filename="statsdialog.cpp" line="186"/>
        <location filename="statsdialog.cpp" line="232"/>
        <source>Errors</source>
        <translation>에러</translation>
    </message>
    <message>
        <location filename="statsdialog.cpp" line="188"/>
        <location filename="statsdialog.cpp" line="233"/>
        <source>Warnings</source>
        <translation>경고</translation>
    </message>
    <message>
        <location filename="statsdialog.cpp" line="190"/>
        <location filename="statsdialog.cpp" line="234"/>
        <source>Style warnings</source>
        <translation>스타일 경고</translation>
    </message>
    <message>
        <location filename="statsdialog.cpp" line="192"/>
        <location filename="statsdialog.cpp" line="235"/>
        <source>Portability warnings</source>
        <translation>이식성 경고</translation>
    </message>
    <message>
        <location filename="statsdialog.cpp" line="194"/>
        <location filename="statsdialog.cpp" line="236"/>
        <source>Performance warnings</source>
        <translation>성능 경고</translation>
    </message>
    <message>
        <location filename="statsdialog.cpp" line="196"/>
        <location filename="statsdialog.cpp" line="237"/>
        <source>Information messages</source>
        <translation>정보 메시지</translation>
    </message>
    <message>
        <location filename="statsdialog.ui" line="450"/>
        <location filename="build/Skrivbord-Felsu00f6k/temp/ui_statsdialog.h" line="420"/>
        <source>Pdf Export</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="statsdialog.cpp" line="199"/>
        <source>Export PDF</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="statsdialog.ui" line="399"/>
        <location filename="build/Skrivbord-Felsu00f6k/temp/ui_statsdialog.h" line="418"/>
        <source>History</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="statsdialog.ui" line="405"/>
        <location filename="build/Skrivbord-Felsu00f6k/temp/ui_statsdialog.h" line="417"/>
        <source>File:</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="statsdialog.cpp" line="112"/>
        <source>File: </source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="statsdialog.cpp" line="112"/>
        <source>No cppcheck build dir</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="statsdialog.ui" line="131"/>
        <location filename="build/Skrivbord-Felsu00f6k/temp/ui_statsdialog.h" line="400"/>
        <source>Undefines:</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="statsdialog.cpp" line="226"/>
        <source>Undefines</source>
        <translation type="unfinished"></translation>
    </message>
</context>
<context>
    <name>ThreadDetails</name>
    <message>
        <location filename="threaddetails.ui" line="14"/>
        <location filename="build/Skrivbord-Felsu00f6k/temp/ui_threaddetails.h" line="47"/>
        <source>Thread Details</source>
        <translation type="unfinished"></translation>
    </message>
</context>
<context>
    <name>ThreadResult</name>
    <message>
        <location filename="threadresult.cpp" line="47"/>
        <source>%1 of %2 files checked</source>
        <translation>%2 중 %1 파일 검사됨</translation>
    </message>
</context>
<context>
    <name>TranslationHandler</name>
    <message>
        <location filename="translationhandler.cpp" line="130"/>
        <source>Failed to change the user interface language:

%1

The user interface language has been reset to English. Open the Preferences-dialog to select any of the available languages.</source>
        <translation type="unfinished">언어 변경 실패:

%1

언어가 영어로 초기화 됐습니다. 설정창을 열어서 설정 가능한 언어를 선택하세요.</translation>
    </message>
    <message>
        <source>Cppcheck</source>
        <translation type="obsolete">Cppcheck</translation>
    </message>
</context>
<context>
    <name>TxtReport</name>
    <message>
        <location filename="txtreport.cpp" line="74"/>
        <source>inconclusive</source>
        <translation>불확실</translation>
    </message>
</context>
<context>
    <name>toFilterString</name>
    <message>
        <location filename="common.cpp" line="58"/>
        <source>All supported files (%1)</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="common.cpp" line="63"/>
        <source>All files (%1)</source>
        <translation type="unfinished"></translation>
    </message>
</context>
</TS>
