/*
 * Cppcheck - A tool for static C/C++ code analysis
 * Copyright (C) 2007-2026 Cppcheck team.
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


#include "analyzerinfo.h"
#include "errorlogger.h"
#include "filesettings.h"
#include "fixture.h"
#include "standards.h"

#include <list>
#include <sstream>

#include "xml.h"

class TestAnalyzerInformation : public TestFixture {
public:
    TestAnalyzerInformation() : TestFixture("TestAnalyzerInformation") {}

private:
    class AnalyzerInformationTest final : public AnalyzerInformation
    {
        friend class TestAnalyzerInformation;
    };

    void run() override {
        TEST_CASE(getAnalyzerInfoFileFromFilesTxt);
        TEST_CASE(getAnalyzerInfoFile);
        TEST_CASE(duplicateFile);
        TEST_CASE(filesTextDuplicateFile);
        TEST_CASE(parse);
        TEST_CASE(skipAnalysis);
    }

    void getAnalyzerInfoFileFromFilesTxt() const {
        constexpr char filesTxt[] = "file1.a4:::file1.c\n";
        std::istringstream f(filesTxt);
        ASSERT_EQUALS("file1.a4", AnalyzerInformationTest::getAnalyzerInfoFileFromFilesTxt(f, "file1.c", "", 0));
        f.str(filesTxt);
        ASSERT_EQUALS("file1.a4", AnalyzerInformationTest::getAnalyzerInfoFileFromFilesTxt(f, "./file1.c", "", 0));
    }

    void getAnalyzerInfoFile() const {
        ASSERT_EQUALS("builddir/file1.c.analyzerinfo", AnalyzerInformation::getAnalyzerInfoFile("builddir", "file1.c", "", 0));
        ASSERT_EQUALS("builddir/file1.c.analyzerinfo", AnalyzerInformation::getAnalyzerInfoFile("builddir", "some/path/file1.c", "", 0));
    }

    void filesTextDuplicateFile() const {
        std::list<FileSettings> fileSettings;
        fileSettings.emplace_back("a.c", Standards::Language::C, 10);
        fileSettings.back().file.setFsFileId(0);
        fileSettings.emplace_back("a.c", Standards::Language::C, 10);
        fileSettings.back().file.setFsFileId(1);

        const char expected[] = "a.a1:::a.c\n"
                                "a.a2::1:a.c\n";

        ASSERT_EQUALS(expected, AnalyzerInformationTest::getFilesTxt({}, fileSettings));
    }

    void duplicateFile() const {
        // same file duplicated
        constexpr char filesTxt[] = "file1.a1::1:file1.c\n"
                                    "file1.a2::2:file1.c\n";
        std::istringstream f1(filesTxt);
        ASSERT_EQUALS("file1.a1", AnalyzerInformationTest::getAnalyzerInfoFileFromFilesTxt(f1, "file1.c", "", 1));
        std::istringstream f2(filesTxt);
        ASSERT_EQUALS("file1.a2", AnalyzerInformationTest::getAnalyzerInfoFileFromFilesTxt(f2, "file1.c", "", 2));
    }

    void parse() const {
        AnalyzerInformation::Info info;

        ASSERT_EQUALS(false, info.parse("a"));
        ASSERT_EQUALS(false, info.parse("a:b"));
        ASSERT_EQUALS(false, info.parse("a:b:c"));
        ASSERT_EQUALS(false, info.parse("a:b:c:d"));

        ASSERT_EQUALS(true, info.parse("a:b::d"));
        ASSERT_EQUALS("a", info.afile);
        ASSERT_EQUALS("b", info.cfg);
        ASSERT_EQUALS(0, info.fsFileId);
        ASSERT_EQUALS("d", info.sourceFile);

        ASSERT_EQUALS(true, info.parse("e:f:12:g"));
        ASSERT_EQUALS("e", info.afile);
        ASSERT_EQUALS("f", info.cfg);
        ASSERT_EQUALS(12, info.fsFileId);
        ASSERT_EQUALS("g", info.sourceFile);

        ASSERT_EQUALS(true, info.parse("odr1.a1:::C:/dm/cppcheck-fix-13333/test/cli/whole-program/odr1.cpp"));
        ASSERT_EQUALS("odr1.a1", info.afile);
        ASSERT_EQUALS("", info.cfg);
        ASSERT_EQUALS(0, info.fsFileId);
        ASSERT_EQUALS("C:/dm/cppcheck-fix-13333/test/cli/whole-program/odr1.cpp", info.sourceFile);
    }

    void skipAnalysis() const {
        // Matching hash with license error (don't skip)
        {
            std::list<ErrorMessage> errorList;
            tinyxml2::XMLDocument doc;

            const tinyxml2::XMLError xmlError = doc.Parse(
                "<?xml version=\"1.0\"?>"
                "<analyzerinfo hash=\"100\">"
                "<error id=\"premium-invalidLicense\" severity=\"error\" msg=\"Invalid license: No license file was found, contact sales@cppchecksolutions.com\" verbose=\"Invalid license: No license file was found, contact sales@cppchecksolutions.com\" file0=\"test.c\">"
                "<location file=\"Cppcheck Premium\" line=\"0\" column=\"0\"/>"
                "</error>"
                "</analyzerinfo>"
                );
            ASSERT_EQUALS(tinyxml2::XML_SUCCESS, xmlError);

            ASSERT_EQUALS("'premium-invalidLicense' encountered", AnalyzerInformationTest::skipAnalysis(doc, 100, errorList));
            ASSERT_EQUALS(0, errorList.size());
        }

        // Matching hash with premium internal error (don't skip)
        {
            std::list<ErrorMessage> errorList;
            tinyxml2::XMLDocument doc;

            const tinyxml2::XMLError xmlError = doc.Parse(
                "<?xml version=\"1.0\"?>"
                "<analyzerinfo hash=\"100\">"
                "<error id=\"premium-internalError\" severity=\"error\" msg=\"Something went wrong\" verbose=\"Something went wrong\" file0=\"test.c\">"
                "<location file=\"Cppcheck\" line=\"0\" column=\"0\"/>"
                "</error>"
                "</analyzerinfo>"
                );
            ASSERT_EQUALS(tinyxml2::XML_SUCCESS, xmlError);

            ASSERT_EQUALS("'premium-internalError' encountered", AnalyzerInformationTest::skipAnalysis(doc, 100, errorList));
            ASSERT_EQUALS(0, errorList.size());
        }

        // Matching hash with internal error (don't skip)
        {
            std::list<ErrorMessage> errorList;
            tinyxml2::XMLDocument doc;

            const tinyxml2::XMLError xmlError = doc.Parse(
                "<?xml version=\"1.0\"?>"
                "<analyzerinfo hash=\"100\">"
                "<error id=\"internalError\" severity=\"error\" msg=\"Something went wrong\" verbose=\"Something went wrong\" file0=\"test.c\">"
                "<location file=\"Cppcheck\" line=\"0\" column=\"0\"/>"
                "</error>"
                "</analyzerinfo>"
                );
            ASSERT_EQUALS(tinyxml2::XML_SUCCESS, xmlError);

            ASSERT_EQUALS("'internalError' encountered", AnalyzerInformationTest::skipAnalysis(doc, 100, errorList));
            ASSERT_EQUALS(0, errorList.size());
        }

        // Matching hash with normal error (skip)
        {
            std::list<ErrorMessage> errorList;
            tinyxml2::XMLDocument doc;

            const tinyxml2::XMLError xmlError = doc.Parse(
                "<?xml version=\"1.0\"?>"
                "<analyzerinfo hash=\"100\">"
                "<error id=\"nullPointer\" severity=\"error\" msg=\"Null pointer dereference: ptr\" verbose=\"Null pointer dereference: ptr\" cwe=\"476\" file0=\"test.c\">"
                "<location file=\"test.c\" line=\"4\" column=\"3\" info=\"Null pointer dereference\"/>"
                "<location file=\"test.c\" line=\"3\" column=\"12\" info=\"Assignment &apos;ptr=NULL&apos;, assigned value is 0\"/>"
                "<symbol>ptr</symbol>"
                "</error>"
                "</analyzerinfo>"
                );
            ASSERT_EQUALS(tinyxml2::XML_SUCCESS, xmlError);

            ASSERT_EQUALS("", AnalyzerInformationTest::skipAnalysis(doc, 100, errorList));
            ASSERT_EQUALS(1, errorList.size());
        }

        // Matching hash with no error (skip)
        {
            std::list<ErrorMessage> errorList;
            tinyxml2::XMLDocument doc;

            const tinyxml2::XMLError xmlError = doc.Parse(
                "<?xml version=\"1.0\"?>"
                "<analyzerinfo hash=\"100\">"
                "</analyzerinfo>"
                );
            ASSERT_EQUALS(tinyxml2::XML_SUCCESS, xmlError);

            ASSERT_EQUALS("", AnalyzerInformationTest::skipAnalysis(doc, 100, errorList));
            ASSERT_EQUALS(0, errorList.size());
        }

        // Different hash with normal error (don't skip)
        {
            std::list<ErrorMessage> errorList;
            tinyxml2::XMLDocument doc;

            const tinyxml2::XMLError xmlError = doc.Parse(
                "<?xml version=\"1.0\"?>"
                "<analyzerinfo hash=\"100\">"
                "<error id=\"nullPointer\" severity=\"error\" msg=\"Null pointer dereference: ptr\" verbose=\"Null pointer dereference: ptr\" cwe=\"476\" file0=\"test.c\">"
                "<location file=\"test.c\" line=\"4\" column=\"3\" info=\"Null pointer dereference\"/>"
                "<location file=\"test.c\" line=\"3\" column=\"12\" info=\"Assignment &apos;ptr=NULL&apos;, assigned value is 0\"/>"
                "<symbol>ptr</symbol>"
                "</error>"
                "</analyzerinfo>"
                );
            ASSERT_EQUALS(tinyxml2::XML_SUCCESS, xmlError);

            ASSERT_EQUALS("hash mismatch", AnalyzerInformationTest::skipAnalysis(doc, 99, errorList));
            ASSERT_EQUALS(0, errorList.size());
        }

        // Empty document (don't skip)
        {
            std::list<ErrorMessage> errorList;
            tinyxml2::XMLDocument doc;

            const tinyxml2::XMLError xmlError = doc.Parse("");
            ASSERT_EQUALS(tinyxml2::XML_ERROR_EMPTY_DOCUMENT, xmlError);

            ASSERT_EQUALS("no root node found", AnalyzerInformationTest::skipAnalysis(doc, 100, errorList));
            ASSERT_EQUALS(0, errorList.size());
        }

        // Unexpected root node
        {
            std::list<ErrorMessage> errorList;
            tinyxml2::XMLDocument doc;

            const tinyxml2::XMLError xmlError = doc.Parse(
                "<?xml version=\"1.0\"?>"
                "<ainfo/>"
                );
            ASSERT_EQUALS(tinyxml2::XML_SUCCESS, xmlError);

            ASSERT_EQUALS("unexpected root node", AnalyzerInformationTest::skipAnalysis(doc, 99, errorList));
            ASSERT_EQUALS(0, errorList.size());
        }

        // No 'hash' attribute found
        {
            std::list<ErrorMessage> errorList;
            tinyxml2::XMLDocument doc;

            const tinyxml2::XMLError xmlError = doc.Parse(
                "<?xml version=\"1.0\"?>"
                "<analyzerinfo/>"
                );
            ASSERT_EQUALS(tinyxml2::XML_SUCCESS, xmlError);

            ASSERT_EQUALS("no 'hash' attribute found", AnalyzerInformationTest::skipAnalysis(doc, 99, errorList));
            ASSERT_EQUALS(0, errorList.size());
        }
    }
};

REGISTER_TEST(TestAnalyzerInformation)
