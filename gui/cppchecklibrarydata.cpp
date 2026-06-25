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

#include "cppchecklibrarydata.h"

#include "utils.h"

#include <stdexcept>
#include <string>
#include <utility>

#include <QObject>
#include <QVariant>
#include <QXmlStreamAttributes>
#include <QXmlStreamReader>
#include <QXmlStreamWriter>

const unsigned int CppcheckLibraryData::Function::Arg::ANY = ~0U;
const unsigned int CppcheckLibraryData::Function::Arg::VARIADIC = ~1U;

NORETURN static void unhandledElement(const QXmlStreamReader &xmlReader)
{
    throw std::runtime_error(QObject::tr("line %1: Unhandled element %2").arg(xmlReader.lineNumber()).arg(xmlReader.name().toString()).toStdString());
}

NORETURN static void mandatoryAttibuteMissing(const QXmlStreamReader &xmlReader, const QString& attributeName)
{
    throw std::runtime_error(QObject::tr("line %1: Mandatory attribute '%2' missing in '%3'")
                             .arg(xmlReader.lineNumber())
                             .arg(attributeName)
                             .arg(xmlReader.name().toString()).toStdString());
}

static CppcheckLibraryData::Container loadContainer(QXmlStreamReader &xmlReader)
{
    CppcheckLibraryData::Container container;
    container.id            = xmlReader.attributes().value("id").toString();
    container.inherits      = xmlReader.attributes().value("inherits").toString();
    container.startPattern  = xmlReader.attributes().value("startPattern").toString();
    container.endPattern    = xmlReader.attributes().value("endPattern").toString();
    container.opLessAllowed = xmlReader.attributes().value("opLessAllowed").toString();
    container.itEndPattern  = xmlReader.attributes().value("itEndPattern").toString();

    QXmlStreamReader::TokenType type;
    while ((type = xmlReader.readNext()) != QXmlStreamReader::EndElement ||
           xmlReader.name().toString() != "container") {
        if (type != QXmlStreamReader::StartElement)
            continue;
        const QString elementName = xmlReader.name().toString();
        if (elementName == "type") {
            container.type.templateParameter = xmlReader.attributes().value("templateParameter").toString();
            container.type.string            = xmlReader.attributes().value("string").toString();
        } else if (elementName == "size" || elementName == "access" || elementName == "other" || elementName == "rangeItemRecordType") {
            const QString indexOperator = xmlReader.attributes().value("indexOperator").toString();
            if (elementName == "access" && indexOperator == "array-like")
                container.access_arrayLike = true;
            const QString templateParameter = xmlReader.attributes().value("templateParameter").toString();
            if (elementName == "size" && !templateParameter.isEmpty())
                container.size_templateParameter = templateParameter.toInt();
            for (;;) {
                type = xmlReader.readNext();
                if (xmlReader.name().toString() == elementName)
                    break;
                if (type != QXmlStreamReader::StartElement)
                    continue;
                CppcheckLibraryData::Container::Function function;
                function.name   = xmlReader.attributes().value("name").toString();
                function.action = xmlReader.attributes().value("action").toString();
                function.yields = xmlReader.attributes().value("yields").toString();
                if (elementName == "size")
                    container.sizeFunctions.append(function);
                else if (elementName == "access")
                    container.accessFunctions.append(function);
                else if (elementName == "rangeItemRecordType") {
                    CppcheckLibraryData::Container::RangeItemRecordType rangeItemRecordType;
                    rangeItemRecordType.name = xmlReader.attributes().value("name").toString();
                    rangeItemRecordType.templateParameter = xmlReader.attributes().value("templateParameter").toString();
                    container.rangeItemRecordTypeList.append(rangeItemRecordType);
                } else
                    container.otherFunctions.append(function);
            }
        } else {
            unhandledElement(xmlReader);
        }
    }
    return container;
}

static CppcheckLibraryData::Define loadDefine(const QXmlStreamReader &xmlReader)
{
    CppcheckLibraryData::Define define;
    define.name = xmlReader.attributes().value("name").toString();
    define.value = xmlReader.attributes().value("value").toString();
    return define;
}

static QString loadUndefine(const QXmlStreamReader &xmlReader)
{
    return xmlReader.attributes().value("name").toString();
}

static CppcheckLibraryData::SmartPointer loadSmartPointer(QXmlStreamReader &xmlReader)
{
    CppcheckLibraryData::SmartPointer smartPointer;
    smartPointer.name = xmlReader.attributes().value("class-name").toString();
    QXmlStreamReader::TokenType type;
    while ((type = xmlReader.readNext()) != QXmlStreamReader::EndElement ||
           xmlReader.name().toString() != "smart-pointer") {
        if (type != QXmlStreamReader::StartElement)
            continue;
        const QString elementName = xmlReader.name().toString();
        if (elementName == "unique") {
            smartPointer.unique = true;
        } else {
            unhandledElement(xmlReader);
        }
    }
    return smartPointer;
}

