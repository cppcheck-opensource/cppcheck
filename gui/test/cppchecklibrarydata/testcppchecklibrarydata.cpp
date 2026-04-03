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

#include "testcppchecklibrarydata.h"

#include <QDir>
#include <QDebug>
#include <QFile>
#include <QIODevice>
#include <QList>
#include <QStringList>
#include <QTextStream>
#include <QtTest>
#if (QT_VERSION >= QT_VERSION_CHECK(6, 5, 0))
#include <QtLogging>
#endif

const QString TestCppcheckLibraryData::TempCfgFile = "./tmp.cfg";

void TestCppcheckLibraryData::init()
{
    result.clear();
    libraryData.clear();
    fileLibraryData.clear();
}

void TestCppcheckLibraryData::xmlReaderError()
{
    loadCfgFile(":/files/xml_reader_error.cfg", fileLibraryData, result);
    QCOMPARE(result.isNull(), false);
    qDebug() << result;
}

void TestCppcheckLibraryData::unhandledElement()
{
    loadCfgFile(":/files/unhandled_element.cfg", fileLibraryData, result);
    QCOMPARE(result.isNull(), false);
    qDebug() << result;

    loadCfgFile(":/files/platform_type_unhandled_element.cfg", fileLibraryData, result);
    QCOMPARE(result.isNull(), false);
    qDebug() << result;

    loadCfgFile(":/files/memory_resource_unhandled_element.cfg", fileLibraryData, result);
    QCOMPARE(result.isNull(), false);
    qDebug() << result;

    loadCfgFile(":/files/container_unhandled_element.cfg", fileLibraryData, result);
    QCOMPARE(result.isNull(), false);
    qDebug() << result;

    loadCfgFile(":/files/reflection_unhandled_element.cfg", fileLibraryData, result);
    QCOMPARE(result.isNull(), false);
    qDebug() << result;

    loadCfgFile(":/files/markup_unhandled_element.cfg", fileLibraryData, result);
    QCOMPARE(result.isNull(), false);
    qDebug() << result;
}

void TestCppcheckLibraryData::mandatoryAttributeMissing()
{
    loadCfgFile(":/files/mandatory_attribute_missing.cfg", fileLibraryData, result);
    QCOMPARE(result.isNull(), false);
    qDebug() << result;

    loadCfgFile(":/files/reflection_mandatory_attribute_missing.cfg", fileLibraryData, result);
    QCOMPARE(result.isNull(), false);
    qDebug() << result;

    loadCfgFile(":/files/markup_mandatory_attribute_missing.cfg", fileLibraryData, result);
    QCOMPARE(result.isNull(), false);
    qDebug() << result;
}

void TestCppcheckLibraryData::podtypeValid()
{
    // Load library data from file
    loadCfgFile(":/files/podtype_valid.cfg", fileLibraryData, result);
    QCOMPARE(result.isNull(), true);

    // Swap library data read from file to other object
    libraryData.swap(fileLibraryData);

    // Do size and content checks against swapped data.
    QCOMPARE(libraryData.podtypes.size(), 2);

    QCOMPARE(libraryData.podtypes[0].name, QString("bool"));
    QCOMPARE(libraryData.podtypes[0].stdtype.isEmpty(), true);
    QCOMPARE(libraryData.podtypes[0].sign.isEmpty(), true);
    QCOMPARE(libraryData.podtypes[0].size.isEmpty(), true);

    QCOMPARE(libraryData.podtypes[1].name, QString("ulong"));
    QCOMPARE(libraryData.podtypes[1].stdtype, QString("uint32_t"));
    QCOMPARE(libraryData.podtypes[1].sign, QString("u"));
    QCOMPARE(libraryData.podtypes[1].size, QString("4"));

    // Save library data to file
    saveCfgFile(TempCfgFile, libraryData);

    fileLibraryData.clear();
    QCOMPARE(fileLibraryData.podtypes.size(), 0);

    // Reload library data from file
    loadCfgFile(TempCfgFile, fileLibraryData, result, true);
    QCOMPARE(result.isNull(), true);

    // Verify no data got lost or modified
    QCOMPARE(libraryData.podtypes.size(), fileLibraryData.podtypes.size());
    QCOMPARE(libraryData.podtypes.size(), 2);
    for (int i=0; i < libraryData.podtypes.size(); i++) {
        QCOMPARE(libraryData.podtypes[i].name, fileLibraryData.podtypes[i].name);
        QCOMPARE(libraryData.podtypes[i].stdtype, fileLibraryData.podtypes[i].stdtype);
        QCOMPARE(libraryData.podtypes[i].sign, fileLibraryData.podtypes[i].sign);
        QCOMPARE(libraryData.podtypes[i].size, fileLibraryData.podtypes[i].size);
    }
}

