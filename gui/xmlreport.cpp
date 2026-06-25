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

#include "xmlreport.h"

#include "report.h"

#include <QFile>
#include <QIODevice>
#include <QXmlStreamAttributes>
#include <QXmlStreamReader>

static constexpr char ResultElementName[] = "results";
static constexpr char VersionAttribute[] = "version";

XmlReport::XmlReport(const QString &filename) :
    Report(filename)
{}

QString XmlReport::quoteMessage(const QString &message)
{
    QString quotedMessage(message);
    quotedMessage.replace("&", "&amp;");
    quotedMessage.replace("\"", "&quot;");
    quotedMessage.replace("'", "&#039;");
    quotedMessage.replace("<", "&lt;");
    quotedMessage.replace(">", "&gt;");
    return quotedMessage;
}

QString XmlReport::unquoteMessage(const QString &message)
{
    QString quotedMessage(message);
    quotedMessage.replace("&amp;", "&");
    quotedMessage.replace("&quot;", "\"");
    quotedMessage.replace("&#039;", "'");
    quotedMessage.replace("&lt;", "<");
    quotedMessage.replace("&gt;", ">");
    return quotedMessage;
}

int XmlReport::determineVersion(const QString &filename)
{
    QFile file;
    file.setFileName(filename);
    const bool succeed = file.open(QIODevice::ReadOnly | QIODevice::Text);
    if (!succeed)
        return 0;

    QXmlStreamReader reader(&file);
    while (!reader.atEnd()) {
        switch (reader.readNext()) {
        case QXmlStreamReader::StartElement:
            if (reader.name() == QString(ResultElementName)) {
                QXmlStreamAttributes attribs = reader.attributes();
                if (attribs.hasAttribute(QString(VersionAttribute))) {
                    const int ver = attribs.value(QString(), VersionAttribute).toString().toInt();
                    return ver;
                }
                return 1;
            }
            break;

        // Not handled
        case QXmlStreamReader::EndElement:
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
    return 0;
}
