lessThan(QT_MAJOR_VERSION, 6): error(requires >= Qt 6 (You used: $$QT_VERSION))

TEMPLATE = app
TARGET = cppcheck-gui
CONFIG += warn_on debug
DEPENDPATH += . \
    ../lib
INCLUDEPATH += . \
    ../lib
QT += widgets
QT += printsupport
QT += help
QT += network

# Build online help
#onlinehelp.target = online-help.qhc
#onlinehelp.commands = qhelpgenerator $$PWD/help/online-help.qhcp -o $$PWD/help/online-help.qhc
#QMAKE_EXTRA_TARGETS += onlinehelp
#PRE_TARGETDEPS += online-help.qhc

contains(LINKCORE, [yY][eE][sS]) {
    LIBS += -l../bin/cppcheck-core
    DEFINES += CPPCHECKLIB_IMPORT
}
LIBS += -L$$PWD/../externals

DESTDIR = .
RCC_DIR = temp
MOC_DIR = temp
OBJECTS_DIR = temp
UI_DIR = temp

isEmpty(QMAKE_CXX) {
    isEmpty(CXX)) {
        QMAKE_CXX = gcc
    } else {
        QMAKE_CXX = $$(CXX)
    }
}

win32 {
   CONFIG += windows
   contains(LINKCORE, [yY][eE][sS]) {
      DESTDIR = ../bin
      RCC_DIR = temp/generated
      MOC_DIR = temp/generated
      OBJECTS_DIR = temp/generated
      UI_DIR = temp/generated
   } else {
      DESTDIR = ../Build/gui
      RCC_DIR = ../BuildTmp/gui
      MOC_DIR = ../BuildTmp/gui
      OBJECTS_DIR = ../BuildTmp/gui
      UI_DIR = ../BuildTmp/gui
   }
}

RESOURCES = gui.qrc
FORMS = about.ui \
        applicationdialog.ui \
        compliancereportdialog.ui \
        fileview.ui \
        helpdialog.ui \
        mainwindow.ui \
        projectfile.ui \
        resultsview.ui \
        scratchpad.ui \
        settings.ui \
        statsdialog.ui \
        librarydialog.ui \
        libraryaddfunctiondialog.ui \
        libraryeditargdialog.ui \
        newsuppressiondialog.ui \
        threaddetails.ui

TRANSLATIONS =  cppcheck_de.ts \
                cppcheck_es.ts \
                cppcheck_fi.ts \
                cppcheck_fr.ts \
                cppcheck_it.ts \
                cppcheck_ja.ts \
                cppcheck_ka.ts \
                cppcheck_ko.ts \
                cppcheck_nl.ts \
                cppcheck_ru.ts \
                cppcheck_sr.ts \
                cppcheck_sv.ts \
                cppcheck_zh_CN.ts \
                cppcheck_zh_TW.ts

# Windows-specific options
CONFIG += embed_manifest_exe

