#include "threaddetails.h"
#include "utils.h"

#include "ui_threaddetails.h"

#include <QMutexLocker>

ThreadDetails* ThreadDetails::mInstance;

ThreadDetails::ThreadDetails(QWidget *parent)
    : QWidget(parent)
    , mUi(new Ui::ThreadDetails)
{
    mInstance = this;
    setWindowFlags(Qt::Window);
    mUi->setupUi(this);
    connect(&mTimer, &QTimer::timeout, this, &ThreadDetails::updateUI);
    mTimer.start(1000);
}

ThreadDetails::~ThreadDetails()
{
    mInstance = nullptr;
    delete mUi;
}

// NOLINTNEXTLINE(performance-unnecessary-value-param) - false positive
void ThreadDetails::threadDetailsUpdated(QMap<int, CheckThread::Details> threadDetails)
{
    QMutexLocker locker(&mMutex);
    mThreadDetails = threadDetails;
    if (threadDetails.empty()) {
        mProgress.clear();
    }
}

// cppcheck-suppress passedByValue
// NOLINTNEXTLINE(performance-unnecessary-value-param) - false positive
void ThreadDetails::progress(QString filename, QString stage, std::size_t value) {
    QMutexLocker locker(&mMutex);
    mProgress[filename] = {QString(), stage + QString(value > 0 ? ": %1%" : "").arg(value)};
}

void ThreadDetails::updateUI() {
    QString text("Thread\tStart time\tFile/Progress\n");
    {
        QMutexLocker locker(&mMutex);
        for (const auto& td: utils::as_const(mThreadDetails)) {
            auto& timeProgress = mProgress[td.file];
            if (timeProgress.first.isEmpty() && !timeProgress.second.isEmpty())
                timeProgress.first = QTime::currentTime().toString(Qt::TextDate);
            text += QString("%1\t%2\t%3\n\t%4\t%5\n").arg(td.threadIndex).arg(td.startTime.toString(Qt::TextDate)).arg(td.file).arg(timeProgress.first).arg(timeProgress.second);
        }
    }
    mUi->plainTextEdit->setPlainText(text);
}
