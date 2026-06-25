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

#include "xmlreportv2.h"

#include "cppcheck.h"
#include "erroritem.h"
#include "report.h"
#include "settings.h"
#include "xmlreport.h"

#include <utility>

#include <QDebug>
#include <QDir>
#include <QFile>
#include <QXmlStreamAttributes>
#include <QXmlStreamReader>
#include <QXmlStreamWriter>
#if (QT_VERSION >= QT_VERSION_CHECK(6, 5, 0))
#include <QtLogging>
#endif

static const QString ResultElementName = "results";
static const QString CppcheckElementName = "cppcheck";
static const QString ErrorElementName = "error";
static const QString ErrorsElementName = "errors";
static const QString LocationElementName = "location";
static const QString CWEAttribute = "cwe";
static const QString HashAttribute = "hash";
static const QString SinceDateAttribute = "sinceDate";
static const QString TagsAttribute = "tag";
static const QString FilenameAttribute = "file";
static const QString IncludedFromFilenameAttribute = "file0";
static const QString InconclusiveAttribute = "inconclusive";
static const QString RemarkAttribute = "remark";
static const QString InfoAttribute = "info";
static const QString LineAttribute = "line";
static const QString ColumnAttribute = "column";
static const QString IdAttribute = "id";
static const QString SeverityAttribute = "severity";
static const QString MsgAttribute = "msg";
static const QString VersionAttribute = "version";
static const QString ProductNameAttribute = "product-name";
static const QString VerboseAttribute = "verbose";

XmlReportV2::XmlReportV2(const QString &filename, QString productName) :
    XmlReport(filename),
    mProductName(std::move(productName)),
    mXmlReader(nullptr),
    mXmlWriter(nullptr)
{}

XmlReportV2::~XmlReportV2()
{
    delete mXmlReader;
    delete mXmlWriter;
}

bool XmlReportV2::create()
{
    if (Report::create()) {
        mXmlWriter = new QXmlStreamWriter(Report::getFile());
        return true;
    }
    return false;
}

bool XmlReportV2::open()
{
    if (Report::open()) {
        mXmlReader = new QXmlStreamReader(Report::getFile());
        return true;
    }
    return false;
}

void XmlReportV2::writeHeader()
{
    const auto nameAndVersion = Settings::getNameAndVersion(mProductName.toStdString());
    const QString name = QString::fromStdString(nameAndVersion.first);
    // TODO: lacks extraVersion
    const QString version = nameAndVersion.first.empty() ? CppCheck::version() : QString::fromStdString(nameAndVersion.second);

    mXmlWriter->setAutoFormatting(true);
    mXmlWriter->writeStartDocument();
    mXmlWriter->writeStartElement(ResultElementName);
    mXmlWriter->writeAttribute(VersionAttribute, QString::number(2));
    mXmlWriter->writeStartElement(CppcheckElementName);
    if (!name.isEmpty())
        mXmlWriter->writeAttribute(ProductNameAttribute, name);
    mXmlWriter->writeAttribute(VersionAttribute, version);
    mXmlWriter->writeEndElement();
    mXmlWriter->writeStartElement(ErrorsElementName);
}

void XmlReportV2::writeFooter()
{
    mXmlWriter->writeEndElement(); // errors
    mXmlWriter->writeEndElement(); // results
    mXmlWriter->writeEndDocument();
}

void XmlReportV2::writeError(const ErrorItem &error)
{
    /*
       Error example from the core program in xml
       <error id="mismatchAllocDealloc" severity="error" msg="Mismatching allocation and deallocation: k"
              verbose="Mismatching allocation and deallocation: k">
       <location file="..\..\test\test.cxx" line="16"/>
       <location file="..\..\test\test.cxx" line="32"/>
       </error>
     */

    mXmlWriter->writeStartElement(ErrorElementName);
    mXmlWriter->writeAttribute(IdAttribute, error.errorId);

    // Don't localize severity so we can read these files
    mXmlWriter->writeAttribute(SeverityAttribute, GuiSeverity::toString(error.severity));
    const QString summary = XmlReport::quoteMessage(error.summary);
    mXmlWriter->writeAttribute(MsgAttribute, summary);
    const QString message = XmlReport::quoteMessage(error.message);
    mXmlWriter->writeAttribute(VerboseAttribute, message);
    if (error.inconclusive)
        mXmlWriter->writeAttribute(InconclusiveAttribute, "true");
    if (!error.remark.isEmpty())
        mXmlWriter->writeAttribute(RemarkAttribute, error.remark);
    if (error.cwe > 0)
        mXmlWriter->writeAttribute(CWEAttribute, QString::number(error.cwe));
    if (error.hash > 0)
        mXmlWriter->writeAttribute(HashAttribute, QString::number(error.hash));
    if (!error.file0.isEmpty())
        mXmlWriter->writeAttribute(IncludedFromFilenameAttribute, quoteMessage(error.file0));
    if (!error.sinceDate.isEmpty())
        mXmlWriter->writeAttribute(SinceDateAttribute, error.sinceDate);
    if (!error.tags.isEmpty())
        mXmlWriter->writeAttribute(TagsAttribute, error.tags);

    for (int i = error.errorPath.count() - 1; i >= 0; i--) {
        mXmlWriter->writeStartElement(LocationElementName);

        QString file = QDir::toNativeSeparators(error.errorPath[i].file);
        mXmlWriter->writeAttribute(FilenameAttribute, XmlReport::quoteMessage(file));
        mXmlWriter->writeAttribute(LineAttribute, QString::number(error.errorPath[i].line));
        if (error.errorPath[i].column > 0)
            mXmlWriter->writeAttribute(ColumnAttribute, QString::number(error.errorPath[i].column));
        if (error.errorPath.count() > 1)
            mXmlWriter->writeAttribute(InfoAttribute, XmlReport::quoteMessage(error.errorPath[i].info));

        mXmlWriter->writeEndElement();
    }

    mXmlWriter->writeEndElement();
}

