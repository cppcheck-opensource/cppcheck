/* -*- C++ -*-
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

#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QFileSystemModel>
#include <QMainWindow>
#include <QObject>
#include <QRegularExpression>
#include <QString>
#include <QStringList>

class QListWidgetItem;
class QTextStream;
class QPoint;
class QWidget;
namespace Ui {
    class MainWindow;
}

class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    MainWindow(const MainWindow &) = delete;
    MainWindow &operator=(const MainWindow &) = delete;
    ~MainWindow() override;

public slots:
    void loadFile();
    void loadFromClipboard();
    void filter(const QString& filterStr);
    void showResult(QListWidgetItem *item);
    void refreshResults();
    void fileTreeFilter(const QString &str);
    void findInFilesClicked();
    void directorytreeDoubleClick();
    void searchResultsDoubleClick();
    void resultsContextMenu(const QPoint& pos);

private:
    Ui::MainWindow *ui;

    void load(QTextStream &textStream);
    bool runProcess(const QString &programName, const QStringList & arguments);
    bool wget(const QString &url);
    bool unpackArchive(const QString &archiveName);
    void showSrcFile(const QString &fileName, const QString &url, int lineNumber);

    QStringList mAllErrors;
    QFileSystemModel mFSmodel;
    const QRegularExpression mVersionRe;

    const QStringList hFiles;
    const QStringList srcFiles;
};

#endif // MAINWINDOW_H
