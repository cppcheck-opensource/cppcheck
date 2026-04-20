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

#include "projectfile.h"

#include "common.h"
#include "config.h"
#include "importproject.h"
#include "settings.h"
#include "utils.h"

#include <utility>

#include <QCoreApplication>
#include <QFile>
#include <QDir>
#include <QIODevice>
#include <QLatin1String>
#include <QRegularExpression>
#include <QXmlStreamAttributes>
#include <QXmlStreamReader>
#include <QXmlStreamWriter>

ProjectFile *ProjectFile::mActiveProject;

ProjectFile::ProjectFile(QObject *parent) :
    QObject(parent)
{
    clear();
}

ProjectFile::ProjectFile(QString filename, QObject *parent) :
    QObject(parent),
    mFilename(std::move(filename))
{
    clear();
    read();
}

void ProjectFile::clear()
{
    const Settings settings;
    clangParser = false;
    mCheckLevel = CheckLevel::normal;
    mRootPath.clear();
    mBuildDir.clear();
    mImportProject.clear();
    mIncludeDirs.clear();
    mDefines.clear();
    mUndefines.clear();
    mUserInclude.clear();
    mPaths.clear();
    mExcludedPaths.clear();
    mLibraries.clear();
    mPlatform.clear();
    mProjectName.clear();
    mSuppressions.clear();
    mAddons.clear();
    mClangAnalyzer = mClangTidy = false;
    mAnalyzeAllVsConfigs = false; // TODO: defaults to true if loading a GUI project via CLI
    mCheckHeaders = true;
    mCheckUnusedTemplates = true;
    mInlineSuppression = true;
    mMaxCtuDepth = settings.maxCtuDepth;
    mMaxTemplateRecursion = settings.maxTemplateRecursion;
    mCheckUnknownFunctionReturn.clear();
    safeChecks.clear();
    mVsConfigurations.clear();
    mTags.clear();
    mWarningTags.clear();

    // Premium
    mBughunting = false;
    mCertIntPrecision = 0;
    mCodingStandards.clear();
    mPremiumLicenseFile.clear();
}

