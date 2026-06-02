<?xml version="1.0" encoding="utf-8"?>
<!DOCTYPE TS>
<TS version="2.1" language="zh_TW">
<context>
    <name>About</name>
    <message>
        <location filename="about.ui" line="14"/>
        <source>About Cppcheck</source>
        <translation>關於 Cppcheck</translation>
    </message>
    <message>
        <location filename="about.ui" line="64"/>
        <source>Version %1</source>
        <translation>版本 %1</translation>
    </message>
    <message>
        <location filename="about.ui" line="71"/>
        <source>Cppcheck - A tool for static C/C++ code analysis.</source>
        <translation>Cppcheck - 一款靜態 C/C++ 程式碼分析工具。</translation>
    </message>
    <message>
        <location filename="about.ui" line="81"/>
        <source>Copyright © 2007-%1 Cppcheck team.</source>
        <translation>著作權 © 2007-%1 Cppcheck 團隊。</translation>
    </message>
    <message>
        <location filename="about.ui" line="91"/>
        <source>This program is licensed under the terms
of the GNU General Public License version 3</source>
        <translation>該程式是根據 GNU 通用公眾授權條款第 3 版
的規定進行授權的</translation>
    </message>
    <message>
        <location filename="about.ui" line="102"/>
        <source>Visit Cppcheck homepage at %1</source>
        <translation>訪問 Cppcheck 主頁: %1</translation>
    </message>
    <message>
        <location filename="about.ui" line="115"/>
        <source>&lt;html&gt;&lt;head/&gt;&lt;body&gt;&lt;p&gt;Many thanks to these libraries that we use:&lt;/p&gt;&lt;ul style=&quot;margin-top: 0px; margin-bottom: 0px; margin-left: 0px; margin-right: 0px; -qt-list-indent: 1;&quot;&gt;&lt;li style=&quot; margin-top:12px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;&quot;&gt;PCRE&lt;/li&gt;&lt;li style=&quot; margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;&quot;&gt;PicoJSON&lt;/li&gt;&lt;li style=&quot; margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;&quot;&gt;Qt&lt;/li&gt;&lt;li style=&quot; margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;&quot;&gt;TinyXML2&lt;/li&gt;&lt;li style=&quot; margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;&quot;&gt;Boost&lt;/li&gt;&lt;/ul&gt;&lt;/body&gt;&lt;/html&gt;</source>
        <oldsource>&lt;html&gt;&lt;head/&gt;&lt;body&gt;&lt;p&gt;Many thanks to these libraries that we use:&lt;/p&gt;&lt;ul style=&quot;margin-top: 0px; margin-bottom: 0px; margin-left: 0px; margin-right: 0px; -qt-list-indent: 1;&quot;&gt;&lt;li style=&quot; margin-top:12px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;&quot;&gt;pcre&lt;/li&gt;&lt;li style=&quot; margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;&quot;&gt;picojson&lt;/li&gt;&lt;li style=&quot; margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;&quot;&gt;qt&lt;/li&gt;&lt;li style=&quot; margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;&quot;&gt;tinyxml2&lt;/li&gt;&lt;/ul&gt;&lt;/body&gt;&lt;/html&gt;</oldsource>
        <translation type="unfinished"></translation>
    </message>
</context>
<context>
    <name>ApplicationDialog</name>
    <message>
        <location filename="applicationdialog.ui" line="23"/>
        <source>Add an application</source>
        <translation>新增應用程式</translation>
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
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="applicationdialog.ui" line="76"/>
        <source>&amp;Name:</source>
        <translation>名稱(&amp;N):</translation>
    </message>
    <message>
        <location filename="applicationdialog.ui" line="86"/>
        <source>&amp;Executable:</source>
        <translation>執行檔(&amp;E):</translation>
    </message>
    <message>
        <location filename="applicationdialog.ui" line="96"/>
        <source>&amp;Parameters:</source>
        <translation>參數(&amp;P):</translation>
    </message>
    <message>
        <location filename="applicationdialog.ui" line="138"/>
        <source>Browse</source>
        <translation>瀏覽</translation>
    </message>
    <message>
        <location filename="applicationdialog.cpp" line="65"/>
        <source>Executable files (*.exe);;All files(*.*)</source>
        <translation>執行檔 (*.exe);;所有檔案 (*.*)</translation>
    </message>
    <message>
        <location filename="applicationdialog.cpp" line="68"/>
        <source>Select viewer application</source>
        <translation>選取檢視器應用程式</translation>
    </message>
    <message>
        <location filename="applicationdialog.cpp" line="83"/>
        <source>Cppcheck</source>
        <translation>Cppcheck</translation>
    </message>
    <message>
        <location filename="applicationdialog.cpp" line="84"/>
        <source>You must specify a name, a path and optionally parameters for the application!</source>
        <translation>您必須為應用程式指定名稱、路徑與選用的參數！</translation>
    </message>
</context>
<context>
    <name>ComplianceReportDialog</name>
    <message>
        <source>Project name</source>
        <translation type="vanished">專案名稱</translation>
    </message>
    <message>
        <source>Project version</source>
        <translation type="vanished">專案版本</translation>
    </message>
    <message>
        <source>HTML files (*.html)</source>
        <translation type="vanished">HTML 檔案 (*.html)</translation>
    </message>
</context>
<context>
    <name>FileViewDialog</name>
    <message>
        <location filename="fileviewdialog.cpp" line="53"/>
        <source>Could not find the file: %1</source>
        <translation>無法找到檔案: %1</translation>
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
        <translation>無法讀取檔案: %1</translation>
    </message>
</context>
<context>
    <name>HelpDialog</name>
    <message>
        <location filename="helpdialog.ui" line="14"/>
        <source>Cppcheck GUI help</source>
        <translation>Cppcheck GUI 幫助</translation>
    </message>
    <message>
        <location filename="helpdialog.ui" line="34"/>
        <source>Contents</source>
        <translation>內容</translation>
    </message>
    <message>
        <location filename="helpdialog.ui" line="44"/>
        <source>Index</source>
        <translation>索引</translation>
    </message>
    <message>
        <location filename="helpdialog.cpp" line="85"/>
        <source>Helpfile &apos;%1&apos; was not found</source>
        <translation>找不到幫助檔 &apos;%1&apos;</translation>
    </message>
    <message>
        <location filename="helpdialog.cpp" line="87"/>
        <source>Cppcheck</source>
        <translation>Cppcheck</translation>
    </message>
</context>
<context>
    <name>LibraryAddFunctionDialog</name>
    <message>
        <location filename="libraryaddfunctiondialog.ui" line="23"/>
        <source>Add function</source>
        <translation>新增函式</translation>
    </message>
    <message>
        <location filename="libraryaddfunctiondialog.ui" line="34"/>
        <source>Function name(s)</source>
        <translation>函式名稱</translation>
    </message>
    <message>
        <location filename="libraryaddfunctiondialog.ui" line="44"/>
        <source>Number of arguments</source>
        <translation>引數數量</translation>
    </message>
</context>
<context>
    <name>LibraryDialog</name>
    <message>
        <location filename="librarydialog.ui" line="14"/>
        <source>Library Editor</source>
        <translation>程式庫編輯器</translation>
    </message>
    <message>
        <location filename="librarydialog.ui" line="22"/>
        <source>Open</source>
        <translation>開啟</translation>
    </message>
    <message>
        <location filename="librarydialog.ui" line="29"/>
        <source>Save</source>
        <translation>儲存</translation>
    </message>
    <message>
        <location filename="librarydialog.ui" line="36"/>
        <source>Save as</source>
        <translation type="unfinished">另存為</translation>
    </message>
    <message>
        <location filename="librarydialog.ui" line="62"/>
        <source>Functions</source>
        <translation>函式</translation>
    </message>
    <message>
        <location filename="librarydialog.ui" line="71"/>
        <source>Sort</source>
        <translation>排序</translation>
    </message>
    <message>
        <location filename="librarydialog.ui" line="111"/>
        <source>Add</source>
        <translation>新增</translation>
    </message>
    <message>
        <location filename="librarydialog.ui" line="131"/>
        <source>Filter:</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="librarydialog.ui" line="164"/>
        <source>Comments</source>
        <translation>註釋</translation>
    </message>
    <message>
        <location filename="librarydialog.ui" line="204"/>
        <source>noreturn</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="librarydialog.ui" line="212"/>
        <source>False</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="librarydialog.ui" line="217"/>
        <source>True</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="librarydialog.ui" line="222"/>
        <source>Unknown</source>
        <translation>未知</translation>
    </message>
    <message>
        <location filename="librarydialog.ui" line="232"/>
        <source>return value must be used</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="librarydialog.ui" line="239"/>
        <source>ignore function in leaks checking</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="librarydialog.ui" line="246"/>
        <source>Arguments</source>
        <translation>引數</translation>
    </message>
    <message>
        <location filename="librarydialog.ui" line="258"/>
        <source>Edit</source>
        <translation>編輯</translation>
    </message>
    <message>
        <location filename="librarydialog.cpp" line="97"/>
        <location filename="librarydialog.cpp" line="169"/>
        <source>Library files (*.cfg)</source>
        <translation>程式庫檔案 (*.cfg)</translation>
    </message>
    <message>
        <location filename="librarydialog.cpp" line="99"/>
        <source>Open library file</source>
        <translation>開啟程式庫檔案</translation>
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
        <translation>無法開啟檔案 %1。</translation>
    </message>
    <message>
        <location filename="librarydialog.cpp" line="123"/>
        <source>Failed to load %1. %2.</source>
        <translation>無法載入 %1. %2。</translation>
    </message>
    <message>
        <location filename="librarydialog.cpp" line="160"/>
        <source>Cannot save file %1.</source>
        <translation>無法儲存檔案 %1。</translation>
    </message>
    <message>
        <location filename="librarydialog.cpp" line="172"/>
        <source>Save the library as</source>
        <translation type="unfinished">另存程式庫為</translation>
    </message>
