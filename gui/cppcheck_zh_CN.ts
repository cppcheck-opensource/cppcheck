<?xml version="1.0" encoding="utf-8"?>
<!DOCTYPE TS>
<TS version="2.1" language="zh_CN">
<context>
    <name>About</name>
    <message>
        <location filename="about.ui" line="14"/>
        <source>About Cppcheck</source>
        <translation>关于 Cppcheck</translation>
    </message>
    <message>
        <location filename="about.ui" line="64"/>
        <source>Version %1</source>
        <translation>版本 %1</translation>
    </message>
    <message>
        <location filename="about.ui" line="71"/>
        <source>Cppcheck - A tool for static C/C++ code analysis.</source>
        <translation>Cppcheck - C/C++ 静态代码分析工具。</translation>
    </message>
    <message>
        <location filename="about.ui" line="81"/>
        <source>Copyright © 2007-%1 Cppcheck team.</source>
        <oldsource>Copyright © 2007-2021 Cppcheck team.</oldsource>
        <translation>版权所有 © 2007-%1 Cppcheck 团队。</translation>
    </message>
    <message>
        <location filename="about.ui" line="91"/>
        <source>This program is licensed under the terms
of the GNU General Public License version 3</source>
        <translation>该程序在 GNU 通用公共授权版本 3 的条款下发布</translation>
    </message>
    <message>
        <location filename="about.ui" line="102"/>
        <source>Visit Cppcheck homepage at %1</source>
        <translation>访问 Cppcheck 主页: %1</translation>
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
        <translation>添加应用程序</translation>
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
        <translation>在这里，你可以添加一个应用程序，用于打开错误文件。请为该应用程序指定一个名称、可执行文件和命令行参数。

当应用程序执行时，在参数中的以下文本将会替换为适当的值:
(file) - 包含错误的文件名称
(line) - 包含错误的行号
(message) - 错误消息
(severity) - 错误严重性

示例：使用 Kate 打开一个文件，并使之滚动到相应的行:
可执行文件: kate
参数: -l(line) (file)</translation>
    </message>
    <message>
        <location filename="applicationdialog.ui" line="76"/>
        <source>&amp;Name:</source>
        <translation>名称(&amp;N):</translation>
    </message>
    <message>
        <location filename="applicationdialog.ui" line="86"/>
        <source>&amp;Executable:</source>
        <translation>可执行文件(&amp;E):</translation>
    </message>
    <message>
        <location filename="applicationdialog.ui" line="96"/>
        <source>&amp;Parameters:</source>
        <translation>参数(&amp;P):</translation>
    </message>
    <message>
        <location filename="applicationdialog.ui" line="138"/>
        <source>Browse</source>
        <translation>浏览</translation>
    </message>
    <message>
        <location filename="applicationdialog.cpp" line="65"/>
        <source>Executable files (*.exe);;All files(*.*)</source>
        <translation>可执行文件(*.exe);;所有文件(*.*)</translation>
    </message>
    <message>
        <location filename="applicationdialog.cpp" line="68"/>
        <source>Select viewer application</source>
        <translation>选择查看应用程序</translation>
    </message>
    <message>
        <location filename="applicationdialog.cpp" line="83"/>
        <source>Cppcheck</source>
        <translation>Cppcheck</translation>
    </message>
    <message>
        <location filename="applicationdialog.cpp" line="84"/>
        <source>You must specify a name, a path and optionally parameters for the application!</source>
        <translation>你必须为应用程序指定名称、路径以及可选参数！</translation>
    </message>
</context>
<context>
    <name>ComplianceReportDialog</name>
    <message>
        <location filename="compliancereportdialog.ui" line="14"/>
        <source>Compliance Report</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="compliancereportdialog.ui" line="29"/>
        <source>Project name</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="compliancereportdialog.ui" line="22"/>
        <source>Project version</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="compliancereportdialog.ui" line="42"/>
        <source>Coding Standard</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="compliancereportdialog.ui" line="50"/>
        <source>Misra C</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="compliancereportdialog.ui" line="55"/>
        <source>Cert C</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="compliancereportdialog.ui" line="60"/>
        <source>Cert C++</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="compliancereportdialog.ui" line="70"/>
        <source>List of files with md5 checksums</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="compliancereportdialog.cpp" line="133"/>
        <source>Compliance report</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="compliancereportdialog.cpp" line="135"/>
        <source>HTML files (*.html)</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="compliancereportdialog.cpp" line="176"/>
        <location filename="compliancereportdialog.cpp" line="239"/>
        <source>Save compliance report</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="compliancereportdialog.cpp" line="177"/>
        <source>Failed to import &apos;%1&apos; (%2), can not show files in compliance report</source>
        <translation type="unfinished"></translation>
    </message>
</context>
<context>
    <name>FileViewDialog</name>
    <message>
        <location filename="fileviewdialog.cpp" line="53"/>
        <source>Could not find the file: %1</source>
        <translation>无法找到文件: %1</translation>
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
        <translation>无法读取文件: %1</translation>
    </message>
</context>
<context>
    <name>HelpDialog</name>
    <message>
        <location filename="helpdialog.ui" line="14"/>
        <source>Cppcheck GUI help</source>
        <translation>Cppcheck GUI 帮助</translation>
    </message>
    <message>
        <location filename="helpdialog.ui" line="34"/>
        <source>Contents</source>
        <translation>内容</translation>
    </message>
    <message>
        <location filename="helpdialog.ui" line="44"/>
        <source>Index</source>
        <translation>索引</translation>
    </message>
    <message>
        <location filename="helpdialog.cpp" line="84"/>
        <source>Helpfile &apos;%1&apos; was not found</source>
        <translation>帮助文件 &apos;%1&apos; 未找到</translation>
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
        <translation>添加函数</translation>
    </message>
    <message>
        <location filename="libraryaddfunctiondialog.ui" line="34"/>
        <source>Function name(s)</source>
        <translation>函数名</translation>
    </message>
    <message>
        <location filename="libraryaddfunctiondialog.ui" line="44"/>
        <source>Number of arguments</source>
        <translation>参数个数</translation>
    </message>
</context>
<context>
    <name>LibraryDialog</name>
    <message>
        <location filename="librarydialog.ui" line="14"/>
        <source>Library Editor</source>
        <translation>库编辑器</translation>
    </message>
    <message>
        <location filename="librarydialog.ui" line="22"/>
        <source>Open</source>
        <translation>打开</translation>
    </message>
    <message>
        <location filename="librarydialog.ui" line="29"/>
        <source>Save</source>
        <translation>保存</translation>
    </message>
    <message>
        <location filename="librarydialog.ui" line="36"/>
        <source>Save as</source>
        <translation>另存为</translation>
    </message>
    <message>
        <location filename="librarydialog.ui" line="62"/>
        <source>Functions</source>
        <translation>函数</translation>
    </message>
    <message>
        <location filename="librarydialog.ui" line="71"/>
        <source>Sort</source>
        <translation>排序</translation>
    </message>
    <message>
        <location filename="librarydialog.ui" line="111"/>
        <source>Add</source>
        <translation>添加</translation>
    </message>
    <message>
        <location filename="librarydialog.ui" line="131"/>
        <source>Filter:</source>
        <translation>过滤：</translation>
    </message>
    <message>
        <location filename="librarydialog.ui" line="164"/>
        <source>Comments</source>
        <translation>注释</translation>
    </message>
    <message>
        <location filename="librarydialog.ui" line="204"/>
        <source>noreturn</source>
        <translation>无返回</translation>
    </message>
    <message>
        <location filename="librarydialog.ui" line="212"/>
        <source>False</source>
        <translation>否</translation>
    </message>
    <message>
        <location filename="librarydialog.ui" line="217"/>
        <source>True</source>
        <translation>是</translation>
    </message>
    <message>
        <location filename="librarydialog.ui" line="222"/>
        <source>Unknown</source>
        <translation>未知</translation>
    </message>
    <message>
        <location filename="librarydialog.ui" line="232"/>
        <source>return value must be used</source>
        <translation>返回值必须被使用</translation>
    </message>
    <message>
        <location filename="librarydialog.ui" line="239"/>
        <source>ignore function in leaks checking</source>
        <translation>在泄漏检查中忽略函数</translation>
    </message>
    <message>
        <location filename="librarydialog.ui" line="246"/>
        <source>Arguments</source>
        <translation>参数</translation>
    </message>
    <message>
        <location filename="librarydialog.ui" line="258"/>
        <source>Edit</source>
        <translation>编辑</translation>
    </message>
    <message>
        <location filename="librarydialog.cpp" line="97"/>
        <location filename="librarydialog.cpp" line="169"/>
        <source>Library files (*.cfg)</source>
        <translation>库文件 (*.cfg)</translation>
    </message>
    <message>
        <location filename="librarydialog.cpp" line="99"/>
        <source>Open library file</source>
        <translation>打开库文件</translation>
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
        <oldsource>Can not open file %1.</oldsource>
        <translation>无法打开文件 %1。</translation>
    </message>
    <message>
        <location filename="librarydialog.cpp" line="123"/>
        <source>Failed to load %1. %2.</source>
        <translation>加载文件 %1 失败。%2。</translation>
    </message>
    <message>
        <location filename="librarydialog.cpp" line="160"/>
        <source>Cannot save file %1.</source>
        <oldsource>Can not save file %1.</oldsource>
        <translation>无法保存文件 %1。</translation>
    </message>
    <message>
        <location filename="librarydialog.cpp" line="172"/>
        <source>Save the library as</source>
        <translation>库另存为</translation>
    </message>
