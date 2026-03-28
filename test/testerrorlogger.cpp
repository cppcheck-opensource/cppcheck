/*
 * Cppcheck - A tool for static C/C++ code analysis
 * Copyright (C) 2007-2025 Cppcheck team.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "checkers.h"
#include "cppcheck.h"
#include "errorlogger.h"
#include "errortypes.h"
#include "fixture.h"
#include "helpers.h"
#include "suppressions.h"

#include <list>
#include <string>
#include <utility>

#include "xml.h"

class TestErrorLogger : public TestFixture {
public:
    TestErrorLogger() : TestFixture("TestErrorLogger") {}

private:
    const std::string templateFormat{"{callstack}: ({severity}) {inconclusive:inconclusive: }{message}"};

    const ErrorMessage::FileLocation fooCpp5{"foo.cpp", 5, 1};
    const ErrorMessage::FileLocation barCpp8{"bar.cpp", 8, 1};
    const ErrorMessage::FileLocation barCpp8_i{"bar.cpp", "ä", 8, 1};

    void run() override {
        TEST_CASE(PatternSearchReplace);
        TEST_CASE(FileLocationConstruct);
        TEST_CASE(FileLocationConstructFile);
        TEST_CASE(FileLocationSetFile);
        TEST_CASE(FileLocationSetFile2);
        TEST_CASE(ErrorMessageConstruct);
        TEST_CASE(ErrorMessageConstructLocations);
        TEST_CASE(ErrorMessageVerbose);
        TEST_CASE(ErrorMessageVerboseLocations);
        TEST_CASE(ErrorMessageFromInternalError);
        TEST_CASE(ErrorMessageColorized);
        TEST_CASE(CustomFormat);
        TEST_CASE(CustomFormat2);
        TEST_CASE(CustomFormatLocations);
        TEST_CASE(ToXmlV2);
        TEST_CASE(ToXmlV2RemarkComment);
        TEST_CASE(ToXmlLocations);
        TEST_CASE(ToXmlV2Encoding);
        TEST_CASE(FromXmlV2);
        TEST_CASE(ToXmlV3);

        // Inconclusive results in xml reports..
        TEST_CASE(InconclusiveXml);

        // Serialize / Deserialize inconclusive message
        TEST_CASE(SerializeInconclusiveMessage);
        TEST_CASE(DeserializeInvalidInput);
        TEST_CASE(SerializeSanitize);
        TEST_CASE(SerializeFileLocation);
        TEST_CASE(SerializeAndDeserialize);

        TEST_CASE(substituteTemplateFormatStatic);
        TEST_CASE(substituteTemplateLocationStatic);

        TEST_CASE(isCriticalErrorId);

        TEST_CASE(TestReportType);
    }

    void TestPatternSearchReplace(const std::string& idPlaceholder, const std::string& id) const {
        const std::string plainText = "text";

        ErrorMessage message;
        message.id = id;

        std::string serialized = message.toString(true, idPlaceholder + plainText + idPlaceholder, "");
        ASSERT_EQUALS(id + plainText + id, serialized);

        serialized = message.toString(true, idPlaceholder + idPlaceholder, "");
        ASSERT_EQUALS(id + id, serialized);

        serialized = message.toString(true, plainText + idPlaceholder + plainText, "");
        ASSERT_EQUALS(plainText + id + plainText, serialized);
    }

    void PatternSearchReplace() const {
        const std::string idPlaceholder = "{id}";

        const std::string empty;
        TestPatternSearchReplace(idPlaceholder, empty);

        const std::string shortIdValue = "ID";
        ASSERT_EQUALS(true, shortIdValue.length() < idPlaceholder.length());
        TestPatternSearchReplace(idPlaceholder, shortIdValue);

        const std::string mediumIdValue = "_ID_";
        ASSERT_EQUALS(mediumIdValue.length(), idPlaceholder.length());
        TestPatternSearchReplace(idPlaceholder, mediumIdValue);

        const std::string longIdValue = "longId";
        ASSERT_EQUALS(true, longIdValue.length() > idPlaceholder.length());
        TestPatternSearchReplace(idPlaceholder, longIdValue);
    }

    void FileLocationConstruct() const {
        {
            const ErrorMessage::FileLocation loc("foo.cpp", 1, 2);
            ASSERT_EQUALS("foo.cpp", loc.getOrigFile(false));
            ASSERT_EQUALS("foo.cpp", loc.getfile(false));
            ASSERT_EQUALS(1, loc.line);
            ASSERT_EQUALS(2, loc.column);
            ASSERT_EQUALS("[foo.cpp:1]", loc.stringify(false));
            ASSERT_EQUALS("[foo.cpp:1:2]", loc.stringify(true));
            ASSERT_EQUALS("", loc.getinfo());
        }
        {
            const ErrorMessage::FileLocation loc("foo.cpp", "info", 1, 2);
            ASSERT_EQUALS("foo.cpp", loc.getOrigFile(false));
            ASSERT_EQUALS("foo.cpp", loc.getfile(false));
            ASSERT_EQUALS(1, loc.line);
            ASSERT_EQUALS(2, loc.column);
            ASSERT_EQUALS("[foo.cpp:1]", loc.stringify(false));
            ASSERT_EQUALS("[foo.cpp:1:2]", loc.stringify(true));
            ASSERT_EQUALS("info", loc.getinfo());
        }
        {
            const SimpleTokenList tokenlist("a", "dir/a.cpp");
            {
                const ErrorMessage::FileLocation loc(tokenlist.front(), &tokenlist.get());
                ASSERT_EQUALS("dir/a.cpp", loc.getOrigFile(false));
                ASSERT_EQUALS("dir/a.cpp", loc.getfile(false));
                ASSERT_EQUALS(1, loc.line);
                ASSERT_EQUALS(1, loc.column);
#if defined(_WIN32)
                ASSERT_EQUALS("[dir\\a.cpp:1]", loc.stringify(false));
                ASSERT_EQUALS("[dir\\a.cpp:1:1]", loc.stringify(true));
#else
                ASSERT_EQUALS("[dir/a.cpp:1]", loc.stringify(false));
                ASSERT_EQUALS("[dir/a.cpp:1:1]", loc.stringify(true));
#endif
                ASSERT_EQUALS("", loc.getinfo());
            }
            {
                const ErrorMessage::FileLocation loc(tokenlist.front(), "info", &tokenlist.get());
                ASSERT_EQUALS("dir/a.cpp", loc.getOrigFile(false));
                ASSERT_EQUALS("dir/a.cpp", loc.getfile(false));
                ASSERT_EQUALS(1, loc.line);
                ASSERT_EQUALS(1, loc.column);
#if defined(_WIN32)
                ASSERT_EQUALS("[dir\\a.cpp:1]", loc.stringify(false));
                ASSERT_EQUALS("[dir\\a.cpp:1:1]", loc.stringify(true));
#else
                ASSERT_EQUALS("[dir/a.cpp:1]", loc.stringify(false));
                ASSERT_EQUALS("[dir/a.cpp:1:1]", loc.stringify(true));
#endif
                ASSERT_EQUALS("info", loc.getinfo());
            }
        }
        {
            const SimpleTokenList tokenlist("a", "dir\\a.cpp");
            {
                const ErrorMessage::FileLocation loc(tokenlist.front(), &tokenlist.get());
                ASSERT_EQUALS("dir\\a.cpp", loc.getOrigFile(false));
                ASSERT_EQUALS("dir/a.cpp", loc.getfile(false));
                ASSERT_EQUALS(1, loc.line);
                ASSERT_EQUALS(1, loc.column);
#if defined(_WIN32)
                ASSERT_EQUALS("[dir\\a.cpp:1]", loc.stringify(false));
                ASSERT_EQUALS("[dir\\a.cpp:1:1]", loc.stringify(true));
#else
                ASSERT_EQUALS("[dir/a.cpp:1]", loc.stringify(false));
                ASSERT_EQUALS("[dir/a.cpp:1:1]", loc.stringify(true));
#endif
                ASSERT_EQUALS("", loc.getinfo());
            }
            {
                const ErrorMessage::FileLocation loc(tokenlist.front(), "info", &tokenlist.get());
                ASSERT_EQUALS("dir\\a.cpp", loc.getOrigFile(false));
                ASSERT_EQUALS("dir/a.cpp", loc.getfile(false));
                ASSERT_EQUALS(1, loc.line);
                ASSERT_EQUALS(1, loc.column);
#if defined(_WIN32)
                ASSERT_EQUALS("[dir\\a.cpp:1]", loc.stringify(false));
                ASSERT_EQUALS("[dir\\a.cpp:1:1]", loc.stringify(true));
#else
                ASSERT_EQUALS("[dir/a.cpp:1]", loc.stringify(false));
                ASSERT_EQUALS("[dir/a.cpp:1:1]", loc.stringify(true));
#endif
                ASSERT_EQUALS("info", loc.getinfo());
            }
        }
    }

    void FileLocationConstructFile() const {
        ASSERT_EQUALS("dir/a.cpp", ErrorMessage::FileLocation("dir/a.cpp", 1, 1).getfile(false));
        ASSERT_EQUALS("dir/a.cpp", ErrorMessage::FileLocation("dir\\a.cpp", 1, 1).getfile(false));
        ASSERT_EQUALS("dir/a.cpp", ErrorMessage::FileLocation("dir/a.cpp", "info", 1, 1).getfile(false));
        ASSERT_EQUALS("dir/a.cpp", ErrorMessage::FileLocation("dir\\a.cpp", "info", 1, 1).getfile(false));
        {
            const SimpleTokenList tokenlist("a", "dir/a.cpp");
            ASSERT_EQUALS("dir/a.cpp", ErrorMessage::FileLocation(tokenlist.front(), &tokenlist.get()).getfile(false));
            ASSERT_EQUALS("dir/a.cpp", ErrorMessage::FileLocation(tokenlist.front(), "info", &tokenlist.get()).getfile(false));
        }
        {
            const SimpleTokenList tokenlist("a", "dir\\a.cpp");
            ASSERT_EQUALS("dir/a.cpp", ErrorMessage::FileLocation(tokenlist.front(), &tokenlist.get()).getfile(false));
            ASSERT_EQUALS("dir/a.cpp", ErrorMessage::FileLocation(tokenlist.front(), "info", &tokenlist.get()).getfile(false));
        }
    }

    void FileLocationSetFile() const {
        ErrorMessage::FileLocation loc("foo1.cpp", 0, 0);
        loc.setfile("foo.cpp");
        ASSERT_EQUALS("foo1.cpp", loc.getOrigFile(false));
        ASSERT_EQUALS("foo.cpp", loc.getfile(false));
        ASSERT_EQUALS(0, loc.line);
        ASSERT_EQUALS(0, loc.column);
        // TODO: the following looks wrong - there is no line or column 0
        ASSERT_EQUALS("[foo.cpp:0]", loc.stringify(false));
        ASSERT_EQUALS("[foo.cpp:0:0]", loc.stringify(true));
    }

    void FileLocationSetFile2() const {
        ErrorMessage::FileLocation loc("foo1.cpp", SuppressionList::Suppression::NO_LINE, 0); // TODO: should not depend on Suppression
        loc.setfile("foo.cpp");
        ASSERT_EQUALS("foo1.cpp", loc.getOrigFile(false));
        ASSERT_EQUALS("foo.cpp", loc.getfile(false));
        ASSERT_EQUALS(SuppressionList::Suppression::NO_LINE, loc.line);
        ASSERT_EQUALS(0, loc.column);
        ASSERT_EQUALS("[foo.cpp]", loc.stringify(false));
        ASSERT_EQUALS("[foo.cpp]", loc.stringify(true));
    }

    void ErrorMessageConstruct() const {
        std::list<ErrorMessage::FileLocation> locs(1, fooCpp5);
        ErrorMessage msg(std::move(locs), "", Severity::error, "Programming error.", "errorId", Certainty::normal);
        ASSERT_EQUALS(1, msg.callStack.size());
        ASSERT_EQUALS("Programming error.", msg.shortMessage());
        ASSERT_EQUALS("Programming error.", msg.verboseMessage());
        ASSERT_EQUALS("[foo.cpp:5]: (error) Programming error.", msg.toString(false, templateFormat, ""));
        ASSERT_EQUALS("[foo.cpp:5]: (error) Programming error.", msg.toString(true, templateFormat, ""));
    }

    void ErrorMessageConstructLocations() const {
        std::list<ErrorMessage::FileLocation> locs = { fooCpp5, barCpp8 };
        ErrorMessage msg(std::move(locs), "", Severity::error, "Programming error.", "errorId", Certainty::normal);
        ASSERT_EQUALS(2, msg.callStack.size());
        ASSERT_EQUALS("Programming error.", msg.shortMessage());
        ASSERT_EQUALS("Programming error.", msg.verboseMessage());
        ASSERT_EQUALS("[foo.cpp:5] -> [bar.cpp:8]: (error) Programming error.", msg.toString(false, templateFormat, ""));
        ASSERT_EQUALS("[foo.cpp:5] -> [bar.cpp:8]: (error) Programming error.", msg.toString(true, templateFormat, ""));
    }

    void ErrorMessageVerbose() const {
        std::list<ErrorMessage::FileLocation> locs(1, fooCpp5);
        ErrorMessage msg(std::move(locs), "", Severity::error, "Programming error.\nVerbose error", "errorId", Certainty::normal);
        ASSERT_EQUALS(1, msg.callStack.size());
        ASSERT_EQUALS("Programming error.", msg.shortMessage());
        ASSERT_EQUALS("Verbose error", msg.verboseMessage());
        ASSERT_EQUALS("[foo.cpp:5]: (error) Programming error.", msg.toString(false, templateFormat, ""));
        ASSERT_EQUALS("[foo.cpp:5]: (error) Verbose error", msg.toString(true, templateFormat, ""));
    }

    void ErrorMessageVerboseLocations() const {
        std::list<ErrorMessage::FileLocation> locs = { fooCpp5, barCpp8 };
        ErrorMessage msg(std::move(locs), "", Severity::error, "Programming error.\nVerbose error", "errorId", Certainty::normal);
        ASSERT_EQUALS(2, msg.callStack.size());
        ASSERT_EQUALS("Programming error.", msg.shortMessage());
        ASSERT_EQUALS("Verbose error", msg.verboseMessage());
        ASSERT_EQUALS("[foo.cpp:5] -> [bar.cpp:8]: (error) Programming error.", msg.toString(false, templateFormat, ""));
        ASSERT_EQUALS("[foo.cpp:5] -> [bar.cpp:8]: (error) Verbose error", msg.toString(true, templateFormat, ""));
    }

    void ErrorMessageFromInternalError() const {
        // TODO: test with token
        {
            InternalError internalError(nullptr, "message", InternalError::INTERNAL);
            const auto msg = ErrorMessage::fromInternalError(internalError, nullptr, "file.c");
            ASSERT_EQUALS(1, msg.callStack.size());
            const auto &loc = *msg.callStack.cbegin();
            ASSERT_EQUALS(0, loc.fileIndex);
            ASSERT_EQUALS(0, loc.line);
            ASSERT_EQUALS(0, loc.column);
            ASSERT_EQUALS("message", msg.shortMessage());
            ASSERT_EQUALS("message", msg.verboseMessage());
            ASSERT_EQUALS("[file.c:0]: (error) message", msg.toString(false, templateFormat, ""));
            ASSERT_EQUALS("[file.c:0]: (error) message", msg.toString(true, templateFormat, ""));
        }
        {
            InternalError internalError(nullptr, "message", "details", InternalError::INTERNAL);
            const auto msg = ErrorMessage::fromInternalError(internalError, nullptr, "file.cpp", "msg");
            ASSERT_EQUALS(1, msg.callStack.size());
            const auto &loc = *msg.callStack.cbegin();
            ASSERT_EQUALS(0, loc.fileIndex);
            ASSERT_EQUALS(0, loc.line);
            ASSERT_EQUALS(0, loc.column);
            ASSERT_EQUALS("msg: message", msg.shortMessage());
            ASSERT_EQUALS("msg: message: details", msg.verboseMessage());
            ASSERT_EQUALS("[file.cpp:0]: (error) msg: message", msg.toString(false, templateFormat, ""));
            ASSERT_EQUALS("[file.cpp:0]: (error) msg: message: details", msg.toString(true, templateFormat, ""));
        }
    }

    #define testReportType(...) testReportType_(__FILE__, __LINE__, __VA_ARGS__)
    void testReportType_(const char *file, int line, ReportType reportType, Severity severity, const std::string &errorId,
                         const std::string &expectedClassification, const std::string &expectedGuideline) const
    {
        std::list<ErrorMessage::FileLocation> locs = { fooCpp5 };
        const auto mapping = createGuidelineMapping(reportType);

        ErrorMessage msg(std::move(locs), "", severity, "", errorId, Certainty::normal);
        msg.guideline = getGuideline(msg.id, reportType, mapping, msg.severity);
        msg.classification = getClassification(msg.guideline, reportType);

        ASSERT_EQUALS_LOC(expectedClassification, msg.classification, file, line);
        ASSERT_EQUALS_LOC(expectedGuideline, msg.guideline, file, line);
    }

    void TestReportType() const {
        testReportType(ReportType::misraC2012, Severity::error, "unusedVariable", "Advisory", "2.8");
        testReportType(ReportType::misraCpp2023, Severity::warning, "premium-misra-cpp-2023-6.8.4", "Advisory", "6.8.4");
        testReportType(ReportType::misraCpp2023, Severity::style, "premium-misra-cpp-2023-19.6.1", "Advisory", "19.6.1");
        testReportType(ReportType::misraCpp2023, Severity::style, "premium-misra-cpp-2023-dir-0.3.1", "Advisory", "Dir 0.3.1");
        testReportType(ReportType::misraCpp2023, Severity::style, "premium-misra-cpp-2023-dir-0.3.2", "Required", "Dir 0.3.2");
        testReportType(ReportType::misraCpp2008, Severity::style, "premium-misra-cpp-2008-3-4-1", "Required", "3-4-1");
        testReportType(ReportType::misraC2012, Severity::style, "premium-misra-c-2012-dir-4.6", "Advisory", "Dir 4.6");
        testReportType(ReportType::misraC2012, Severity::style, "misra-c2012-dir-4.6", "Advisory", "Dir 4.6");
        testReportType(ReportType::certC, Severity::error, "resourceLeak", "L3", "FIO42-C");
    }

    void ErrorMessageColorized() const {
        const bool oDisableColors = gDisableColors;
        gDisableColors = false;
        setenv("CLICOLOR_FORCE", "1", 1);
        std::list<ErrorMessage::FileLocation> locs = { };
        {
            ErrorMessage msg(std::move(locs), "", Severity::error, "Programming error.\nVerbose error", "errorId",
                             Certainty::normal);
            ASSERT_EQUALS("{bold} \x1b[31merror: Programming error.", msg.toString(false, "{bold} {severity}: {message}", ""));
        }
        {
            ErrorMessage msg(std::move(locs), "", Severity::warning, "Programming warning.\nVerbose warning", "errorId",
                             Certainty::normal);
            ASSERT_EQUALS("{bold} \x1b[35mwarning: Programming warning.", msg.toString(false, "{bold} {severity}: {message}", ""));
        }
        {
            ErrorMessage msg(std::move(locs), "", Severity::style, "Style.\nVerbose style", "errorId", Certainty::normal);
            ASSERT_EQUALS("{bold} \x1b[1mstyle: Style.", msg.toString(false, "{bold} {severity}: {message}", ""));
        }
        {
            ErrorMessage msg(std::move(locs), "", Severity::information, "Programming information.\nProgramming information",
                             "errorId", Certainty::normal);
            ASSERT_EQUALS("{bold} \x1b[32minformation: Programming information.", msg.toString(false, "{bold} {severity}: {message}", ""));
        }
        setenv("CLICOLOR_FORCE", "", 1);
        gDisableColors = oDisableColors;
    }

    void CustomFormat() const {
        std::list<ErrorMessage::FileLocation> locs(1, fooCpp5);
        ErrorMessage msg(std::move(locs), "", Severity::error, "Programming error.\nVerbose error", "errorId", Certainty::normal);
        ASSERT_EQUALS(1, msg.callStack.size());
        ASSERT_EQUALS("Programming error.", msg.shortMessage());
        ASSERT_EQUALS("Verbose error", msg.verboseMessage());
        ASSERT_EQUALS("foo.cpp:5,error,errorId,Programming error.", msg.toString(false, "{file}:{line},{severity},{id},{message}", ""));
        ASSERT_EQUALS("foo.cpp:5,error,errorId,Verbose error", msg.toString(true, "{file}:{line},{severity},{id},{message}", ""));
    }

    void CustomFormat2() const {
        std::list<ErrorMessage::FileLocation> locs(1, fooCpp5);
        ErrorMessage msg(std::move(locs), "", Severity::error, "Programming error.\nVerbose error", "errorId", Certainty::normal);
        ASSERT_EQUALS(1, msg.callStack.size());
        ASSERT_EQUALS("Programming error.", msg.shortMessage());
        ASSERT_EQUALS("Verbose error", msg.verboseMessage());
        ASSERT_EQUALS("Programming error. - foo.cpp(5):(error,errorId)", msg.toString(false, "{message} - {file}({line}):({severity},{id})", ""));
        ASSERT_EQUALS("Verbose error - foo.cpp(5):(error,errorId)", msg.toString(true, "{message} - {file}({line}):({severity},{id})", ""));
    }

    void CustomFormatLocations() const {
        // Check that first location from location stack is used in template
        std::list<ErrorMessage::FileLocation> locs = { fooCpp5, barCpp8 };
        ErrorMessage msg(std::move(locs), "", Severity::error, "Programming error.\nVerbose error", "errorId", Certainty::normal);
        ASSERT_EQUALS(2, msg.callStack.size());
        ASSERT_EQUALS("Programming error.", msg.shortMessage());
        ASSERT_EQUALS("Verbose error", msg.verboseMessage());
        ASSERT_EQUALS("Programming error. - bar.cpp(8):(error,errorId)", msg.toString(false, "{message} - {file}({line}):({severity},{id})", ""));
        ASSERT_EQUALS("Verbose error - bar.cpp(8):(error,errorId)", msg.toString(true, "{message} - {file}({line}):({severity},{id})", ""));
    }

    void ToXmlV2() const {
        std::list<ErrorMessage::FileLocation> locs(1, fooCpp5);
        ErrorMessage msg(std::move(locs), "", Severity::error, "Programming error.\nVerbose error", "errorId", Certainty::normal);
        std::string header("<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n<results version=\"2\">\n");
        header += "    <cppcheck version=\"";
        header += CppCheck::version();
        header += "\"/>\n    <errors>";
        ASSERT_EQUALS(header, ErrorMessage::getXMLHeader(""));
        ASSERT_EQUALS("    </errors>\n</results>", ErrorMessage::getXMLFooter(2));
        std::string message("        <error id=\"errorId\" severity=\"error\"");
        message += " msg=\"Programming error.\" verbose=\"Verbose error\">\n";
        message += "            <location file=\"foo.cpp\" line=\"5\" column=\"1\"/>\n        </error>";
        ASSERT_EQUALS(message, msg.toXML());
    }

    void ToXmlV2RemarkComment() const {
        ErrorMessage msg({}, "", Severity::warning, "", "id", Certainty::normal);
        msg.remark = "remark";
        ASSERT_EQUALS("        <error id=\"id\" severity=\"warning\" msg=\"\" verbose=\"\" remark=\"remark\"/>", msg.toXML());
    }

    void ToXmlLocations() const {
        const ErrorMessage::FileLocation dir1loc{"dir1/a.cpp", 1, 1};
        const ErrorMessage::FileLocation dir2loc{"dir2\\a.cpp", 1, 1};
        ErrorMessage::FileLocation dir3loc{"dir/a.cpp", 1, 1};
        dir3loc.setfile("dir3/a.cpp");
        ErrorMessage::FileLocation dir4loc{"dir/a.cpp", 1, 1};
        dir4loc.setfile("dir4\\a.cpp");
        std::list<ErrorMessage::FileLocation> locs = { dir4loc, dir3loc, dir2loc, dir1loc, fooCpp5, barCpp8_i };

        ErrorMessage msg(std::move(locs), "", Severity::error, "Programming error.\nVerbose error", "errorId", Certainty::normal);
        std::string message;
        message += "        <error id=\"errorId\" severity=\"error\" msg=\"Programming error.\" verbose=\"Verbose error\">\n";
        message += "            <location file=\"bar.cpp\" line=\"8\" column=\"1\" info=\"\\303\\244\"/>\n";
        message += "            <location file=\"foo.cpp\" line=\"5\" column=\"1\"/>\n";
        message += "            <location file=\"dir1/a.cpp\" line=\"1\" column=\"1\"/>\n";
        message += "            <location origfile=\"dir2\\a.cpp\" file=\"dir2/a.cpp\" line=\"1\" column=\"1\"/>\n";
        message += "            <location origfile=\"dir/a.cpp\" file=\"dir3/a.cpp\" line=\"1\" column=\"1\"/>\n";
        message += "            <location origfile=\"dir/a.cpp\" file=\"dir4/a.cpp\" line=\"1\" column=\"1\"/>\n";
        message += "        </error>";
        ASSERT_EQUALS(message, msg.toXML());
    }

    void ToXmlV2Encoding() const {
        {
            ErrorMessage msg({}, "", Severity::error, "Programming error.\nComparing \"\203\" with \"\003\"", "errorId", Certainty::normal);
            const std::string expected("        <error id=\"errorId\" severity=\"error\" msg=\"Programming error.\" verbose=\"Comparing &quot;\\203&quot; with &quot;\\003&quot;\"/>");
            ASSERT_EQUALS(expected, msg.toXML());
        }
        {
            const char code1[]="äöü";
            const char code2[]="\x12\x00\x00\x01";
            ErrorMessage msg1({}, "", Severity::error, std::string("Programming error.\nReading \"")+code1+"\"", "errorId", Certainty::normal);
            ASSERT_EQUALS("        <error id=\"errorId\" severity=\"error\" msg=\"Programming error.\" verbose=\"Reading &quot;\\303\\244\\303\\266\\303\\274&quot;\"/>", msg1.toXML());
            ErrorMessage msg2({}, "", Severity::error, std::string("Programming error.\nReading \"")+code2+"\"", "errorId", Certainty::normal);
            ASSERT_EQUALS("        <error id=\"errorId\" severity=\"error\" msg=\"Programming error.\" verbose=\"Reading &quot;\\022&quot;\"/>", msg2.toXML());
        }
    }

    void FromXmlV2() const {
        const char xmldata[] = "<?xml version=\"1.0\"?>\n"
                               "<error id=\"errorId\""
                               " severity=\"error\""
                               " cwe=\"123\""
                               " inconclusive=\"true\""
                               " msg=\"Programming error.\""
                               " verbose=\"Verbose error\""
                               " hash=\"456\""
                               ">\n"
                               "  <location file=\"bar.cpp\" line=\"8\" column=\"1\"/>\n"
                               "  <location origfile=\"proj/foo.cpp\" file=\"foo.cpp\" line=\"5\" column=\"2\"/>\n"
                               "</error>";
        tinyxml2::XMLDocument doc;
        ASSERT(doc.Parse(xmldata, sizeof(xmldata)) == tinyxml2::XML_SUCCESS);
        const auto * const rootnode = doc.FirstChildElement();
        ASSERT(rootnode);
        ErrorMessage msg(doc.FirstChildElement());
        ASSERT_EQUALS("errorId", msg.id);
        ASSERT_EQUALS_ENUM(Severity::error, msg.severity);
        ASSERT_EQUALS(123u, msg.cwe.id);
        ASSERT_EQUALS_ENUM(Certainty::inconclusive, msg.certainty);
        ASSERT_EQUALS("Programming error.", msg.shortMessage());
        ASSERT_EQUALS("Verbose error", msg.verboseMessage());
        ASSERT_EQUALS(456u, msg.hash);
        ASSERT_EQUALS(2u, msg.callStack.size());
        ASSERT_EQUALS("proj/foo.cpp", msg.callStack.front().getOrigFile(false));
        ASSERT_EQUALS("foo.cpp", msg.callStack.front().getfile(false));
        ASSERT_EQUALS(5, msg.callStack.front().line);
        ASSERT_EQUALS(2u, msg.callStack.front().column);
        ASSERT_EQUALS("bar.cpp", msg.callStack.back().getOrigFile(false));
        ASSERT_EQUALS("bar.cpp", msg.callStack.back().getfile(false));
        ASSERT_EQUALS(8, msg.callStack.back().line);
        ASSERT_EQUALS(1u, msg.callStack.back().column);
    }

    void ToXmlV3() const {
        std::string header("<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n<results version=\"3\">\n");
        header += "    <cppcheck version=\"";
        header += CppCheck::version();
        header += "\"/>\n    <errors>";
        ASSERT_EQUALS(header, ErrorMessage::getXMLHeader("", 3));

        ASSERT_EQUALS("</results>", ErrorMessage::getXMLFooter(3));
    }

    void InconclusiveXml() const {
        // Location
        std::list<ErrorMessage::FileLocation> locs(1, fooCpp5);

        // Inconclusive error message
        ErrorMessage msg(std::move(locs), "", Severity::error, "Programming error", "errorId", Certainty::inconclusive);

        // xml version 2 error message
        ASSERT_EQUALS("        <error id=\"errorId\" severity=\"error\" msg=\"Programming error\" verbose=\"Programming error\" inconclusive=\"true\">\n"
                      "            <location file=\"foo.cpp\" line=\"5\" column=\"1\"/>\n"
                      "        </error>",
                      msg.toXML());
    }

    void SerializeInconclusiveMessage() const {
        // Inconclusive error message
        ErrorMessage msg({}, "", Severity::error, "Programming error", "errorId", Certainty::inconclusive);
        msg.file0 = "test.cpp";

        const std::string msg_str = msg.serialize();
        ASSERT_EQUALS("7 errorId"
                      "5 error"
                      "1 0"
                      "1 0"
                      "0 "
                      "8 test.cpp"
                      "1 1"
                      "17 Programming error"
                      "17 Programming error"
                      "0 "
                      "0 ", msg_str);

        ErrorMessage msg2;
        ASSERT_NO_THROW(msg2.deserialize(msg_str));
        ASSERT_EQUALS("errorId", msg2.id);
        ASSERT_EQUALS_ENUM(Severity::error, msg2.severity);
        ASSERT_EQUALS("test.cpp", msg2.file0);
        ASSERT_EQUALS_ENUM(Certainty::inconclusive, msg2.certainty);
        ASSERT_EQUALS("Programming error", msg2.shortMessage());
        ASSERT_EQUALS("Programming error", msg2.verboseMessage());
    }

    void DeserializeInvalidInput() const {
        {
            // missing/invalid length
            // missing separator
            ErrorMessage msg;
            ASSERT_THROW_INTERNAL_EQUALS(msg.deserialize("500foobar"), INTERNAL, "Internal Error: Deserialization of error message failed - invalid separator");
        }
        {
            // invalid length
            ErrorMessage msg;
            ASSERT_THROW_INTERNAL_EQUALS(msg.deserialize("foo foobar"), INTERNAL, "Internal Error: Deserialization of error message failed - invalid length");
        }
        {
            // mismatching length
            ErrorMessage msg;
            ASSERT_THROW_INTERNAL_EQUALS(msg.deserialize("8 errorId"), INTERNAL, "Internal Error: Deserialization of error message failed - premature end of data");
        }
        {
            // incomplete message
            ErrorMessage msg;
            ASSERT_THROW_INTERNAL_EQUALS(msg.deserialize("7 errorId"), INTERNAL, "Internal Error: Deserialization of error message failed - invalid length");
        }
        {
            // invalid CWE ID
            const char str[] = "7 errorId"
                               "5 error"
                               "7 invalid" // cwe
                               "1 0"
                               "0 "
                               "8 test.cpp"
                               "17 Programming error"
                               "17 Programming error"
                               "0 "
                               "0 ";
            ErrorMessage msg;
            ASSERT_THROW_INTERNAL_EQUALS(msg.deserialize(str), INTERNAL, "Internal Error: Deserialization of error message failed - invalid CWE ID - not an integer");
        }
        {
            // invalid hash
            const char str[] = "7 errorId"
                               "5 error"
                               "1 0"
                               "7 invalid" // hash
                               "1 0"
                               "0 "
                               "8 test.cpp"
                               "17 Programming error"
                               "17 Programming error"
                               "0 "
                               "0 ";
            ErrorMessage msg;
            ASSERT_THROW_INTERNAL_EQUALS(msg.deserialize(str), INTERNAL, "Internal Error: Deserialization of error message failed - invalid hash - not an integer");
        }
        {
            // out-of-range CWE ID
            const char str[] = "7 errorId"
                               "5 error"
                               "5 65536" // max +1
                               "1 0"
                               "1 0"
                               "0 "
                               "8 test.cpp"
                               "17 Programming error"
                               "17 Programming error"
                               "0 "
                               "0 ";
            ErrorMessage msg;
            ASSERT_THROW_INTERNAL_EQUALS(msg.deserialize(str), INTERNAL, "Internal Error: Deserialization of error message failed - invalid CWE ID - out of range (limits)");
        }
    }

    void SerializeSanitize() const {
        ErrorMessage msg({}, "", Severity::error, std::string("Illegal character in \"foo\001bar\""), "errorId", Certainty::normal);
        msg.file0 = "1.c";

        const std::string msg_str = msg.serialize();
        ASSERT_EQUALS("7 errorId"
                      "5 error"
                      "1 0"
                      "1 0"
                      "0 "
                      "3 1.c"
                      "1 0"
                      "33 Illegal character in \"foo\\001bar\""
                      "33 Illegal character in \"foo\\001bar\""
                      "0 "
                      "0 ", msg_str);

        ErrorMessage msg2;
        ASSERT_NO_THROW(msg2.deserialize(msg_str));
        ASSERT_EQUALS("errorId", msg2.id);
        ASSERT_EQUALS_ENUM(Severity::error, msg2.severity);
        ASSERT_EQUALS("1.c", msg2.file0);
        ASSERT_EQUALS("Illegal character in \"foo\\001bar\"", msg2.shortMessage());
        ASSERT_EQUALS("Illegal character in \"foo\\001bar\"", msg2.verboseMessage());
    }

    void SerializeFileLocation() const {
        ErrorMessage::FileLocation loc1(":/,;", "abcd:/,", 654, 33);
        loc1.setfile("[]:;,()");

        ErrorMessage msg({std::move(loc1)}, "", Severity::error, "Programming error", "errorId", Certainty::inconclusive);

        const std::string msg_str = msg.serialize();
        ASSERT_EQUALS("7 errorId"
                      "5 error"
                      "1 0"
                      "1 0"
                      "0 "
                      "0 "
                      "1 1"
                      "17 Programming error"
                      "17 Programming error"
                      "0 "
                      "1 "
                      "27 654\t33\t[]:;,()\t:/,;\tabcd:/,", msg_str);

        ErrorMessage msg2;
        ASSERT_NO_THROW(msg2.deserialize(msg_str));
        ASSERT_EQUALS("[]:;,()", msg2.callStack.front().getfile(false));
        ASSERT_EQUALS(":/,;", msg2.callStack.front().getOrigFile(false));
        ASSERT_EQUALS(654, msg2.callStack.front().line);
        ASSERT_EQUALS(33, msg2.callStack.front().column);
        ASSERT_EQUALS("abcd:/,", msg2.callStack.front().getinfo());
    }

    void SerializeAndDeserialize() const {
        ErrorMessage msg({}, "", Severity::warning, "$symbol:var\nmessage $symbol", "id", Certainty::normal);
        msg.remark = "some remark";

        ErrorMessage msg2;
        ASSERT_NO_THROW(msg2.deserialize(msg.serialize()));

        ASSERT_EQUALS(msg.callStack.size(), msg2.callStack.size());
        ASSERT_EQUALS(msg.file0, msg2.file0);
        ASSERT_EQUALS_ENUM(msg.severity, msg2.severity);
        ASSERT_EQUALS(msg.shortMessage(), msg2.shortMessage());
        ASSERT_EQUALS(msg.verboseMessage(), msg2.verboseMessage());
        ASSERT_EQUALS(msg.id, msg2.id);
        ASSERT_EQUALS_ENUM(msg.certainty, msg2.certainty);
        ASSERT_EQUALS(msg.cwe.id, msg2.cwe.id);
        ASSERT_EQUALS(msg.hash, msg2.hash);
        ASSERT_EQUALS(msg.remark, msg2.remark);
        ASSERT_EQUALS(msg.symbolNames(), msg2.symbolNames());
    }

    void substituteTemplateFormatStatic() const
    {
        {
            std::string s;
            ::substituteTemplateFormatStatic(s);
            ASSERT_EQUALS("", s);
        }
        {
            std::string s = "template{black}\\z";
            ::substituteTemplateFormatStatic(s);
            ASSERT_EQUALS("template{black}\\z", s);
        }
        {
            std::string s = "{reset}{bold}{dim}{red}{blue}{magenta}{default}\\b\\n\\r\\t";
            ::substituteTemplateFormatStatic(s);
            ASSERT_EQUALS("\b\n\r\t", s);
        }
        {
            std::string s = "\\\\n";
            ::substituteTemplateFormatStatic(s);
            ASSERT_EQUALS("\\\n", s);
        }
        {
            std::string s = "{{red}";
            ::substituteTemplateFormatStatic(s);
            ASSERT_EQUALS("{", s);
        }
    }

    void substituteTemplateLocationStatic() const
    {
        {
            std::string s;
            ::substituteTemplateLocationStatic(s);
            ASSERT_EQUALS("", s);
        }
        {
            std::string s = "template";
            ::substituteTemplateLocationStatic(s);
            ASSERT_EQUALS("template", s);
        }
        {
            std::string s = "template{black}\\z";
            ::substituteTemplateFormatStatic(s);
            ASSERT_EQUALS("template{black}\\z", s);
        }
        {
            std::string s = "{reset}{bold}{dim}{red}{blue}{magenta}{default}\\b\\n\\r\\t";
            ::substituteTemplateFormatStatic(s);
            ASSERT_EQUALS("\b\n\r\t", s);
        }
        {
            std::string s = "\\\\n";
            ::substituteTemplateFormatStatic(s);
            ASSERT_EQUALS("\\\n", s);
        }
        {
            std::string s = "{{red}";
            ::substituteTemplateFormatStatic(s);
            ASSERT_EQUALS("{", s);
        }
    }

    void isCriticalErrorId() const {
        // It does not abort all the analysis of the file. Like "missingInclude" there can be false negatives.
        ASSERT_EQUALS(false, ErrorLogger::isCriticalErrorId("misra-config"));
    }
};

REGISTER_TEST(TestErrorLogger)