</context>
<context>
    <name>LibraryEditArgDialog</name>
    <message>
        <location filename="libraryeditargdialog.ui" line="14"/>
        <source>Edit argument</source>
        <translation>編輯引數</translation>
    </message>
    <message>
        <location filename="libraryeditargdialog.ui" line="20"/>
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
        <source>Not bool</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="libraryeditargdialog.ui" line="35"/>
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
        <source>Not null</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="libraryeditargdialog.ui" line="50"/>
        <source>Not uninit</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="libraryeditargdialog.ui" line="57"/>
        <source>String</source>
        <translation>字串</translation>
    </message>
    <message>
        <location filename="libraryeditargdialog.ui" line="70"/>
        <source>Format string</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="libraryeditargdialog.ui" line="92"/>
        <source>Min size of buffer</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="libraryeditargdialog.ui" line="101"/>
        <location filename="libraryeditargdialog.ui" line="203"/>
        <source>Type</source>
        <translation>型別</translation>
    </message>
    <message>
        <location filename="libraryeditargdialog.ui" line="109"/>
        <location filename="libraryeditargdialog.ui" line="214"/>
        <source>None</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="libraryeditargdialog.ui" line="114"/>
        <location filename="libraryeditargdialog.ui" line="219"/>
        <source>argvalue</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="libraryeditargdialog.ui" line="119"/>
        <location filename="libraryeditargdialog.ui" line="224"/>
        <source>mul</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="libraryeditargdialog.ui" line="124"/>
        <location filename="libraryeditargdialog.ui" line="229"/>
        <source>strlen</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="libraryeditargdialog.ui" line="132"/>
        <location filename="libraryeditargdialog.ui" line="237"/>
        <source>Arg</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="libraryeditargdialog.ui" line="159"/>
        <location filename="libraryeditargdialog.ui" line="264"/>
        <source>Arg2</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="libraryeditargdialog.ui" line="194"/>
        <source>and</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="libraryeditargdialog.ui" line="310"/>
        <source>Valid values</source>
        <translation>有效值</translation>
    </message>
</context>
<context>
    <name>MainWindow</name>
    <message>
        <location filename="mainwindow.ui" line="26"/>
        <location filename="mainwindow.ui" line="687"/>
        <location filename="mainwindow.cpp" line="459"/>
        <location filename="mainwindow.cpp" line="662"/>
        <location filename="mainwindow.cpp" line="747"/>
        <location filename="mainwindow.cpp" line="853"/>
        <location filename="mainwindow.cpp" line="875"/>
        <location filename="mainwindow.cpp" line="1462"/>
        <location filename="mainwindow.cpp" line="1587"/>
        <location filename="mainwindow.cpp" line="1882"/>
        <location filename="mainwindow.cpp" line="1890"/>
        <location filename="mainwindow.cpp" line="1945"/>
        <location filename="mainwindow.cpp" line="1954"/>
        <location filename="mainwindow.cpp" line="2026"/>
        <location filename="mainwindow.cpp" line="2114"/>
        <source>Cppcheck</source>
        <translation>Cppcheck</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="84"/>
        <source>Checking for updates</source>
        <translation>檢查更新</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="100"/>
        <source>Hide</source>
        <translation>隱藏</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="132"/>
        <source>&amp;File</source>
        <translation>檔案(&amp;F)</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="151"/>
        <source>&amp;View</source>
        <translation>檢視(&amp;V)</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="155"/>
        <source>&amp;Toolbars</source>
        <translation>工具條(&amp;T)</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="163"/>
        <location filename="mainwindow.ui" line="979"/>
        <source>Report</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="198"/>
        <source>&amp;Help</source>
        <translation>幫助(&amp;H)</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="209"/>
        <source>A&amp;nalyze</source>
        <translation>分析(&amp;N)</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="213"/>
        <source>C++ standard</source>
        <translation>C++ 標準</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="225"/>
        <source>&amp;C standard</source>
        <translation>C 標準(&amp;C)</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="253"/>
        <source>&amp;Edit</source>
        <translation>編輯(&amp;E)</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="266"/>
        <source>Standard</source>
        <translation>標準</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="284"/>
        <source>Categories</source>
        <translation>分類</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="303"/>
        <source>Filter</source>
        <translation>篩選</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="314"/>
        <source>&amp;License...</source>
        <translation>授權(&amp;L)...</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="319"/>
        <source>A&amp;uthors...</source>
        <translation>作者(&amp;U)...</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="328"/>
        <source>&amp;About...</source>
        <translation>關於(&amp;A)...</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="333"/>
        <source>&amp;Files...</source>
        <translation>檔案(&amp;F)...</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="336"/>
        <location filename="mainwindow.ui" line="339"/>
        <source>Analyze files</source>
        <translation>分析檔案</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="342"/>
        <source>Ctrl+F</source>
        <translation>Ctrl+F</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="351"/>
        <source>&amp;Directory...</source>
        <translation>目錄(&amp;D)...</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="354"/>
        <location filename="mainwindow.ui" line="357"/>
        <source>Analyze directory</source>
        <translation>分析目錄</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="360"/>
        <source>Ctrl+D</source>
        <translation>Ctrl+D</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="369"/>
        <source>&amp;Reanalyze modified files</source>
        <translation>重新分析已修改的檔案(&amp;R)</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="372"/>
        <source>Ctrl+R</source>
        <translation>Ctrl+R</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="381"/>
        <source>Reanal&amp;yze all files</source>
        <translation>重新分析所有檔案(&amp;Y)</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="390"/>
        <source>&amp;Stop</source>
        <translation>停止(&amp;S)</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="393"/>
        <location filename="mainwindow.ui" line="396"/>
        <source>Stop analysis</source>
        <translation>停止分析</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="399"/>
        <source>Esc</source>
        <translation>Esc</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="408"/>
        <source>&amp;Save results to file...</source>
        <translation>儲存結果為檔案(&amp;S)...</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="411"/>
        <source>Ctrl+S</source>
        <translation>Ctrl+S</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="416"/>
        <source>&amp;Quit</source>
        <translation>退出(&amp;Q)</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="419"/>
        <source>Ctrl+Q</source>
        <translation>Ctrl+Q</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="428"/>
        <source>&amp;Clear results</source>
        <translation>清除結果(&amp;C)</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="437"/>
        <source>&amp;Preferences</source>
        <translation>偏好設定(&amp;P)</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="449"/>
        <source>Style war&amp;nings</source>
        <translation>樣式警告(&amp;N)</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="452"/>
        <location filename="mainwindow.ui" line="455"/>
        <source>Show style warnings</source>
        <translation>顯示樣式警告</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="467"/>
        <source>E&amp;rrors</source>
        <translation>錯誤(&amp;R)</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="470"/>
        <location filename="mainwindow.ui" line="473"/>
        <location filename="mainwindow.cpp" line="2340"/>
        <source>Show errors</source>
        <translation>顯示錯誤</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="478"/>
        <source>&amp;Check all</source>
        <translation>全部檢查(&amp;C)</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="483"/>
        <source>&amp;Uncheck all</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="488"/>
        <source>Collapse &amp;all</source>
        <translation>全部摺疊(&amp;A)</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="493"/>
        <source>&amp;Expand all</source>
        <translation>全部展開(&amp;E)</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="501"/>
        <source>&amp;Standard</source>
        <translation>標準(&amp;S)</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="504"/>
        <source>Standard items</source>
        <translatorcomment>標準項目</translatorcomment>
        <translation></translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="509"/>
        <source>&amp;Contents</source>
        <translation>內容(&amp;C)</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="512"/>
        <source>Open the help contents</source>
        <translation>開啟幫助內容</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="515"/>
        <source>F1</source>
        <translation>F1</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="520"/>
        <source>Toolbar</source>
        <translation>工具條</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="528"/>
        <source>&amp;Categories</source>
        <translation>分類(&amp;C)</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="531"/>
        <source>Error categories</source>
        <translation>錯誤分類</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="536"/>
        <source>&amp;Open XML...</source>
        <translation>開啟 XML(&amp;O)...</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="545"/>
        <source>Open P&amp;roject File...</source>
        <translation>開啟專案檔(&amp;R)...</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="548"/>
        <source>Ctrl+Shift+O</source>
        <translation>Ctrl+Shift+O</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="557"/>
        <source>Sh&amp;ow Scratchpad...</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="562"/>
        <source>&amp;New Project File...</source>
        <translation>新增專案檔(&amp;N)...</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="565"/>
        <source>Ctrl+Shift+N</source>
        <translation>Ctrl+Shift+N</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="570"/>
        <source>&amp;Log View</source>
        <translation>日誌檢視(&amp;L)</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="573"/>
        <source>Log View</source>
        <translation>日誌檢視</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="581"/>
        <source>C&amp;lose Project File</source>
        <translation>關閉專案檔(&amp;L)</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="589"/>
        <source>&amp;Edit Project File...</source>
        <translation>編輯專案檔(&amp;E)...</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="601"/>
        <source>&amp;Statistics</source>
        <translation>統計資料(&amp;S)</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="613"/>
        <source>&amp;Warnings</source>
        <translation>警告(&amp;W)</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="616"/>
        <location filename="mainwindow.ui" line="619"/>
        <location filename="mainwindow.cpp" line="2341"/>
        <source>Show warnings</source>
        <translation>顯示警告</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="631"/>
        <source>Per&amp;formance warnings</source>
        <translation>效能警告(&amp;F)</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="634"/>
        <location filename="mainwindow.ui" line="637"/>
        <source>Show performance warnings</source>
        <translation>顯示下效能警告</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="645"/>
        <source>Show &amp;hidden</source>
        <translation>顯示隱藏項目(&amp;H)</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="657"/>
        <source>&amp;Information</source>
        <translation>資訊(&amp;I)</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="660"/>
        <source>Show information messages</source>
        <translation>顯示資訊訊息</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="672"/>
        <source>&amp;Portability</source>
        <translation>可移植性(&amp;P)</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="675"/>
        <source>Show portability warnings</source>
        <translation>顯示可移植性警告</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="690"/>
        <source>Show Cppcheck results</source>
        <translation>顯示 Cppcheck 結果</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="702"/>
        <source>Clang</source>
        <translation>Clang</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="705"/>
        <source>Show Clang results</source>
        <translation>顯示 Clang 結果</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="713"/>
        <source>&amp;Filter</source>
        <translation>篩選(&amp;F)</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="716"/>
        <source>Filter results</source>
        <translation>篩選結果</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="732"/>
        <source>Windows 32-bit ANSI</source>
        <translation>Windows 32 位元 ANSI</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="740"/>
        <source>Windows 32-bit Unicode</source>
        <translation>Windows 32 位元 Unicode</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="748"/>
        <source>Unix 32-bit</source>
        <translation>Unix 32 位元</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="756"/>
        <source>Unix 64-bit</source>
        <translation>Unix 64 位元</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="764"/>
        <source>Windows 64-bit</source>
        <translation>Windows 64 位元</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="772"/>
        <source>P&amp;latforms</source>
        <translation>平臺(&amp;L)</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="786"/>
        <source>C++&amp;11</source>
        <translation>C++&amp;11</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="797"/>
        <source>C&amp;99</source>
        <translation>C&amp;99</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="805"/>
        <source>&amp;Posix</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="813"/>
        <source>C&amp;11</source>
        <translation>C&amp;11</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="845"/>
        <source>&amp;C89</source>
        <translation>&amp;C89</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="853"/>
        <source>&amp;C++03</source>
        <translation>&amp;C++03</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="858"/>
        <source>&amp;Print...</source>
        <translation>列印(&amp;P)...</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="861"/>
        <source>Print the Current Report</source>
        <translation>列印當前報告</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="866"/>
        <source>Print Pre&amp;view...</source>
        <translation>列印預覽(&amp;V)...</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="869"/>
        <source>Open a Print Preview Dialog for the Current Results</source>
        <translation>開啟當前結果的列印預覽視窗</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="874"/>
        <source>&amp;Library Editor...</source>
        <translation>程式庫編輯器(&amp;L)...</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="877"/>
        <source>Open library editor</source>
        <translation>開啟程式庫編輯器</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="885"/>
        <source>&amp;Auto-detect language</source>
        <translation>自動偵測語言(&amp;A)</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="893"/>
        <source>&amp;Enforce C++</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="901"/>
        <source>E&amp;nforce C</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="912"/>
        <source>C++14</source>
        <translation>C++14</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="920"/>
        <source>Reanalyze and check library</source>
        <translation>重新分析並檢查程式庫</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="928"/>
        <source>Check configuration (defines, includes)</source>
        <translation>檢查組態 (定義、包含)</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="936"/>
        <source>C++17</source>
        <translation>C++17</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="947"/>
        <source>C++20</source>
        <translation>C++20</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="974"/>
        <source>Compliance report...</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="987"/>
        <source>Normal</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="995"/>
        <source>Misra C</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="1003"/>
        <source>Misra C++ 2008</source>
        <translation type="unfinished">Misra C++ 2008</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="1011"/>
        <source>Cert C</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="1019"/>
        <source>Cert C++</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="1027"/>
        <source>Misra C++ 2023</source>
        <translation type="unfinished">Misra C++ 2023</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="1035"/>
        <source>Autosar</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="1040"/>
        <source>EULA...</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="1045"/>
        <source>Thread Details</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="1048"/>
        <source>Show thread details</source>
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
        <location filename="main.cpp" line="115"/>
        <source>Cppcheck GUI - Command line parameters</source>
        <translation>Cppcheck GUI - 命令行參數</translation>
    </message>
    <message>
        <location filename="mainwindow.cpp" line="153"/>
        <location filename="mainwindow.cpp" line="1751"/>
        <source>Quick Filter:</source>
        <translation>快速篩選:</translation>
    </message>
    <message>
        <location filename="mainwindow.cpp" line="237"/>
        <location filename="mainwindow.cpp" line="1753"/>
        <location filename="mainwindow.cpp" line="1819"/>
        <location filename="mainwindow.cpp" line="1994"/>
        <source>Project:</source>
        <translation>專案:</translation>
    </message>
    <message>
        <location filename="mainwindow.cpp" line="454"/>
        <source>There was a problem with loading the editor application settings.