</context>
<context>
    <name>LibraryEditArgDialog</name>
    <message>
        <location filename="libraryeditargdialog.ui" line="14"/>
        <source>Edit argument</source>
        <translation>编辑参数</translation>
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
&lt;p&gt;是否允许布尔值？ 例如，来自比较结果或来自 &apos;!&apos; 操作符。&lt;/p&gt;
&lt;p&gt;通常，如果参数是指针、大小等，则设置此参数。&lt;/p&gt;
&lt;p&gt;例子：&lt;/p&gt;
&lt;pre&gt;    memcmp(x, y, i == 123);   // 最后一个参数不应该有bool值&lt;/pre&gt;
&lt;/body&gt;&lt;/html&gt;</translation>
    </message>
    <message>
        <location filename="libraryeditargdialog.ui" line="28"/>
        <source>Not bool</source>
        <translation>非布尔值</translation>
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
&lt;p&gt;是否允许空参数值?&lt;/p&gt;
&lt;p&gt;通常这应该用于任何不允许空指针的参数。&lt;/p&gt;
&lt;p&gt;例子：&lt;/p&gt;
&lt;pre&gt;    strcpy(x,y); // 无论 x 或 y 都不允许为空。&lt;/pre&gt;
&lt;/body&gt;&lt;/html&gt;</translation>
    </message>
    <message>
        <location filename="libraryeditargdialog.ui" line="43"/>
        <source>Not null</source>
        <translation>非空</translation>
    </message>
    <message>
        <location filename="libraryeditargdialog.ui" line="50"/>
        <source>Not uninit</source>
        <translation>非未初始化</translation>
    </message>
    <message>
        <location filename="libraryeditargdialog.ui" line="57"/>
        <source>String</source>
        <translation>字符串</translation>
    </message>
    <message>
        <location filename="libraryeditargdialog.ui" line="70"/>
        <source>Format string</source>
        <translation>格式化字符串</translation>
    </message>
    <message>
        <location filename="libraryeditargdialog.ui" line="92"/>
        <source>Min size of buffer</source>
        <translation>最小缓冲区大小</translation>
    </message>
    <message>
        <location filename="libraryeditargdialog.ui" line="101"/>
        <location filename="libraryeditargdialog.ui" line="203"/>
        <source>Type</source>
        <translation>类型</translation>
    </message>
    <message>
        <location filename="libraryeditargdialog.ui" line="109"/>
        <location filename="libraryeditargdialog.ui" line="214"/>
        <source>None</source>
        <translation>无</translation>
    </message>
    <message>
        <location filename="libraryeditargdialog.ui" line="114"/>
        <location filename="libraryeditargdialog.ui" line="219"/>
        <source>argvalue</source>
        <translation>argvalue</translation>
    </message>
    <message>
        <location filename="libraryeditargdialog.ui" line="119"/>
        <location filename="libraryeditargdialog.ui" line="224"/>
        <source>mul</source>
        <translation>mul</translation>
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
        <translation>参数1</translation>
    </message>
    <message>
        <location filename="libraryeditargdialog.ui" line="159"/>
        <location filename="libraryeditargdialog.ui" line="264"/>
        <source>Arg2</source>
        <translation>参数2</translation>
    </message>
    <message>
        <location filename="libraryeditargdialog.ui" line="194"/>
        <source>and</source>
        <translation>并且</translation>
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
        <location filename="mainwindow.ui" line="132"/>
        <source>&amp;File</source>
        <translation>文件(&amp;F)</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="152"/>
        <source>&amp;View</source>
        <translation>查看(&amp;V)</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="156"/>
        <source>&amp;Toolbars</source>
        <translation>工具栏(&amp;T)</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="199"/>
        <source>&amp;Help</source>
        <translation>帮助(&amp;H)</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="214"/>
        <source>C++ standard</source>
        <translation>C++ 标准</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="226"/>
        <source>&amp;C standard</source>
        <oldsource>C standard</oldsource>
        <translation>&amp;C 标准</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="254"/>
        <source>&amp;Edit</source>
        <translation>编辑(&amp;E)</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="267"/>
        <source>Standard</source>
        <translation>标准</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="285"/>
        <source>Categories</source>
        <translation>分类</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="315"/>
        <source>&amp;License...</source>
        <translation>许可证(&amp;L)...</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="320"/>
        <source>A&amp;uthors...</source>
        <translation>作者(&amp;U)...</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="329"/>
        <source>&amp;About...</source>
        <translation>关于(&amp;A)...</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="334"/>
        <source>&amp;Files...</source>
        <translation>文件(&amp;F)...</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="337"/>
        <location filename="mainwindow.ui" line="340"/>
        <source>Analyze files</source>
        <oldsource>Check files</oldsource>
        <translation>分析文件</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="343"/>
        <source>Ctrl+F</source>
        <translation>Ctrl+F</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="352"/>
        <source>&amp;Directory...</source>
        <translation>目录(&amp;D)...</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="355"/>
        <location filename="mainwindow.ui" line="358"/>
        <source>Analyze directory</source>
        <oldsource>Check directory</oldsource>
        <translation>分析目录</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="361"/>
        <source>Ctrl+D</source>
        <translation>Ctrl+D</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="373"/>
        <source>Ctrl+R</source>
        <translation>Ctrl+R</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="391"/>
        <source>&amp;Stop</source>
        <translation>停止(&amp;S)</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="394"/>
        <location filename="mainwindow.ui" line="397"/>
        <source>Stop analysis</source>
        <oldsource>Stop checking</oldsource>
        <translation>停止分析</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="400"/>
        <source>Esc</source>
        <translation>Esc</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="409"/>
        <source>&amp;Save results to file...</source>
        <translation>保存结果到文件(&amp;S)...</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="412"/>
        <source>Ctrl+S</source>
        <translation>Ctrl+S</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="417"/>
        <source>&amp;Quit</source>
        <translation>退出(&amp;Q)</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="429"/>
        <source>&amp;Clear results</source>
        <translation>清空结果(&amp;C)</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="438"/>
        <source>&amp;Preferences</source>
        <translation>首选项(&amp;P)</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="453"/>
        <location filename="mainwindow.ui" line="456"/>
        <source>Show style warnings</source>
        <translation>显示风格警告</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="471"/>
        <location filename="mainwindow.ui" line="474"/>
        <location filename="mainwindow.cpp" line="2394"/>
        <source>Show errors</source>
        <translation>显示错误</translation>
    </message>
    <message>
        <location filename="mainwindow.cpp" line="986"/>
        <location filename="mainwindow.cpp" line="1024"/>
        <source>Information</source>
        <translation>信息</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="661"/>
        <source>Show information messages</source>
        <translation>显示信息消息</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="676"/>
        <source>Show portability warnings</source>
        <translation>显示可移植性警告</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="691"/>
        <source>Show Cppcheck results</source>
        <translation>显示 Cppcheck 结果</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="703"/>
        <source>Clang</source>
        <translation>Clang</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="706"/>
        <source>Show Clang results</source>
        <translation>显示 Clang 结果</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="714"/>
        <source>&amp;Filter</source>
        <translation>滤器(&amp;F)</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="717"/>
        <source>Filter results</source>
        <translation>过滤结果</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="733"/>
        <source>Windows 32-bit ANSI</source>
        <translation></translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="741"/>
        <source>Windows 32-bit Unicode</source>
        <translation></translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="749"/>
        <source>Unix 32-bit</source>
        <translation></translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="757"/>
        <source>Unix 64-bit</source>
        <translation></translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="765"/>
        <source>Windows 64-bit</source>
        <translation></translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="859"/>
        <source>&amp;Print...</source>
        <translation>打印(&amp;P)...</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="862"/>
        <source>Print the Current Report</source>
        <translation>打印当前报告</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="867"/>
        <source>Print Pre&amp;view...</source>
        <translation>打印预览(&amp;v)...</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="870"/>
        <source>Open a Print Preview Dialog for the Current Results</source>
        <translation>打开当前结果的打印预览窗口</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="878"/>
        <source>Open library editor</source>
        <translation>打开库编辑器</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="582"/>
        <source>C&amp;lose Project File</source>
        <translation>关闭项目文件(&amp;L)</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="590"/>
        <source>&amp;Edit Project File...</source>
        <translation>编辑项目文件(&amp;E)...</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="602"/>
        <source>&amp;Statistics</source>
        <translation>统计(&amp;S)</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="617"/>
        <location filename="mainwindow.ui" line="620"/>
        <location filename="mainwindow.cpp" line="2395"/>
        <source>Show warnings</source>
        <translation>显示警告</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="635"/>
        <location filename="mainwindow.ui" line="638"/>
        <source>Show performance warnings</source>
        <translation>显示性能警告</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="646"/>
        <source>Show &amp;hidden</source>
        <translation>显示隐藏项(&amp;H)</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="479"/>
        <source>&amp;Check all</source>
        <translation>全部选中(&amp;C)</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="84"/>
        <source>Checking for updates</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="100"/>
        <source>Hide</source>
        <translation type="unfinished">隐藏</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="164"/>
        <location filename="mainwindow.ui" line="980"/>
        <source>Report</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="210"/>
        <source>A&amp;nalyze</source>
        <translation>分析(&amp;A)</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="304"/>
        <source>Filter</source>
        <translation>滤器</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="370"/>
        <source>&amp;Reanalyze modified files</source>
        <oldsource>&amp;Recheck modified files</oldsource>
        <translation>重新分析已修改的文件(&amp;R)</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="382"/>
        <source>Reanal&amp;yze all files</source>
        <translation>重新分析全部文件(&amp;y)</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="420"/>
        <source>Ctrl+Q</source>
        <translation>Ctrl+Q</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="450"/>
        <source>Style war&amp;nings</source>
        <translation>风格警告(&amp;n)</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="468"/>
        <source>E&amp;rrors</source>
        <translation>编辑(&amp;r)</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="484"/>
        <source>&amp;Uncheck all</source>
        <translation>全部取消选中(&amp;U)</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="489"/>
        <source>Collapse &amp;all</source>
        <translation>全部折叠(&amp;A)</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="494"/>
        <source>&amp;Expand all</source>
        <translation>全部展开(&amp;E)</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="502"/>
        <source>&amp;Standard</source>
        <translation>标准(&amp;S)</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="505"/>
        <source>Standard items</source>
        <translation>标准项</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="510"/>
        <source>&amp;Contents</source>
        <translation>内容(&amp;C)</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="513"/>
        <source>Open the help contents</source>
        <translation>打开帮助内容</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="516"/>
        <source>F1</source>
        <translation>F1</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="521"/>
        <source>Toolbar</source>
        <translation>工具栏</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="529"/>
        <source>&amp;Categories</source>
        <translation>分类(&amp;C)</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="532"/>
        <source>Error categories</source>
        <translation>错误分类</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="537"/>
        <source>&amp;Open XML...</source>
        <translation>打开 XML (&amp;O)...</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="546"/>
        <source>Open P&amp;roject File...</source>
        <translation>打开项目文件(&amp;R)...</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="549"/>
        <source>Ctrl+Shift+O</source>
        <translation>Ctrl+Shift+O</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="558"/>
        <source>Sh&amp;ow Scratchpad...</source>
        <translation>显示便条(&amp;o)...</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="563"/>
        <source>&amp;New Project File...</source>
        <translation>新建项目文件(&amp;N)...</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="566"/>
        <source>Ctrl+Shift+N</source>
        <translation>Ctrl+Shift+N</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="571"/>
        <source>&amp;Log View</source>
        <translation>日志视图(&amp;L)</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="574"/>
        <source>Log View</source>
        <translation>日志视图</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="614"/>
        <source>&amp;Warnings</source>
        <translation>警告(&amp;W)</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="632"/>
        <source>Per&amp;formance warnings</source>
        <translation>性能警告(&amp;f)</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="658"/>
        <source>&amp;Information</source>
        <translation>信息(&amp;I)</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="673"/>
        <source>&amp;Portability</source>
        <translation>可移植性(&amp;P)</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="773"/>
        <source>P&amp;latforms</source>
        <translation>平台(&amp;l)</translation>
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
        <translation>&amp;Posix</translation>
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
        <translation>库编辑器(&amp;L)...</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="886"/>
        <source>&amp;Auto-detect language</source>
        <translation>自动检测语言(&amp;A)</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="894"/>
        <source>&amp;Enforce C++</source>
        <translation>&amp;Enforce C++</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="902"/>
        <source>E&amp;nforce C</source>
        <translation>E&amp;nforce C</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="913"/>
        <source>C++14</source>
        <translation>C++14</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="921"/>
        <source>Reanalyze and check library</source>
        <translation>重新分析并检查库</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="929"/>
        <source>Check configuration (defines, includes)</source>
        <translation>检查配置(defines, includes)</translation>
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
        <translation type="unfinished">常规</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="996"/>
        <source>Misra C</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="1004"/>
        <source>Misra C++ 2008</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="1012"/>
        <source>Cert C</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="1020"/>
        <source>Cert C++</source>
        <translation type="unfinished"></translation>
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
        <location filename="mainwindow.cpp" line="461"/>
        <source>There was a problem with loading the editor application settings.