void TestCppcheckLibraryData::typechecksValid()
{
    // Load library data from file
    loadCfgFile(":/files/typechecks_valid.cfg", fileLibraryData, result);
    QCOMPARE(result.isNull(), true);

    // Swap library data read from file to other object
    libraryData.swap(fileLibraryData);

    // Do size and content checks against swapped data.
    QCOMPARE(libraryData.typeChecks.size(), 3);

    CppcheckLibraryData::TypeChecks check = libraryData.typeChecks[0];
    QCOMPARE(check.size(), 2);
    QCOMPARE(check[0].first, QString("suppress"));
    QCOMPARE(check[0].second, QString("std::insert_iterator"));
    QCOMPARE(check[1].first, QString("check"));
    QCOMPARE(check[1].second, QString("std::pair"));

    check = libraryData.typeChecks[1];
    QCOMPARE(check.isEmpty(), true);

    check = libraryData.typeChecks[2];
    QCOMPARE(check.size(), 1);
    QCOMPARE(check[0].first, QString("check"));
    QCOMPARE(check[0].second, QString("std::tuple"));

    // Save library data to file
    saveCfgFile(TempCfgFile, libraryData);

    fileLibraryData.clear();
    QCOMPARE(fileLibraryData.typeChecks.size(), 0);

    // Reload library data from file
    loadCfgFile(TempCfgFile, fileLibraryData, result, true);
    QCOMPARE(result.isNull(), true);

    // Verify no data got lost or modified
    QCOMPARE(libraryData.typeChecks.size(), fileLibraryData.typeChecks.size());
    QCOMPARE(libraryData.typeChecks.size(), 3);
    for (int idx=0; idx < libraryData.typeChecks.size(); idx++) {
        CppcheckLibraryData::TypeChecks lhs = libraryData.typeChecks[idx];
        CppcheckLibraryData::TypeChecks rhs = fileLibraryData.typeChecks[idx];
        QCOMPARE(lhs.size(), rhs.size());
        QCOMPARE(lhs, rhs);
    }
}

void TestCppcheckLibraryData::smartPointerValid()
{
    // Load library data from file
    loadCfgFile(":/files/smartptr_valid.cfg", fileLibraryData, result);
    QCOMPARE(result.isNull(), true);

    // Swap library data read from file to other object
    libraryData.swap(fileLibraryData);

    // Do size and content checks against swapped data.
    QCOMPARE(libraryData.smartPointers.size(), 3);

    QCOMPARE(libraryData.smartPointers[0].name, QString("wxObjectDataPtr"));
    QCOMPARE(libraryData.smartPointers[0].unique, false);
    QCOMPARE(libraryData.smartPointers[1].name, QString("wxScopedArray"));
    QCOMPARE(libraryData.smartPointers[1].unique, true);
    QCOMPARE(libraryData.smartPointers[2].name, QString("wxScopedPtr"));
    QCOMPARE(libraryData.smartPointers[2].unique, false);

    // Save library data to file
    saveCfgFile(TempCfgFile, libraryData);

    fileLibraryData.clear();
    QCOMPARE(fileLibraryData.smartPointers.size(), 0);

    // Reload library data from file
    loadCfgFile(TempCfgFile, fileLibraryData, result, true);
    QCOMPARE(result.isNull(), true);

    // Verify no data got lost or modified
    QCOMPARE(libraryData.smartPointers.size(), fileLibraryData.smartPointers.size());
    QCOMPARE(libraryData.smartPointers.size(), 3);
    for (int idx=0; idx < libraryData.smartPointers.size(); idx++) {
        QCOMPARE(libraryData.smartPointers[idx].name, fileLibraryData.smartPointers[idx].name);
        QCOMPARE(libraryData.smartPointers[idx].unique, fileLibraryData.smartPointers[idx].unique);
    }
}