This is probably because the settings were changed between the Cppcheck versions. Please check (and fix) the editor application settings, otherwise the editor program might not start correctly.</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="mainwindow.cpp" line="663"/>
        <source>No suitable files found to analyze!</source>
        <translation>找不到適合的檔案來分析！</translation>
    </message>
    <message>
        <location filename="mainwindow.cpp" line="748"/>
        <source>You must close the project file before selecting new files or directories!</source>
        <translation>您必須在選取新檔案或目錄之前關閉該專案檔！</translation>
    </message>
    <message>
        <location filename="mainwindow.cpp" line="762"/>
        <source>C/C++ Source</source>
        <translation>C/C++ 來源檔</translation>
    </message>
    <message>
        <location filename="mainwindow.cpp" line="763"/>
        <source>Compile database</source>
        <translation>編譯資料庫</translation>
    </message>
    <message>
        <location filename="mainwindow.cpp" line="764"/>
        <source>Visual Studio</source>
        <translation>Visual Studio</translation>
    </message>
    <message>
        <location filename="mainwindow.cpp" line="765"/>
        <source>Borland C++ Builder 6</source>
        <translation>Borland C++ Builder 6</translation>
    </message>
    <message>
        <location filename="mainwindow.cpp" line="768"/>
        <source>Select files to analyze</source>
        <translation>選取要分析的檔案</translation>
    </message>
    <message>
        <location filename="mainwindow.cpp" line="783"/>
        <source>Select directory to analyze</source>
        <translation>選取要分析的目錄</translation>
    </message>
    <message>
        <location filename="mainwindow.cpp" line="824"/>
        <source>Select configuration</source>
        <translation>選取組態</translation>
    </message>
    <message>
        <location filename="mainwindow.cpp" line="824"/>
        <source>Select the configuration that will be analyzed</source>
        <translation>選取要分析的組態</translation>
    </message>
    <message>
        <location filename="mainwindow.cpp" line="854"/>
        <source>Found project file: %1

Do you want to load this project file instead?</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="mainwindow.cpp" line="876"/>
        <source>Found project files from the directory.

Do you want to proceed analysis without using any of these project files?</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="mainwindow.cpp" line="979"/>
        <location filename="mainwindow.cpp" line="1017"/>
        <source>Information</source>
        <translation>資訊</translation>
    </message>
    <message>
        <location filename="mainwindow.cpp" line="979"/>
        <source>The library &apos;%1&apos; contains unknown elements:
%2</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="mainwindow.cpp" line="988"/>
        <source>File not found</source>
        <translation>找不到檔案</translation>
    </message>
    <message>
        <location filename="mainwindow.cpp" line="991"/>
        <source>Bad XML</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="mainwindow.cpp" line="994"/>
        <source>Missing attribute</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="mainwindow.cpp" line="997"/>
        <source>Bad attribute value</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="mainwindow.cpp" line="1000"/>
        <source>Unsupported format</source>
        <translation>未支援的格式</translation>
    </message>
    <message>
        <location filename="mainwindow.cpp" line="1003"/>
        <source>Duplicate platform type</source>
        <translation>重複的平臺型別</translation>
    </message>
    <message>
        <location filename="mainwindow.cpp" line="1006"/>
        <source>Platform type redefined</source>
        <translation>平臺型別重定義</translation>
    </message>
    <message>
        <location filename="mainwindow.cpp" line="1009"/>
        <source>Duplicate define</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="mainwindow.cpp" line="1012"/>
        <source>Unknown element</source>
        <translation>未知的元素</translation>
    </message>
    <message>
        <location filename="mainwindow.cpp" line="1017"/>
        <source>Failed to load the selected library &apos;%1&apos;.
%2</source>
        <translation>無法載入選取的程式庫 &apos;%1&apos;。
%2</translation>
    </message>
    <message>
        <location filename="mainwindow.cpp" line="1028"/>
        <source>File not found: &apos;%1&apos;</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="mainwindow.cpp" line="1051"/>
        <source>Failed to load/setup addon %1: %2</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="mainwindow.cpp" line="1074"/>
        <location filename="mainwindow.cpp" line="1084"/>
        <location filename="mainwindow.cpp" line="1096"/>
        <location filename="mainwindow.cpp" line="1193"/>
        <source>Error</source>
        <translation>錯誤</translation>
    </message>
    <message>
        <location filename="mainwindow.cpp" line="1074"/>
        <source>Failed to load %1. Your Cppcheck installation is broken. You can use --data-dir=&lt;directory&gt; at the command line to specify where this file is located. Please note that --data-dir is supposed to be used by installation scripts and therefore the GUI does not start when it is used, all that happens is that the setting is configured.