This is probably because the settings were changed between the Cppcheck versions. Please check (and fix) the editor application settings, otherwise the editor program might not start correctly.</source>
        <translation>加载编辑器应用程序设置出错。

这可能是因为 Cppcheck 不同版本间的设置有所不同。请检查(并修复)编辑器应用程序设置，否则编辑器程序可能不会正确启动。</translation>
    </message>
    <message>
        <location filename="mainwindow.cpp" line="756"/>
        <source>You must close the project file before selecting new files or directories!</source>
        <translation>在选择新的文件或目录之前，你必须先关闭此项目文件！</translation>
    </message>
    <message>
        <location filename="mainwindow.cpp" line="157"/>
        <location filename="mainwindow.cpp" line="1782"/>
        <source>Quick Filter:</source>
        <translation>快速滤器:</translation>
    </message>
    <message>
        <location filename="mainwindow.cpp" line="831"/>
        <source>Select configuration</source>
        <translation>选择配置</translation>
    </message>
    <message>
        <location filename="mainwindow.cpp" line="861"/>
        <source>Found project file: %1

Do you want to load this project file instead?</source>
        <translation>找到项目文件: %1

你是否想加载该项目文件？</translation>
    </message>
    <message>
        <location filename="mainwindow.cpp" line="986"/>
        <source>The library &apos;%1&apos; contains unknown elements:
%2</source>
        <translation>库 &apos;%1&apos; 包含未知元素：
%2</translation>
    </message>
    <message>
        <location filename="mainwindow.cpp" line="995"/>
        <source>File not found</source>
        <translation>文件未找到</translation>
    </message>
    <message>
        <location filename="mainwindow.cpp" line="998"/>
        <source>Bad XML</source>
        <translation>无效的 XML</translation>
    </message>
    <message>
        <location filename="mainwindow.cpp" line="1001"/>
        <source>Missing attribute</source>
        <translation>缺失属性</translation>
    </message>
    <message>
        <location filename="mainwindow.cpp" line="1004"/>
        <source>Bad attribute value</source>
        <translation>无效的属性值</translation>
    </message>
    <message>
        <location filename="mainwindow.cpp" line="1007"/>
        <source>Unsupported format</source>
        <translation>不支持的格式</translation>
    </message>
    <message>
        <location filename="mainwindow.cpp" line="1010"/>
        <source>Duplicate platform type</source>
        <translation>重复的平台类型</translation>
    </message>
    <message>
        <location filename="mainwindow.cpp" line="1013"/>
        <source>Platform type redefined</source>
        <translation>平台类型重定义</translation>
    </message>
    <message>
        <location filename="mainwindow.cpp" line="1019"/>
        <source>Unknown element</source>
        <translation>位置元素</translation>
    </message>
    <message>
        <location filename="mainwindow.cpp" line="1019"/>
        <source>Unknown element</source>
        <oldsource>Unknown issue</oldsource>
        <translation type="unfinished">未知问题</translation>
    </message>
    <message>
        <location filename="mainwindow.cpp" line="1024"/>
        <source>Failed to load the selected library &apos;%1&apos;.
%2</source>
        <translation>选择的库 &apos;%1&apos; 加载失败。
%2</translation>
    </message>
    <message>
        <location filename="mainwindow.cpp" line="1081"/>
        <location filename="mainwindow.cpp" line="1091"/>
        <location filename="mainwindow.cpp" line="1103"/>
        <location filename="mainwindow.cpp" line="1198"/>
        <source>Error</source>
        <translation>错误</translation>
    </message>
    <message>
        <source>Failed to load %1. Your Cppcheck installation is broken. You can use --data-dir=&lt;directory&gt; at the command line to specify where this file is located. Please note that --data-dir is supposed to be used by installation scripts and therefore the GUI does not start when it is used, all that happens is that the setting is configured.</source>
        <translation type="vanished">加载 %1 失败。您的 Cppcheck 安装已损坏。您可以在命令行添加 --data-dir=&lt;目录&gt; 参数来指定文件位置。请注意，&apos;--data-dir&apos; 参数应当由安装脚本使用，因此，当使用此参数时，GUI不会启动，所发生的一切只是配置了设置。</translation>
    </message>
    <message>
        <location filename="mainwindow.cpp" line="1483"/>
        <location filename="mainwindow.cpp" line="1682"/>
        <source>XML files (*.xml)</source>
        <translation>XML 文件(*.xml)</translation>
    </message>
    <message>
        <location filename="mainwindow.cpp" line="1485"/>
        <source>Open the report file</source>
        <translation>打开报告文件</translation>
    </message>
    <message>
        <location filename="mainwindow.cpp" line="1648"/>
        <source>License</source>
        <translation>许可证</translation>
    </message>
    <message>
        <location filename="mainwindow.cpp" line="1655"/>
        <source>Authors</source>
        <translation>作者</translation>
    </message>
    <message>
        <location filename="mainwindow.cpp" line="1675"/>
        <source>Save the report file</source>
        <translation>保存报告文件</translation>
    </message>
    <message>
        <location filename="mainwindow.cpp" line="1686"/>
        <source>Text files (*.txt)</source>
        <translation>文本文件(*.txt)</translation>
    </message>
    <message>
        <location filename="mainwindow.cpp" line="1690"/>
        <source>CSV files (*.csv)</source>
        <translation>CSV 文件(*.csv)</translation>
    </message>
    <message>
        <location filename="mainwindow.cpp" line="1713"/>
        <source>Cannot generate a compliance report right now, an analysis must finish successfully. Try to reanalyze the code and ensure there are no critical errors.</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="mainwindow.cpp" line="1820"/>
        <source>Project files (*.cppcheck);;All files(*.*)</source>
        <translation>项目文件(*.cppcheck);;所有文件(*.*)</translation>
    </message>
    <message>
        <location filename="mainwindow.cpp" line="1822"/>
        <source>Select Project File</source>
        <translation>选择项目文件</translation>
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
        <location filename="mainwindow.cpp" line="242"/>
        <location filename="mainwindow.cpp" line="1784"/>
        <location filename="mainwindow.cpp" line="1850"/>
        <location filename="mainwindow.cpp" line="2048"/>
        <source>Project:</source>
        <translation>项目:</translation>
    </message>
    <message>
        <location filename="mainwindow.cpp" line="671"/>
        <source>No suitable files found to analyze!</source>
        <translation>没有找到合适的文件来分析!</translation>
    </message>
    <message>
        <location filename="mainwindow.cpp" line="770"/>
        <source>C/C++ Source</source>
        <translation>C/C++ 源码</translation>
    </message>
    <message>
        <location filename="mainwindow.cpp" line="771"/>
        <source>Compile database</source>
        <translation>Compile database</translation>
    </message>
    <message>
        <location filename="mainwindow.cpp" line="772"/>
        <source>Visual Studio</source>
        <translation>Visual Studio</translation>
    </message>
    <message>
        <location filename="mainwindow.cpp" line="773"/>
        <source>Borland C++ Builder 6</source>
        <translation>Borland C++ Builder 6</translation>
    </message>
    <message>
        <location filename="mainwindow.cpp" line="776"/>
        <source>Select files to analyze</source>
        <translation>选择要分析的文件</translation>
    </message>
    <message>
        <location filename="mainwindow.cpp" line="791"/>
        <source>Select directory to analyze</source>
        <translation>选择要分析的目录</translation>
    </message>
    <message>
        <location filename="mainwindow.cpp" line="831"/>
        <source>Select the configuration that will be analyzed</source>
        <translation>选择要分析的配置</translation>
    </message>
    <message>
        <location filename="mainwindow.cpp" line="883"/>
        <source>Found project files from the directory.

Do you want to proceed analysis without using any of these project files?</source>
        <translation>在目录中发现项目文件。

您想在不使用这些项目文件的情况下进行分析吗？</translation>
    </message>
    <message>
        <location filename="mainwindow.cpp" line="1016"/>
        <source>Duplicate define</source>
        <translation type="unfinished"></translation>
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
        <location filename="mainwindow.cpp" line="1468"/>
        <source>Current results will be cleared.

Opening a new XML file will clear current results.
Do you want to proceed?</source>
        <translation>当前结果将被清除。

打开一个新的XML文件将清除当前的结果。
你想继续吗?</translation>
    </message>
    <message>
        <location filename="mainwindow.cpp" line="1592"/>
        <source>Analyzer is running.

Do you want to stop the analysis and exit Cppcheck?</source>
        <translation>分析正在运行。