static CppcheckLibraryData::TypeChecks loadTypeChecks(QXmlStreamReader &xmlReader)
{
    CppcheckLibraryData::TypeChecks typeChecks;
    QXmlStreamReader::TokenType type;
    while ((type = xmlReader.readNext()) != QXmlStreamReader::EndElement ||
           xmlReader.name().toString() != "type-checks") {
        if (type != QXmlStreamReader::StartElement)
            continue;
        const QString elementName = xmlReader.name().toString();
        if (elementName == "suppress" || elementName == "check") {
            std::pair<QString, QString> entry(elementName, xmlReader.readElementText());
            typeChecks.append(entry);
        }
    }
    return typeChecks;
}

static CppcheckLibraryData::Function::Arg loadFunctionArg(QXmlStreamReader &xmlReader)
{
    CppcheckLibraryData::Function::Arg arg;
    QString argnr = xmlReader.attributes().value("nr").toString();
    if (argnr == "any")
        arg.nr = CppcheckLibraryData::Function::Arg::ANY;
    else if (argnr == "variadic")
        arg.nr = CppcheckLibraryData::Function::Arg::VARIADIC;
    else
        arg.nr = argnr.toUInt();
    arg.defaultValue = xmlReader.attributes().value("default").toString();

    QXmlStreamReader::TokenType type;
    while ((type = xmlReader.readNext()) != QXmlStreamReader::EndElement ||
           xmlReader.name().toString() != "arg") {
        if (type != QXmlStreamReader::StartElement)
            continue;
        const QString elementName = xmlReader.name().toString();
        if (elementName == "not-bool")
            arg.notbool = true;
        else if (elementName == "not-null")
            arg.notnull = true;
        else if (elementName == "not-uninit")
            arg.notuninit = true;
        else if (elementName == "strz")
            arg.strz = true;
        else if (elementName == "formatstr")
            arg.formatstr = true;
        else if (elementName == "valid")
            arg.valid = xmlReader.readElementText();
        else if (elementName == "minsize") {
            CppcheckLibraryData::Function::Arg::MinSize minsize;
            minsize.type = xmlReader.attributes().value("type").toString();
            minsize.arg  = xmlReader.attributes().value("arg").toString();
            minsize.arg2 = xmlReader.attributes().value("arg2").toString();
            arg.minsizes.append(minsize);
        } else if (elementName == "iterator") {
            arg.iterator.container = xmlReader.attributes().value("container").toInt();
            arg.iterator.type = xmlReader.attributes().value("type").toString();
        } else {
            unhandledElement(xmlReader);
        }
    }
    return arg;
}

static CppcheckLibraryData::Function loadFunction(QXmlStreamReader &xmlReader, const QString &comments)
{
    CppcheckLibraryData::Function function;
    function.comments = comments;
    function.name = xmlReader.attributes().value("name").toString();
    QXmlStreamReader::TokenType type;
    while ((type = xmlReader.readNext()) != QXmlStreamReader::EndElement ||
           xmlReader.name().toString() != "function") {
        if (type != QXmlStreamReader::StartElement)
            continue;
        const QString elementName = xmlReader.name().toString();
        if (elementName == "noreturn")
            function.noreturn = (xmlReader.readElementText() == "true") ? CppcheckLibraryData::Function::True : CppcheckLibraryData::Function::False;
        else if (elementName == "pure")
            function.gccPure = true;
        else if (elementName == "const")
            function.gccConst = true;
        else if (elementName == "leak-ignore")
            function.leakignore = true;
        else if (elementName == "use-retval")
            function.useretval = true;
        else if (elementName == "returnValue") {
            const QString container = xmlReader.attributes().value("container").toString();
            function.returnValue.container = container.isNull() ? -1 : container.toInt();
            function.returnValue.type = xmlReader.attributes().value("type").toString();
            function.returnValue.value = xmlReader.readElementText();
        } else if (elementName == "formatstr") {
            function.formatstr.scan   = xmlReader.attributes().value("scan").toString();
            function.formatstr.secure = xmlReader.attributes().value("secure").toString();
        } else if (elementName == "arg")
            function.args.append(loadFunctionArg(xmlReader));
        else if (elementName == "warn") {
            function.warn.severity     = xmlReader.attributes().value("severity").toString();
            function.warn.cstd         = xmlReader.attributes().value("cstd").toString();
            function.warn.reason       = xmlReader.attributes().value("reason").toString();
            function.warn.alternatives = xmlReader.attributes().value("alternatives").toString();
            function.warn.msg          = xmlReader.readElementText();
        } else if (elementName == "not-overlapping-data") {
            const QStringList attributeList {"ptr1-arg", "ptr2-arg", "size-arg", "strlen-arg"};
            for (const QString &attr : attributeList) {
                if (xmlReader.attributes().hasAttribute(attr)) {
                    function.notOverlappingDataArgs[attr] = xmlReader.attributes().value(attr).toString();
                }
            }
        } else if (elementName == "container") {
            const QStringList attributeList {"action", "yields"};
            for (const QString &attr : attributeList) {
                if (xmlReader.attributes().hasAttribute(attr)) {
                    function.containerAttributes[attr] = xmlReader.attributes().value(attr).toString();
                }
            }
        } else {
            unhandledElement(xmlReader);
        }
    }
    return function;
}