Analysis is aborted.</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="mainwindow.cpp" line="1084"/>
        <source>Failed to load %1 - %2

Analysis is aborted.</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="mainwindow.cpp" line="1096"/>
        <location filename="mainwindow.cpp" line="1193"/>
        <source>%1

Analysis is aborted.</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="mainwindow.cpp" line="1463"/>
        <source>Current results will be cleared.

Opening a new XML file will clear current results.
Do you want to proceed?</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="mainwindow.cpp" line="1478"/>
        <location filename="mainwindow.cpp" line="1673"/>
        <source>XML files (*.xml)</source>
        <translation>XML 檔案 (*.xml)</translation>
    </message>
    <message>
        <location filename="mainwindow.cpp" line="1480"/>
        <source>Open the report file</source>
        <translation>開啟報告檔</translation>
    </message>
    <message>
        <location filename="mainwindow.cpp" line="1583"/>
        <source>Analyzer is running.

Do you want to stop the analysis and exit Cppcheck?</source>
        <translation>分析正在執行

您想停止分析並離開 Cppcheck 嗎？</translation>
    </message>
    <message>
        <location filename="mainwindow.cpp" line="1625"/>
        <source>About</source>
        <translation>關於</translation>
    </message>
    <message>
        <location filename="mainwindow.cpp" line="1639"/>
        <source>License</source>
        <translation>授權</translation>
    </message>
    <message>
        <location filename="mainwindow.cpp" line="1646"/>
        <source>Authors</source>
        <translation>作者</translation>
    </message>
    <message>
        <location filename="mainwindow.cpp" line="1664"/>
        <source>XML files (*.xml);;Text files (*.txt);;CSV files (*.csv)</source>
        <translation>XML 檔案 (*.xml);;文字檔 (*.txt);;CSV 檔案 (*.csv)</translation>
    </message>
    <message>
        <location filename="mainwindow.cpp" line="1666"/>
        <source>Save the report file</source>
        <translation>儲存報告檔</translation>
    </message>
    <message>
        <location filename="mainwindow.cpp" line="1677"/>
        <source>Text files (*.txt)</source>
        <translation>文字檔 (*.txt)</translation>
    </message>
    <message>
        <location filename="mainwindow.cpp" line="1681"/>
        <source>CSV files (*.csv)</source>
        <translation>CSV 檔案 (*.csv)</translation>
    </message>
    <message>
        <location filename="mainwindow.cpp" line="1789"/>
        <source>Project files (*.cppcheck);;All files(*.*)</source>
        <translation>專案檔 (*.cppcheck);;所有檔案 (*.*)</translation>
    </message>
    <message>
        <location filename="mainwindow.cpp" line="1791"/>
        <source>Select Project File</source>
        <translation>選取專案檔</translation>
    </message>
    <message>
        <location filename="mainwindow.cpp" line="1883"/>
        <source>Build dir &apos;%1&apos; does not exist, create it?</source>
        <translation>建置目錄 &apos;%1&apos; 不存在，是否建立它？</translation>
    </message>
    <message>
        <location filename="mainwindow.cpp" line="1891"/>
        <source>To check the project using addons, you need a build directory.</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="mainwindow.cpp" line="1924"/>
        <source>Failed to open file</source>
        <translation>無法開啟檔案</translation>
    </message>
    <message>
        <location filename="mainwindow.cpp" line="1927"/>
        <source>Unknown project file format</source>
        <translation>未知的專案檔格式</translation>
    </message>
    <message>
        <location filename="mainwindow.cpp" line="1930"/>
        <source>Failed to import project file</source>
        <translation>無法匯入專案檔</translation>
    </message>
    <message>
        <location filename="mainwindow.cpp" line="1946"/>
        <source>Failed to import &apos;%1&apos;: %2

Analysis is stopped.</source>
        <translation>無法匯入 &apos;%1&apos;: %2

停止分析。</translation>
    </message>
    <message>
        <location filename="mainwindow.cpp" line="1955"/>
        <source>Failed to import &apos;%1&apos; (%2), analysis is stopped</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="mainwindow.cpp" line="2313"/>
        <source>Show Mandatory</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="mainwindow.cpp" line="2314"/>
        <source>Show Required</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="mainwindow.cpp" line="2315"/>
        <source>Show Advisory</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="mainwindow.cpp" line="2316"/>
        <source>Show Document</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="mainwindow.cpp" line="2333"/>
        <source>Show L1</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="mainwindow.cpp" line="2334"/>
        <source>Show L2</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="mainwindow.cpp" line="2335"/>
        <source>Show L3</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="mainwindow.cpp" line="2342"/>
        <source>Show style</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="mainwindow.cpp" line="2343"/>
        <source>Show portability</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="mainwindow.cpp" line="2344"/>
        <source>Show performance</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="mainwindow.cpp" line="2345"/>
        <source>Show information</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Failed to import &apos;%1&apos;, analysis is stopped</source>
        <translation type="vanished">無法匯入 &apos;%1&apos;，停止分析</translation>
    </message>
    <message>
        <location filename="mainwindow.cpp" line="1979"/>
        <source>Project files (*.cppcheck)</source>
        <translation>專案檔 (*.cppcheck)</translation>
    </message>
    <message>
        <location filename="mainwindow.cpp" line="1981"/>
        <source>Select Project Filename</source>
        <translation>選取專案檔案名稱</translation>
    </message>
    <message>
        <location filename="mainwindow.cpp" line="2027"/>
        <source>No project file loaded</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="mainwindow.cpp" line="2109"/>
        <source>The project file

%1

 could not be found!

Do you want to remove the file from the recently used projects -list?</source>
        <translation>專案檔

%1

 找不到！

您要從最近使用的專案列表中移除該檔案嗎？</translation>
    </message>
    <message>
        <location filename="mainwindow.cpp" line="2271"/>
        <source>Install</source>
        <translation>安章</translation>
    </message>
    <message>
        <location filename="mainwindow.cpp" line="2275"/>
        <source>New version available: %1. %2</source>
        <translation>可用的新版本: %1. %2</translation>
    </message>
</context>
<context>
    <name>NewSuppressionDialog</name>
    <message>
        <location filename="newsuppressiondialog.ui" line="17"/>
        <source>New suppression</source>
        <translation>新建抑制</translation>
    </message>
    <message>
        <location filename="newsuppressiondialog.ui" line="25"/>
        <source>Error ID</source>
        <translation>錯誤 ID</translation>
    </message>
    <message>
        <location filename="newsuppressiondialog.ui" line="32"/>
        <source>File name</source>
        <translation>檔案名稱</translation>
    </message>
    <message>
        <location filename="newsuppressiondialog.ui" line="42"/>
        <source>Line number</source>
        <translation>行號</translation>
    </message>
    <message>
        <location filename="newsuppressiondialog.ui" line="52"/>
        <source>Symbol name</source>
        <translation>符號名稱</translation>
    </message>
    <message>
        <location filename="newsuppressiondialog.cpp" line="83"/>
        <source>Edit suppression</source>
        <translation>編輯抑制</translation>
    </message>
</context>
<context>
    <name>Platforms</name>
    <message>
        <location filename="platforms.cpp" line="38"/>
        <source>Native</source>
        <translation>原生</translation>
    </message>
    <message>
        <location filename="platforms.cpp" line="39"/>
        <source>Unix 32-bit</source>
        <translation>Unix 32 位元</translation>
    </message>
    <message>
        <location filename="platforms.cpp" line="40"/>
        <source>Unix 64-bit</source>
        <translation>Unix 64 位元</translation>
    </message>
    <message>
        <location filename="platforms.cpp" line="41"/>
        <source>Windows 32-bit ANSI</source>
        <translation>Windows 32 位元 ANSI</translation>
    </message>
    <message>
        <location filename="platforms.cpp" line="42"/>
        <source>Windows 32-bit Unicode</source>
        <translation>Windows 32 位元 Unicode</translation>
    </message>
    <message>
        <location filename="platforms.cpp" line="43"/>
        <source>Windows 64-bit</source>
        <translation>Windows 64 位元</translation>
    </message>