您想停止分析并退出 Cppcheck 吗？</translation>
    </message>
    <message>
        <location filename="mainwindow.cpp" line="1634"/>
        <source>About</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="mainwindow.cpp" line="1673"/>
        <source>XML files (*.xml);;Text files (*.txt);;CSV files (*.csv)</source>
        <translation>XML 文件 (*.xml);;文本文件 (*.txt);;CSV 文件 (*.csv)</translation>
    </message>
    <message>
        <location filename="mainwindow.cpp" line="1938"/>
        <source>Build dir &apos;%1&apos; does not exist, create it?</source>
        <translation>构建文件夹 &apos;%1&apos; 不能存在，创建它吗？</translation>
    </message>
    <message>
        <location filename="mainwindow.cpp" line="1946"/>
        <source>To check the project using addons, you need a build directory.</source>
        <translation>要使用插件检查项目，您需要一个构建目录。</translation>
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
        <translation type="vanished">导入 &apos;%1&apos; 失败，分析已停止</translation>
    </message>
    <message>
        <location filename="mainwindow.cpp" line="2033"/>
        <source>Project files (*.cppcheck)</source>
        <translation>项目文件 (*.cppcheck)</translation>
    </message>
    <message>
        <location filename="mainwindow.cpp" line="2035"/>
        <source>Select Project Filename</source>
        <translation>选择项目文件名</translation>
    </message>
    <message>
        <location filename="mainwindow.cpp" line="2081"/>
        <source>No project file loaded</source>
        <translation>项目文件未加载</translation>
    </message>
    <message>
        <location filename="mainwindow.cpp" line="2163"/>
        <source>The project file

%1

 could not be found!

Do you want to remove the file from the recently used projects -list?</source>
        <translation>项目文件

%1

未找到！

你要从最近使用的项目列表中删除此文件吗？</translation>
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
        <oldsource>Cppcheck GUI.

Syntax:
    cppcheck-gui [OPTIONS] [files or paths]

Options:
    -h, --help              Print this help
    -p &lt;file&gt;               Open given project file and start checking it
    -l &lt;file&gt;               Open given results xml file
    -d &lt;directory&gt;          Specify the directory that was checked to generate the results xml specified with -l
    -v, --version           Show program version
    --data-dir=&lt;directory&gt;  Specify directory where GUI datafiles are located (translations, cfg)</oldsource>
        <translation>Cppcheck GUI.

语法:
    cppcheck-gui [选项] [文件或路径]

选项:
    -h, --help              打印此帮助
    -p &lt;file&gt;               打开指定的项目文件并开始检查它
    -l &lt;file&gt;               打开指定的 xml 结果文件
    -d &lt;directory&gt;          指定检查的目录，用以生成用 -l 指定的 xml 结果
    -v, --version           显示程序版本
    --data-dir=&lt;directory&gt;  这个选项用于安装脚本，这样他们可以配置数据文件所在的目录(translations, cfg)。
                            当使用这个选项时，GUI不会启动。</translation>
    </message>
    <message>
        <location filename="main.cpp" line="115"/>
        <source>Cppcheck GUI - Command line parameters</source>
        <translation>Cppcheck GUI - 命令行参数</translation>
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
        <translation>错误 ID</translation>
    </message>
    <message>
        <location filename="newsuppressiondialog.ui" line="32"/>
        <source>File name</source>
        <translation>文件名</translation>
    </message>
    <message>
        <location filename="newsuppressiondialog.ui" line="42"/>
        <source>Line number</source>
        <translation>行号</translation>
    </message>
    <message>
        <location filename="newsuppressiondialog.ui" line="52"/>
        <source>Symbol name</source>
        <translation>符号名</translation>
    </message>
    <message>
        <location filename="newsuppressiondialog.cpp" line="83"/>
        <source>Edit suppression</source>
        <translation>编辑抑制</translation>
    </message>
</context>
<context>
    <name>Platforms</name>
    <message>
        <location filename="platforms.cpp" line="38"/>
        <source>Native</source>
        <translation>本地</translation>
    </message>
    <message>
        <location filename="platforms.cpp" line="39"/>
        <source>Unix 32-bit</source>
        <translation></translation>
    </message>
    <message>
        <location filename="platforms.cpp" line="40"/>
        <source>Unix 64-bit</source>
        <translation></translation>
    </message>
    <message>
        <location filename="platforms.cpp" line="41"/>
        <source>Windows 32-bit ANSI</source>
        <translation></translation>
    </message>
    <message>
        <location filename="platforms.cpp" line="42"/>
        <source>Windows 32-bit Unicode</source>
        <translation></translation>
    </message>
    <message>
        <location filename="platforms.cpp" line="43"/>
        <source>Windows 64-bit</source>
        <translation></translation>
    </message>