static CppcheckLibraryData::MemoryResource loadMemoryResource(QXmlStreamReader &xmlReader)
{
    CppcheckLibraryData::MemoryResource memoryresource;
    memoryresource.type = xmlReader.name().toString();
    QXmlStreamReader::TokenType type;
    while ((type = xmlReader.readNext()) != QXmlStreamReader::EndElement ||
           xmlReader.name().toString() != memoryresource.type) {
        if (type != QXmlStreamReader::StartElement)
            continue;
        const QString elementName = xmlReader.name().toString();
        if (elementName == "alloc" || elementName == "realloc") {
            CppcheckLibraryData::MemoryResource::Alloc alloc;
            alloc.isRealloc = (elementName == "realloc");
            alloc.noFail = (xmlReader.attributes().value("no-fail").toString() == "true");
            alloc.init = (xmlReader.attributes().value("init").toString() == "true");
            if (xmlReader.attributes().hasAttribute("arg")) {
                alloc.arg = xmlReader.attributes().value("arg").toInt();
            }
            if (alloc.isRealloc && xmlReader.attributes().hasAttribute("realloc-arg")) {
                alloc.reallocArg = xmlReader.attributes().value("realloc-arg").toInt();
            }
            if (memoryresource.type == "memory") {
                alloc.bufferSize = xmlReader.attributes().value("buffer-size").toString();
            }
            alloc.name = xmlReader.readElementText();
            memoryresource.alloc.append(alloc);
        } else if (elementName == "dealloc") {
            CppcheckLibraryData::MemoryResource::Dealloc dealloc;
            if (xmlReader.attributes().hasAttribute("arg")) {
                dealloc.arg = xmlReader.attributes().value("arg").toInt();
            }
            dealloc.name = xmlReader.readElementText();
            memoryresource.dealloc.append(dealloc);
        } else if (elementName == "use")
            memoryresource.use.append(xmlReader.readElementText());
        else
            unhandledElement(xmlReader);
    }
    return memoryresource;
}

static CppcheckLibraryData::PodType loadPodType(const QXmlStreamReader &xmlReader)
{
    CppcheckLibraryData::PodType podtype;
    podtype.name = xmlReader.attributes().value("name").toString();
    if (podtype.name.isEmpty()) {
        mandatoryAttibuteMissing(xmlReader, "name");
    }
    podtype.stdtype = xmlReader.attributes().value("stdtype").toString();
    podtype.size = xmlReader.attributes().value("size").toString();
    podtype.sign = xmlReader.attributes().value("sign").toString();
    return podtype;
}

static CppcheckLibraryData::PlatformType loadPlatformType(QXmlStreamReader &xmlReader)
{
    CppcheckLibraryData::PlatformType platformType;
    platformType.name = xmlReader.attributes().value("name").toString();
    platformType.value = xmlReader.attributes().value("value").toString();

    QXmlStreamReader::TokenType type;
    while ((type = xmlReader.readNext()) != QXmlStreamReader::EndElement ||
           xmlReader.name().toString() != "platformtype") {
        if (type != QXmlStreamReader::StartElement)
            continue;
        const QString elementName = xmlReader.name().toString();
        if (QStringList({"unsigned", "long", "pointer", "const_ptr", "ptr_ptr"}).contains(elementName)) {
            platformType.types.append(elementName);
        } else if (elementName == "platform") {
            platformType.platforms.append(xmlReader.attributes().value("type").toString());
        } else {
            unhandledElement(xmlReader);
        }
    }
    return platformType;
}

static CppcheckLibraryData::Reflection loadReflection(QXmlStreamReader &xmlReader)
{
    CppcheckLibraryData::Reflection reflection;

    QXmlStreamReader::TokenType type;
    while ((type = xmlReader.readNext()) != QXmlStreamReader::EndElement ||
           xmlReader.name().toString() != "reflection") {
        if (type != QXmlStreamReader::StartElement)
            continue;
        const QString elementName = xmlReader.name().toString();
        if (elementName == "call") {
            CppcheckLibraryData::Reflection::Call call;
            if (xmlReader.attributes().hasAttribute("arg")) {
                call.arg = xmlReader.attributes().value("arg").toInt();
            } else {
                mandatoryAttibuteMissing(xmlReader, "arg");
            }
            call.name = xmlReader.readElementText();
            reflection.calls.append(call);
        } else {
            unhandledElement(xmlReader);
        }
    }

    return reflection;
}