</context>
<context>
    <name>ProjectFile</name>
    <message>
        <location filename="projectfile.ui" line="14"/>
        <source>Project File</source>
        <translation>專案檔</translation>
    </message>
    <message>
        <location filename="projectfile.ui" line="24"/>
        <source>Paths and Defines</source>
        <translation>路徑與定義</translation>
    </message>
    <message>
        <location filename="projectfile.ui" line="30"/>
        <source>Import Project (Visual studio / compile database/ Borland C++ Builder 6)</source>
        <translation>匯入專案 (Visual Studio / 編譯資料庫 / Borland C++ Builder 6)</translation>
    </message>
    <message>
        <location filename="projectfile.ui" line="60"/>
        <location filename="projectfile.ui" line="450"/>
        <source>Browse...</source>
        <translation>瀏覽...</translation>
    </message>
    <message>
        <location filename="projectfile.ui" line="73"/>
        <source>&lt;html&gt;&lt;head/&gt;&lt;body&gt;&lt;p&gt;You have a choice:&lt;/p&gt;&lt;p&gt; * Analyze all Debug and Release configurations&lt;/p&gt;&lt;p&gt; * Only analyze the first matching Debug configuration&lt;/p&gt;&lt;p&gt;&lt;br/&gt;&lt;/p&gt;&lt;/body&gt;&lt;/html&gt;</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="projectfile.ui" line="76"/>
        <source>Analyze all Visual Studio configurations</source>
        <translation>分析所有 Visual Studio 組態</translation>
    </message>
    <message>
        <location filename="projectfile.ui" line="113"/>
        <source>Selected VS Configurations</source>
        <translation>選取 VS 組態</translation>
    </message>
    <message>
        <location filename="projectfile.ui" line="147"/>
        <source>Paths:</source>
        <translation>路徑:</translation>
    </message>
    <message>
        <location filename="projectfile.ui" line="181"/>
        <location filename="projectfile.ui" line="296"/>
        <source>Add...</source>
        <translation>新增...</translation>
    </message>
    <message>
        <location filename="projectfile.ui" line="188"/>
        <location filename="projectfile.ui" line="303"/>
        <location filename="projectfile.ui" line="699"/>
        <source>Edit</source>
        <translation>編輯</translation>
    </message>
    <message>
        <location filename="projectfile.ui" line="195"/>
        <location filename="projectfile.ui" line="310"/>
        <location filename="projectfile.ui" line="706"/>
        <location filename="projectfile.ui" line="749"/>
        <source>Remove</source>
        <translation>移除</translation>
    </message>
    <message>
        <location filename="projectfile.ui" line="221"/>
        <source>Defines:</source>
        <translation>定義:</translation>
    </message>
    <message>
        <location filename="projectfile.ui" line="231"/>
        <source>Defines must be separated by a semicolon. Example: DEF1;DEF2=5;DEF3=int</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="projectfile.ui" line="242"/>
        <source>Undefines:</source>
        <translation>未定義:</translation>
    </message>
    <message>
        <location filename="projectfile.ui" line="252"/>
        <source>Undefines must be separated by a semicolon. Example: UNDEF1;UNDEF2;UNDEF3</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="projectfile.ui" line="265"/>
        <source>Include Paths:</source>
        <translation>包含路徑:</translation>
    </message>
    <message>
        <location filename="projectfile.ui" line="330"/>
        <source>Up</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="projectfile.ui" line="337"/>
        <source>Down</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="projectfile.ui" line="350"/>
        <source>Include file</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="projectfile.ui" line="360"/>
        <source>&lt;html&gt;&lt;head/&gt;&lt;body&gt;&lt;p&gt;&lt;span style=&quot; font-family:&apos;monospace&apos;; color:#000000; background-color:#ffffff;&quot;&gt;Force inclusion of a header file&lt;/span&gt;&lt;/p&gt;&lt;/body&gt;&lt;/html&gt;</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="projectfile.ui" line="367"/>
        <source>Browse..</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="projectfile.ui" line="390"/>
        <source>Types and Functions</source>
        <translation>型別與函式</translation>
    </message>
    <message>
        <location filename="projectfile.ui" line="396"/>
        <source>Platform</source>
        <translation>平臺</translation>
    </message>
    <message>
        <location filename="projectfile.ui" line="408"/>
        <source>Libraries</source>
        <translation>程式庫</translation>
    </message>
    <message>
        <location filename="projectfile.ui" line="417"/>
        <source>Note: Put your own custom .cfg files in the same folder as the project file. You should see them above.</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="projectfile.ui" line="431"/>
        <location filename="projectfile.ui" line="516"/>
        <source>Analysis</source>
        <translation>分析</translation>
    </message>
    <message>
        <location filename="projectfile.ui" line="437"/>
        <source>Cppcheck build dir (whole program analysis, incremental analysis, statistics, etc)</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="projectfile.ui" line="443"/>
        <source>This is a workfolder that Cppcheck will use for various purposes.</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="projectfile.ui" line="460"/>
        <source>Parser</source>
        <translation>剖析器</translation>
    </message>
    <message>
        <location filename="projectfile.ui" line="466"/>
        <source>Cppcheck (built in)</source>
        <translation>Cppcheck (內建)</translation>
    </message>
    <message>
        <location filename="projectfile.ui" line="476"/>
        <source>Clang (experimental)</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="projectfile.ui" line="486"/>
        <source>Check level</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="projectfile.ui" line="492"/>
        <source>Reduced -- meant for usage where developer wants results directly. Limited and faster analysis with fewer results.</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="projectfile.ui" line="499"/>
        <source>Normal -- meant for normal analysis in CI. Analysis should finish in reasonable time.</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="projectfile.ui" line="506"/>
        <source>Exhaustive -- meant for nightly builds etc. Analysis time can be longer (10x slower than compilation is OK).</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="projectfile.ui" line="522"/>
        <source>If you want to design your classes to be as flexible and robust as possible then the public interface must be very robust. Cppcheck will asumme that arguments can take *any* value.</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="projectfile.ui" line="525"/>
        <source>Check that each class has a safe public interface</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="projectfile.ui" line="541"/>
        <source>Limit analysis</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="projectfile.ui" line="547"/>
        <source>Check code in headers  (should be ON normally. if you want a limited quick analysis then turn this OFF)</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="projectfile.ui" line="557"/>
        <source>Check code in unused templates (should be ON normally, however in theory you can safely ignore warnings in unused templates)</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="projectfile.ui" line="569"/>
        <source>Max CTU depth</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="projectfile.ui" line="600"/>
        <source>Max recursion in template instantiation</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="projectfile.ui" line="636"/>
        <source>Warning options</source>
        <translation>警告選項</translation>
    </message>
    <message>
        <location filename="projectfile.ui" line="642"/>
        <source>Root path:</source>
        <translation>根路徑:</translation>
    </message>
    <message>
        <location filename="projectfile.ui" line="648"/>
        <source>Filepaths in warnings will be relative to this path</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="projectfile.ui" line="658"/>
        <source>Warning tags (separated by semicolon)</source>
        <translation>警告標記 (由分號分隔)</translation>
    </message>
    <message>
        <location filename="projectfile.ui" line="664"/>
        <source>If tags are added, you will be able to right click on warnings and set one of these tags. You can manually categorize warnings.</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="projectfile.ui" line="674"/>
        <source>Exclude source files</source>
        <translation>排除來源檔</translation>
    </message>
    <message>
        <location filename="projectfile.ui" line="685"/>
        <source>Exclude folder...</source>
        <translation>排除資料夾...</translation>
    </message>
    <message>
        <location filename="projectfile.ui" line="692"/>
        <source>Exclude file...</source>
        <translation>排除檔案...</translation>
    </message>
    <message>
        <location filename="projectfile.ui" line="731"/>
        <source>Suppressions</source>
        <translation>抑制</translation>
    </message>
    <message>
        <location filename="projectfile.ui" line="742"/>
        <source>Add</source>
        <translation>新增</translation>
    </message>
    <message>
        <location filename="projectfile.ui" line="774"/>
        <source>Enable inline suppressions</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="projectfile.ui" line="795"/>
        <location filename="projectfile.ui" line="801"/>
        <source>Addons</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="projectfile.ui" line="807"/>
        <source>Note: Addons require &lt;a href=&quot;https://www.python.org/&quot;&gt;Python&lt;/a&gt; being installed.</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="projectfile.ui" line="817"/>
        <source>Y2038</source>
        <translation>Y2038</translation>
    </message>
    <message>
        <location filename="projectfile.ui" line="824"/>
        <source>Thread safety</source>
        <translation>執行緒安全</translation>
    </message>
    <message>
        <location filename="projectfile.ui" line="834"/>
        <source>Coding standards</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="projectfile.ui" line="842"/>
        <source>Misra C</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="projectfile.ui" line="850"/>
        <source>2012</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="projectfile.ui" line="855"/>
        <location filename="projectfile.ui" line="923"/>
        <source>2023</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="projectfile.ui" line="860"/>
        <source>2025</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="projectfile.ui" line="910"/>
        <source>Misra C++</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="projectfile.ui" line="918"/>
        <source>2008</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Misra C 2012</source>
        <translation type="vanished">Misra C 2012</translation>
    </message>
    <message>
        <location filename="projectfile.ui" line="885"/>
        <source>MISRA rule texts</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="projectfile.ui" line="892"/>
        <source>&lt;html&gt;&lt;head/&gt;&lt;body&gt;&lt;p&gt;Copy/paste the text from Appendix A &amp;quot;Summary of guidelines&amp;quot; from the MISRA C 2012 pdf to a text file.&lt;/p&gt;&lt;/body&gt;&lt;/html&gt;</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="projectfile.ui" line="899"/>
        <source>...</source>
        <translation>...</translation>
    </message>
    <message>
        <source>Misra C++ 2008</source>
        <translation type="vanished">Misra C++ 2008</translation>
    </message>
    <message>
        <location filename="projectfile.ui" line="946"/>
        <source>Cert C</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="projectfile.ui" line="955"/>
        <source>CERT-INT35-C:  int precision (if size equals precision, you can leave empty)</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="projectfile.ui" line="967"/>
        <source>Cert C++</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="projectfile.ui" line="974"/>
        <source>Autosar</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="projectfile.ui" line="981"/>
        <source>Safety profiles (defined in C++ core guidelines)</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="projectfile.ui" line="991"/>
        <source>Bug hunting (Premium)</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="projectfile.ui" line="997"/>
        <source>Bug hunting</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="projectfile.ui" line="1007"/>
        <source>External tools</source>
        <translation>外部工具</translation>
    </message>
    <message>
        <location filename="projectfile.ui" line="1013"/>
        <source>Clang-tidy</source>
        <translation>Clang-tidy</translation>
    </message>
    <message>
        <location filename="projectfile.ui" line="1020"/>
        <source>Clang analyzer</source>
        <translation>Clang 分析器</translation>
    </message>
