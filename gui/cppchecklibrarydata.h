/* -*- C++ -*-
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

#ifndef CPPCHECKLIBRARYDATA_H
#define CPPCHECKLIBRARYDATA_H

#include <cstdint>
#include <utility>

#include <QList>
#include <QMap>
#include <QString>
#include <QStringList>

class QIODevice;

class CppcheckLibraryData {
public:
    CppcheckLibraryData() = default;

    struct Container {
        QString id;
        QString inherits;
        QString startPattern;
        QString endPattern;
        QString opLessAllowed;
        QString itEndPattern;

        bool access_arrayLike{};
        int size_templateParameter = -1;

        struct {
            QString templateParameter;
            QString string;
        } type;

        struct RangeItemRecordType {
            QString name;
            QString templateParameter;
        };

        struct Function {
            QString name;
            QString yields;
            QString action;
        };
        QList<Function> accessFunctions;
        QList<Function> otherFunctions;
        QList<Function> sizeFunctions;
        QList<RangeItemRecordType> rangeItemRecordTypeList;
    };

    struct Define {
        QString name;
        QString value;
    };

    struct Function {
        QString comments;
        QString name;
        enum TrueFalseUnknown : std::uint8_t { False, True, Unknown } noreturn = Unknown;
        bool gccPure{};
        bool gccConst{};
        bool leakignore{};
        bool useretval{};
        struct ReturnValue {
            QString type;
            QString value;
            int container = -1;
            bool empty() const {
                return type.isNull() && value.isNull() && container < 0;
            }
        } returnValue;
        struct {
            QString scan;
            QString secure;
        } formatstr;
        struct Arg {
            QString name;
            unsigned int nr{};
            static const unsigned int ANY;
            static const unsigned int VARIADIC;
            QString defaultValue;
            bool notbool{};
            bool notnull{};
            bool notuninit{};
            bool formatstr{};
            bool strz{};
            QString valid;
            struct MinSize {
                QString type;
                QString arg;
                QString arg2;
            };
            QList<MinSize> minsizes;
            struct Iterator {
                int container = -1;
                QString type;
            } iterator;
        };
        QList<Arg> args;

        struct {
            QString severity;
            QString cstd;
            QString reason;
            QString alternatives;
            QString msg;

            bool isEmpty() const {
                return cstd.isEmpty() &&
                       severity.isEmpty() &&
                       reason.isEmpty() &&
                       alternatives.isEmpty() &&
                       msg.isEmpty();
            }
        } warn;

        QMap<QString, QString> notOverlappingDataArgs;
        QMap<QString, QString> containerAttributes;
    };

    struct MemoryResource {
        QString type; // "memory" or "resource"
        struct Alloc {
            bool isRealloc{};
            bool init{};
            bool noFail{};
            int arg = -1; // -1: Has no optional "realloc-arg" attribute
            int reallocArg = -1; // -1: Has no optional "arg" attribute
            QString bufferSize;
            QString name;
        };
        struct Dealloc {
            int arg = -1; // -1: Has no optional "arg" attribute
            QString name;
        };

        QList<Alloc> alloc;
        QList<Dealloc> dealloc;
        QStringList use;
    };

    struct PodType {
        QString name;
        QString stdtype;
        QString size;
        QString sign;
    };

    struct PlatformType {
        QString name;
        QString value;
        QStringList types;      // Keeps element names w/o attribute (e.g. unsigned)
        QStringList platforms;  // Keeps "type" attribute of each "platform" element
    };

    using TypeChecks = QList<std::pair<QString, QString>>;

    struct Reflection {
        struct Call {
            int arg = -1; // -1: Mandatory "arg" attribute not available
            QString name;
        };

        QList<Call> calls;
    };

    struct Markup {
        struct CodeBlocks {
            QStringList blocks;
            int offset = -1;
            QString start;
            QString end;
        };

        struct Exporter {
            QString prefix;
            QStringList prefixList;
            QStringList suffixList;
        };

        QString ext;
        bool afterCode{};
        bool reportErrors{};
        QStringList keywords;
        QStringList importer;
        QList<CodeBlocks> codeBlocks;
        QList<Exporter> exporter;
    };

    struct SmartPointer {
        QString name;
        bool unique{};
    };

    struct Entrypoint {
        QString name;
    };

    void clear() {
        containers.clear();
        defines.clear();
        undefines.clear();
        functions.clear();
        memoryresource.clear();
        podtypes.clear();
        smartPointers.clear();
        typeChecks.clear();
        platformTypes.clear();
        reflections.clear();
        markups.clear();
        entrypoints.clear();
    }

    void swap(CppcheckLibraryData &other) noexcept {
        containers.swap(other.containers);
        defines.swap(other.defines);
        undefines.swap(other.undefines);
        functions.swap(other.functions);
        memoryresource.swap(other.memoryresource);
        podtypes.swap(other.podtypes);
        smartPointers.swap(other.smartPointers);
        typeChecks.swap(other.typeChecks);
        platformTypes.swap(other.platformTypes);
        reflections.swap(other.reflections);
        markups.swap(other.markups);
        entrypoints.swap(other.entrypoints);
    }

    QString open(QIODevice &file);
    QString toString() const;

    QList<Container> containers;
    QList<Define> defines;
    QList<Function> functions;
    QList<MemoryResource> memoryresource;
    QList<PodType> podtypes;
    QList<TypeChecks> typeChecks;
    QList<PlatformType> platformTypes;
    QStringList undefines;
    QList<SmartPointer> smartPointers;
    QList<Reflection> reflections;
    QList<Markup> markups;
    QList<Entrypoint> entrypoints;
};

#endif // CPPCHECKLIBRARYDATA_H