void TestCppcheckLibraryData::platformTypeValid()
{
    // Load library data from file
    loadCfgFile(":/files/platform_type_valid.cfg", fileLibraryData, result);
    QCOMPARE(result.isNull(), true);

    // Swap library data read from file to other object
    libraryData.swap(fileLibraryData);

    // Do size and content checks against swapped data.
    QCOMPARE(libraryData.platformTypes.size(), 3);

    QCOMPARE(libraryData.platformTypes[0].name, QString("platform"));
    QCOMPARE(libraryData.platformTypes[0].value, QString("with attribute and empty"));
    QCOMPARE(libraryData.platformTypes[0].types.size(), 0);
    QCOMPARE(libraryData.platformTypes[0].platforms.size(), 2);
    QCOMPARE(libraryData.platformTypes[0].platforms[0], QString("win64"));
    QCOMPARE(libraryData.platformTypes[0].platforms[1].isEmpty(), true);

    QCOMPARE(libraryData.platformTypes[1].name, QString("types"));
    QCOMPARE(libraryData.platformTypes[1].value, QString("all"));
    QCOMPARE(libraryData.platformTypes[1].types.size(), 5);
    QCOMPARE(libraryData.platformTypes[1].types,
             QStringList({"unsigned", "long", "pointer", "const_ptr", "ptr_ptr"}));
    QCOMPARE(libraryData.platformTypes[1].platforms.isEmpty(), true);

    QCOMPARE(libraryData.platformTypes[2].name, QString("types and platform"));
    QCOMPARE(libraryData.platformTypes[2].value.isEmpty(), true);
    QCOMPARE(libraryData.platformTypes[2].types.size(), 2);
    QCOMPARE(libraryData.platformTypes[2].types, QStringList({"pointer", "ptr_ptr"}));
    QCOMPARE(libraryData.platformTypes[2].platforms.size(), 1);
    QCOMPARE(libraryData.platformTypes[2].platforms[0], QString("win32"));

    // Save library data to file
    saveCfgFile(TempCfgFile, libraryData);

    fileLibraryData.clear();
    QCOMPARE(fileLibraryData.platformTypes.size(), 0);

    // Reload library data from file
    loadCfgFile(TempCfgFile, fileLibraryData, result, true);
    QCOMPARE(result.isNull(), true);

    // Verify no data got lost or modified
    QCOMPARE(libraryData.platformTypes.size(), fileLibraryData.platformTypes.size());
    QCOMPARE(libraryData.platformTypes.size(), 3);
    for (int idx=0; idx < libraryData.platformTypes.size(); idx++) {
        CppcheckLibraryData::PlatformType lhs = libraryData.platformTypes[idx];
        CppcheckLibraryData::PlatformType rhs = fileLibraryData.platformTypes[idx];
        QCOMPARE(lhs.name, rhs.name);
        QCOMPARE(lhs.value, rhs.value);
        QCOMPARE(lhs.types.size(), rhs.types.size());
        QCOMPARE(lhs.types, rhs.types);
        QCOMPARE(lhs.platforms.size(), rhs.platforms.size());
        QCOMPARE(lhs.platforms, rhs.platforms);
    }
}

