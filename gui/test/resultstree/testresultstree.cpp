/*
 * Cppcheck - A tool for static C/C++ code analysis
 * Copyright (C) 2007-2021 Cppcheck team.
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

#include "testresultstree.h"

#include "resultstree.h"

// headers that declare mocked functions/variables
#include "applicationlist.h"
#include "common.h"
#include "filesettings.h"
#include "projectfile.h"
#include "threadhandler.h"
#include "threadresult.h"

#include "application.h"
#include "checkers.h"
#include "erroritem.h"
#include "errorlogger.h"
#include "errortypes.h"
#include "report.h"
#include "resultitem.h"
#include "showtypes.h"
#include "suppressions.h"
#include "xmlreport.h"

#include <cstddef>
#include <string>
#include <utility>

#include <QList>
#include <QModelIndex>
#include <QString>
#include <QtTest>

class TestReport : public Report {
public:
    explicit TestReport(QString format) : Report(QString()), format(std::move(format)) {}
    void writeHeader() override {
        output.clear();
    }
    void writeFooter() override {}
    void writeError(const ErrorItem &error) override {
        QString line = format;
        line.replace("{id}", error.errorId);
        line.replace("{classification}", error.classification);
        line.replace("{guideline}", error.guideline);
        output += (output.isEmpty() ? "" : "\n") + line;
    }
    QString format;
    QString output;
};

// Mock GUI...
ProjectFile::ProjectFile(QObject *parent) : QObject(parent) {}
ProjectFile *ProjectFile::mActiveProject;
void ProjectFile::addSuppression(const SuppressionList::Suppression & /*unused*/) {}
QString ProjectFile::getWarningTags(std::size_t /*unused*/) const {
    return QString();
}
void ProjectFile::setWarningTags(std::size_t /*unused*/, const QString& /*unused*/) {}
bool ProjectFile::write(const QString & /*unused*/) {
    return true;
}
ApplicationList::ApplicationList(QObject *parent) : QObject(parent) {}
ApplicationList::~ApplicationList() = default;
int ApplicationList::getApplicationCount() const {
    return 0;
}
ThreadHandler::ThreadHandler(QObject *parent) : QObject(parent) {}
ThreadHandler::~ThreadHandler() = default;
bool ThreadHandler::isChecking() const {
    return false;
}
void ThreadHandler::stop() {
    throw 1;
}
void ThreadHandler::threadDone() {
    throw 1;
}
// NOLINTBEGIN(performance-unnecessary-value-param)
void ThreadHandler::startCheck(CheckThread::Details /*unused*/) {
    throw 1;
}
void ThreadHandler::finishCheck(CheckThread::Details /*unused*/) {
    throw 1;
}
// NOLINTEND(performance-unnecessary-value-param)
Application& ApplicationList::getApplication(const int /*unused*/) {
    throw 1;
}
const Application& ApplicationList::getApplication(const int index) const {
    return mApplications.at(index);
}
QString XmlReport::quoteMessage(const QString &message) {
    return message;
}
QString XmlReport::unquoteMessage(const QString &message) {
    return message;
}
XmlReport::XmlReport(const QString& filename) : Report(filename) {}
// NOLINTNEXTLINE(performance-unnecessary-value-param)
void ThreadResult::finishCheck(CheckThread::Details /*unused*/) {
    throw 1;
}
void ThreadResult::reportOut(const std::string & /*unused*/, Color /*unused*/) {
    throw 1;
}
void ThreadResult::reportErr(const ErrorMessage & /*unused*/) {
    throw 1;
}
void ThreadResult::reportProgress(const std::string &/*filename*/, const char /*stage*/[], const std::size_t /*value*/) {
    throw 1;
}

// Test...

void TestResultsTree::test1() const
{
    // #12772 : GUI: information messages are shown even though information tool button is deselected
    ResultsTree tree(nullptr);
    tree.showResults(ShowTypes::ShowType::ShowInformation, false);
    ErrorItem errorItem;
    errorItem.errorPath << QErrorPathItem();
    errorItem.severity = Severity::information;
    tree.addErrorItem(errorItem);
    QCOMPARE(tree.isRowHidden(0,QModelIndex()), true);  // Added item is hidden
    tree.showResults(ShowTypes::ShowType::ShowInformation, true);
    QCOMPARE(tree.isRowHidden(0,QModelIndex()), false); // Show item
}