</context>
<context>
    <name>ProjectFile</name>
    <message>
        <location filename="projectfile.ui" line="14"/>
        <source>Project File</source>
        <translation>项目文件</translation>
    </message>
    <message>
        <location filename="projectfile.ui" line="24"/>
        <source>Paths and Defines</source>
        <translation>路径和定义</translation>
    </message>
    <message>
        <location filename="projectfile.ui" line="30"/>
        <source>Import Project (Visual studio / compile database/ Borland C++ Builder 6)</source>
        <oldsource>Import Project (Visual studio / compile database)</oldsource>
        <translation>导入项目 (Visual studio / compile database/ Borland C++ Builder 6)</translation>
    </message>
    <message>
        <location filename="projectfile.ui" line="231"/>
        <source>Defines must be separated by a semicolon. Example: DEF1;DEF2=5;DEF3=int</source>
        <oldsource>Defines must be separated by a semicolon &apos;;&apos;</oldsource>
        <translation>定义必须用分号分隔。例如：DEF1;DEF2=5;DEF3=int</translation>
    </message>
    <message>
        <location filename="projectfile.ui" line="389"/>
        <source>Note: Put your own custom .cfg files in the same folder as the project file. You should see them above.</source>
        <translation>注意：把你自己的 .cfg 文件放在和项目文件相同的文件夹中。你应该在上面看到它们。</translation>
    </message>
    <message>
        <location filename="projectfile.ui" line="655"/>
        <source>If tags are added, you will be able to right click on warnings and set one of these tags. You can manually categorize warnings.</source>
        <translation>如果添加了标记，您将能够右键单击警告并设置其中一个标记。您可以手动对警告进行分类。</translation>
    </message>
    <message>
        <location filename="projectfile.ui" line="665"/>
        <source>Exclude source files</source>
        <translation>排除源文件</translation>
    </message>
    <message>
        <location filename="projectfile.ui" line="676"/>
        <source>Exclude folder...</source>
        <translation>排除文件夹...</translation>
    </message>
    <message>
        <location filename="projectfile.ui" line="683"/>
        <source>Exclude file...</source>
        <translation>排除文件...</translation>
    </message>
    <message>
        <source>MISRA C 2012</source>
        <translation type="vanished">MISRA C 2012</translation>
    </message>
    <message>
        <location filename="projectfile.ui" line="876"/>
        <source>MISRA rule texts</source>
        <translation>MISRA 规则文本</translation>
    </message>
    <message>
        <location filename="projectfile.ui" line="883"/>
        <source>&lt;html&gt;&lt;head/&gt;&lt;body&gt;&lt;p&gt;Copy/paste the text from Appendix A &amp;quot;Summary of guidelines&amp;quot; from the MISRA C 2012 pdf to a text file.&lt;/p&gt;&lt;/body&gt;&lt;/html&gt;</source>
        <translation>&lt;html&gt;&lt;head/&gt;&lt;body&gt;&lt;p&gt;从 MISRA C 2012 PDF 的附录 A &amp;quot;指南摘要&amp;quot; 复制/粘贴文本到一个文本文件。&lt;/p&gt;&lt;/body&gt;&lt;/html&gt;</translation>
    </message>
    <message>
        <location filename="projectfile.ui" line="890"/>
        <source>...</source>
        <translation>...</translation>
    </message>
    <message>
        <location filename="projectfile.ui" line="73"/>
        <source>&lt;html&gt;&lt;head/&gt;&lt;body&gt;&lt;p&gt;You have a choice:&lt;/p&gt;&lt;p&gt; * Analyze all Debug and Release configurations&lt;/p&gt;&lt;p&gt; * Only analyze the first matching Debug configuration&lt;/p&gt;&lt;p&gt;&lt;br/&gt;&lt;/p&gt;&lt;/body&gt;&lt;/html&gt;</source>
        <translation>&lt;html&gt;&lt;head/&gt;&lt;body&gt;&lt;p&gt;您有一个选择：&lt;/p&gt;&lt;p&gt; * 分析所有的 Debug 和 Release 配置&lt;/p&gt;&lt;p&gt; * 只分析第一个匹配的 Debug 配置&lt;/p&gt;&lt;p&gt;&lt;br/&gt;&lt;/p&gt;&lt;/body&gt;&lt;/html&gt;</translation>
    </message>
    <message>
        <location filename="projectfile.ui" line="60"/>
        <location filename="projectfile.ui" line="422"/>
        <location filename="projectfile.ui" line="616"/>
        <source>Browse...</source>
        <translation>浏览...</translation>
    </message>
    <message>
        <location filename="projectfile.ui" line="76"/>
        <source>Analyze all Visual Studio configurations</source>
        <translation>分析全部 Visual Studio 配置</translation>
    </message>
    <message>
        <location filename="projectfile.ui" line="113"/>
        <source>Selected VS Configurations</source>
        <translation>已选择的 VS 配置</translation>
    </message>
    <message>
        <location filename="projectfile.ui" line="147"/>
        <source>Paths:</source>
        <translation>路径:</translation>
    </message>
    <message>
        <location filename="projectfile.ui" line="181"/>
        <location filename="projectfile.ui" line="296"/>
        <source>Add...</source>
        <translation>添加...</translation>
    </message>
    <message>
        <location filename="projectfile.ui" line="188"/>
        <location filename="projectfile.ui" line="303"/>
        <location filename="projectfile.ui" line="690"/>
        <source>Edit</source>
        <translation>编辑</translation>
    </message>
    <message>
        <location filename="projectfile.ui" line="195"/>
        <location filename="projectfile.ui" line="310"/>
        <location filename="projectfile.ui" line="697"/>
        <location filename="projectfile.ui" line="740"/>
        <source>Remove</source>
        <translation>移除</translation>
    </message>
    <message>
        <location filename="projectfile.ui" line="242"/>
        <source>Undefines:</source>
        <translation>未定义：</translation>
    </message>
    <message>
        <location filename="projectfile.ui" line="252"/>
        <source>Undefines must be separated by a semicolon. Example: UNDEF1;UNDEF2;UNDEF3</source>
        <translation>未定义必须用分号分隔。例如：UNDEF1;UNDEF2;UNDEF3</translation>
    </message>
    <message>
        <location filename="projectfile.ui" line="265"/>
        <source>Include Paths:</source>
        <translation>包含目录：</translation>
    </message>
    <message>
        <location filename="projectfile.ui" line="362"/>
        <source>Types and Functions</source>
        <translation>类型和函数</translation>
    </message>
    <message>
        <location filename="projectfile.ui" line="403"/>
        <location filename="projectfile.ui" line="488"/>
        <source>Analysis</source>
        <translation>分析</translation>
    </message>
    <message>
        <location filename="projectfile.ui" line="415"/>
        <source>This is a workfolder that Cppcheck will use for various purposes.</source>
        <translation>这是一个 Cppcheck 将用于各种目的的工作文件夹。</translation>
    </message>
    <message>
        <location filename="projectfile.ui" line="432"/>
        <source>Parser</source>
        <translation>解析器</translation>
    </message>
    <message>
        <location filename="projectfile.ui" line="438"/>
        <source>Cppcheck (built in)</source>
        <translation>Cppcheck (内建)</translation>
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
        <location filename="projectfile.ui" line="497"/>
        <source>Check that each class has a safe public interface</source>
        <translation>检查每个类是否有一个安全的公共接口</translation>
    </message>
    <message>
        <location filename="projectfile.ui" line="513"/>
        <source>Limit analysis</source>
        <translation>极限分析</translation>
    </message>
    <message>
        <location filename="projectfile.ui" line="529"/>
        <source>Check code in unused templates (should be ON normally, however in theory you can safely ignore warnings in unused templates)</source>
        <oldsource>Check code in unused templates  (slower and less accurate analysis)</oldsource>
        <translation>检查未使用模板中的代码（正常情况下应该是打开，但理论上可以忽略未使用模板中的警告）</translation>
    </message>
    <message>
        <location filename="projectfile.ui" line="541"/>
        <source>Max CTU depth</source>
        <translation>最大 CTU 深度</translation>
    </message>
    <message>
        <location filename="projectfile.ui" line="607"/>
        <source>Premium License</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="projectfile.ui" line="765"/>
        <source>Enable inline suppressions</source>
        <translation type="unfinished">启用内联方案</translation>
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
        <translation type="unfinished"></translation>
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
        <translation>外部工具</translation>
    </message>
    <message>
        <location filename="projectfile.ui" line="330"/>
        <source>Up</source>
        <translation>向上</translation>
    </message>
    <message>
        <location filename="projectfile.ui" line="337"/>
        <source>Down</source>
        <translation>向下</translation>
    </message>
    <message>
        <location filename="projectfile.ui" line="368"/>
        <source>Platform</source>
        <translation>平台</translation>
    </message>
    <message>
        <location filename="projectfile.ui" line="448"/>
        <source>Clang (experimental)</source>
        <translation>Clang (实验性的)</translation>
    </message>
    <message>
        <location filename="projectfile.ui" line="494"/>
        <source>If you want to design your classes to be as flexible and robust as possible then the public interface must be very robust. Cppcheck will asumme that arguments can take *any* value.</source>
        <translation>如果你想要设计你的类尽可能的灵活和健壮，那么公共接口必须非常健壮。Cppcheck 将假设参数可以取 *任何* 值。</translation>
    </message>
    <message>
        <location filename="projectfile.ui" line="519"/>
        <source>Check code in headers  (should be ON normally. if you want a limited quick analysis then turn this OFF)</source>
        <translation>检查头文件中的代码（通常应该是打开的。如果您想要一个有限的快速分析，那么关掉它)）</translation>
    </message>
    <message>
        <location filename="projectfile.ui" line="572"/>
        <source>Max recursion in template instantiation</source>
        <translation>模板实例化中的最大递归</translation>
    </message>
    <message>
        <location filename="projectfile.ui" line="627"/>
        <source>Warning options</source>
        <translation>警告选项</translation>
    </message>
    <message>
        <location filename="projectfile.ui" line="633"/>
        <source>Root path:</source>
        <translation>根路径：</translation>
    </message>
    <message>
        <location filename="projectfile.ui" line="639"/>
        <source>Filepaths in warnings will be relative to this path</source>
        <translation>警告中的文件路径将相对于此路径</translation>
    </message>
    <message>
        <location filename="projectfile.ui" line="649"/>
        <source>Warning tags (separated by semicolon)</source>
        <translation>警告标志（用分号隔开）</translation>
    </message>
    <message>
        <location filename="projectfile.ui" line="409"/>
        <source>Cppcheck build dir (whole program analysis, incremental analysis, statistics, etc)</source>
        <translation>Cppcheck 构建目录 (整个程序分析、增量分析、统计数据等)</translation>
    </message>
    <message>
        <location filename="projectfile.ui" line="380"/>
        <source>Libraries</source>
        <translation>库</translation>
    </message>
    <message>
        <location filename="projectfile.ui" line="722"/>
        <source>Suppressions</source>
        <translation>抑制</translation>
    </message>
    <message>
        <location filename="projectfile.ui" line="733"/>
        <source>Add</source>
        <translation>添加</translation>
    </message>
    <message>
        <location filename="projectfile.ui" line="786"/>
        <location filename="projectfile.ui" line="792"/>
        <source>Addons</source>
        <translation>插件</translation>
    </message>
    <message>
        <location filename="projectfile.ui" line="798"/>
        <source>Note: Addons require &lt;a href=&quot;https://www.python.org/&quot;&gt;Python&lt;/a&gt; being installed.</source>
        <translation>注意：插件需要安装  &lt;a href=&quot;https://www.python.org/&quot;&gt;Python&lt;/a&gt;。</translation>
    </message>
    <message>
        <location filename="projectfile.ui" line="808"/>
        <source>Y2038</source>
        <translation>Y2038</translation>
    </message>
    <message>
        <location filename="projectfile.ui" line="815"/>
        <source>Thread safety</source>
        <translation>线程安全</translation>
    </message>
    <message>
        <location filename="projectfile.ui" line="825"/>
        <source>Coding standards</source>
        <translation>编码标准</translation>
    </message>
    <message>
        <location filename="projectfile.ui" line="833"/>
        <source>Misra C</source>
        <translation type="unfinished"></translation>
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
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="projectfile.ui" line="982"/>
        <source>Bug hunting (Premium)</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="projectfile.ui" line="1011"/>
        <source>Clang analyzer</source>
        <translation>Clang analyzer</translation>
    </message>
    <message>
        <location filename="projectfile.ui" line="1004"/>
        <source>Clang-tidy</source>
        <translation>Clang-tidy</translation>
    </message>
    <message>
        <location filename="projectfile.ui" line="221"/>
        <source>Defines:</source>
        <translation>定义:</translation>
    </message>
</context>
<context>
    <name>ProjectFileDialog</name>
    <message>
        <location filename="projectfiledialog.cpp" line="138"/>
        <source>Project file: %1</source>
        <translation>项目文件: %1</translation>
    </message>
    <message>
        <location filename="projectfiledialog.cpp" line="590"/>
        <source>Select Cppcheck build dir</source>
        <translation>选择 Cppcheck 构建目录</translation>
    </message>
    <message>
        <location filename="projectfiledialog.cpp" line="851"/>
        <source>Select include directory</source>
        <translation>选择 Include 目录</translation>
    </message>
    <message>
        <location filename="projectfiledialog.cpp" line="831"/>
        <source>Select a directory to check</source>
        <translation>选择一个检查目录</translation>
    </message>
    <message>
        <location filename="projectfiledialog.cpp" line="396"/>
        <source>Note: Open source Cppcheck does not fully implement Misra C 2012</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="projectfiledialog.cpp" line="461"/>
        <source>Clang-tidy (not found)</source>
        <translation>Clang-tidy (未找到)</translation>
    </message>
    <message>
        <location filename="projectfiledialog.cpp" line="629"/>
        <source>Visual Studio</source>
        <translation>Visual Studio</translation>
    </message>
    <message>
        <location filename="projectfiledialog.cpp" line="630"/>
        <source>Compile database</source>
        <translation>Compile database</translation>
    </message>
    <message>
        <location filename="projectfiledialog.cpp" line="631"/>
        <source>Borland C++ Builder 6</source>
        <translation>Borland C++ Builder 6</translation>
    </message>
    <message>
        <location filename="projectfiledialog.cpp" line="632"/>
        <source>Import Project</source>
        <translation>导入项目</translation>
    </message>
    <message>
        <location filename="projectfiledialog.cpp" line="871"/>
        <source>Select directory to ignore</source>
        <translation>选择忽略的目录</translation>
    </message>
    <message>
        <location filename="projectfiledialog.cpp" line="879"/>
        <source>Source files</source>
        <translation>源文件</translation>
    </message>
    <message>
        <location filename="projectfiledialog.cpp" line="880"/>
        <source>All files</source>
        <translation>全部文件</translation>
    </message>
    <message>
        <location filename="projectfiledialog.cpp" line="881"/>
        <source>Exclude file</source>
        <translation>排除文件</translation>
    </message>
    <message>
        <location filename="projectfiledialog.cpp" line="964"/>
        <source>Select MISRA rule texts file</source>
        <translation>选择 MISRA 规则文本文件</translation>
    </message>
    <message>
        <location filename="projectfiledialog.cpp" line="966"/>
        <source>MISRA rule texts file (%1)</source>
        <translation>MISRA 规则文本文件 (%1)</translation>
    </message>
