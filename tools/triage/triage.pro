QT += core gui widgets
TARGET = triage
TEMPLATE = app
QMAKE_CXXFLAGS += -std=c++17
INCLUDEPATH += ../../gui ../../lib
MOC_DIR = temp
OBJECTS_DIR = temp
UI_DIR = temp
SOURCES += main.cpp \
        mainwindow.cpp \
        ../../gui/codeeditorstyle.cpp \
        ../../gui/codeeditor.cpp \
        ../../lib/utils.cpp
HEADERS  += mainwindow.h \
        ../../gui/codeeditorstyle.h \
        ../../gui/codeeditor.h \
        ../../lib/utils.h
FORMS    += mainwindow.ui
