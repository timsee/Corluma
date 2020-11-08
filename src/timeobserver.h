#ifndef TIMEOUTOBSERVER_H
#define TIMEOUTOBSERVER_H

#include <QObject>
#include <QTime>


/*!
 * \copyright
 * Copyright (C) 2015 - 2020.
 * Released under the GNU General Public License.
 *
 *
 * \brief The TimeObserver class observes time based events across the app, such as the last time
 * that the StateObserver changed anything.
 */
class TimeObserver : public QObject {
    Q_OBJECT
public:
    explicit TimeObserver(QObject* parent) : QObject(parent) {}

    /// time of last update
    const QTime lastUpdateTime() const noexcept { return mLastUpdateTime; }

    /// number of seconds since last update
    std::uint32_t secondsSinceLstUpdate() {
        auto currentTime = QTime::currentTime();
        auto msecDiff = mLastUpdateTime.msecsTo(currentTime);
        return msecDiff / 1000;
    }

    /// updates the update time to the current time
    void updateTime() { mLastUpdateTime = QTime::currentTime(); }

private:
    /// last time that an update ocurred
    QTime mLastUpdateTime;
};

#endif // TIMEOUTOBSERVER_H