</context>
<context>
    <name>ProjectFileDialog</name>
    <message>
        <location filename="projectfiledialog.cpp" line="137"/>
        <source>Project file: %1</source>
        <translation>專案檔: %1</translation>
    </message>
    <message>
        <location filename="projectfiledialog.cpp" line="395"/>
        <source>Note: Open source Cppcheck does not fully implement Misra C 2012</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="projectfiledialog.cpp" line="460"/>
        <source>Clang-tidy (not found)</source>
        <translation>Clang-tidy (找不到)</translation>
    </message>
    <message>
        <location filename="projectfiledialog.cpp" line="588"/>
        <source>Select Cppcheck build dir</source>
        <translation>選取 Cppcheck 建置目錄</translation>
    </message>
    <message>
        <location filename="projectfiledialog.cpp" line="627"/>
        <source>Visual Studio</source>
        <translation>Visual Studio</translation>
    </message>
    <message>
        <location filename="projectfiledialog.cpp" line="628"/>
        <source>Compile database</source>
        <translation>編譯資料庫</translation>
    </message>
    <message>
        <location filename="projectfiledialog.cpp" line="629"/>
        <source>Borland C++ Builder 6</source>
        <translation>Borland C++ Builder 6</translation>
    </message>
    <message>
        <location filename="projectfiledialog.cpp" line="630"/>
        <source>Import Project</source>
        <translation>匯入專案</translation>
    </message>
    <message>
        <location filename="projectfiledialog.cpp" line="649"/>
        <source>C/C++ header</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="projectfiledialog.cpp" line="651"/>
        <source>Include file</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="projectfiledialog.cpp" line="844"/>
        <source>Select a directory to check</source>
        <translation>選取要檢查的目錄</translation>
    </message>
    <message>
        <location filename="projectfiledialog.cpp" line="864"/>
        <source>Select include directory</source>
        <translation>選取包含目錄</translation>
    </message>
    <message>
        <location filename="projectfiledialog.cpp" line="884"/>
        <source>Select directory to ignore</source>
        <translation>選取要忽略的目錄</translation>
    </message>
    <message>
        <location filename="projectfiledialog.cpp" line="892"/>
        <source>Source files</source>
        <translation>來源檔</translation>
    </message>
    <message>
        <location filename="projectfiledialog.cpp" line="650"/>
        <location filename="projectfiledialog.cpp" line="893"/>
        <source>All files</source>
        <translation>所有檔案</translation>
    </message>
    <message>
        <location filename="projectfiledialog.cpp" line="894"/>
        <source>Exclude file</source>
        <translation>排除檔案</translation>
    </message>
    <message>
        <location filename="projectfiledialog.cpp" line="977"/>
        <source>Select MISRA rule texts file</source>
        <translation>選取 MISRA 規則文字檔</translation>
    </message>
    <message>
        <location filename="projectfiledialog.cpp" line="979"/>
        <source>MISRA rule texts file (%1)</source>
        <translation>MISRA 規則文字檔 (%1)</translation>
    </message>
</context>
<context>
    <name>QObject</name>
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
        <translation>編輯器前景色</translation>
    </message>
    <message>
        <location filename="codeeditstyledialog.cpp" line="83"/>
        <source>Editor Background Color</source>
        <translation>編輯器背景色</translation>
    </message>
    <message>
        <location filename="codeeditstyledialog.cpp" line="86"/>
        <source>Highlight Background Color</source>
        <translation>標明背景色</translation>
    </message>
    <message>
        <location filename="codeeditstyledialog.cpp" line="89"/>
        <source>Line Number Foreground Color</source>
        <translation>行號前景色</translation>
    </message>
    <message>
        <location filename="codeeditstyledialog.cpp" line="92"/>
        <source>Line Number Background Color</source>
        <translation>行號背景色</translation>
    </message>
    <message>
        <location filename="codeeditstyledialog.cpp" line="95"/>
        <source>Keyword Foreground Color</source>
        <translation>關鍵字前景色</translation>
    </message>
    <message>
        <location filename="codeeditstyledialog.cpp" line="98"/>
        <source>Keyword Font Weight</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="codeeditstyledialog.cpp" line="101"/>
        <source>Class Foreground Color</source>
        <translation>類別前景色</translation>
    </message>
    <message>
        <location filename="codeeditstyledialog.cpp" line="104"/>
        <source>Class Font Weight</source>
        <translation type="unfinished">類別字型粗細</translation>
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
        <translation>符號前景色</translation>
    </message>
    <message>
        <location filename="codeeditstyledialog.cpp" line="122"/>
        <source>Symbol Background Color</source>
        <translation>符號被景色</translation>
    </message>
    <message>
        <location filename="codeeditstyledialog.cpp" line="125"/>
        <source>Symbol Font Weight</source>
        <translation>符號字型粗細</translation>
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
        <translation> (找不到)</translation>
    </message>
    <message>
        <location filename="translationhandler.cpp" line="91"/>
        <source>Unknown language specified!</source>
        <translation>指定了未知語言！</translation>
    </message>
    <message>
        <location filename="translationhandler.cpp" line="117"/>
        <source>Language file %1 not found!</source>
        <translation>找不到語言檔 %1！</translation>
    </message>
    <message>
        <location filename="translationhandler.cpp" line="122"/>
        <source>Failed to load translation for language %1 from file %2</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="resultstree.cpp" line="113"/>
        <source>File</source>
        <translation type="unfinished">檔案</translation>
    </message>
    <message>
        <location filename="resultstree.cpp" line="114"/>
        <source>Line</source>
        <translation type="unfinished">行號</translation>
    </message>
    <message>
        <location filename="resultstree.cpp" line="115"/>
        <source>Severity</source>
        <translation type="unfinished">安全性</translation>
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
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="resultstree.cpp" line="120"/>
        <source>Id</source>
        <translation type="unfinished">識別號</translation>
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
        <translation>確認</translation>
    </message>
    <message>
        <location filename="translationhandler.cpp" line="42"/>
        <source>Cancel</source>
        <translation>取消</translation>
    </message>
    <message>
        <location filename="translationhandler.cpp" line="43"/>
        <source>Close</source>
        <translation>關閉</translation>
    </message>
    <message>
        <location filename="translationhandler.cpp" line="44"/>
        <source>Save</source>
        <translation>儲存</translation>
    </message>
</context>
<context>
    <name>ResultsTree</name>
    <message>
        <source>Undefined file</source>
        <translation type="vanished">未定義的檔案</translation>
    </message>
    <message>
        <location filename="resultstree.cpp" line="307"/>
        <source>note</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="resultstree.cpp" line="355"/>
        <source>style</source>
        <translation>樣式</translation>
    </message>
    <message>
        <location filename="resultstree.cpp" line="358"/>
        <source>error</source>
        <translation>錯誤</translation>
    </message>
    <message>
        <location filename="resultstree.cpp" line="361"/>
        <source>warning</source>
        <translation>警告</translation>
    </message>
    <message>
        <location filename="resultstree.cpp" line="364"/>
        <source>performance</source>
        <translation>效能</translation>
    </message>
    <message>
        <location filename="resultstree.cpp" line="367"/>
        <source>portability</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="resultstree.cpp" line="370"/>
        <source>information</source>
        <translation>資訊</translation>
    </message>
    <message>
        <location filename="resultstree.cpp" line="373"/>
        <source>debug</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="resultstree.cpp" line="376"/>
        <source>internal</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="resultstree.cpp" line="668"/>
        <source>Copy</source>
        <translation>複製</translation>
    </message>
    <message>
        <source>Hide</source>
        <translation type="vanished">隱藏</translation>
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
        <location filename="resultstree.cpp" line="692"/>
        <source>Suppress selected id(s)</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="resultstree.cpp" line="712"/>
        <source>Tag</source>
        <translation>標記</translation>
    </message>
    <message>
        <location filename="resultstree.cpp" line="714"/>
        <source>No tag</source>
        <translation>取消標記</translation>
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
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="resultstree.cpp" line="759"/>
        <source>No default editor application selected.

Please select the default editor application in preferences/Applications.</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="resultstree.cpp" line="779"/>
        <source>Could not find the file!</source>
        <translation>找不到該檔案！</translation>
    </message>
    <message>
        <location filename="resultstree.cpp" line="822"/>
        <source>Could not start %1

Please check the application path and parameters are correct.</source>
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
        <translation>請選取資料夾 &apos;%1&apos;</translation>
    </message>
    <message>
        <location filename="resultstree.cpp" line="841"/>
        <source>Select Directory &apos;%1&apos;</source>
        <translation>選取目錄 &apos;%1&apos;</translation>
    </message>
    <message>
        <location filename="resultstree.cpp" line="843"/>
        <source>Please select the directory where file is located.</source>
        <translation>請選取資料夾所在的目錄。</translation>
    </message>
    <message>
        <location filename="resultstree.cpp" line="844"/>
        <source>Select Directory</source>
        <translation>選取目錄</translation>
    </message>
    <message>
        <source>File</source>
        <translation type="vanished">檔案</translation>
    </message>
    <message>
        <source>Severity</source>
        <translation type="vanished">安全性</translation>
    </message>
    <message>
        <source>Line</source>
        <translation type="vanished">行號</translation>
    </message>
    <message>
        <source>Id</source>
        <translation type="vanished">識別號</translation>
    </message>