static CppcheckLibraryData::Markup loadMarkup(QXmlStreamReader &xmlReader)
{
    CppcheckLibraryData::Markup markup;

    QXmlStreamReader::TokenType type;
    if (xmlReader.attributes().hasAttribute("ext")) {
        markup.ext = xmlReader.attributes().value("ext").toString();
    } else {
        mandatoryAttibuteMissing(xmlReader, "ext");
    }
    if (xmlReader.attributes().hasAttribute("aftercode")) {
        markup.afterCode = (xmlReader.attributes().value("aftercode") == QString("true"));
    } else {
        mandatoryAttibuteMissing(xmlReader, "aftercode");
    }
    if (xmlReader.attributes().hasAttribute("reporterrors")) {
        markup.reportErrors = (xmlReader.attributes().value("reporterrors") == QString("true"));
    } else {
        mandatoryAttibuteMissing(xmlReader, "reporterrors");
    }

    while ((type = xmlReader.readNext()) != QXmlStreamReader::EndElement ||
           xmlReader.name().toString() != "markup") {
        if (type != QXmlStreamReader::StartElement)
            continue;
        const QString elementName = xmlReader.name().toString();
        if (elementName == "keywords") {
            while ((type = xmlReader.readNext()) != QXmlStreamReader::EndElement ||
                   xmlReader.name().toString() != "keywords") {
                if (type != QXmlStreamReader::StartElement)
                    continue;
                if (xmlReader.name().toString() == "keyword") {
                    markup.keywords.append(xmlReader.attributes().value("name").toString());
                } else {
                    unhandledElement(xmlReader);
                }
            }
        } else if (elementName == "codeblocks") {
            CppcheckLibraryData::Markup::CodeBlocks codeBlock;

            while ((type = xmlReader.readNext()) != QXmlStreamReader::EndElement ||
                   xmlReader.name().toString() != "codeblocks") {
                if (type != QXmlStreamReader::StartElement)
                    continue;
                if (xmlReader.name().toString() == "block") {
                    codeBlock.blocks.append(xmlReader.attributes().value("name").toString());
                } else if (xmlReader.name().toString() == "structure") {
                    codeBlock.offset = xmlReader.attributes().value("offset").toInt();
                    codeBlock.start = xmlReader.attributes().value("start").toString();
                    codeBlock.end = xmlReader.attributes().value("end").toString();
                } else {
                    unhandledElement(xmlReader);
                }
            }
            markup.codeBlocks.append(codeBlock);
        } else if (elementName == "exported") {
            CppcheckLibraryData::Markup::Exporter exporter;

            while ((type = xmlReader.readNext()) != QXmlStreamReader::EndElement ||
                   xmlReader.name().toString() != "exported") {
                if (type != QXmlStreamReader::StartElement)
                    continue;
                if (xmlReader.name().toString() == "exporter") {
                    exporter.prefix = xmlReader.attributes().value("prefix").toString();
                } else if (xmlReader.name().toString() == "prefix") {
                    exporter.prefixList.append(xmlReader.readElementText());
                } else if (xmlReader.name().toString() == "suffix") {
                    exporter.suffixList.append(xmlReader.readElementText());
                } else {
                    unhandledElement(xmlReader);
                }
            }
            markup.exporter.append(exporter);
        } else if (elementName == "imported") {
            while ((type = xmlReader.readNext()) != QXmlStreamReader::EndElement ||
                   xmlReader.name().toString() != "imported") {
                if (type != QXmlStreamReader::StartElement)
                    continue;
                if (xmlReader.name().toString() == "importer") {
                    markup.importer.append(xmlReader.readElementText());
                } else {
                    unhandledElement(xmlReader);
                }
            }
        } else {
            unhandledElement(xmlReader);
        }
    }

    return markup;
}

static CppcheckLibraryData::Entrypoint loadEntrypoint(const QXmlStreamReader &xmlReader)
{
    CppcheckLibraryData::Entrypoint entrypoint;
    entrypoint.name = xmlReader.attributes().value("name").toString();
    return entrypoint;
}