void TestCppcheckLibraryData::memoryResourceValid()
{
    // Load library data from file
    loadCfgFile(":/files/memory_resource_valid.cfg", fileLibraryData, result);
    QCOMPARE(result.isNull(), true);

    // Swap library data read from file to other object
    libraryData.swap(fileLibraryData);

    // Do size and content checks against swapped data.
    QCOMPARE(libraryData.memoryresource.size(), 2);
    QCOMPARE(libraryData.memoryresource[0].type, QString("memory"));
    QCOMPARE(libraryData.memoryresource[0].alloc.size(), 5);
    QCOMPARE(libraryData.memoryresource[0].dealloc.size(), 1);
    QCOMPARE(libraryData.memoryresource[0].use.size(), 0);

    QCOMPARE(libraryData.memoryresource[0].alloc[0].name, QString("malloc"));
    QCOMPARE(libraryData.memoryresource[0].alloc[0].bufferSize, QString("malloc"));
    QCOMPARE(libraryData.memoryresource[0].alloc[0].isRealloc, false);
    QCOMPARE(libraryData.memoryresource[0].alloc[0].init, false);
    QCOMPARE(libraryData.memoryresource[0].alloc[0].noFail, false);
    QCOMPARE(libraryData.memoryresource[0].alloc[0].arg, -1);
    QCOMPARE(libraryData.memoryresource[0].alloc[0].reallocArg, -1);

    QCOMPARE(libraryData.memoryresource[0].alloc[1].name, QString("calloc"));
    QCOMPARE(libraryData.memoryresource[0].alloc[1].bufferSize, QString("calloc"));
    QCOMPARE(libraryData.memoryresource[0].alloc[1].isRealloc, false);
    QCOMPARE(libraryData.memoryresource[0].alloc[1].init, true);
    QCOMPARE(libraryData.memoryresource[0].alloc[1].noFail, false);
    QCOMPARE(libraryData.memoryresource[0].alloc[1].arg, -1);
    QCOMPARE(libraryData.memoryresource[0].alloc[1].reallocArg, -1);

    QCOMPARE(libraryData.memoryresource[0].alloc[2].name, QString("realloc"));
    QCOMPARE(libraryData.memoryresource[0].alloc[2].bufferSize, QString("malloc:2"));
    QCOMPARE(libraryData.memoryresource[0].alloc[2].isRealloc, true);
    QCOMPARE(libraryData.memoryresource[0].alloc[2].init, false);
    QCOMPARE(libraryData.memoryresource[0].alloc[2].noFail, false);
    QCOMPARE(libraryData.memoryresource[0].alloc[2].arg, -1);
    QCOMPARE(libraryData.memoryresource[0].alloc[2].reallocArg, -1);

    QCOMPARE(libraryData.memoryresource[0].alloc[3].name, QString("UuidToString"));
    QCOMPARE(libraryData.memoryresource[0].alloc[3].bufferSize.isEmpty(), true);
    QCOMPARE(libraryData.memoryresource[0].alloc[3].isRealloc, false);
    QCOMPARE(libraryData.memoryresource[0].alloc[3].init, false);
    QCOMPARE(libraryData.memoryresource[0].alloc[3].noFail, false);
    QCOMPARE(libraryData.memoryresource[0].alloc[3].arg, 2);
    QCOMPARE(libraryData.memoryresource[0].alloc[3].reallocArg, -1);

    QCOMPARE(libraryData.memoryresource[0].alloc[4].name, QString("g_malloc"));
    QCOMPARE(libraryData.memoryresource[0].alloc[4].bufferSize, QString("malloc"));
    QCOMPARE(libraryData.memoryresource[0].alloc[4].isRealloc, false);
    QCOMPARE(libraryData.memoryresource[0].alloc[4].init, false);
    QCOMPARE(libraryData.memoryresource[0].alloc[4].noFail, true);
    QCOMPARE(libraryData.memoryresource[0].alloc[4].arg, -1);
    QCOMPARE(libraryData.memoryresource[0].alloc[4].reallocArg, -1);

    QCOMPARE(libraryData.memoryresource[0].dealloc[0].name, QString("HeapFree"));
    QCOMPARE(libraryData.memoryresource[0].dealloc[0].arg, 3);

    QCOMPARE(libraryData.memoryresource[1].type, QString("resource"));
    QCOMPARE(libraryData.memoryresource[1].alloc.size(), 1);
    QCOMPARE(libraryData.memoryresource[1].dealloc.size(), 1);
    QCOMPARE(libraryData.memoryresource[1].use.size(), 0);

    QCOMPARE(libraryData.memoryresource[1].alloc[0].name, QString("_wfopen_s"));
    QCOMPARE(libraryData.memoryresource[1].alloc[0].bufferSize.isEmpty(), true);
    QCOMPARE(libraryData.memoryresource[1].alloc[0].isRealloc, false);
    QCOMPARE(libraryData.memoryresource[1].alloc[0].init, true);
    QCOMPARE(libraryData.memoryresource[1].alloc[0].noFail, false);
    QCOMPARE(libraryData.memoryresource[1].alloc[0].arg, 1);
    QCOMPARE(libraryData.memoryresource[1].alloc[0].reallocArg, -1);

    QCOMPARE(libraryData.memoryresource[1].dealloc[0].name, QString("fclose"));
    QCOMPARE(libraryData.memoryresource[1].dealloc[0].arg, -1);

    // Save library data to file
    saveCfgFile(TempCfgFile, libraryData);

    fileLibraryData.clear();
    QCOMPARE(fileLibraryData.memoryresource.size(), 0);

    // Reload library data from file
    loadCfgFile(TempCfgFile, fileLibraryData, result, true);
    QCOMPARE(result.isNull(), true);

    // Verify no data got lost or modified
    QCOMPARE(libraryData.memoryresource.size(), fileLibraryData.memoryresource.size());
    QCOMPARE(libraryData.memoryresource.size(), 2);

    for (int idx=0; idx < libraryData.memoryresource.size(); idx++) {
        CppcheckLibraryData::MemoryResource lhs = libraryData.memoryresource[idx];
        CppcheckLibraryData::MemoryResource rhs = fileLibraryData.memoryresource[idx];

        QCOMPARE(lhs.type, rhs.type);
        QCOMPARE(lhs.alloc.size(), rhs.alloc.size());
        QCOMPARE(lhs.dealloc.size(), rhs.dealloc.size());
        QCOMPARE(lhs.use, rhs.use);

        for (int num=0; num < lhs.alloc.size(); num++) {
            QCOMPARE(lhs.alloc[num].name, rhs.alloc[num].name);
            QCOMPARE(lhs.alloc[num].bufferSize, rhs.alloc[num].bufferSize);
            QCOMPARE(lhs.alloc[num].isRealloc, rhs.alloc[num].isRealloc);
            QCOMPARE(lhs.alloc[num].init, rhs.alloc[num].init);
            QCOMPARE(lhs.alloc[num].noFail, rhs.alloc[num].noFail);
            QCOMPARE(lhs.alloc[num].arg, rhs.alloc[num].arg);
            QCOMPARE(lhs.alloc[num].reallocArg, rhs.alloc[num].reallocArg);
        }
        for (int num=0; num < lhs.dealloc.size(); num++) {
            QCOMPARE(lhs.dealloc[num].name, rhs.dealloc[num].name);
            QCOMPARE(lhs.dealloc[num].arg, rhs.dealloc[num].arg);
        }
    }
}