contains(LINKCORE, [yY][eE][sS]) {
} else {
    INCLUDEPATH += $$PWD/../lib
    HEADERS += $$PWD/../lib/*.h
    SOURCES += $$PWD/../lib/*.cpp

    INCLUDEPATH += $$PWD/../frontend
    HEADERS += $$PWD/../frontend/*.h
    SOURCES += $$PWD/../frontend/*.cpp

    INCLUDEPATH += $$PWD/../externals/picojson
    HEADERS += $$PWD/../externals/picojson/picojson.h

    INCLUDEPATH += $$PWD/../externals/simplecpp
    HEADERS += $$PWD/../externals/simplecpp/simplecpp.h
    SOURCES += $$PWD/../externals/simplecpp/simplecpp.cpp
    DEFINES += SIMPLECPP_TOKENLIST_ALLOW_PTR

    INCLUDEPATH += $$PWD/../externals/tinyxml2
    HEADERS += $$PWD/../externals/tinyxml2/tinyxml2.h
    SOURCES += $$PWD/../externals/tinyxml2/tinyxml2.cpp
}

win32-msvc* {
    MSVC_VER = $$(VisualStudioVersion)
    message($$MSVC_VER)
    MSVC_VER_SPLIT = $$split(MSVC_VER, .)
    MSVC_VER_MAJOR = $$first(MSVC_VER_SPLIT)
    # doesn't compile with older VS versions - assume VS2019 (16.x) is the first working for now
    !lessThan(MSVC_VER_MAJOR, 16) {
        message("using precompiled header")
        CONFIG += precompile_header
        PRECOMPILED_HEADER = precompiled_qmake.h
    }
}

HEADERS += aboutdialog.h \
           application.h \
           applicationdialog.h \
           applicationlist.h \
           checkstatistics.h \
           checkthread.h \
           codeeditstylecontrols.h \
           codeeditorstyle.h \
           codeeditstyledialog.h \
           codeeditor.h \
           common.h \
           compliancereportdialog.h \
           csvreport.h \
           erroritem.h \
           filelist.h \
           fileviewdialog.h \
           helpdialog.h \
           mainwindow.h \
           platforms.h \
           printablereport.h \
           projectfile.h \
           projectfiledialog.h \
           report.h \
           resultitem.h \
           resultstree.h \
           resultsview.h \
           scratchpad.h \
           settingsdialog.h \
           showtypes.h \
           statsdialog.h \
           threaddetails.h \
           threadhandler.h \
           threadresult.h \
           translationhandler.h \
           txtreport.h \
           xmlreport.h \
           xmlreportv2.h \
           librarydialog.h \
           cppchecklibrarydata.h \
           libraryaddfunctiondialog.h \
           libraryeditargdialog.h \
           newsuppressiondialog.h

SOURCES += aboutdialog.cpp \
           application.cpp \
           applicationdialog.cpp \
           applicationlist.cpp \
           checkstatistics.cpp \
           checkthread.cpp \
           codeeditorstyle.cpp \
           codeeditstylecontrols.cpp \
           codeeditstyledialog.cpp \
           codeeditor.cpp \
           common.cpp \
           compliancereportdialog.cpp \
           csvreport.cpp \
           erroritem.cpp \
           filelist.cpp \
           fileviewdialog.cpp \
           helpdialog.cpp \
           main.cpp \
           mainwindow.cpp\
           platforms.cpp \
           printablereport.cpp \
           projectfile.cpp \
           projectfiledialog.cpp \
           report.cpp \
           resultitem.cpp \
           resultstree.cpp \
           resultsview.cpp \
           scratchpad.cpp \
           settingsdialog.cpp \
           showtypes.cpp \
           statsdialog.cpp \
           threaddetails.cpp \
           threadhandler.cpp \
           threadresult.cpp \
           translationhandler.cpp \
           txtreport.cpp \
           xmlreport.cpp \
           xmlreportv2.cpp \
           librarydialog.cpp \
           cppchecklibrarydata.cpp \
           libraryaddfunctiondialog.cpp \
           libraryeditargdialog.cpp \
           newsuppressiondialog.cpp

win32 {
    RC_FILE = cppcheck-gui.rc
    HEADERS += ../lib/version.h
    contains(LINKCORE, [yY][eE][sS]) {
    } else {
        LIBS += -lshlwapi
    }
}

contains(QMAKE_CC, gcc) {
    QMAKE_CXXFLAGS += -std=c++17 -pedantic -Wall -Wextra -Wcast-qual -Wno-deprecated-declarations -Wfloat-equal -Wmissing-declarations -Wmissing-format-attribute -Wno-long-long -Wpacked -Wredundant-decls -Wundef -Wno-shadow -Wno-missing-field-initializers -Wno-missing-braces -Wno-sign-compare -Wno-multichar
}

contains(QMAKE_CXX, clang++) {
    QMAKE_CXXFLAGS += -std=c++17 -pedantic -Wall -Wextra -Wcast-qual -Wno-deprecated-declarations -Wfloat-equal -Wmissing-declarations -Wmissing-format-attribute -Wno-long-long -Wpacked -Wredundant-decls -Wundef -Wno-shadow -Wno-missing-field-initializers -Wno-missing-braces -Wno-sign-compare -Wno-multichar
}

contains(HAVE_QCHART, [yY][eE][sS]) {
    QT += charts
} else {
    message("Charts disabled - to enable it pass HAVE_QCHART=yes to qmake.")
}