void TestResultsTree::duplicateResults() const
{
    // #14359 - filter out duplicate warnings
    ResultsTree tree(nullptr);

    ErrorItem errorItem;
    errorItem.summary = errorItem.message = "test";
    errorItem.severity = Severity::error;
    errorItem.errorPath << QErrorPathItem();
    QVERIFY(tree.addErrorItem(errorItem));
    QVERIFY(!tree.addErrorItem(errorItem));
}

static QErrorPathItem createErrorPathItem(QString file, int line, int column, QString info) {
    QErrorPathItem ret;
    ret.file = std::move(file);
    ret.line = line;
    ret.column = column;
    ret.info = std::move(info);
    return ret;
}

static ErrorItem createErrorItem(const QString& file, int line, Severity sev, const QString& message, QString id) {
    ErrorItem ret;
    ret.errorId = std::move(id);
    ret.severity = sev;
    ret.cwe = ret.hash = 0;
    ret.file0 = file;
    ret.inconclusive = false;
    ret.message = ret.summary = message;
    ret.errorPath << createErrorPathItem(file, line, 1, message);
    return ret;
}

void TestResultsTree::multiLineResult() const
{
    // Create tree with 1 multiline message
    ResultsTree tree(nullptr);
    ErrorItem errorItem = createErrorItem("file1.c", 10, Severity::style, "test", "bugId");
    errorItem.errorPath << createErrorPathItem("file2.c", 23, 2, "abc");
    tree.addErrorItem(errorItem);

    // Verify model
    const auto* model = dynamic_cast<QStandardItemModel*>(tree.model());
    QVERIFY(model != nullptr);
    QVERIFY(model->rowCount() == 1);

    // Verify file item
    const ResultItem* fileItem = dynamic_cast<ResultItem*>(model->item(0,0));
    QVERIFY(fileItem != nullptr);
    QCOMPARE(fileItem->getType(), ResultItem::Type::file);
    QCOMPARE(fileItem->text(), "file2.c");
    QCOMPARE(fileItem->getErrorPathItem().file, "file2.c");
    QVERIFY(fileItem->rowCount() == 1);

    // Verify message item
    const ResultItem* res = dynamic_cast<ResultItem*>(fileItem->child(0,0));
    QVERIFY(res != nullptr);
    QCOMPARE(res->text(), "file2.c");
    QVERIFY(res->errorItem != nullptr);
    QCOMPARE(res->errorItem->toString(),
             "file2.c:23:2:style: test [bugId]\n"
             "file1.c:10:1:note: test\n"
             "file2.c:23:2:note: abc");
    QCOMPARE(res->getErrorPathItem().file, "file2.c");
    QVERIFY(res->rowCount() == 2);
    QVERIFY(res->columnCount() > 5);
    // Verify both notes
    for (int row = 0; row < 2; ++row) {
        for (int col = 0; col < res->columnCount(); ++col) {
            const ResultItem* item = dynamic_cast<ResultItem*>(res->child(row,col));
            QVERIFY(item);
            QCOMPARE(item->errorItem.get(), res->errorItem.get());
            QCOMPARE(item->getType(), ResultItem::Type::note);
            QCOMPARE(item->getErrorPathItem().file, row == 0 ? "file1.c" : "file2.c");
        }
    }
}