</context>
<context>
    <name>ResultsView</name>
    <message>
        <location filename="resultsview.ui" line="26"/>
        <source>Results</source>
        <translation>結果</translation>
    </message>
    <message>
        <location filename="resultsview.ui" line="60"/>
        <source>Critical errors</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="resultsview.ui" line="92"/>
        <source>Analysis Log</source>
        <translation>分析日誌</translation>
    </message>
    <message>
        <location filename="resultsview.ui" line="114"/>
        <source>Warning Details</source>
        <translation>警告詳細資訊</translation>
    </message>
    <message>
        <location filename="resultsview.cpp" line="236"/>
        <location filename="resultsview.cpp" line="244"/>
        <source>Failed to save the report.</source>
        <translation>無法載入報告。</translation>
    </message>
    <message>
        <location filename="resultsview.cpp" line="254"/>
        <source>Print Report</source>
        <translation>列印報告</translation>
    </message>
    <message>
        <location filename="resultsview.cpp" line="273"/>
        <source>No errors found, nothing to print.</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="resultsview.cpp" line="324"/>
        <source>%p% (%1 of %2 files checked)</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="resultsview.cpp" line="348"/>
        <location filename="resultsview.cpp" line="359"/>
        <source>Cppcheck</source>
        <translation>Cppcheck</translation>
    </message>
    <message>
        <location filename="resultsview.cpp" line="349"/>
        <source>No errors found.</source>
        <translation>找不到錯誤。</translation>
    </message>
    <message>
        <location filename="resultsview.cpp" line="356"/>
        <source>Errors were found, but they are configured to be hidden.
To toggle what kind of errors are shown, open view menu.</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="resultsview.cpp" line="398"/>
        <location filename="resultsview.cpp" line="417"/>
        <source>Failed to read the report.</source>
        <translation>無法讀取報告。</translation>
    </message>
    <message>
        <location filename="resultsview.cpp" line="405"/>
        <source>XML format version 1 is no longer supported.</source>
        <translation>不再支援 XML 格式版本 1。</translation>
    </message>
    <message>
        <location filename="resultsview.cpp" line="456"/>
        <source>First included by</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="resultsview.cpp" line="461"/>
        <source>Id</source>
        <translation>識別號</translation>
    </message>
    <message>
        <location filename="resultsview.cpp" line="535"/>
        <source>Clear Log</source>
        <translation>清除日誌</translation>
    </message>
    <message>
        <location filename="resultsview.cpp" line="536"/>
        <source>Copy this Log entry</source>
        <translation>複製該日誌條目</translation>
    </message>
    <message>
        <location filename="resultsview.cpp" line="537"/>
        <source>Copy complete Log</source>
        <translation>複製完整的日誌</translation>
    </message>
    <message>
        <location filename="resultsview.cpp" line="544"/>
        <source>Analysis was stopped</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="resultsview.cpp" line="558"/>
        <source>There was a critical error with id &apos;%1&apos;</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="resultsview.cpp" line="560"/>
        <source>when checking %1</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="resultsview.cpp" line="562"/>
        <source>when checking a file</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="resultsview.cpp" line="563"/>
        <source>Analysis was aborted.</source>
        <translation type="unfinished"></translation>
    </message>
</context>
<context>
    <name>ScratchPad</name>
    <message>
        <location filename="scratchpad.ui" line="14"/>
        <source>Scratchpad</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="scratchpad.ui" line="20"/>
        <source>Copy or write some C/C++ code here:</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="scratchpad.ui" line="37"/>
        <source>Optionally enter a filename (mainly for automatic language detection) and click on &quot;Check&quot;:</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="scratchpad.ui" line="71"/>
        <source>filename</source>
        <translation>檔案名稱</translation>
    </message>
    <message>
        <location filename="scratchpad.ui" line="78"/>
        <source>Check</source>
        <translation type="unfinished"></translation>
    </message>
</context>
<context>
    <name>Settings</name>
    <message>
        <location filename="settings.ui" line="14"/>
        <source>Preferences</source>
        <translation>偏好設定</translation>
    </message>
    <message>
        <location filename="settings.ui" line="24"/>
        <source>General</source>
        <translation>一般</translation>
    </message>
    <message>
        <location filename="settings.ui" line="41"/>
        <source>Number of threads: </source>
        <translation>執行緒數量: </translation>
    </message>
    <message>
        <location filename="settings.ui" line="79"/>
        <source>Max count:</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="settings.ui" line="108"/>
        <source>Force checking all #ifdef configurations</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="settings.ui" line="115"/>
        <source>Show full path of files</source>
        <translation>顯示檔案的完整路徑</translation>
    </message>
    <message>
        <location filename="settings.ui" line="122"/>
        <source>Show &quot;No errors found&quot; message when no errors found</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="settings.ui" line="129"/>
        <source>Display error Id in column &quot;Id&quot;</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="settings.ui" line="136"/>
        <source>Enable inline suppressions</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="settings.ui" line="143"/>
        <source>Check for inconclusive errors also</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="settings.ui" line="150"/>
        <source>Show statistics on check completion</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="settings.ui" line="157"/>
        <source>Check for updates</source>
        <translation>檢查更新</translation>
    </message>
    <message>
        <location filename="settings.ui" line="177"/>
        <source>Show internal warnings in log</source>
        <translation>顯示日誌中的內部警告</translation>
    </message>
    <message>
        <location filename="settings.ui" line="185"/>
        <source>Applications</source>
        <translation>應用程式</translation>
    </message>
    <message>
        <location filename="settings.ui" line="196"/>
        <source>Add...</source>
        <translation>新增...</translation>
    </message>
    <message>
        <location filename="settings.ui" line="203"/>
        <location filename="settings.ui" line="479"/>
        <source>Edit...</source>
        <translation>編輯...</translation>
    </message>
    <message>
        <location filename="settings.ui" line="210"/>
        <source>Remove</source>
        <translation>移除</translation>
    </message>
    <message>
        <location filename="settings.ui" line="217"/>
        <source>Set as default</source>
        <translation>設定為預設值</translation>
    </message>
    <message>
        <location filename="settings.ui" line="240"/>
        <source>Reports</source>
        <translation>報告</translation>
    </message>
    <message>
        <location filename="settings.ui" line="246"/>
        <source>Save all errors when creating report</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="settings.ui" line="253"/>
        <source>Save full path to files in reports</source>
        <translation>在報告中儲存檔案的完整路徑</translation>
    </message>
    <message>
        <location filename="settings.ui" line="274"/>
        <source>Language</source>
        <translation>語言</translation>
    </message>
    <message>
        <location filename="settings.ui" line="288"/>
        <source>Addons</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="settings.ui" line="294"/>
        <source>Python binary (leave this empty to use python in the PATH)</source>
        <translation type="unfinished"></translation>
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
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="settings.ui" line="332"/>
        <source>MISRA rule texts file</source>
        <translation>MISRA 規則文字檔</translation>
    </message>
    <message>
        <location filename="settings.ui" line="339"/>
        <source>&lt;html&gt;&lt;head/&gt;&lt;body&gt;&lt;p&gt;Copy/paste the text from Appendix A &amp;quot;Summary of guidelines&amp;quot; from the MISRA C 2012 pdf to a text file.&lt;/p&gt;&lt;/body&gt;&lt;/html&gt;</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="settings.ui" line="372"/>
        <source>Clang</source>
        <translation>Clang</translation>
    </message>
    <message>
        <location filename="settings.ui" line="378"/>
        <source>Clang path (leave empty to use system PATH)</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="settings.ui" line="401"/>
        <source>Visual Studio headers</source>
        <translation>Visual Studio 標頭檔</translation>
    </message>
    <message>
        <location filename="settings.ui" line="407"/>
        <source>&lt;html&gt;&lt;head/&gt;&lt;body&gt;&lt;p&gt;Paths to Visual Studio headers, separated by semicolon &apos;;&apos;.&lt;/p&gt;&lt;p&gt;You can open a Visual Studio command prompt, write &amp;quot;SET INCLUDE&amp;quot;. Then copy/paste the paths.&lt;/p&gt;&lt;/body&gt;&lt;/html&gt;</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="settings.ui" line="437"/>
        <source>Code Editor</source>
        <translation>程式碼編輯器</translation>
    </message>
    <message>
        <location filename="settings.ui" line="443"/>
        <source>Code Editor Style</source>
        <translation>程式碼編輯器樣式</translation>
    </message>
    <message>
        <location filename="settings.ui" line="449"/>
        <source>System Style</source>
        <translation>系統樣式</translation>
    </message>
    <message>
        <location filename="settings.ui" line="456"/>
        <source>Default Light Style</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="settings.ui" line="463"/>
        <source>Default Dark Style</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="settings.ui" line="472"/>
        <source>Custom</source>
        <translation>自訂</translation>
    </message>