</context>
<context>
    <name>QObject</name>
    <message>
        <location filename="translationhandler.cpp" line="91"/>
        <source>Unknown language specified!</source>
        <translation>指定了未知语言！</translation>
    </message>
    <message>
        <location filename="translationhandler.cpp" line="117"/>
        <source>Language file %1 not found!</source>
        <translation>语言文件 %1 不存在！</translation>
    </message>
    <message>
        <location filename="translationhandler.cpp" line="122"/>
        <source>Failed to load translation for language %1 from file %2</source>
        <translation>无法从文件 %2 中为语言 %1 加载翻译文件</translation>
    </message>
    <message>
        <location filename="cppchecklibrarydata.cpp" line="38"/>
        <source>line %1: Unhandled element %2</source>
        <translation>第%1行：未处理元素 %2</translation>
    </message>
    <message>
        <location filename="cppchecklibrarydata.cpp" line="43"/>
        <source>line %1: Mandatory attribute &apos;%2&apos; missing in &apos;%3&apos;</source>
        <translation>第%1行：在 &quot;%3&quot; 中缺失的必选属性 &quot;%2&quot;</translation>
    </message>
    <message>
        <location filename="projectfiledialog.cpp" line="284"/>
        <source> (Not found)</source>
        <translation> (未找到)</translation>
    </message>
    <message>
        <location filename="codeeditstylecontrols.cpp" line="76"/>
        <source>Thin</source>
        <translation>极细</translation>
    </message>
    <message>
        <location filename="codeeditstylecontrols.cpp" line="78"/>
        <source>ExtraLight</source>
        <translation>更细</translation>
    </message>
    <message>
        <location filename="codeeditstylecontrols.cpp" line="80"/>
        <source>Light</source>
        <translation>细</translation>
    </message>
    <message>
        <location filename="codeeditstylecontrols.cpp" line="82"/>
        <source>Normal</source>
        <translation>常规</translation>
    </message>
    <message>
        <location filename="codeeditstylecontrols.cpp" line="84"/>
        <source>Medium</source>
        <translation>中等</translation>
    </message>
    <message>
        <location filename="codeeditstylecontrols.cpp" line="86"/>
        <source>DemiBold</source>
        <translation>较粗</translation>
    </message>
    <message>
        <location filename="codeeditstylecontrols.cpp" line="88"/>
        <source>Bold</source>
        <translation>粗</translation>
    </message>
    <message>
        <location filename="codeeditstylecontrols.cpp" line="90"/>
        <source>ExtraBold</source>
        <translation>更粗</translation>
    </message>
    <message>
        <location filename="codeeditstylecontrols.cpp" line="92"/>
        <source>Black</source>
        <translation>极粗</translation>
    </message>
    <message>
        <location filename="codeeditstyledialog.cpp" line="80"/>
        <source>Editor Foreground Color</source>
        <translation>编辑器前景色</translation>
    </message>
    <message>
        <location filename="codeeditstyledialog.cpp" line="83"/>
        <source>Editor Background Color</source>
        <translation>编辑器背景色</translation>
    </message>
    <message>
        <location filename="codeeditstyledialog.cpp" line="86"/>
        <source>Highlight Background Color</source>
        <translation>高亮背景色</translation>
    </message>
    <message>
        <location filename="codeeditstyledialog.cpp" line="89"/>
        <source>Line Number Foreground Color</source>
        <translation>行号前景色</translation>
    </message>
    <message>
        <location filename="codeeditstyledialog.cpp" line="92"/>
        <source>Line Number Background Color</source>
        <translation>行号背景色</translation>
    </message>
    <message>
        <location filename="codeeditstyledialog.cpp" line="95"/>
        <source>Keyword Foreground Color</source>
        <translation>关键字前景色</translation>
    </message>
    <message>
        <location filename="codeeditstyledialog.cpp" line="98"/>
        <source>Keyword Font Weight</source>
        <translation>关键字字体大小</translation>
    </message>
    <message>
        <location filename="codeeditstyledialog.cpp" line="101"/>
        <source>Class Foreground Color</source>
        <oldsource>Class ForegroundColor</oldsource>
        <translation>类前景色</translation>
    </message>
    <message>
        <location filename="codeeditstyledialog.cpp" line="104"/>
        <source>Class Font Weight</source>
        <translation>类字体大小</translation>
    </message>
    <message>
        <location filename="codeeditstyledialog.cpp" line="107"/>
        <source>Quote Foreground Color</source>
        <translation>引用前景色</translation>
    </message>
    <message>
        <location filename="codeeditstyledialog.cpp" line="110"/>
        <source>Quote Font Weight</source>
        <translation>引用字体大小</translation>
    </message>
    <message>
        <location filename="codeeditstyledialog.cpp" line="113"/>
        <source>Comment Foreground Color</source>
        <translation>注释前景色</translation>
    </message>
    <message>
        <location filename="codeeditstyledialog.cpp" line="116"/>
        <source>Comment Font Weight</source>
        <translation>注释字体大小</translation>
    </message>
    <message>
        <location filename="codeeditstyledialog.cpp" line="119"/>
        <source>Symbol Foreground Color</source>
        <translation>符号前景色</translation>
    </message>
    <message>
        <location filename="codeeditstyledialog.cpp" line="122"/>
        <source>Symbol Background Color</source>
        <translation>符号背景色</translation>
    </message>
    <message>
        <location filename="codeeditstyledialog.cpp" line="125"/>
        <source>Symbol Font Weight</source>
        <translation>符号字体大小</translation>
    </message>
    <message>
        <location filename="codeeditstyledialog.cpp" line="141"/>
        <source>Set to Default Light</source>
        <translation>设置为默认亮色</translation>
    </message>
    <message>
        <location filename="codeeditstyledialog.cpp" line="143"/>
        <source>Set to Default Dark</source>
        <translation>设置为默认暗色</translation>
    </message>
    <message>
        <location filename="resultstree.cpp" line="113"/>
        <source>File</source>
        <translation type="unfinished">文件</translation>
    </message>
    <message>
        <location filename="resultstree.cpp" line="114"/>
        <source>Line</source>
        <translation type="unfinished">行</translation>
    </message>
    <message>
        <location filename="resultstree.cpp" line="115"/>
        <source>Severity</source>
        <translation type="unfinished">严重性</translation>
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
        <translation type="unfinished">不确定的</translation>
    </message>
    <message>
        <location filename="resultstree.cpp" line="119"/>
        <source>Summary</source>
        <translation type="unfinished">概要</translation>
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
        <translation type="unfinished">日期</translation>
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
        <translation>确定</translation>
    </message>
    <message>
        <location filename="translationhandler.cpp" line="42"/>
        <source>Cancel</source>
        <translation>取消</translation>
    </message>
    <message>
        <location filename="translationhandler.cpp" line="43"/>
        <source>Close</source>
        <translation>关闭</translation>
    </message>
    <message>
        <location filename="translationhandler.cpp" line="44"/>
        <source>Save</source>
        <translation>保存</translation>
    </message>
</context>
<context>
    <name>ResultsTree</name>
    <message>
        <source>File</source>
        <translation type="vanished">文件</translation>
    </message>
    <message>
        <source>Severity</source>
        <translation type="vanished">严重性</translation>
    </message>
    <message>
        <source>Line</source>
        <translation type="vanished">行</translation>
    </message>
    <message>
        <source>Summary</source>
        <translation type="vanished">概要</translation>
    </message>
    <message>
        <source>Undefined file</source>
        <translation type="vanished">未定义文件</translation>
    </message>
    <message>
        <location filename="resultstree.cpp" line="668"/>
        <source>Copy</source>
        <translation>复制</translation>
    </message>
    <message>
        <location filename="resultstree.cpp" line="836"/>
        <source>Could not find file:</source>
        <translation>找不到文件：</translation>
    </message>
    <message>
        <location filename="resultstree.cpp" line="840"/>
        <source>Please select the folder &apos;%1&apos;</source>
        <translation>请选择文件夹 &apos;%1&apos;</translation>
    </message>
    <message>
        <location filename="resultstree.cpp" line="841"/>
        <source>Select Directory &apos;%1&apos;</source>
        <translation>选择目录 &apos;%1&apos;</translation>
    </message>
    <message>
        <location filename="resultstree.cpp" line="843"/>
        <source>Please select the directory where file is located.</source>
        <translation>请选择文件所在的目录。</translation>
    </message>
    <message>
        <location filename="resultstree.cpp" line="373"/>
        <source>debug</source>
        <translation>调试</translation>
    </message>
    <message>
        <location filename="resultstree.cpp" line="307"/>
        <source>note</source>
        <translation>注意</translation>
    </message>
    <message>
        <source>Recheck</source>
        <translation type="vanished">重新检查</translation>
    </message>
    <message>
        <source>Hide</source>
        <translation type="vanished">隐藏</translation>
    </message>
    <message>
        <location filename="resultstree.cpp" line="670"/>
        <source>Hide all with id</source>
        <translation>隐藏全部 ID</translation>
    </message>
    <message>
        <location filename="resultstree.cpp" line="692"/>
        <source>Suppress selected id(s)</source>
        <translation>抑制选择的 ID</translation>
    </message>
    <message>
        <location filename="resultstree.cpp" line="671"/>
        <source>Open containing folder</source>
        <translation>打开包含的文件夹</translation>
    </message>
    <message>
        <location filename="resultstree.cpp" line="376"/>
        <source>internal</source>
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
        <location filename="resultstree.cpp" line="712"/>
        <source>Tag</source>
        <translation>标记</translation>
    </message>
    <message>
        <location filename="resultstree.cpp" line="714"/>
        <source>No tag</source>
        <translation>取消标记</translation>
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
        <oldsource>Configure the text file viewer program in Cppcheck preferences/Applications.</oldsource>
        <translation>编辑应用程序未配置。

在“首先项 / 应用程序”中为 Cppcheck 配置编辑应用程序。</translation>
    </message>
    <message>
        <location filename="resultstree.cpp" line="759"/>
        <source>No default editor application selected.

Please select the default editor application in preferences/Applications.</source>
        <translation>未选中默认编辑应用程序。

请在“首先项 / 应用程序”中选择默认应用程序。</translation>
    </message>
    <message>
        <location filename="resultstree.cpp" line="779"/>
        <source>Could not find the file!</source>
        <translation>找不到文件！</translation>
    </message>
    <message>
        <location filename="resultstree.cpp" line="822"/>
        <source>Could not start %1

Please check the application path and parameters are correct.</source>
        <translation>无法启动 %1

请检查此应用程序的路径与参数是否正确。</translation>
    </message>
    <message>
        <location filename="resultstree.cpp" line="844"/>
        <source>Select Directory</source>
        <translation>选择目录</translation>
    </message>
    <message>
        <source>Id</source>
        <translation type="vanished">Id</translation>
    </message>
    <message>
        <source>Inconclusive</source>
        <translation type="vanished">不确定的</translation>
    </message>
    <message>
        <source>Since date</source>
        <translation type="vanished">日期</translation>
    </message>
    <message>
        <location filename="resultstree.cpp" line="355"/>
        <source>style</source>
        <translation>风格</translation>
    </message>
    <message>
        <location filename="resultstree.cpp" line="358"/>
        <source>error</source>
        <translation>错误</translation>
    </message>
    <message>
        <location filename="resultstree.cpp" line="361"/>
        <source>warning</source>
        <translation>警告</translation>
    </message>
    <message>
        <location filename="resultstree.cpp" line="364"/>
        <source>performance</source>
        <translation>性能</translation>
    </message>
    <message>
        <location filename="resultstree.cpp" line="367"/>
        <source>portability</source>
        <translation>移植可能性</translation>
    </message>
    <message>
        <location filename="resultstree.cpp" line="370"/>
        <source>information</source>
        <translation>信息</translation>
    </message>