void TestResultsTree::resultsInSameFile() const
{
    ResultsTree tree(nullptr);
    tree.addErrorItem(createErrorItem("file1.c", 10, Severity::style, "test", "bugId"));
    tree.addErrorItem(createErrorItem("file1.c", 20, Severity::style, "test", "bugId"));
    const auto* model = dynamic_cast<QStandardItemModel*>(tree.model());
    QVERIFY(model != nullptr);
    QVERIFY(model->rowCount() == 1);

    const ResultItem* fileItem = dynamic_cast<ResultItem*>(model->item(0,0));
    QVERIFY(fileItem != nullptr);
    QCOMPARE(fileItem->getType(), ResultItem::Type::file);
    QCOMPARE(fileItem->text(), "file1.c");
    QCOMPARE(fileItem->getErrorPathItem().file, "file1.c");
    QVERIFY(fileItem->rowCount() == 2);

    const ResultItem* res1 = dynamic_cast<ResultItem*>(fileItem->child(0,0));
    QVERIFY(res1 != nullptr);
    QCOMPARE(res1->text(), "file1.c");
    QVERIFY(res1->errorItem != nullptr);
    QCOMPARE(res1->errorItem->toString(), "file1.c:10:1:style: test [bugId]");
    QVERIFY(res1->rowCount() == 0);
    for (int col = 0; col < fileItem->columnCount(); ++col) {
        const ResultItem* item = dynamic_cast<ResultItem*>(fileItem->child(0,col));
        QVERIFY(item);
        QCOMPARE(item->errorItem.get(), res1->errorItem.get());
        QCOMPARE(item->getType(), ResultItem::Type::message);
    }

    const ResultItem* res2 = dynamic_cast<ResultItem*>(fileItem->child(1,0));
    QVERIFY(res2 != nullptr);
    QCOMPARE(res2->text(), "file1.c");
    QVERIFY(res2->errorItem != nullptr);
    QCOMPARE(res2->errorItem->toString(), "file1.c:20:1:style: test [bugId]");
    QVERIFY(res2->rowCount() == 0);
    for (int col = 0; col < fileItem->columnCount(); ++col) {
        const ResultItem* item = dynamic_cast<ResultItem*>(fileItem->child(1,col));
        QVERIFY(item);
        QCOMPARE(item->errorItem.get(), res2->errorItem.get());
        QCOMPARE(item->getType(), ResultItem::Type::message);
    }
}

void TestResultsTree::testReportType() const
{
    TestReport report("{id},{classification},{guideline}");

    int msgCount = 0;
    auto createErrorItem = [&msgCount](const Severity severity, const QString& errorId) -> ErrorItem {
        ++msgCount;
        ErrorItem errorItem;
        errorItem.errorPath << QErrorPathItem(ErrorMessage::FileLocation("file1.c", msgCount, 1));
        errorItem.severity = severity;
        errorItem.errorId = errorId;
        errorItem.summary = "test summary " + QString::number(msgCount);
        return errorItem;
    };

    // normal report with 2 errors
    ResultsTree tree(nullptr);
    tree.updateSettings(false, false, false, false, false);
    tree.addErrorItem(createErrorItem(Severity::style, "id1"));
    tree.addErrorItem(createErrorItem(Severity::style, "unusedVariable")); // Misra C 2.8
    tree.saveResults(&report);
    QCOMPARE(report.output, "id1,,\nunusedVariable,,");

    // switch to Misra C report and check that "id1" is not shown
    tree.setReportType(ReportType::misraC2012);
    tree.saveResults(&report);
    QCOMPARE(report.output, "unusedVariable,Advisory,2.8");

    // add "missingReturn" and check that it is added properly
    tree.addErrorItem(createErrorItem(Severity::warning, "missingReturn")); // Misra C 17.4
    tree.saveResults(&report);
    QCOMPARE(report.output,
             "unusedVariable,Advisory,2.8\n"
             "missingReturn,Mandatory,17.4");
}

void TestResultsTree::testReportTypeIcon() const {
    ResultsTree tree(nullptr);
    tree.setReportType(ReportType::misraC2012);
    tree.addErrorItem(createErrorItem("file1.c", 10, Severity::style, "some rule text", "premium-misra-c-2012-1.1")); // Required
    tree.addErrorItem(createErrorItem("file1.c", 20, Severity::style, "some rule text", "premium-misra-c-2012-1.2")); // Advisory
    tree.addErrorItem(createErrorItem("file1.c", 30, Severity::style, "some rule text", "premium-misra-c-2012-9.1")); // Mandatory

    const auto* model = dynamic_cast<QStandardItemModel*>(tree.model());
    QVERIFY(model != nullptr);
    const ResultItem* fileItem = dynamic_cast<ResultItem*>(model->item(0,0));

    const ResultItem* err1 = dynamic_cast<ResultItem*>(fileItem->child(0,0));
    const ResultItem* err2 = dynamic_cast<ResultItem*>(fileItem->child(1,0));
    const ResultItem* err3 = dynamic_cast<ResultItem*>(fileItem->child(2,0));

    QCOMPARE(err1->getIconFileName(), ":images/dialog-warning.png");
    QCOMPARE(err2->getIconFileName(), ":images/applications-development.png");
    QCOMPARE(err3->getIconFileName(), ":images/dialog-error.png");

    tree.setReportType(ReportType::normal);
    QCOMPARE(err1->getIconFileName(), ":images/applications-development.png");
    QCOMPARE(err2->getIconFileName(), ":images/applications-development.png");
    QCOMPARE(err3->getIconFileName(), ":images/applications-development.png");

    tree.setReportType(ReportType::misraC2012);
    QCOMPARE(err1->getIconFileName(), ":images/dialog-warning.png");
    QCOMPARE(err2->getIconFileName(), ":images/applications-development.png");
    QCOMPARE(err3->getIconFileName(), ":images/dialog-error.png");
}