void TestCppcheckLibraryData::defineValid()
{
    // Load library data from file
    loadCfgFile(":/files/define_valid.cfg", fileLibraryData, result);
    QCOMPARE(result.isNull(), true);

    // Swap library data read from file to other object
    libraryData.swap(fileLibraryData);

    // Do size and content checks against swapped data.
    QCOMPARE(libraryData.defines.size(), 2);
    QCOMPARE(libraryData.defines[0].name, QString("INT8_MIN"));
    QCOMPARE(libraryData.defines[0].value, QString("-128"));
    QCOMPARE(libraryData.defines[1].name.isEmpty(), true);
    QCOMPARE(libraryData.defines[1].value.isEmpty(), true);

    // Save library data to file
    saveCfgFile(TempCfgFile, libraryData);

    fileLibraryData.clear();
    QCOMPARE(fileLibraryData.defines.size(), 0);

    // Reload library data from file
    loadCfgFile(TempCfgFile, fileLibraryData, result, true);
    QCOMPARE(result.isNull(), true);

    // Verify no data got lost or modified
    QCOMPARE(libraryData.defines.size(), fileLibraryData.defines.size());
    QCOMPARE(libraryData.defines.size(), 2);
    for (int idx=0; idx < libraryData.defines.size(); idx++) {
        QCOMPARE(libraryData.defines[idx].name, fileLibraryData.defines[idx].name);
        QCOMPARE(libraryData.defines[idx].value, fileLibraryData.defines[idx].value);
    }
}