</context>
<context>
    <name>ResultsView</name>
    <message>
        <location filename="resultsview.ui" line="26"/>
        <source>Results</source>
        <translation>结果</translation>
    </message>
    <message>
        <location filename="resultsview.ui" line="60"/>
        <source>Critical errors</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="resultsview.ui" line="92"/>
        <source>Analysis Log</source>
        <translation>分析日志</translation>
    </message>
    <message>
        <location filename="resultsview.ui" line="114"/>
        <source>Warning Details</source>
        <translation>警告详情</translation>
    </message>
    <message>
        <location filename="resultsview.cpp" line="237"/>
        <location filename="resultsview.cpp" line="245"/>
        <source>Failed to save the report.</source>
        <translation>保存报告失败。</translation>
    </message>
    <message>
        <location filename="resultsview.cpp" line="255"/>
        <source>Print Report</source>
        <translation>打印报告</translation>
    </message>
    <message>
        <location filename="resultsview.cpp" line="274"/>
        <source>No errors found, nothing to print.</source>
        <translation>没有错误发现，没有可打印内容。</translation>
    </message>
    <message>
        <location filename="resultsview.cpp" line="326"/>
        <source>%p% (%1 of %2 files checked)</source>
        <translation>%p% (%2 个文件已检查 %1 个)</translation>
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
        <translation>未发现错误。</translation>
    </message>
    <message>
        <location filename="resultsview.cpp" line="358"/>
        <source>Errors were found, but they are configured to be hidden.
To toggle what kind of errors are shown, open view menu.</source>
        <translation>发现错误，但它们被设为隐藏。
打开“查看”菜单，切换需要显示的错误。</translation>
    </message>
    <message>
        <location filename="resultsview.cpp" line="402"/>
        <location filename="resultsview.cpp" line="421"/>
        <source>Failed to read the report.</source>
        <translation>读取报告失败。</translation>
    </message>
    <message>
        <location filename="resultsview.cpp" line="409"/>
        <source>XML format version 1 is no longer supported.</source>
        <translation>不再支持 XML 格式版本 1。</translation>
    </message>
    <message>
        <location filename="resultsview.cpp" line="460"/>
        <source>First included by</source>
        <translation>首次包含于</translation>
    </message>
    <message>
        <location filename="resultsview.cpp" line="465"/>
        <source>Id</source>
        <translation>Id</translation>
    </message>
    <message>
        <source>Bug hunting analysis is incomplete</source>
        <translation type="vanished">错误搜寻分析未完成</translation>
    </message>
    <message>
        <location filename="resultsview.cpp" line="539"/>
        <source>Clear Log</source>
        <translation>清空日志</translation>
    </message>
    <message>
        <location filename="resultsview.cpp" line="540"/>
        <source>Copy this Log entry</source>
        <translation>复制此日志条目</translation>
    </message>
    <message>
        <location filename="resultsview.cpp" line="541"/>
        <source>Copy complete Log</source>
        <translation>复制完整日志</translation>
    </message>
    <message>
        <location filename="resultsview.cpp" line="549"/>
        <source>Analysis was stopped</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="resultsview.cpp" line="563"/>
        <source>There was a critical error with id &apos;%1&apos;</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="resultsview.cpp" line="565"/>
        <source>when checking %1</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="resultsview.cpp" line="567"/>
        <source>when checking a file</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="resultsview.cpp" line="568"/>
        <source>Analysis was aborted.</source>
        <translation type="unfinished"></translation>
    </message>
</context>
<context>
    <name>ScratchPad</name>
    <message>
        <location filename="scratchpad.ui" line="14"/>
        <source>Scratchpad</source>
        <translation>便条</translation>
    </message>
    <message>
        <location filename="scratchpad.ui" line="20"/>
        <source>Copy or write some C/C++ code here:</source>
        <translation>在这里复制或输入一些 C/C++ 代码：</translation>
    </message>
    <message>
        <location filename="scratchpad.ui" line="37"/>
        <source>Optionally enter a filename (mainly for automatic language detection) and click on &quot;Check&quot;:</source>
        <translation>可选择输入文件名 (主要用来自动语言检测) 然后点击 &quot;检查&quot;：</translation>
    </message>
    <message>
        <location filename="scratchpad.ui" line="71"/>
        <source>filename</source>
        <translation>文件名</translation>
    </message>
    <message>
        <location filename="scratchpad.ui" line="78"/>
        <source>Check</source>
        <translation>检查</translation>
    </message>