QString CppcheckLibraryData::open(QIODevice &file)
{
    clear();
    QString comments;
    QXmlStreamReader xmlReader(&file);
    while (!xmlReader.atEnd()) {
        const QXmlStreamReader::TokenType t = xmlReader.readNext();
        switch (t) {
        case QXmlStreamReader::Comment:
            if (!comments.isEmpty())
                comments += "\n";
            comments += xmlReader.text().toString();
            break;
        case QXmlStreamReader::StartElement:
            try {
                const QString elementName(xmlReader.name().toString());
                if (elementName == "def")
                    ;
                else if (elementName == "container")
                    containers.append(loadContainer(xmlReader));
                else if (elementName == "define")
                    defines.append(loadDefine(xmlReader));
                else if (elementName == "undefine")
                    undefines.append(loadUndefine(xmlReader));
                else if (elementName == "function")
                    functions.append(loadFunction(xmlReader, comments));
                else if (elementName == "memory" || elementName == "resource")
                    memoryresource.append(loadMemoryResource(xmlReader));
                else if (elementName == "podtype")
                    podtypes.append(loadPodType(xmlReader));
                else if (elementName == "smart-pointer")
                    smartPointers.append(loadSmartPointer(xmlReader));
                else if (elementName == "type-checks")
                    typeChecks.append(loadTypeChecks(xmlReader));
                else if (elementName == "platformtype")
                    platformTypes.append(loadPlatformType(xmlReader));
                else if (elementName == "reflection")
                    reflections.append(loadReflection(xmlReader));
                else if (elementName == "markup")
                    markups.append(loadMarkup(xmlReader));
                else if (elementName == "entrypoint")
                    entrypoints.append(loadEntrypoint(xmlReader));
                else
                    unhandledElement(xmlReader);
            } catch (std::runtime_error &e) {
                return e.what();
            }
            comments.clear();
            break;
        default:
            break;
        }
    }
    if (xmlReader.hasError())
        return xmlReader.errorString();
    return QString();
}

static void writeContainerFunctions(QXmlStreamWriter &xmlWriter, const QString &name, int extra, const QList<CppcheckLibraryData::Container::Function> &functions)
{
    if (functions.isEmpty() && extra < 0)
        return;
    xmlWriter.writeStartElement(name);
    if (extra >= 0) {
        if (name == "access")
            xmlWriter.writeAttribute("indexOperator", "array-like");
        else if (name == "size")
            xmlWriter.writeAttribute("templateParameter", QString::number(extra));
    }
    for (const CppcheckLibraryData::Container::Function &function : functions) {
        xmlWriter.writeStartElement("function");
        xmlWriter.writeAttribute("name", function.name);
        if (!function.action.isEmpty())
            xmlWriter.writeAttribute("action", function.action);
        if (!function.yields.isEmpty())
            xmlWriter.writeAttribute("yields", function.yields);
        xmlWriter.writeEndElement();
    }
    xmlWriter.writeEndElement();
}

static void writeContainerRangeItemRecords(QXmlStreamWriter &xmlWriter, const QList<CppcheckLibraryData::Container::RangeItemRecordType> &rangeItemRecords)
{
    if (rangeItemRecords.isEmpty())
        return;
    xmlWriter.writeStartElement("rangeItemRecordType");
    for (const CppcheckLibraryData::Container::RangeItemRecordType &item : rangeItemRecords) {
        xmlWriter.writeStartElement("member");
        xmlWriter.writeAttribute("name", item.name);
        xmlWriter.writeAttribute("templateParameter", item.templateParameter);
        xmlWriter.writeEndElement();
    }
    xmlWriter.writeEndElement();
}

static void writeContainer(QXmlStreamWriter &xmlWriter, const CppcheckLibraryData::Container &container)
{
    xmlWriter.writeStartElement("container");
    xmlWriter.writeAttribute("id", container.id);
    if (!container.startPattern.isEmpty())
        xmlWriter.writeAttribute("startPattern", container.startPattern);
    if (!container.endPattern.isNull())
        xmlWriter.writeAttribute("endPattern", container.endPattern);
    if (!container.inherits.isEmpty())
        xmlWriter.writeAttribute("inherits", container.inherits);
    if (!container.opLessAllowed.isEmpty())
        xmlWriter.writeAttribute("opLessAllowed", container.opLessAllowed);
    if (!container.itEndPattern.isEmpty())
        xmlWriter.writeAttribute("itEndPattern", container.itEndPattern);

    if (!container.type.templateParameter.isEmpty() || !container.type.string.isEmpty()) {
        xmlWriter.writeStartElement("type");
        if (!container.type.templateParameter.isEmpty())
            xmlWriter.writeAttribute("templateParameter", container.type.templateParameter);
        if (!container.type.string.isEmpty())
            xmlWriter.writeAttribute("string", container.type.string);
        xmlWriter.writeEndElement();
    }
    writeContainerFunctions(xmlWriter, "size", container.size_templateParameter, container.sizeFunctions);
    writeContainerFunctions(xmlWriter, "access", container.access_arrayLike?1:-1, container.accessFunctions);
    writeContainerFunctions(xmlWriter, "other", -1, container.otherFunctions);
    writeContainerRangeItemRecords(xmlWriter, container.rangeItemRecordTypeList);
    xmlWriter.writeEndElement();
}