</context>
<context>
    <name>SettingsDialog</name>
    <message>
        <location filename="settingsdialog.cpp" line="232"/>
        <source>The executable file &quot;%1&quot; is not available</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="settingsdialog.cpp" line="243"/>
        <source>Add a new application</source>
        <translation>新增一個新應用程式</translation>
    </message>
    <message>
        <location filename="settingsdialog.cpp" line="273"/>
        <source>Modify an application</source>
        <translation>修改一個應用程式</translation>
    </message>
    <message>
        <location filename="settingsdialog.cpp" line="278"/>
        <source> [Default]</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="settingsdialog.cpp" line="303"/>
        <source>[Default]</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="settingsdialog.cpp" line="358"/>
        <source>Select python binary</source>
        <translation>選取 python 二進位檔</translation>
    </message>
    <message>
        <location filename="settingsdialog.cpp" line="365"/>
        <source>Select MISRA File</source>
        <translation>選取 MISRA 檔案</translation>
    </message>
    <message>
        <location filename="settingsdialog.cpp" line="396"/>
        <source>Select clang path</source>
        <translation>選取 clang 路徑</translation>
    </message>
</context>
<context>
    <name>StatsDialog</name>
    <message>
        <location filename="statsdialog.ui" line="14"/>
        <location filename="statsdialog.ui" line="248"/>
        <location filename="statsdialog.cpp" line="184"/>
        <location filename="statsdialog.cpp" line="231"/>
        <source>Statistics</source>
        <translation>統計資料</translation>
    </message>
    <message>
        <location filename="statsdialog.ui" line="27"/>
        <location filename="statsdialog.cpp" line="222"/>
        <source>Project</source>
        <translation>專案</translation>
    </message>
    <message>
        <location filename="statsdialog.ui" line="33"/>
        <source>Project:</source>
        <translation>專案:</translation>
    </message>
    <message>
        <location filename="statsdialog.ui" line="53"/>
        <source>Paths:</source>
        <translation>路徑:</translation>
    </message>
    <message>
        <location filename="statsdialog.ui" line="85"/>
        <source>Include paths:</source>
        <translation>包含路徑:</translation>
    </message>
    <message>
        <location filename="statsdialog.ui" line="108"/>
        <source>Defines:</source>
        <translation>定義:</translation>
    </message>
    <message>
        <location filename="statsdialog.ui" line="131"/>
        <source>Undefines:</source>
        <translation>未定義:</translation>
    </message>
    <message>
        <location filename="statsdialog.ui" line="165"/>
        <location filename="statsdialog.cpp" line="227"/>
        <source>Previous Scan</source>
        <translation>上一次掃描</translation>
    </message>
    <message>
        <location filename="statsdialog.ui" line="171"/>
        <source>Path Selected:</source>
        <translation>選取的路徑:</translation>
    </message>
    <message>
        <location filename="statsdialog.ui" line="181"/>
        <source>Number of Files Scanned:</source>
        <translation>已掃描的檔案數量:</translation>
    </message>
    <message>
        <location filename="statsdialog.ui" line="201"/>
        <source>Scan Duration:</source>
        <translation>掃描時間:</translation>
    </message>
    <message>
        <location filename="statsdialog.ui" line="254"/>
        <source>Errors:</source>
        <translation>錯誤:</translation>
    </message>
    <message>
        <location filename="statsdialog.ui" line="271"/>
        <source>Warnings:</source>
        <translation>警告:</translation>
    </message>
    <message>
        <location filename="statsdialog.ui" line="288"/>
        <source>Stylistic warnings:</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="statsdialog.ui" line="305"/>
        <source>Portability warnings:</source>
        <translation>可移植性警告:</translation>
    </message>
    <message>
        <location filename="statsdialog.ui" line="322"/>
        <source>Performance issues:</source>
        <translation>效能議題:</translation>
    </message>
    <message>
        <location filename="statsdialog.ui" line="339"/>
        <source>Information messages:</source>
        <translation>資訊訊息:</translation>
    </message>
    <message>
        <location filename="statsdialog.ui" line="356"/>
        <source>Active checkers:</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="statsdialog.ui" line="374"/>
        <source>Checkers</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="statsdialog.ui" line="399"/>
        <source>History</source>
        <translation>歷史紀錄</translation>
    </message>
    <message>
        <location filename="statsdialog.ui" line="405"/>
        <source>File:</source>
        <translation>檔案:</translation>
    </message>
    <message>
        <location filename="statsdialog.ui" line="443"/>
        <source>Copy to Clipboard</source>
        <translation>複製到剪貼簿</translation>
    </message>
    <message>
        <location filename="statsdialog.ui" line="450"/>
        <source>Pdf Export</source>
        <translation>Pdf 匯出</translation>
    </message>
    <message>
        <location filename="statsdialog.cpp" line="112"/>
        <source>File: </source>
        <translation>檔案: </translation>
    </message>
    <message>
        <location filename="statsdialog.cpp" line="112"/>
        <source>No cppcheck build dir</source>
        <translation>沒有 cppcheck 建置目錄</translation>
    </message>
    <message>
        <location filename="statsdialog.cpp" line="160"/>
        <source>1 day</source>
        <translation>1 天</translation>
    </message>
    <message>
        <location filename="statsdialog.cpp" line="160"/>
        <source>%1 days</source>
        <translation>%1 天</translation>
    </message>
    <message>
        <location filename="statsdialog.cpp" line="162"/>
        <source>1 hour</source>
        <translation>1 小時</translation>
    </message>
    <message>
        <location filename="statsdialog.cpp" line="162"/>
        <source>%1 hours</source>
        <translation>%1 小時</translation>
    </message>
    <message>
        <location filename="statsdialog.cpp" line="164"/>
        <source>1 minute</source>
        <translation>1 分鐘</translation>
    </message>
    <message>
        <location filename="statsdialog.cpp" line="164"/>
        <source>%1 minutes</source>
        <translation>%1 分鐘</translation>
    </message>
    <message>
        <location filename="statsdialog.cpp" line="166"/>
        <source>1 second</source>
        <translation>1 秒鐘</translation>
    </message>
    <message>
        <location filename="statsdialog.cpp" line="166"/>
        <source>%1 seconds</source>
        <translation>%1 秒鐘</translation>
    </message>
    <message>
        <location filename="statsdialog.cpp" line="170"/>
        <source>0.%1 seconds</source>
        <translation>0.%1 秒鐘</translation>
    </message>
    <message>
        <location filename="statsdialog.cpp" line="172"/>
        <source> and </source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="statsdialog.cpp" line="186"/>
        <location filename="statsdialog.cpp" line="232"/>
        <source>Errors</source>
        <translation>錯誤</translation>
    </message>
    <message>
        <location filename="statsdialog.cpp" line="188"/>
        <location filename="statsdialog.cpp" line="233"/>
        <source>Warnings</source>
        <translation>警告</translation>
    </message>
    <message>
        <location filename="statsdialog.cpp" line="190"/>
        <location filename="statsdialog.cpp" line="234"/>
        <source>Style warnings</source>
        <translation>樣式警告</translation>
    </message>
    <message>
        <location filename="statsdialog.cpp" line="192"/>
        <location filename="statsdialog.cpp" line="235"/>
        <source>Portability warnings</source>
        <translation>可移植性警告</translation>
    </message>
    <message>
        <location filename="statsdialog.cpp" line="194"/>
        <location filename="statsdialog.cpp" line="236"/>
        <source>Performance warnings</source>
        <translation>效能警告</translation>
    </message>
    <message>
        <location filename="statsdialog.cpp" line="196"/>
        <location filename="statsdialog.cpp" line="237"/>
        <source>Information messages</source>
        <translation>資訊訊息</translation>
    </message>
    <message>
        <location filename="statsdialog.cpp" line="199"/>
        <source>Export PDF</source>
        <translation>匯出 PDF</translation>
    </message>
    <message>
        <location filename="statsdialog.cpp" line="221"/>
        <source>Project Settings</source>
        <translation>專案設定</translation>
    </message>
    <message>
        <location filename="statsdialog.cpp" line="223"/>
        <source>Paths</source>
        <translation>路徑</translation>
    </message>
    <message>
        <location filename="statsdialog.cpp" line="224"/>
        <source>Include paths</source>
        <translation>包含路徑</translation>
    </message>
    <message>
        <location filename="statsdialog.cpp" line="225"/>
        <source>Defines</source>
        <translation>定義</translation>
    </message>
    <message>
        <location filename="statsdialog.cpp" line="226"/>
        <source>Undefines</source>
        <translation>未定義</translation>
    </message>
    <message>
        <location filename="statsdialog.cpp" line="228"/>
        <source>Path selected</source>
        <translation>選取的路徑</translation>
    </message>
    <message>
        <location filename="statsdialog.cpp" line="229"/>
        <source>Number of files scanned</source>
        <translation>已掃描的檔案數量</translation>
    </message>
    <message>
        <location filename="statsdialog.cpp" line="230"/>
        <source>Scan duration</source>
        <translation>掃描時間</translation>
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
        <translation type="unfinished"></translation>
    </message>
</context>
<context>
    <name>TranslationHandler</name>
    <message>
        <location filename="translationhandler.cpp" line="130"/>
        <source>Failed to change the user interface language:

%1

The user interface language has been reset to English. Open the Preferences-dialog to select any of the available languages.</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="translationhandler.cpp" line="136"/>
        <source>Cppcheck</source>
        <translation></translation>
    </message>
</context>
<context>
    <name>TxtReport</name>
    <message>
        <location filename="txtreport.cpp" line="74"/>
        <source>inconclusive</source>
        <translation type="unfinished"></translation>
    </message>
</context>
<context>
    <name>toFilterString</name>
    <message>
        <location filename="common.cpp" line="58"/>
        <source>All supported files (%1)</source>
        <translation>所有支援的檔案 (%1)</translation>
    </message>
    <message>
        <location filename="common.cpp" line="63"/>
        <source>All files (%1)</source>
        <translation>所有檔案 (%1)</translation>
    </message>
</context>
</TS>