</context>
<context>
    <name>Settings</name>
    <message>
        <location filename="settings.ui" line="14"/>
        <source>Preferences</source>
        <translation>首选项</translation>
    </message>
    <message>
        <location filename="settings.ui" line="24"/>
        <source>General</source>
        <translation>常规</translation>
    </message>
    <message>
        <location filename="settings.ui" line="196"/>
        <source>Add...</source>
        <translation>添加...</translation>
    </message>
    <message>
        <location filename="settings.ui" line="41"/>
        <source>Number of threads: </source>
        <translation>线程个数: </translation>
    </message>
    <message>
        <source>Ideal count:</source>
        <translation type="vanished">理想个数:</translation>
    </message>
    <message>
        <location filename="settings.ui" line="108"/>
        <source>Force checking all #ifdef configurations</source>
        <oldsource>Check all #ifdef configurations</oldsource>
        <translation>强制检查所有 #ifdef 配置</translation>
    </message>
    <message>
        <location filename="settings.ui" line="115"/>
        <source>Show full path of files</source>
        <translation>显示文件的完整路径</translation>
    </message>
    <message>
        <location filename="settings.ui" line="122"/>
        <source>Show &quot;No errors found&quot; message when no errors found</source>
        <translation>当未找到错误，显示“未发现错误”消息</translation>
    </message>
    <message>
        <location filename="settings.ui" line="129"/>
        <source>Display error Id in column &quot;Id&quot;</source>
        <translation>在列“Id”中显示错误 Id</translation>
    </message>
    <message>
        <location filename="settings.ui" line="136"/>
        <source>Enable inline suppressions</source>
        <translation>启用内联方案</translation>
    </message>
    <message>
        <location filename="settings.ui" line="143"/>
        <source>Check for inconclusive errors also</source>
        <translation>检查不确定的错误</translation>
    </message>
    <message>
        <location filename="settings.ui" line="150"/>
        <source>Show statistics on check completion</source>
        <translation>检查完成后显示统计数据</translation>
    </message>
    <message>
        <location filename="settings.ui" line="157"/>
        <source>Check for updates</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="settings.ui" line="177"/>
        <source>Show internal warnings in log</source>
        <translation>在日志中显示内建警告</translation>
    </message>
    <message>
        <location filename="settings.ui" line="288"/>
        <source>Addons</source>
        <translation>插件</translation>
    </message>
    <message>
        <location filename="settings.ui" line="294"/>
        <source>Python binary (leave this empty to use python in the PATH)</source>
        <translation>Python 二进制 (留空将使用 PATH 路径中的 python)</translation>
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
        <translation>MISRA 插件</translation>
    </message>
    <message>
        <location filename="settings.ui" line="332"/>
        <source>MISRA rule texts file</source>
        <translation>MISRA 规则文本文件</translation>
    </message>
    <message>
        <location filename="settings.ui" line="339"/>
        <source>&lt;html&gt;&lt;head/&gt;&lt;body&gt;&lt;p&gt;Copy/paste the text from Appendix A &amp;quot;Summary of guidelines&amp;quot; from the MISRA C 2012 pdf to a text file.&lt;/p&gt;&lt;/body&gt;&lt;/html&gt;</source>
        <translation>&lt;html&gt;&lt;head/&gt;&lt;body&gt;&lt;p&gt;从 MISRA C 2012 PDF 的附录 A &amp;quot;指南摘要&amp;quot; 复制/粘贴文本到一个文本文件。&lt;/p&gt;&lt;/body&gt;&lt;/html&gt;</translation>
    </message>
    <message>
        <location filename="settings.ui" line="372"/>
        <source>Clang</source>
        <translation>Clang</translation>
    </message>
    <message>
        <location filename="settings.ui" line="378"/>
        <source>Clang path (leave empty to use system PATH)</source>
        <translation>Clang 路径 (留空将使用系统 PATH 路径)</translation>
    </message>
    <message>
        <location filename="settings.ui" line="401"/>
        <source>Visual Studio headers</source>
        <translation>Visual Studio 头文件</translation>
    </message>
    <message>
        <location filename="settings.ui" line="407"/>
        <source>&lt;html&gt;&lt;head/&gt;&lt;body&gt;&lt;p&gt;Paths to Visual Studio headers, separated by semicolon &apos;;&apos;.&lt;/p&gt;&lt;p&gt;You can open a Visual Studio command prompt, write &amp;quot;SET INCLUDE&amp;quot;. Then copy/paste the paths.&lt;/p&gt;&lt;/body&gt;&lt;/html&gt;</source>
        <translation>&lt;html&gt;&lt;head/&gt;&lt;body&gt;&lt;p&gt;Visual Studio 头文件路径，用分号 &apos;;&apos; 分割。&lt;/p&gt;&lt;p&gt;你可以打开一个 Visual Studio 命令提示符，输入 &amp;quot;SET INCLUDE&amp;quot;。然后复制/粘贴路径。&lt;/p&gt;&lt;/body&gt;&lt;/html&gt;</translation>
    </message>
    <message>
        <location filename="settings.ui" line="437"/>
        <source>Code Editor</source>
        <translation>代码编辑器</translation>
    </message>
    <message>
        <location filename="settings.ui" line="443"/>
        <source>Code Editor Style</source>
        <translation>代码编辑器风格</translation>
    </message>
    <message>
        <location filename="settings.ui" line="449"/>
        <source>System Style</source>
        <translation>系统风格</translation>
    </message>
    <message>
        <location filename="settings.ui" line="456"/>
        <source>Default Light Style</source>
        <translation>默认浅色风格</translation>
    </message>
    <message>
        <location filename="settings.ui" line="463"/>
        <source>Default Dark Style</source>
        <translation>默认深色风格</translation>
    </message>
    <message>
        <location filename="settings.ui" line="472"/>
        <source>Custom</source>
        <translation>自定义</translation>
    </message>
    <message>
        <location filename="settings.ui" line="210"/>
        <source>Remove</source>
        <translation>移除</translation>
    </message>
    <message>
        <location filename="settings.ui" line="79"/>
        <source>Max count:</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="settings.ui" line="185"/>
        <source>Applications</source>
        <translation>应用程序</translation>
    </message>
    <message>
        <location filename="settings.ui" line="203"/>
        <location filename="settings.ui" line="479"/>
        <source>Edit...</source>
        <translation>编辑...</translation>
    </message>
    <message>
        <location filename="settings.ui" line="217"/>
        <source>Set as default</source>
        <translation>设为默认</translation>
    </message>
    <message>
        <location filename="settings.ui" line="240"/>
        <source>Reports</source>
        <translation>报告</translation>
    </message>
    <message>
        <location filename="settings.ui" line="246"/>
        <source>Save all errors when creating report</source>
        <translation>创建报告时，保存所有错误</translation>
    </message>
    <message>
        <location filename="settings.ui" line="253"/>
        <source>Save full path to files in reports</source>
        <translation>在报告中保存文件的完整路径</translation>
    </message>
    <message>
        <location filename="settings.ui" line="274"/>
        <source>Language</source>
        <translation>语言</translation>
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
        <translation>可执行文件 &quot;%1&quot; 不可用</translation>
    </message>
    <message>
        <location filename="settingsdialog.cpp" line="243"/>
        <source>Add a new application</source>
        <translation>添加一个新的应用程序</translation>
    </message>
    <message>
        <location filename="settingsdialog.cpp" line="273"/>
        <source>Modify an application</source>
        <translation>修改一个应用程序</translation>
    </message>
    <message>
        <location filename="settingsdialog.cpp" line="278"/>
        <source> [Default]</source>
        <translation> [默认]</translation>
    </message>
    <message>
        <location filename="settingsdialog.cpp" line="303"/>
        <source>[Default]</source>
        <translation>[默认]</translation>
    </message>
    <message>
        <location filename="settingsdialog.cpp" line="358"/>
        <source>Select python binary</source>
        <translation>选择 python 二进制</translation>
    </message>
    <message>
        <location filename="settingsdialog.cpp" line="365"/>
        <source>Select MISRA File</source>
        <translation>选择 MISRA 文件</translation>
    </message>
    <message>
        <location filename="settingsdialog.cpp" line="396"/>
        <source>Select clang path</source>
        <translation>选择 clang 路径</translation>
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
        <translation>统计</translation>
    </message>
    <message>
        <location filename="statsdialog.ui" line="27"/>
        <location filename="statsdialog.cpp" line="221"/>
        <source>Project</source>
        <translation>项目</translation>
    </message>
    <message>
        <location filename="statsdialog.ui" line="33"/>
        <source>Project:</source>
        <translation>项目:</translation>
    </message>
    <message>
        <location filename="statsdialog.ui" line="53"/>
        <source>Paths:</source>
        <translation>路径:</translation>
    </message>
    <message>
        <location filename="statsdialog.ui" line="85"/>
        <source>Include paths:</source>
        <translation>包含路径:</translation>
    </message>
    <message>
        <location filename="statsdialog.ui" line="108"/>
        <source>Defines:</source>
        <translation>定义:</translation>
    </message>
    <message>
        <location filename="statsdialog.ui" line="131"/>
        <source>Undefines:</source>
        <translation>未定义：</translation>
    </message>
    <message>
        <location filename="statsdialog.ui" line="165"/>
        <location filename="statsdialog.cpp" line="226"/>
        <source>Previous Scan</source>
        <translation>上一次扫描</translation>
    </message>
    <message>
        <location filename="statsdialog.ui" line="171"/>
        <source>Path Selected:</source>
        <translation>选中的路径:</translation>
    </message>
    <message>
        <location filename="statsdialog.ui" line="181"/>
        <source>Number of Files Scanned:</source>
        <translation>扫描的文件数:</translation>
    </message>
    <message>
        <location filename="statsdialog.ui" line="201"/>
        <source>Scan Duration:</source>
        <translation>扫描时间:</translation>
    </message>
    <message>
        <location filename="statsdialog.ui" line="254"/>
        <source>Errors:</source>
        <translation>错误:</translation>
    </message>
    <message>
        <location filename="statsdialog.ui" line="271"/>
        <source>Warnings:</source>
        <translation>警告:</translation>
    </message>
    <message>
        <location filename="statsdialog.ui" line="288"/>
        <source>Stylistic warnings:</source>
        <translation>Stylistic 警告:</translation>
    </message>
    <message>
        <location filename="statsdialog.ui" line="305"/>
        <source>Portability warnings:</source>
        <translation>可移植性警告:</translation>
    </message>
    <message>
        <location filename="statsdialog.ui" line="322"/>
        <source>Performance issues:</source>
        <translation>性能警告:</translation>
    </message>
    <message>
        <location filename="statsdialog.ui" line="339"/>
        <source>Information messages:</source>
        <translation>信息:</translation>
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
        <translation>历史</translation>
    </message>
    <message>
        <location filename="statsdialog.ui" line="405"/>
        <source>File:</source>
        <translation>文件：</translation>
    </message>
    <message>
        <location filename="statsdialog.ui" line="443"/>
        <source>Copy to Clipboard</source>
        <translation>复制到剪贴板</translation>
    </message>
    <message>
        <location filename="statsdialog.ui" line="450"/>
        <source>Pdf Export</source>
        <translation>导出 PDF</translation>
    </message>
    <message>
        <location filename="statsdialog.cpp" line="159"/>
        <source>1 day</source>
        <translation>1 天</translation>
    </message>
    <message>
        <location filename="statsdialog.cpp" line="159"/>
        <source>%1 days</source>
        <translation>%1 天</translation>
    </message>
    <message>
        <location filename="statsdialog.cpp" line="161"/>
        <source>1 hour</source>
        <translation>1 小时</translation>
    </message>
    <message>
        <location filename="statsdialog.cpp" line="161"/>
        <source>%1 hours</source>
        <translation>%1 小时</translation>
    </message>
    <message>
        <location filename="statsdialog.cpp" line="163"/>
        <source>1 minute</source>
        <translation>1 分钟</translation>
    </message>
    <message>
        <location filename="statsdialog.cpp" line="163"/>
        <source>%1 minutes</source>
        <translation>%1 分钟</translation>
    </message>
    <message>
        <location filename="statsdialog.cpp" line="165"/>
        <source>1 second</source>
        <translation>1 秒</translation>
    </message>
    <message>
        <location filename="statsdialog.cpp" line="165"/>
        <source>%1 seconds</source>
        <translation>%1 秒</translation>
    </message>
    <message>
        <location filename="statsdialog.cpp" line="169"/>
        <source>0.%1 seconds</source>
        <translation>0.%1 秒</translation>
    </message>
    <message>
        <location filename="statsdialog.cpp" line="171"/>
        <source> and </source>
        <translation> 与 </translation>
    </message>
    <message>
        <location filename="statsdialog.cpp" line="198"/>
        <source>Export PDF</source>
        <translation>导出 PDF</translation>
    </message>
    <message>
        <location filename="statsdialog.cpp" line="220"/>
        <source>Project Settings</source>
        <translation>项目设置</translation>
    </message>
    <message>
        <location filename="statsdialog.cpp" line="222"/>
        <source>Paths</source>
        <translation>路径</translation>
    </message>
    <message>
        <location filename="statsdialog.cpp" line="223"/>
        <source>Include paths</source>
        <translation>包含路径</translation>
    </message>
    <message>
        <location filename="statsdialog.cpp" line="224"/>
        <source>Defines</source>
        <translation>定义</translation>
    </message>
    <message>
        <location filename="statsdialog.cpp" line="225"/>
        <source>Undefines</source>
        <translation>未定义</translation>
    </message>
    <message>
        <location filename="statsdialog.cpp" line="227"/>
        <source>Path selected</source>
        <translation>选中的路径</translation>
    </message>
    <message>
        <location filename="statsdialog.cpp" line="228"/>
        <source>Number of files scanned</source>
        <translation>扫描的文件数</translation>
    </message>
    <message>
        <location filename="statsdialog.cpp" line="229"/>
        <source>Scan duration</source>
        <translation>扫描时间</translation>
    </message>
    <message>
        <location filename="statsdialog.cpp" line="185"/>
        <location filename="statsdialog.cpp" line="231"/>
        <source>Errors</source>
        <translation>错误</translation>
    </message>
    <message>
        <location filename="statsdialog.cpp" line="112"/>
        <source>File: </source>
        <translation>文件: </translation>
    </message>
    <message>
        <location filename="statsdialog.cpp" line="112"/>
        <source>No cppcheck build dir</source>
        <translation>没有 cppcheck 构建目录</translation>
    </message>
    <message>
        <location filename="statsdialog.cpp" line="187"/>
        <location filename="statsdialog.cpp" line="232"/>
        <source>Warnings</source>
        <translation>警告</translation>
    </message>
    <message>
        <location filename="statsdialog.cpp" line="189"/>
        <location filename="statsdialog.cpp" line="233"/>
        <source>Style warnings</source>
        <translation>风格警告</translation>
    </message>
    <message>
        <location filename="statsdialog.cpp" line="191"/>
        <location filename="statsdialog.cpp" line="234"/>
        <source>Portability warnings</source>
        <translation>移植可能性警告</translation>
    </message>
    <message>
        <location filename="statsdialog.cpp" line="193"/>
        <location filename="statsdialog.cpp" line="235"/>
        <source>Performance warnings</source>
        <translation>性能警告</translation>
    </message>
    <message>
        <location filename="statsdialog.cpp" line="195"/>
        <location filename="statsdialog.cpp" line="236"/>
        <source>Information messages</source>
        <translation>信息</translation>
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
        <translation>%2 个文件已检查 %1 个</translation>
    </message>
</context>
<context>
    <name>TranslationHandler</name>
    <message>
        <location filename="translationhandler.cpp" line="130"/>
        <source>Failed to change the user interface language:

%1

The user interface language has been reset to English. Open the Preferences-dialog to select any of the available languages.</source>
        <translation>更改用户界面语言失败:

%1

用户界面语言已被重置为英语。打开“首选项”对话框，选择任何可用的语言。</translation>
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
        <translation>不确定的</translation>
    </message>
</context>
<context>
    <name>toFilterString</name>
    <message>
        <location filename="common.cpp" line="58"/>
        <source>All supported files (%1)</source>
        <translation>全部支持的文件 (%1)</translation>
    </message>
    <message>
        <location filename="common.cpp" line="63"/>
        <source>All files (%1)</source>
        <translation>全部文件 (%1)</translation>
    </message>
</context>
</TS>