void TestCppcheckLibraryData::undefineValid()
{
    // Load library data from file
    loadCfgFile(":/files/undefine_valid.cfg", fileLibraryData, result);
    QCOMPARE(result.isNull(), true);

    // Swap library data read from file to other object
    libraryData.swap(fileLibraryData);

    // Do size and content checks against swapped data.
    QCOMPARE(libraryData.undefines.size(), 2);
    QCOMPARE(libraryData.undefines[0], QString("INT8_MIN"));
    QCOMPARE(libraryData.undefines[1].isEmpty(), true);

    // Save library data to file
    saveCfgFile(TempCfgFile, libraryData);

    fileLibraryData.clear();
    QCOMPARE(fileLibraryData.undefines.size(), 0);

    // Reload library data from file
    loadCfgFile(TempCfgFile, fileLibraryData, result, true);
    QCOMPARE(result.isNull(), true);

    // Verify no data got lost or modified
    QCOMPARE(libraryData.undefines.size(), fileLibraryData.undefines.size());
    QCOMPARE(libraryData.undefines.size(), 2);
    QCOMPARE(libraryData.undefines, fileLibraryData.undefines);
}

void TestCppcheckLibraryData::reflectionValid()
{
    // Load library data from file
    loadCfgFile(":/files/reflection_valid.cfg", fileLibraryData, result);
    QCOMPARE(result.isNull(), true);

    // Swap library data read from file to other object
    libraryData.swap(fileLibraryData);

    // Do size and content checks against swapped data.
    QCOMPARE(libraryData.reflections.size(), 2);
    QCOMPARE(libraryData.reflections[0].calls.size(), 2);
    QCOMPARE(libraryData.reflections[0].calls[0].arg, 2);
    QCOMPARE(libraryData.reflections[0].calls[0].name, QString("invokeMethod"));
    QCOMPARE(libraryData.reflections[0].calls[1].arg, 1);
    QCOMPARE(libraryData.reflections[0].calls[1].name, QString("callFunction"));
    QCOMPARE(libraryData.reflections[1].calls.isEmpty(), true);

    // Save library data to file
    saveCfgFile(TempCfgFile, libraryData);

    fileLibraryData.clear();
    QCOMPARE(fileLibraryData.reflections.size(), 0);

    // Reload library data from file
    loadCfgFile(TempCfgFile, fileLibraryData, result, true);
    QCOMPARE(result.isNull(), true);

    // Verify no data got lost or modified
    QCOMPARE(libraryData.reflections.size(), fileLibraryData.reflections.size());
    QCOMPARE(libraryData.reflections.size(), 2);
    for (int idx=0; idx < libraryData.reflections.size(); idx++) {
        CppcheckLibraryData::Reflection lhs = libraryData.reflections[idx];
        CppcheckLibraryData::Reflection rhs = fileLibraryData.reflections[idx];

        QCOMPARE(lhs.calls.size(), rhs.calls.size());
        for (int num=0; num < lhs.calls.size(); num++) {
            QCOMPARE(lhs.calls[num].arg, rhs.calls[num].arg);
            QCOMPARE(lhs.calls[num].name, rhs.calls[num].name);
        }
    }
}