bool ProjectFile::read(const QString &filename)
{
    if (!filename.isEmpty())
        mFilename = filename;

    QFile file(mFilename);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
        return false;

    clear();

    QXmlStreamReader xmlReader(&file);
    bool insideProject = false;
    bool projectTagFound = false;
    while (!xmlReader.atEnd()) {
        switch (xmlReader.readNext()) {
        case QXmlStreamReader::StartElement:
            if (xmlReader.name() == QString(CppcheckXml::ProjectElementName)) {
                insideProject = true;
                projectTagFound = true;
                break;
            }
            if (!insideProject)
                break;

            // Read root path from inside project element
            if (xmlReader.name() == QString(CppcheckXml::RootPathName))
                readRootPath(xmlReader);

            // Read root path from inside project element
            if (xmlReader.name() == QString(CppcheckXml::BuildDirElementName))
                readBuildDir(xmlReader);

            // Find paths to check from inside project element
            if (xmlReader.name() == QString(CppcheckXml::PathsElementName))
                readCheckPaths(xmlReader);

            if (xmlReader.name() == QString(CppcheckXml::ImportProjectElementName))
                readImportProject(xmlReader);

            if (xmlReader.name() == QString(CppcheckXml::AnalyzeAllVsConfigsElementName))
                mAnalyzeAllVsConfigs = readBool(xmlReader);

            if (xmlReader.name() == QString(CppcheckXml::Parser))
                clangParser = true;

            if (xmlReader.name() == QString(CppcheckXml::CheckHeadersElementName))
                mCheckHeaders = readBool(xmlReader);

            if (xmlReader.name() == QString(CppcheckXml::CheckUnusedTemplatesElementName))
                mCheckUnusedTemplates = readBool(xmlReader);

            if (xmlReader.name() == QString(CppcheckXml::InlineSuppression))
                mInlineSuppression = readBool(xmlReader);

            if (xmlReader.name() == QString(CppcheckXml::CheckLevelExhaustiveElementName))
                mCheckLevel = CheckLevel::exhaustive;

            if (xmlReader.name() == QString(CppcheckXml::CheckLevelNormalElementName))
                mCheckLevel = CheckLevel::normal;

            if (xmlReader.name() == QString(CppcheckXml::CheckLevelReducedElementName))
                mCheckLevel = CheckLevel::reduced;

            // Find include directory from inside project element
            if (xmlReader.name() == QString(CppcheckXml::IncludeDirElementName))
                readIncludeDirs(xmlReader);

            // Find preprocessor define from inside project element
            if (xmlReader.name() == QString(CppcheckXml::DefinesElementName))
                readDefines(xmlReader);

            // Find preprocessor define from inside project element
            if (xmlReader.name() == QString(CppcheckXml::UndefinesElementName))
                readStringList(mUndefines, xmlReader, CppcheckXml::UndefineName);

            if (xmlReader.name() == QString(CppcheckXml::UserIncludeElementName))
                mUserInclude = readString(xmlReader);

            // Find exclude list from inside project element
            if (xmlReader.name() == QString(CppcheckXml::ExcludeElementName))
                readExcludes(xmlReader);

            // Find ignore list from inside project element
            // These are read for compatibility
            if (xmlReader.name() == QString(CppcheckXml::IgnoreElementName))
                readExcludes(xmlReader);

            // Find libraries list from inside project element
            if (xmlReader.name() == QString(CppcheckXml::LibrariesElementName))
                readStringList(mLibraries, xmlReader, CppcheckXml::LibraryElementName);

            if (xmlReader.name() == QString(CppcheckXml::PlatformElementName))
                readPlatform(xmlReader);

            // Find suppressions list from inside project element
            if (xmlReader.name() == QString(CppcheckXml::SuppressionsElementName))
                readSuppressions(xmlReader);

            // Unknown function return values
            if (xmlReader.name() == QString(CppcheckXml::CheckUnknownFunctionReturn))
                readStringList(mCheckUnknownFunctionReturn, xmlReader, CppcheckXml::Name);

            // check all function parameter values
            if (xmlReader.name() == QString(Settings::SafeChecks::XmlRootName))
                safeChecks.loadFromXml(xmlReader);

            // Addons
            if (xmlReader.name() == QString(CppcheckXml::AddonsElementName))
                readStringList(mAddons, xmlReader, CppcheckXml::AddonElementName);

            // Tools
            if (xmlReader.name() == QString(CppcheckXml::ToolsElementName)) {
                QStringList tools;
                readStringList(tools, xmlReader, CppcheckXml::ToolElementName);
                mClangAnalyzer = tools.contains(CLANG_ANALYZER);
                mClangTidy = tools.contains(CLANG_TIDY);
            }

            if (xmlReader.name() == QString(CppcheckXml::TagsElementName))
                readStringList(mTags, xmlReader, CppcheckXml::TagElementName);

            if (xmlReader.name() == QString(CppcheckXml::TagWarningsElementName))
                readTagWarnings(xmlReader, xmlReader.attributes().value(QString(), CppcheckXml::TagAttributeName).toString());

            if (xmlReader.name() == QString(CppcheckXml::MaxCtuDepthElementName))
                mMaxCtuDepth = readInt(xmlReader, mMaxCtuDepth);

            if (xmlReader.name() == QString(CppcheckXml::MaxTemplateRecursionElementName))
                mMaxTemplateRecursion = readInt(xmlReader, mMaxTemplateRecursion);

            // VSConfiguration
            if (xmlReader.name() == QString(CppcheckXml::VSConfigurationElementName))
                readVsConfigurations(xmlReader);

            // Cppcheck Premium
            if (xmlReader.name() == QString(CppcheckXml::BughuntingElementName))
                mBughunting = true;
            if (xmlReader.name() == QString(CppcheckXml::CodingStandardsElementName))
                readStringList(mCodingStandards, xmlReader, CppcheckXml::CodingStandardElementName);
            if (xmlReader.name() == QString(CppcheckXml::CertIntPrecisionElementName))
                mCertIntPrecision = readInt(xmlReader, 0);
            if (xmlReader.name() == QString(CppcheckXml::LicenseFileElementName))
                mPremiumLicenseFile = readString(xmlReader);
            if (xmlReader.name() == QString(CppcheckXml::ProjectNameElementName))
                mProjectName = readString(xmlReader);

            break;

        case QXmlStreamReader::EndElement:
            if (xmlReader.name() == QString(CppcheckXml::ProjectElementName))
                insideProject = false;
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

    file.close();
    return projectTagFound;
}

void ProjectFile::readRootPath(const QXmlStreamReader &reader)
{
    QXmlStreamAttributes attribs = reader.attributes();
    QString name = attribs.value(QString(), CppcheckXml::RootPathNameAttrib).toString();
    if (!name.isEmpty())
        mRootPath = name;
}

void ProjectFile::readBuildDir(QXmlStreamReader &reader)
{
    mBuildDir.clear();
    do {
        const QXmlStreamReader::TokenType type = reader.readNext();
        switch (type) {
        case QXmlStreamReader::Characters:
            mBuildDir = reader.text().toString();
            FALLTHROUGH;
        case QXmlStreamReader::EndElement:
            return;
        // Not handled
        case QXmlStreamReader::StartElement:
        case QXmlStreamReader::NoToken:
        case QXmlStreamReader::Invalid:
        case QXmlStreamReader::StartDocument:
        case QXmlStreamReader::EndDocument:
        case QXmlStreamReader::Comment:
        case QXmlStreamReader::DTD:
        case QXmlStreamReader::EntityReference:
        case QXmlStreamReader::ProcessingInstruction:
            break;
        }
    } while (true);
}

void ProjectFile::readImportProject(QXmlStreamReader &reader)
{
    mImportProject.clear();
    do {
        const QXmlStreamReader::TokenType type = reader.readNext();
        switch (type) {
        case QXmlStreamReader::Characters:
            mImportProject = reader.text().toString();
            FALLTHROUGH;
        case QXmlStreamReader::EndElement:
            return;
        // Not handled
        case QXmlStreamReader::StartElement:
        case QXmlStreamReader::NoToken:
        case QXmlStreamReader::Invalid:
        case QXmlStreamReader::StartDocument:
        case QXmlStreamReader::EndDocument:
        case QXmlStreamReader::Comment:
        case QXmlStreamReader::DTD:
        case QXmlStreamReader::EntityReference:
        case QXmlStreamReader::ProcessingInstruction:
            break;
        }
    } while (true);
}

bool ProjectFile::readBool(QXmlStreamReader &reader)
{
    bool ret = false;
    do {
        const QXmlStreamReader::TokenType type = reader.readNext();
        switch (type) {
        case QXmlStreamReader::Characters:
            ret = (reader.text().toString() == "true");
            FALLTHROUGH;
        case QXmlStreamReader::EndElement:
            return ret;
        // Not handled
        case QXmlStreamReader::StartElement:
        case QXmlStreamReader::NoToken:
        case QXmlStreamReader::Invalid:
        case QXmlStreamReader::StartDocument:
        case QXmlStreamReader::EndDocument:
        case QXmlStreamReader::Comment:
        case QXmlStreamReader::DTD:
        case QXmlStreamReader::EntityReference:
        case QXmlStreamReader::ProcessingInstruction:
            break;
        }
    } while (true);
}

int ProjectFile::readInt(QXmlStreamReader &reader, int defaultValue)
{
    do {
        const QXmlStreamReader::TokenType type = reader.readNext();
        switch (type) {
        case QXmlStreamReader::Characters:
            return reader.text().toString().toInt();
        case QXmlStreamReader::EndElement:
            return defaultValue;
        // Not handled
        case QXmlStreamReader::StartElement:
        case QXmlStreamReader::NoToken:
        case QXmlStreamReader::Invalid:
        case QXmlStreamReader::StartDocument:
        case QXmlStreamReader::EndDocument:
        case QXmlStreamReader::Comment:
        case QXmlStreamReader::DTD:
        case QXmlStreamReader::EntityReference:
        case QXmlStreamReader::ProcessingInstruction:
            break;
        }
    } while (true);
}

QString ProjectFile::readString(QXmlStreamReader &reader)
{
    do {
        const QXmlStreamReader::TokenType type = reader.readNext();
        switch (type) {
        case QXmlStreamReader::Characters:
            return reader.text().toString();
        case QXmlStreamReader::EndElement:
            return {};
        // Not handled
        case QXmlStreamReader::StartElement:
        case QXmlStreamReader::NoToken:
        case QXmlStreamReader::Invalid:
        case QXmlStreamReader::StartDocument:
        case QXmlStreamReader::EndDocument:
        case QXmlStreamReader::Comment:
        case QXmlStreamReader::DTD:
        case QXmlStreamReader::EntityReference:
        case QXmlStreamReader::ProcessingInstruction:
            break;
        }
    } while (true);
}

void ProjectFile::readIncludeDirs(QXmlStreamReader &reader)
{
    bool allRead = false;
    do {
        QXmlStreamReader::TokenType type = reader.readNext();
        switch (type) {
        case QXmlStreamReader::StartElement:

            // Read dir-elements
            if (reader.name().toString() == CppcheckXml::DirElementName) {
                QXmlStreamAttributes attribs = reader.attributes();
                QString name = attribs.value(QString(), CppcheckXml::DirNameAttrib).toString();
                if (!name.isEmpty())
                    mIncludeDirs << name;
            }
            break;

        case QXmlStreamReader::EndElement:
            if (reader.name().toString() == CppcheckXml::IncludeDirElementName)
                allRead = true;
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
    } while (!allRead);
}

void ProjectFile::readDefines(QXmlStreamReader &reader)
{
    bool allRead = false;
    do {
        QXmlStreamReader::TokenType type = reader.readNext();
        switch (type) {
        case QXmlStreamReader::StartElement:
            // Read define-elements
            if (reader.name().toString() == CppcheckXml::DefineName) {
                QXmlStreamAttributes attribs = reader.attributes();
                QString name = attribs.value(QString(), CppcheckXml::DefineNameAttrib).toString();
                if (!name.isEmpty())
                    mDefines << name;
            }
            break;

        case QXmlStreamReader::EndElement:
            if (reader.name().toString() == CppcheckXml::DefinesElementName)
                allRead = true;
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
    } while (!allRead);
}

void ProjectFile::readCheckPaths(QXmlStreamReader &reader)
{
    bool allRead = false;
    do {
        QXmlStreamReader::TokenType type = reader.readNext();
        switch (type) {
        case QXmlStreamReader::StartElement:

            // Read dir-elements
            if (reader.name().toString() == CppcheckXml::PathName) {
                QXmlStreamAttributes attribs = reader.attributes();
                QString name = attribs.value(QString(), CppcheckXml::PathNameAttrib).toString();
                if (!name.isEmpty())
                    mPaths << name;
            }
            break;

        case QXmlStreamReader::EndElement:
            if (reader.name().toString() == CppcheckXml::PathsElementName)
                allRead = true;
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
    } while (!allRead);
}

void ProjectFile::readExcludes(QXmlStreamReader &reader)
{
    bool allRead = false;
    do {
        QXmlStreamReader::TokenType type = reader.readNext();
        switch (type) {
        case QXmlStreamReader::StartElement:
            // Read exclude-elements
            if (reader.name().toString() == CppcheckXml::ExcludePathName) {
                QXmlStreamAttributes attribs = reader.attributes();
                QString name = attribs.value(QString(), CppcheckXml::ExcludePathNameAttrib).toString();
                if (!name.isEmpty())
                    mExcludedPaths << name;
            }
            // Read ignore-elements - deprecated but support reading them
            else if (reader.name().toString() == CppcheckXml::IgnorePathName) {
                QXmlStreamAttributes attribs = reader.attributes();
                QString name = attribs.value(QString(), CppcheckXml::IgnorePathNameAttrib).toString();
                if (!name.isEmpty())
                    mExcludedPaths << name;
            }
            break;

        case QXmlStreamReader::EndElement:
            if (reader.name().toString() == CppcheckXml::IgnoreElementName)
                allRead = true;
            if (reader.name().toString() == CppcheckXml::ExcludeElementName)
                allRead = true;
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
    } while (!allRead);
}

void ProjectFile::readVsConfigurations(QXmlStreamReader &reader)
{
    do {
        QXmlStreamReader::TokenType type = reader.readNext();
        switch (type) {
        case QXmlStreamReader::StartElement:
            // Read library-elements
            if (reader.name().toString() == CppcheckXml::VSConfigurationName) {
                QString config;
                type = reader.readNext();
                if (type == QXmlStreamReader::Characters) {
                    config = reader.text().toString();
                }
                mVsConfigurations << config;
            }
            break;

        case QXmlStreamReader::EndElement:
            if (reader.name().toString() != CppcheckXml::VSConfigurationName)
                return;
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
    } while (true);
}

void ProjectFile::readPlatform(QXmlStreamReader &reader)
{
    do {
        const QXmlStreamReader::TokenType type = reader.readNext();
        switch (type) {
        case QXmlStreamReader::Characters:
            mPlatform = reader.text().toString();
            FALLTHROUGH;
        case QXmlStreamReader::EndElement:
            return;
        // Not handled
        case QXmlStreamReader::StartElement:
        case QXmlStreamReader::NoToken:
        case QXmlStreamReader::Invalid:
        case QXmlStreamReader::StartDocument:
        case QXmlStreamReader::EndDocument:
        case QXmlStreamReader::Comment:
        case QXmlStreamReader::DTD:
        case QXmlStreamReader::EntityReference:
        case QXmlStreamReader::ProcessingInstruction:
            break;
        }
    } while (true);
}


void ProjectFile::readSuppressions(QXmlStreamReader &reader)
{
    do {
        QXmlStreamReader::TokenType type = reader.readNext();
        switch (type) {
        case QXmlStreamReader::StartElement:
            // Read library-elements
            if (reader.name().toString() == CppcheckXml::SuppressionElementName) {
                SuppressionList::Suppression suppression;
                if (reader.attributes().hasAttribute(QString(),"fileName"))
                    suppression.fileName = reader.attributes().value(QString(),"fileName").toString().toStdString();
                if (reader.attributes().hasAttribute(QString(),"lineNumber"))
                    suppression.lineNumber = reader.attributes().value(QString(),"lineNumber").toInt();
                if (reader.attributes().hasAttribute(QString(),"symbolName"))
                    suppression.symbolName = reader.attributes().value(QString(),"symbolName").toString().toStdString();
                if (reader.attributes().hasAttribute(QString(),"hash"))
                    suppression.hash = reader.attributes().value(QString(),"hash").toULongLong();
                type = reader.readNext();
                if (type == QXmlStreamReader::Characters) {
                    suppression.errorId = reader.text().toString().toStdString();
                }
                mSuppressions << suppression;
            }
            break;

        case QXmlStreamReader::EndElement:
            if (reader.name().toString() != CppcheckXml::SuppressionElementName)
                return;
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
    } while (true);
}


void ProjectFile::readTagWarnings(QXmlStreamReader &reader, const QString &tag)
{
    do {
        QXmlStreamReader::TokenType type = reader.readNext();
        switch (type) {
        case QXmlStreamReader::StartElement:
            // Read library-elements
            if (reader.name().toString() == CppcheckXml::WarningElementName) {
                const std::size_t hash = reader.attributes().value(QString(), CppcheckXml::HashAttributeName).toULongLong();
                mWarningTags[hash] = tag;
            }
            break;

        case QXmlStreamReader::EndElement:
            if (reader.name().toString() != CppcheckXml::WarningElementName)
                return;
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
    } while (true);
}


void ProjectFile::readStringList(QStringList &stringlist, QXmlStreamReader &reader, const char elementname[])
{
    bool allRead = false;
    do {
        QXmlStreamReader::TokenType type = reader.readNext();
        switch (type) {
        case QXmlStreamReader::StartElement:
            // Read library-elements
            if (reader.name().toString() == elementname) {
                type = reader.readNext();
                if (type == QXmlStreamReader::Characters) {
                    QString text = reader.text().toString();
                    stringlist << text;
                }
            }
            break;

        case QXmlStreamReader::EndElement:
            if (reader.name().toString() != elementname)
                allRead = true;
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
    } while (!allRead);
}

void ProjectFile::setIncludes(const QStringList &includes)
{
    mIncludeDirs = includes;
}

void ProjectFile::setDefines(const QStringList &defines)
{
    mDefines = defines;
}

void ProjectFile::setUndefines(const QStringList &undefines)
{
    mUndefines = undefines;
}

void ProjectFile::setCheckPaths(const QStringList &paths)
{
    mPaths = paths;
}

void ProjectFile::setExcludedPaths(const QStringList &paths)
{
    mExcludedPaths = paths;
}

void ProjectFile::setLibraries(const QStringList &libraries)
{
    mLibraries = libraries;
}

void ProjectFile::setPlatform(const QString &platform)
{
    mPlatform = platform;
}

QList<SuppressionList::Suppression> ProjectFile::getCheckingSuppressions() const
{
    const QRegularExpression re1("^[a-zA-Z0-9_\\-]+/.*");
    const QRegularExpression re2("^[^/]+$");
    QList<SuppressionList::Suppression> result;
    for (SuppressionList::Suppression suppression : mSuppressions) {
        if (re1.match(suppression.fileName.c_str()).hasMatch() || re2.match(suppression.fileName.c_str()).hasMatch()) {
            if (suppression.fileName[0] != '*')
                suppression.fileName = QFileInfo(mFilename).absolutePath().toStdString() + "/" + suppression.fileName;
        }
        result << suppression;
    }
    return result;
}

void ProjectFile::setSuppressions(const QList<SuppressionList::Suppression> &suppressions)
{
    mSuppressions = suppressions;
}

void ProjectFile::addSuppression(const SuppressionList::Suppression &suppression)
{
    mSuppressions.append(suppression);
}

void ProjectFile::setAddons(const QStringList &addons)
{
    mAddons = addons;
}

void ProjectFile::setVSConfigurations(const QStringList &vsConfigs)
{
    mVsConfigurations = vsConfigs;
}

void ProjectFile::setCheckLevel(ProjectFile::CheckLevel checkLevel)
{
    mCheckLevel = checkLevel;
}

void ProjectFile::setWarningTags(std::size_t hash, const QString& tags)
{
    if (tags.isEmpty())
        mWarningTags.erase(hash);
    else if (hash > 0)
        mWarningTags[hash] = tags;
}

QString ProjectFile::getWarningTags(std::size_t hash) const
{
    auto it = mWarningTags.find(hash);
    return (it != mWarningTags.end()) ? it->second : QString();
}

bool ProjectFile::write(const QString &filename)
{
    if (!filename.isEmpty())
        mFilename = filename;

    QFile file(mFilename);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text))
        return false;

    QXmlStreamWriter xmlWriter(&file);
    xmlWriter.setAutoFormatting(true);
    xmlWriter.writeStartDocument("1.0");
    xmlWriter.writeStartElement(CppcheckXml::ProjectElementName);
    xmlWriter.writeAttribute(CppcheckXml::ProjectVersionAttrib, CppcheckXml::ProjectFileVersion);

    if (!mRootPath.isEmpty()) {
        xmlWriter.writeStartElement(CppcheckXml::RootPathName);
        xmlWriter.writeAttribute(CppcheckXml::RootPathNameAttrib, mRootPath);
        xmlWriter.writeEndElement();
    }

    if (!mBuildDir.isEmpty()) {
        xmlWriter.writeStartElement(CppcheckXml::BuildDirElementName);
        xmlWriter.writeCharacters(mBuildDir);
        xmlWriter.writeEndElement();
    }

    if (!mPlatform.isEmpty()) {
        xmlWriter.writeStartElement(CppcheckXml::PlatformElementName);
        xmlWriter.writeCharacters(mPlatform);
        xmlWriter.writeEndElement();
    }

    if (!mImportProject.isEmpty()) {
        xmlWriter.writeStartElement(CppcheckXml::ImportProjectElementName);
        xmlWriter.writeCharacters(mImportProject);
        xmlWriter.writeEndElement();
    }

    xmlWriter.writeStartElement(CppcheckXml::AnalyzeAllVsConfigsElementName);
    xmlWriter.writeCharacters(bool_to_string(mAnalyzeAllVsConfigs));
    xmlWriter.writeEndElement();

    if (clangParser) {
        xmlWriter.writeStartElement(CppcheckXml::Parser);
        xmlWriter.writeCharacters("clang");
        xmlWriter.writeEndElement();
    }

    xmlWriter.writeStartElement(CppcheckXml::CheckHeadersElementName);
    xmlWriter.writeCharacters(bool_to_string(mCheckHeaders));
    xmlWriter.writeEndElement();

    xmlWriter.writeStartElement(CppcheckXml::CheckUnusedTemplatesElementName);
    xmlWriter.writeCharacters(bool_to_string(mCheckUnusedTemplates));
    xmlWriter.writeEndElement();

    xmlWriter.writeStartElement(CppcheckXml::InlineSuppression);
    xmlWriter.writeCharacters(bool_to_string(mInlineSuppression));
    xmlWriter.writeEndElement();

    xmlWriter.writeStartElement(CppcheckXml::MaxCtuDepthElementName);
    xmlWriter.writeCharacters(QString::number(mMaxCtuDepth));
    xmlWriter.writeEndElement();

    xmlWriter.writeStartElement(CppcheckXml::MaxTemplateRecursionElementName);
    xmlWriter.writeCharacters(QString::number(mMaxTemplateRecursion));
    xmlWriter.writeEndElement();

    if (!mIncludeDirs.isEmpty()) {
        xmlWriter.writeStartElement(CppcheckXml::IncludeDirElementName);
        for (const QString& incdir : utils::as_const(mIncludeDirs)) {
            xmlWriter.writeStartElement(CppcheckXml::DirElementName);
            xmlWriter.writeAttribute(CppcheckXml::DirNameAttrib, incdir);
            xmlWriter.writeEndElement();
        }
        xmlWriter.writeEndElement();
    }

    if (!mDefines.isEmpty()) {
        xmlWriter.writeStartElement(CppcheckXml::DefinesElementName);
        for (const QString& define : utils::as_const(mDefines)) {
            xmlWriter.writeStartElement(CppcheckXml::DefineName);
            xmlWriter.writeAttribute(CppcheckXml::DefineNameAttrib, define);
            xmlWriter.writeEndElement();
        }
        xmlWriter.writeEndElement();
    }

    if (!mUserInclude.isEmpty()) {
        xmlWriter.writeStartElement(CppcheckXml::UserIncludeElementName);
        xmlWriter.writeCharacters(mUserInclude);
        xmlWriter.writeEndElement();
    }

    if (!mVsConfigurations.isEmpty()) {
        writeStringList(xmlWriter,
                        mVsConfigurations,
                        CppcheckXml::VSConfigurationElementName,
                        CppcheckXml::VSConfigurationName);
    }

    writeStringList(xmlWriter,
                    mUndefines,
                    CppcheckXml::UndefinesElementName,
                    CppcheckXml::UndefineName);

    if (!mPaths.isEmpty()) {
        xmlWriter.writeStartElement(CppcheckXml::PathsElementName);
        for (const QString& path : utils::as_const(mPaths)) {
            xmlWriter.writeStartElement(CppcheckXml::PathName);
            xmlWriter.writeAttribute(CppcheckXml::PathNameAttrib, path);
            xmlWriter.writeEndElement();
        }
        xmlWriter.writeEndElement();
    }

    if (!mExcludedPaths.isEmpty()) {
        xmlWriter.writeStartElement(CppcheckXml::ExcludeElementName);
        for (const QString& path : utils::as_const(mExcludedPaths)) {
            xmlWriter.writeStartElement(CppcheckXml::ExcludePathName);
            xmlWriter.writeAttribute(CppcheckXml::ExcludePathNameAttrib, path);
            xmlWriter.writeEndElement();
        }
        xmlWriter.writeEndElement();
    }

    writeStringList(xmlWriter,
                    mLibraries,
                    CppcheckXml::LibrariesElementName,
                    CppcheckXml::LibraryElementName);

    if (!mSuppressions.isEmpty()) {
        xmlWriter.writeStartElement(CppcheckXml::SuppressionsElementName);
        for (const SuppressionList::Suppression &suppression : utils::as_const(mSuppressions)) {
            xmlWriter.writeStartElement(CppcheckXml::SuppressionElementName);
            if (!suppression.fileName.empty())
                xmlWriter.writeAttribute("fileName", QString::fromStdString(suppression.fileName));
            if (suppression.lineNumber > 0)
                xmlWriter.writeAttribute("lineNumber", QString::number(suppression.lineNumber));
            if (!suppression.symbolName.empty())
                xmlWriter.writeAttribute("symbolName", QString::fromStdString(suppression.symbolName));
            if (suppression.hash > 0)
                xmlWriter.writeAttribute(CppcheckXml::HashAttributeName, QString::number(suppression.hash));
            if (!suppression.errorId.empty())
                xmlWriter.writeCharacters(QString::fromStdString(suppression.errorId));
            xmlWriter.writeEndElement();
        }
        xmlWriter.writeEndElement();
    }

    writeStringList(xmlWriter,
                    mCheckUnknownFunctionReturn,
                    CppcheckXml::CheckUnknownFunctionReturn,
                    CppcheckXml::Name);

    safeChecks.saveToXml(xmlWriter);

    writeStringList(xmlWriter,
                    mAddons,
                    CppcheckXml::AddonsElementName,
                    CppcheckXml::AddonElementName);

    QStringList tools;
    if (mClangAnalyzer)
        tools << CLANG_ANALYZER;
    if (mClangTidy)
        tools << CLANG_TIDY;
    writeStringList(xmlWriter,
                    tools,
                    CppcheckXml::ToolsElementName,
                    CppcheckXml::ToolElementName);

    writeStringList(xmlWriter, mTags, CppcheckXml::TagsElementName, CppcheckXml::TagElementName);
    if (!mWarningTags.empty()) {
        QStringList tags;
        for (const auto& wt: mWarningTags) {
            if (!tags.contains(wt.second))
                tags.append(wt.second);
        }
        for (const QString &tag: tags) {
            xmlWriter.writeStartElement(CppcheckXml::TagWarningsElementName);
            xmlWriter.writeAttribute(CppcheckXml::TagAttributeName, tag);
            for (const auto& wt: mWarningTags) {
                if (wt.second == tag) {
                    xmlWriter.writeStartElement(CppcheckXml::WarningElementName);
                    xmlWriter.writeAttribute(CppcheckXml::HashAttributeName, QString::number(wt.first));
                    xmlWriter.writeEndElement();
                }
            }
            xmlWriter.writeEndElement();
        }
    }

    switch (mCheckLevel) {
    case CheckLevel::reduced:
        xmlWriter.writeStartElement(CppcheckXml::CheckLevelReducedElementName);
        xmlWriter.writeEndElement();
        break;
    case CheckLevel::normal:
        xmlWriter.writeStartElement(CppcheckXml::CheckLevelNormalElementName);
        xmlWriter.writeEndElement();
        break;
    case CheckLevel::exhaustive:
        xmlWriter.writeStartElement(CppcheckXml::CheckLevelExhaustiveElementName);
        xmlWriter.writeEndElement();
        break;
    }

    // Cppcheck Premium
    if (mBughunting) {
        xmlWriter.writeStartElement(CppcheckXml::BughuntingElementName);
        xmlWriter.writeEndElement();
    }

    writeStringList(xmlWriter,
                    mCodingStandards,
                    CppcheckXml::CodingStandardsElementName,
                    CppcheckXml::CodingStandardElementName);

    if (mCertIntPrecision > 0) {
        xmlWriter.writeStartElement(CppcheckXml::CertIntPrecisionElementName);
        xmlWriter.writeCharacters(QString::number(mCertIntPrecision));
        xmlWriter.writeEndElement();
    }

    if (!mProjectName.isEmpty()) {
        xmlWriter.writeStartElement(CppcheckXml::ProjectNameElementName);
        xmlWriter.writeCharacters(mProjectName);
        xmlWriter.writeEndElement();
    }

    if (!mPremiumLicenseFile.isEmpty()) {
        xmlWriter.writeStartElement(CppcheckXml::LicenseFileElementName);
        xmlWriter.writeCharacters(mPremiumLicenseFile);
        xmlWriter.writeEndElement();
    }

    xmlWriter.writeEndDocument();
    file.close();
    return true;
}

void ProjectFile::writeStringList(QXmlStreamWriter &xmlWriter, const QStringList &stringlist, const char startelementname[], const char stringelementname[])
{
    if (stringlist.isEmpty())
        return;

    xmlWriter.writeStartElement(startelementname);
    for (const QString& str : stringlist) {
        xmlWriter.writeStartElement(stringelementname);
        xmlWriter.writeCharacters(str);
        xmlWriter.writeEndElement();
    }
    xmlWriter.writeEndElement();
}

QStringList ProjectFile::fromNativeSeparators(const QStringList &paths)
{
    QStringList ret;
    for (const QString &path : paths)
        ret << QDir::fromNativeSeparators(path);
    return ret;
}

QStringList ProjectFile::getAddonsAndTools() const
{
    QStringList ret(mAddons);
    if (mClangAnalyzer)
        ret << CLANG_ANALYZER;
    if (mClangTidy)
        ret << CLANG_TIDY;
    return ret;
}

void ProjectFile::SafeChecks::loadFromXml(QXmlStreamReader &xmlReader)
{
    classes = externalFunctions = internalFunctions = externalVariables = false;

    int level = 0;

    do {
        const QXmlStreamReader::TokenType type = xmlReader.readNext();
        switch (type) {
        case QXmlStreamReader::StartElement:
            ++level;
            if (xmlReader.name() == QString(Settings::SafeChecks::XmlClasses))
                classes = true;
            else if (xmlReader.name() == QString(Settings::SafeChecks::XmlExternalFunctions))
                externalFunctions = true;
            else if (xmlReader.name() == QString(Settings::SafeChecks::XmlInternalFunctions))
                internalFunctions = true;
            else if (xmlReader.name() == QString(Settings::SafeChecks::XmlExternalVariables))
                externalVariables = true;
            break;
        case QXmlStreamReader::EndElement:
            if (level <= 0)
                return;
            level--;
            break;
        // Not handled
        case QXmlStreamReader::Characters:
        case QXmlStreamReader::NoToken:
        case QXmlStreamReader::Invalid:
        case QXmlStreamReader::StartDocument:
        case QXmlStreamReader::EndDocument:
        case QXmlStreamReader::Comment:
        case QXmlStreamReader::DTD:
        case QXmlStreamReader::EntityReference:
        case QXmlStreamReader::ProcessingInstruction:
            break;
        }
    } while (true);
}

void ProjectFile::SafeChecks::saveToXml(QXmlStreamWriter &xmlWriter) const
{
    if (!classes && !externalFunctions && !internalFunctions && !externalVariables)
        return;
    xmlWriter.writeStartElement(QString(Settings::SafeChecks::XmlRootName));
    if (classes) {
        xmlWriter.writeStartElement(QString(Settings::SafeChecks::XmlClasses));
        xmlWriter.writeEndElement();
    }
    if (externalFunctions) {
        xmlWriter.writeStartElement(QString(Settings::SafeChecks::XmlExternalFunctions));
        xmlWriter.writeEndElement();
    }
    if (internalFunctions) {
        xmlWriter.writeStartElement(QString(Settings::SafeChecks::XmlInternalFunctions));
        xmlWriter.writeEndElement();
    }
    if (externalVariables) {
        xmlWriter.writeStartElement(QString(Settings::SafeChecks::XmlExternalVariables));
        xmlWriter.writeEndElement();
    }
    xmlWriter.writeEndElement();
}

QString ProjectFile::getAddonFilePath(QString filesDir, const QString &addon)
{
    if (QFile(addon).exists())
        return addon;

    if (!filesDir.endsWith("/"))
        filesDir += "/";

    QStringList searchPaths;
    searchPaths << filesDir << (filesDir + "addons/") << (filesDir + "../addons/")
#ifdef FILESDIR
        << (QLatin1String(FILESDIR) + "/addons/")
#endif
    ;

    for (const QString& path : utils::as_const(searchPaths)) {
        QString f = path + addon + ".py";
        if (QFile(f).exists())
            return f;
    }

    return QString();
}

QStringList ProjectFile::getSearchPaths(const QString& projectPath, const QString& appPath, const QString& datadir, const QString& dir) {
    QStringList ret;
    ret << appPath << (appPath + "/" + dir) << projectPath;
#ifdef FILESDIR
    if (FILESDIR[0])
        ret << FILESDIR << (FILESDIR "/" + dir);
#endif
    if (!datadir.isEmpty())
        ret << datadir << (datadir + "/" + dir);
    return ret;
}

QStringList ProjectFile::getSearchPaths(const QString& dir) const {
    const QFileInfo inf(mFilename);
    const QString applicationFilePath = QCoreApplication::applicationFilePath();
    const QString appPath = QFileInfo(applicationFilePath).canonicalPath();
    const QString datadir = getDataDir();
    return getSearchPaths(inf.canonicalFilePath(), appPath, datadir, dir);
}