QList<ErrorItem> XmlReportV2::read()
{
    QList<ErrorItem> errors;
    bool insideResults = false;
    if (!mXmlReader) {
        qDebug() << "You must Open() the file before reading it!";
        return errors;
    }
    while (!mXmlReader->atEnd()) {
        switch (mXmlReader->readNext()) {
        case QXmlStreamReader::StartElement:
            if (mXmlReader->name() == ResultElementName)
                insideResults = true;

            // Read error element from inside result element
            if (insideResults && mXmlReader->name() == ErrorElementName) {
                ErrorItem item = readError(mXmlReader);
                errors.append(item);
            }
            break;

        case QXmlStreamReader::EndElement:
            if (mXmlReader->name() == ResultElementName)
                insideResults = false;
            break;

        // Not handled
        case QXmlStreamReader::NoToken:
        case QXmlStreamReader::Invalid:
        case QXmlStreamReader::StartDocument:
        case QXmlStreamReader::EndDocument:
        case QXmlStreamReader::Characters:
        case QXmlStreamReader::Comment:
        case QXmlStreamReader::DTD:
        case QXmlStreamReader::EntityReference:
        case QXmlStreamReader::ProcessingInstruction:
            break;
        }
    }
    return errors;
}

ErrorItem XmlReportV2::readError(const QXmlStreamReader *reader)
{
    /*
       Error example from the core program in xml
       <error id="mismatchAllocDealloc" severity="error" msg="Mismatching allocation and deallocation: k"
              verbose="Mismatching allocation and deallocation: k">
       <location file="..\..\test\test.cxx" line="16"/>
       <location file="..\..\test\test.cxx" line="32"/>
       </error>
     */

    ErrorItem item;

    // Read error element from inside errors element
    if (mXmlReader->name() == ErrorElementName) {
        QXmlStreamAttributes attribs = reader->attributes();
        item.errorId = attribs.value(QString(), IdAttribute).toString();
        item.severity = GuiSeverity::fromString(attribs.value(QString(), SeverityAttribute).toString());
        const QString summary = attribs.value(QString(), MsgAttribute).toString();
        item.summary = XmlReport::unquoteMessage(summary);
        const QString message = attribs.value(QString(), VerboseAttribute).toString();
        item.message = XmlReport::unquoteMessage(message);
        if (attribs.hasAttribute(QString(), InconclusiveAttribute))
            item.inconclusive = true;
        if (attribs.hasAttribute(QString(), RemarkAttribute))
            item.remark = attribs.value(QString(), RemarkAttribute).toString();
        if (attribs.hasAttribute(QString(), CWEAttribute))
            item.cwe = attribs.value(QString(), CWEAttribute).toInt();
        if (attribs.hasAttribute(QString(), HashAttribute))
            item.hash = attribs.value(QString(), HashAttribute).toULongLong();
        if (attribs.hasAttribute(QString(), IncludedFromFilenameAttribute))
            item.file0 = attribs.value(QString(), IncludedFromFilenameAttribute).toString();
        if (attribs.hasAttribute(QString(), SinceDateAttribute))
            item.sinceDate = attribs.value(QString(), SinceDateAttribute).toString();
        if (attribs.hasAttribute(QString(), TagsAttribute))
            item.tags = attribs.value(QString(), TagsAttribute).toString();
    }

    bool errorRead = false;
    while (!errorRead && !mXmlReader->atEnd()) {
        switch (mXmlReader->readNext()) {
        case QXmlStreamReader::StartElement:

            // Read location element from inside error element
            if (mXmlReader->name() == LocationElementName) {
                QXmlStreamAttributes attribs = mXmlReader->attributes();
                QString file0 = attribs.value(QString(), IncludedFromFilenameAttribute).toString();
                if (!file0.isEmpty())
                    item.file0 = XmlReport::unquoteMessage(file0);
                QErrorPathItem loc;
                loc.file = XmlReport::unquoteMessage(attribs.value(QString(), FilenameAttribute).toString());
                loc.line = attribs.value(QString(), LineAttribute).toString().toUInt();
                if (attribs.hasAttribute(QString(), ColumnAttribute))
                    loc.column = attribs.value(QString(), ColumnAttribute).toString().toInt();
                if (attribs.hasAttribute(QString(), InfoAttribute))
                    loc.info = XmlReport::unquoteMessage(attribs.value(QString(), InfoAttribute).toString());
                item.errorPath.push_front(loc);
            }
            break;

        case QXmlStreamReader::EndElement:
            if (mXmlReader->name() == ErrorElementName)
                errorRead = true;
            break;

        // Not handled
        case QXmlStreamReader::NoToken:
        case QXmlStreamReader::Invalid:
        case QXmlStreamReader::StartDocument:
        case QXmlStreamReader::EndDocument:
        case QXmlStreamReader::Characters:
        case QXmlStreamReader::Comment:
        case QXmlStreamReader::DTD:
        case QXmlStreamReader::EntityReference:
        case QXmlStreamReader::ProcessingInstruction:
            break;
        }
    }

    if (item.errorPath.size() == 1 && item.errorPath[0].info.isEmpty())
        item.errorPath[0].info = item.message;

    return item;
}
