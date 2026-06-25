#ifndef THREADDETAILS_H
#define THREADDETAILS_H

#include <QWidget>
#include <QMap>
#include <QMutex>
#include <QTimer>
#include "checkthread.h"

namespace Ui {
    class ThreadDetails;
}

class ThreadDetails : public QWidget
{
    Q_OBJECT

public:
    explicit ThreadDetails(QWidget *parent = nullptr);
    ~ThreadDetails() override;

    static ThreadDetails* instance() {
        return mInstance;
    }

public slots:
    void threadDetailsUpdated(QMap<int, CheckThread::Details> threadDetails);
    void progress(QString filename, QString stage, std::size_t value);

private slots:
    void updateUI();

private:
    static ThreadDetails* mInstance;
    Ui::ThreadDetails *mUi;
    QMap<QString, QPair<QString,QString>> mProgress;
    QMap<int, CheckThread::Details> mThreadDetails;
    QTimer mTimer;

    /** accessing mProgress and mThreadDetails in slots that are triggered from different threads */
    QMutex mMutex;
};

#endif // THREADDETAILS_H