void TestResultsTree::testGetGuidelineError() const
{
    TestReport report("{id},{classification},{guideline}");

    int msgCount = 0;
    auto createErrorItem = [&msgCount](const Severity severity, const QString& errorId) -> ErrorItem {
        ++msgCount;
        ErrorItem errorItem;
        errorItem.errorPath << QErrorPathItem(ErrorMessage::FileLocation("file1.c", msgCount, 1));
        errorItem.severity = severity;
        errorItem.errorId = errorId;
        errorItem.summary = "test summary " + QString::number(msgCount);
        return errorItem;
    };

    // normal report with 2 errors
    ResultsTree tree(nullptr);
    tree.setReportType(ReportType::misraC2012);
    tree.addErrorItem(createErrorItem(Severity::error, "id1")); // error severity => guideline 1.3
    tree.saveResults(&report);
    QCOMPARE(report.output, "id1,Required,1.3");
}

void TestResultsTree::misraCReportShowClassifications() const
{
    ResultsTree tree(nullptr);
    tree.showResults(ShowTypes::ShowType::ShowErrors, true);
    tree.showResults(ShowTypes::ShowType::ShowWarnings, true);
    tree.showResults(ShowTypes::ShowType::ShowStyle, true);
    tree.setReportType(ReportType::misraC2012);
    tree.addErrorItem(createErrorItem("file1.c", 10, Severity::style, "some rule text", "premium-misra-c-2012-1.1")); // Required
    tree.addErrorItem(createErrorItem("file1.c", 20, Severity::style, "some rule text", "premium-misra-c-2012-1.2")); // Advisory
    tree.addErrorItem(createErrorItem("file1.c", 30, Severity::style, "some rule text", "premium-misra-c-2012-9.1")); // Mandatory
    QCOMPARE(tree.isRowHidden(0, QModelIndex()), false);

    const auto* model = dynamic_cast<QStandardItemModel*>(tree.model());
    QVERIFY(model != nullptr);
    QVERIFY(model->rowCount() == 1);
    const ResultItem* fileItem = dynamic_cast<ResultItem*>(model->item(0,0));
    QVERIFY(fileItem != nullptr);
    QVERIFY(fileItem->rowCount() == 3);

    QCOMPARE(tree.isRowHidden(0, fileItem->index()), false);
    QCOMPARE(tree.isRowHidden(1, fileItem->index()), false);
    QCOMPARE(tree.isRowHidden(2, fileItem->index()), false);

    tree.showResults(ShowTypes::ShowType::ShowErrors, false);
    tree.showResults(ShowTypes::ShowType::ShowWarnings, true);
    tree.showResults(ShowTypes::ShowType::ShowStyle, true);
    QCOMPARE(tree.isRowHidden(0, fileItem->index()), false);
    QCOMPARE(tree.isRowHidden(1, fileItem->index()), false);
    QCOMPARE(tree.isRowHidden(2, fileItem->index()), true);

    tree.showResults(ShowTypes::ShowType::ShowErrors, true);
    tree.showResults(ShowTypes::ShowType::ShowWarnings, false);
    tree.showResults(ShowTypes::ShowType::ShowStyle, true);
    QCOMPARE(tree.isRowHidden(0, fileItem->index()), true);
    QCOMPARE(tree.isRowHidden(1, fileItem->index()), false);
    QCOMPARE(tree.isRowHidden(2, fileItem->index()), false);

    tree.showResults(ShowTypes::ShowType::ShowErrors, true);
    tree.showResults(ShowTypes::ShowType::ShowWarnings, true);
    tree.showResults(ShowTypes::ShowType::ShowStyle, false);
    QCOMPARE(tree.isRowHidden(0, fileItem->index()), false);
    QCOMPARE(tree.isRowHidden(1, fileItem->index()), true);
    QCOMPARE(tree.isRowHidden(2, fileItem->index()), false);
}

QTEST_MAIN(TestResultsTree)