static void writeFunction(QXmlStreamWriter &xmlWriter, const CppcheckLibraryData::Function &function)
{
    QString comments = function.comments;
    while (comments.startsWith("\n"))
        comments = comments.mid(1);
    while (comments.endsWith("\n"))
        comments.chop(1);
    for (const QString &comment : comments.split('\n')) {
        if (comment.length() >= 1)
            xmlWriter.writeComment(comment);
    }

    xmlWriter.writeStartElement("function");
    xmlWriter.writeAttribute("name", function.name);

    if (function.useretval)
        xmlWriter.writeEmptyElement("use-retval");
    if (function.gccConst)
        xmlWriter.writeEmptyElement("const");
    if (function.gccPure)
        xmlWriter.writeEmptyElement("pure");
    if (!function.returnValue.empty()) {
        xmlWriter.writeStartElement("returnValue");
        if (!function.returnValue.type.isNull())
            xmlWriter.writeAttribute("type", function.returnValue.type);
        if (function.returnValue.container >= 0)
            xmlWriter.writeAttribute("container", QString::number(function.returnValue.container));
        if (!function.returnValue.value.isNull())
            xmlWriter.writeCharacters(function.returnValue.value);
        xmlWriter.writeEndElement();
    }
    if (function.noreturn != CppcheckLibraryData::Function::Unknown)
        xmlWriter.writeTextElement("noreturn", bool_to_string(function.noreturn == CppcheckLibraryData::Function::True));
    if (function.leakignore)
        xmlWriter.writeEmptyElement("leak-ignore");
    // Argument info..
    for (const CppcheckLibraryData::Function::Arg &arg : function.args) {
        if (arg.formatstr) {
            xmlWriter.writeStartElement("formatstr");
            if (!function.formatstr.scan.isNull())
                xmlWriter.writeAttribute("scan", function.formatstr.scan);
            if (!function.formatstr.secure.isNull())
                xmlWriter.writeAttribute("secure", function.formatstr.secure);
            xmlWriter.writeEndElement();
        }

        xmlWriter.writeStartElement("arg");
        if (arg.nr == CppcheckLibraryData::Function::Arg::ANY)
            xmlWriter.writeAttribute("nr", "any");
        else if (arg.nr == CppcheckLibraryData::Function::Arg::VARIADIC)
            xmlWriter.writeAttribute("nr", "variadic");
        else
            xmlWriter.writeAttribute("nr", QString::number(arg.nr));
        if (!arg.defaultValue.isNull())
            xmlWriter.writeAttribute("default", arg.defaultValue);
        if (arg.formatstr)
            xmlWriter.writeEmptyElement("formatstr");
        if (arg.notnull)
            xmlWriter.writeEmptyElement("not-null");
        if (arg.notuninit)
            xmlWriter.writeEmptyElement("not-uninit");
        if (arg.notbool)
            xmlWriter.writeEmptyElement("not-bool");
        if (arg.strz)
            xmlWriter.writeEmptyElement("strz");

        if (!arg.valid.isEmpty())
            xmlWriter.writeTextElement("valid",arg.valid);

        for (const CppcheckLibraryData::Function::Arg::MinSize &minsize : arg.minsizes) {
            xmlWriter.writeStartElement("minsize");
            xmlWriter.writeAttribute("type", minsize.type);
            xmlWriter.writeAttribute("arg", minsize.arg);
            if (!minsize.arg2.isEmpty())
                xmlWriter.writeAttribute("arg2", minsize.arg2);
            xmlWriter.writeEndElement();
        }

        if (arg.iterator.container >= 0 || !arg.iterator.type.isNull()) {
            xmlWriter.writeStartElement("iterator");
            if (arg.iterator.container >= 0)
                xmlWriter.writeAttribute("container", QString::number(arg.iterator.container));
            if (!arg.iterator.type.isNull())
                xmlWriter.writeAttribute("type", arg.iterator.type);
            xmlWriter.writeEndElement();
        }

        xmlWriter.writeEndElement();
    }

    if (!function.warn.isEmpty()) {
        xmlWriter.writeStartElement("warn");

        if (!function.warn.severity.isEmpty())
            xmlWriter.writeAttribute("severity", function.warn.severity);

        if (!function.warn.cstd.isEmpty())
            xmlWriter.writeAttribute("cstd", function.warn.cstd);

        if (!function.warn.alternatives.isEmpty())
            xmlWriter.writeAttribute("alternatives", function.warn.alternatives);

        if (!function.warn.reason.isEmpty())
            xmlWriter.writeAttribute("reason", function.warn.reason);

        if (!function.warn.msg.isEmpty())
            xmlWriter.writeCharacters(function.warn.msg);

        xmlWriter.writeEndElement();
    }
    if (!function.notOverlappingDataArgs.isEmpty()) {
        xmlWriter.writeStartElement("not-overlapping-data");
        for (const QString& value : function.notOverlappingDataArgs) {
            xmlWriter.writeAttribute(function.notOverlappingDataArgs.key(value), value);
        }
        xmlWriter.writeEndElement();
    }
    if (!function.containerAttributes.isEmpty()) {
        xmlWriter.writeStartElement("container");
        for (const QString& value : function.containerAttributes) {
            xmlWriter.writeAttribute(function.containerAttributes.key(value), value);
        }
        xmlWriter.writeEndElement();
    }
    xmlWriter.writeEndElement();
}