void TestCppcheckLibraryData::markupValid()
{
    // Load library data from file
    loadCfgFile(":/files/markup_valid.cfg", fileLibraryData, result);
    QCOMPARE(result.isNull(), true);

    // Swap library data read from file to other object
    libraryData.swap(fileLibraryData);

    // Do size and content checks against swapped data.
    QCOMPARE(libraryData.markups.size(), 1);
    QCOMPARE(libraryData.markups[0].ext, QString(".qml"));
    QCOMPARE(libraryData.markups[0].reportErrors, false);
    QCOMPARE(libraryData.markups[0].afterCode, true);

    QCOMPARE(libraryData.markups[0].keywords.size(), 4);
    QCOMPARE(libraryData.markups[0].keywords, QStringList({"if", "while", "typeof", "for"}));

    QCOMPARE(libraryData.markups[0].importer.size(), 1);
    QCOMPARE(libraryData.markups[0].importer, QStringList("connect"));

    QCOMPARE(libraryData.markups[0].exporter.size(), 1);
    QCOMPARE(libraryData.markups[0].exporter[0].prefix, QString("Q_PROPERTY"));
    QCOMPARE(libraryData.markups[0].exporter[0].suffixList.size(), 1);
    QCOMPARE(libraryData.markups[0].exporter[0].suffixList, QStringList("READ"));
    QCOMPARE(libraryData.markups[0].exporter[0].prefixList.size(), 3);
    QCOMPARE(libraryData.markups[0].exporter[0].prefixList, QStringList({"READ", "WRITE", "NOTIFY"}));

    QCOMPARE(libraryData.markups[0].codeBlocks.size(), 2);
    QCOMPARE(libraryData.markups[0].codeBlocks[0].blocks.size(), 5);
    QCOMPARE(libraryData.markups[0].codeBlocks[0].blocks, QStringList({"onClicked", "onFinished", "onTriggered", "onPressed", "onTouch"}));
    QCOMPARE(libraryData.markups[0].codeBlocks[0].offset, 3);
    QCOMPARE(libraryData.markups[0].codeBlocks[0].start, QString("{"));
    QCOMPARE(libraryData.markups[0].codeBlocks[0].end, QString("}"));
    QCOMPARE(libraryData.markups[0].codeBlocks[1].blocks.size(), 1);
    QCOMPARE(libraryData.markups[0].codeBlocks[1].blocks, QStringList("function"));
    QCOMPARE(libraryData.markups[0].codeBlocks[1].offset, 2);
    QCOMPARE(libraryData.markups[0].codeBlocks[1].start, QString("{"));
    QCOMPARE(libraryData.markups[0].codeBlocks[1].end, QString("}"));

    // Save library data to file
    saveCfgFile(TempCfgFile, libraryData);

    fileLibraryData.clear();
    QCOMPARE(fileLibraryData.markups.size(), 0);

    // Reload library data from file
    loadCfgFile(TempCfgFile, fileLibraryData, result, true);
    QCOMPARE(result.isNull(), true);

    // Verify no data got lost or modified
    QCOMPARE(libraryData.markups.size(), fileLibraryData.markups.size());
    for (int idx=0; idx < libraryData.markups.size(); idx++) {
        CppcheckLibraryData::Markup lhs = libraryData.markups[idx];
        CppcheckLibraryData::Markup rhs = fileLibraryData.markups[idx];

        QCOMPARE(lhs.ext, rhs.ext);
        QCOMPARE(lhs.reportErrors, rhs.reportErrors);
        QCOMPARE(lhs.afterCode, rhs.afterCode);
        QCOMPARE(lhs.keywords, rhs.keywords);
        QCOMPARE(lhs.importer, rhs.importer);
        for (int num=0; num < lhs.exporter.size(); num++) {
            QCOMPARE(lhs.exporter[num].prefix, rhs.exporter[num].prefix);
            QCOMPARE(lhs.exporter[num].suffixList, rhs.exporter[num].suffixList);
            QCOMPARE(lhs.exporter[num].prefixList, rhs.exporter[num].prefixList);
        }
        for (int num=0; num < lhs.codeBlocks.size(); num++) {
            QCOMPARE(lhs.codeBlocks[num].blocks, rhs.codeBlocks[num].blocks);
            QCOMPARE(lhs.codeBlocks[num].offset, rhs.codeBlocks[num].offset);
            QCOMPARE(lhs.codeBlocks[num].start, rhs.codeBlocks[num].start);
            QCOMPARE(lhs.codeBlocks[num].end, rhs.codeBlocks[num].end);
        }
    }
}

void TestCppcheckLibraryData::containerValid()
{
    // Load library data from file
    loadCfgFile(":/files/container_valid.cfg", fileLibraryData, result);
    QCOMPARE(result.isNull(), true);

    // Swap library data read from file to other object
    libraryData.swap(fileLibraryData);

    // Do size and content checks against swapped data.
    QCOMPARE(libraryData.containers.size(), 1);

    QCOMPARE(libraryData.containers[0].rangeItemRecordTypeList.size(), 2);
    QCOMPARE(libraryData.containers[0].rangeItemRecordTypeList[0].name, QString("first"));
    QCOMPARE(libraryData.containers[0].rangeItemRecordTypeList[0].templateParameter, QString("0"));
    QCOMPARE(libraryData.containers[0].rangeItemRecordTypeList[1].name, QString("second"));
    QCOMPARE(libraryData.containers[0].rangeItemRecordTypeList[1].templateParameter, QString("1"));

    // Save library data to file
    saveCfgFile(TempCfgFile, libraryData);

    fileLibraryData.clear();
    QCOMPARE(fileLibraryData.containers.size(), 0);

    // Reload library data from file
    loadCfgFile(TempCfgFile, fileLibraryData, result, true);
    QCOMPARE(result.isNull(), true);

    // Verify no data got lost or modified
    QCOMPARE(libraryData.containers.size(), fileLibraryData.containers.size());
    for (int idx=0; idx < libraryData.containers.size(); idx++) {
        CppcheckLibraryData::Container lhs = libraryData.containers[idx];
        CppcheckLibraryData::Container rhs = fileLibraryData.containers[idx];
        for (int num=0; num < lhs.rangeItemRecordTypeList.size(); num++) {
            QCOMPARE(lhs.rangeItemRecordTypeList[num].name, rhs.rangeItemRecordTypeList[num].name);
            QCOMPARE(lhs.rangeItemRecordTypeList[num].templateParameter, rhs.rangeItemRecordTypeList[num].templateParameter);
        }
    }
}

void TestCppcheckLibraryData::loadCfgFile(const QString &filename, CppcheckLibraryData &data, QString &res, bool removeFile)
{
    QFile file(filename);
    QVERIFY(file.open(QIODevice::ReadOnly | QIODevice::Text));
    res = data.open(file);
    file.close();
    if (removeFile) {
        file.remove();
    }
}

void TestCppcheckLibraryData::saveCfgFile(const QString &filename, const CppcheckLibraryData &data)
{
    QFile file(filename);
    QVERIFY(file.open(QIODevice::WriteOnly | QIODevice::Text));
    QTextStream textStream(&file);
    textStream << data.toString() << '\n';
    file.close();
}

void TestCppcheckLibraryData::validateAllCfg()
{
    const QDir dir(QString(SRCDIR) + "/../../../cfg/");
    const QStringList files = dir.entryList(QStringList() << "*.cfg",QDir::Files);
    QVERIFY(!files.empty());
    bool error = false;
    for (const QString& f : files) {
        loadCfgFile(dir.absolutePath() + "/" + f, fileLibraryData, result);
        if (!result.isNull()) {
            error = true;
            qDebug() << f << " - " << result;
        }
    }
    QCOMPARE(error, false);
}

QTEST_MAIN(TestCppcheckLibraryData)