static void writeMemoryResource(QXmlStreamWriter &xmlWriter, const CppcheckLibraryData::MemoryResource &mr)
{
    xmlWriter.writeStartElement(mr.type);
    for (const CppcheckLibraryData::MemoryResource::Alloc &alloc : mr.alloc) {
        if (alloc.isRealloc) {
            xmlWriter.writeStartElement("realloc");
        } else {
            xmlWriter.writeStartElement("alloc");
        }
        xmlWriter.writeAttribute("init", bool_to_string(alloc.init));
        if (alloc.noFail)
            xmlWriter.writeAttribute("no-fail", bool_to_string(alloc.noFail));
        if (alloc.arg != -1) {
            xmlWriter.writeAttribute("arg", QString("%1").arg(alloc.arg));
        }
        if (alloc.isRealloc && alloc.reallocArg != -1) {
            xmlWriter.writeAttribute("realloc-arg", QString("%1").arg(alloc.reallocArg));
        }
        if (mr.type == "memory" && !alloc.bufferSize.isEmpty()) {
            xmlWriter.writeAttribute("buffer-size", alloc.bufferSize);
        }
        xmlWriter.writeCharacters(alloc.name);
        xmlWriter.writeEndElement();
    }

    for (const CppcheckLibraryData::MemoryResource::Dealloc &dealloc : mr.dealloc) {
        xmlWriter.writeStartElement("dealloc");
        if (dealloc.arg != -1) {
            xmlWriter.writeAttribute("arg", QString("%1").arg(dealloc.arg));
        }
        xmlWriter.writeCharacters(dealloc.name);
        xmlWriter.writeEndElement();
    }

    for (const QString &use : mr.use) {
        xmlWriter.writeTextElement("use", use);
    }
    xmlWriter.writeEndElement();
}

static void writeTypeChecks(QXmlStreamWriter &xmlWriter, const CppcheckLibraryData::TypeChecks &typeChecks)
{
    xmlWriter.writeStartElement("type-checks");
    if (!typeChecks.isEmpty()) {
        xmlWriter.writeStartElement("unusedvar");
    }
    for (const std::pair<QString, QString> &check : typeChecks) {
        xmlWriter.writeStartElement(check.first);
        xmlWriter.writeCharacters(check.second);
        xmlWriter.writeEndElement();
    }
    if (!typeChecks.isEmpty()) {
        xmlWriter.writeEndElement();
    }
    xmlWriter.writeEndElement();
}

static void writePlatformType(QXmlStreamWriter &xmlWriter, const CppcheckLibraryData::PlatformType &pt)
{
    xmlWriter.writeStartElement("platformtype");
    xmlWriter.writeAttribute("name", pt.name);
    xmlWriter.writeAttribute("value", pt.value);
    for (const QString &type : pt.types) {
        xmlWriter.writeStartElement(type);
        xmlWriter.writeEndElement();
    }
    for (const QString &platform : pt.platforms) {
        xmlWriter.writeStartElement("platform");
        if (!platform.isEmpty()) {
            xmlWriter.writeAttribute("type", platform);
        }
        xmlWriter.writeEndElement();
    }
    xmlWriter.writeEndElement();
}

static void writeReflection(QXmlStreamWriter &xmlWriter, const CppcheckLibraryData::Reflection &refl)
{
    xmlWriter.writeStartElement("reflection");
    for (const CppcheckLibraryData::Reflection::Call &call : refl.calls) {
        xmlWriter.writeStartElement("call");
        xmlWriter.writeAttribute("arg", QString("%1").arg(call.arg));
        xmlWriter.writeCharacters(call.name);
        xmlWriter.writeEndElement();
    }
    xmlWriter.writeEndElement();
}

static void writeMarkup(QXmlStreamWriter &xmlWriter, const CppcheckLibraryData::Markup &mup)
{
    xmlWriter.writeStartElement("markup");
    xmlWriter.writeAttribute("ext", mup.ext);
    xmlWriter.writeAttribute("aftercode", QVariant(mup.afterCode).toString());
    xmlWriter.writeAttribute("reporterrors", QVariant(mup.reportErrors).toString());
    if (!mup.keywords.isEmpty()) {
        xmlWriter.writeStartElement("keywords");
        for (const QString &keyword : mup.keywords) {
            xmlWriter.writeStartElement("keyword");
            xmlWriter.writeAttribute("name", keyword);
            xmlWriter.writeEndElement();
        }
        xmlWriter.writeEndElement();
    }
    if (!mup.importer.isEmpty()) {
        xmlWriter.writeStartElement("imported");
        for (const QString &import : mup.importer) {
            xmlWriter.writeStartElement("importer");
            xmlWriter.writeCharacters(import);
            xmlWriter.writeEndElement();
        }
        xmlWriter.writeEndElement();
    }
    if (!mup.exporter.isEmpty()) {
        xmlWriter.writeStartElement("exported");
        for (const CppcheckLibraryData::Markup::Exporter &exporter : mup.exporter) {
            xmlWriter.writeStartElement("exporter");
            xmlWriter.writeAttribute("prefix", exporter.prefix);
            for (const QString &prefix : exporter.prefixList) {
                xmlWriter.writeStartElement("prefix");
                xmlWriter.writeCharacters(prefix);
                xmlWriter.writeEndElement();
            }
            for (const QString &suffix : exporter.suffixList) {
                xmlWriter.writeStartElement("suffix");
                xmlWriter.writeCharacters(suffix);
                xmlWriter.writeEndElement();
            }
            xmlWriter.writeEndElement();
        }
        xmlWriter.writeEndElement();
    }
    if (!mup.codeBlocks.isEmpty()) {
        for (const CppcheckLibraryData::Markup::CodeBlocks &codeblock : mup.codeBlocks) {
            xmlWriter.writeStartElement("codeblocks");
            for (const QString &block : codeblock.blocks) {
                xmlWriter.writeStartElement("block");
                xmlWriter.writeAttribute("name", block);
                xmlWriter.writeEndElement();
            }
            xmlWriter.writeStartElement("structure");
            xmlWriter.writeAttribute("offset", QString("%1").arg(codeblock.offset));
            xmlWriter.writeAttribute("start", codeblock.start);
            xmlWriter.writeAttribute("end", codeblock.end);
            xmlWriter.writeEndElement();
            xmlWriter.writeEndElement();
        }
    }
    xmlWriter.writeEndElement();
}

QString CppcheckLibraryData::toString() const
{
    QString outputString;
    QXmlStreamWriter xmlWriter(&outputString);
    xmlWriter.setAutoFormatting(true);
    xmlWriter.setAutoFormattingIndent(2);
    xmlWriter.writeStartDocument("1.0");
    xmlWriter.writeStartElement("def");
    xmlWriter.writeAttribute("format","2");

    for (const Define &define : defines) {
        xmlWriter.writeStartElement("define");
        xmlWriter.writeAttribute("name", define.name);
        xmlWriter.writeAttribute("value", define.value);
        xmlWriter.writeEndElement();
    }

    for (const QString &undef : undefines) {
        xmlWriter.writeStartElement("undefine");
        xmlWriter.writeAttribute("name", undef);
        xmlWriter.writeEndElement();
    }

    for (const Function &function : functions) {
        writeFunction(xmlWriter, function);
    }

    for (const MemoryResource &mr : memoryresource) {
        writeMemoryResource(xmlWriter, mr);
    }

    for (const Container &container : containers) {
        writeContainer(xmlWriter, container);
    }

    for (const PodType &podtype : podtypes) {
        xmlWriter.writeStartElement("podtype");
        xmlWriter.writeAttribute("name", podtype.name);
        if (!podtype.stdtype.isEmpty())
            xmlWriter.writeAttribute("stdtype", podtype.stdtype);
        if (!podtype.sign.isEmpty())
            xmlWriter.writeAttribute("sign", podtype.sign);
        if (!podtype.size.isEmpty())
            xmlWriter.writeAttribute("size", podtype.size);
        xmlWriter.writeEndElement();
    }

    for (const TypeChecks &check : typeChecks) {
        writeTypeChecks(xmlWriter, check);
    }

    for (const SmartPointer &smartPtr : smartPointers) {
        xmlWriter.writeStartElement("smart-pointer");
        xmlWriter.writeAttribute("class-name", smartPtr.name);
        if (smartPtr.unique) {
            xmlWriter.writeEmptyElement("unique");
        }
        xmlWriter.writeEndElement();
    }

    for (const PlatformType &pt : platformTypes) {
        writePlatformType(xmlWriter, pt);
    }

    for (const Reflection &refl : reflections) {
        writeReflection(xmlWriter, refl);
    }

    for (const Markup &mup : markups) {
        writeMarkup(xmlWriter, mup);
    }

    for (const Entrypoint &ent : entrypoints) {
        xmlWriter.writeStartElement("entrypoint");
        xmlWriter.writeAttribute("name", ent.name);
        xmlWriter.writeEndElement();
    }

    xmlWriter.writeEndElement();

    return outputString;
}
