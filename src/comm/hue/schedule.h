#ifndef HUE_SCHEDULE_H
#define HUE_SCHEDULE_H

#include <QDebug>
#include <QJsonObject>
#include <QRegExp>
#include <QString>
#include <QStringList>
#include <QTime>
#include "comm/hue/command.h"

namespace hue {
/*!
 * \copyright
 * Copyright (C) 2015 - 2020.
 * Released under the GNU General Public License.
 *
 *
 *
 * \brief The hue::Scheduele class is a schedule for hues. Schedules are stored
 *        in the bridge and will execute even when no application is connected.
 *        Schedules also stay around if autodelete is set to false, and must be
 *        deleted explicitly.
 */
class Schedule {
public:
    Schedule() : mName{"INVALID"}, mDescription{"INVALID"}, mIndex{0u} {}

    /// constructor
    Schedule(QJsonObject object, int i) {
        // check if valid packet
        if (object["localtime"].isString() && object["command"].isObject()
            && object["time"].isString() && object["created"].isString()) {
            mTime = object["time"].toString();
            mCreated = object["created"].toString();
            mLocaltime = object["localtime"].toString();

            mIndex = i;

            if (object["name"].isString()) {
                mName = object["name"].toString();
            } else {
                mName = "schedule";
            }

            if (object["description"].isString()) {
                mDescription = object["description"].toString();
            }

            if (object["status"].isString()) {
                QString status = object["status"].toString();
                mStatus = (status == "enabled");
            } else {
                mStatus = true;
            }

            if (object["autodelete"].isBool()) {
                mAutodelete = object["autodelete"].toBool();
            } else {
                mAutodelete = false;
            }

            mCommand = Command(object["command"].toObject());
        }
    }

    /// name of the schedule
    const QString& name() const noexcept { return mName; }

    /// description of the schedule, optional
    const QString& description() const noexcept { return mDescription; }

    /// time and date of creation
    const QString& created() const noexcept { return mCreated; }

    /// true if active and valid, false if there are any issues.
    bool status() const noexcept { return mStatus; }

    /// setter for status
    void status(bool status) { mStatus = status; }

    /// true if schedule should delete when its done, false if it should stick around
    bool autodelete() const noexcept { return mAutodelete; }

    /// the command to execute when the schedule completes
    const Command& command() const noexcept { return mCommand; }

    /// the time the schedule will execute
    const QString& time() const noexcept { return mTime; }

    /// the time the schedule was created in localtime
    const QString& localtime() const noexcept { return mLocaltime; }

    /// setter for localtime.
    void localtime(const QString& time) { mLocaltime = time; }

    /// the index of the schedule
    int index() const noexcept { return mIndex; }

    void updateCreatedTime() {
        auto currentTimeUtc = QDateTime::currentDateTimeUtc();
        auto date = currentTimeUtc.date();
        auto time = currentTimeUtc.time();
        auto dateString = QString::number(date.year()) + "-" + QString::number(date.month()) + "-"
                          + QString::number(date.day());
        auto timeString = QString::number(time.hour()) + ":" + QString::number(time.minute()) + ":"
                          + QString::number(time.second());
        auto finalString = dateString + "T" + timeString;
        mCreated = finalString;
    }

    /// returns how many seconds its been since the schedule was created. Will return 0u if there
    /// are any errors.
    std::uint64_t secondsSinceCreation() {
        QRegExp nonDigitRegex("(\\D)");
        QRegExp tRegex("(T)");
        QStringList createdStringList = created().split(tRegex);
        createdStringList.removeAll("");
        if (createdStringList.size() != 2) {
            // exit out if the string is in the wrong size
            return 0u;
        }
        auto createdTimes = createdStringList[1].split(nonDigitRegex);
        createdTimes.removeAll("");
        if (createdTimes.size() != 3) {
            // exit out if the string is in the wrong size
            return 0u;
        }
        QTime creationTime(createdTimes[0].toInt(),
                           createdTimes[1].toInt(),
                           createdTimes[2].toInt());
        QTime currentTime = QDateTime::currentDateTimeUtc().time();
        return creationTime.msecsTo(currentTime) / 1000;
    }

    /// returns the number of minutes until a timeout, if the schedule is a tiemout schedule. If it
    /// is not a timeout schedule, or if there are any errors, this returns -1.
    int secondsUntilTimeout() {
        // not all schedules are timeouts, return -1 if no timeout could exist
        if (!autodelete() || !name().contains("Corluma_timeout_")) {
            return -1;
        }
        QRegExp nonDigitRegex("(\\D)");
        auto creationSeconds = secondsSinceCreation();
        // convert timeout string to minutes
        auto localTimeStringList = localtime().split(nonDigitRegex);
        localTimeStringList.removeAll("");
        if (localTimeStringList.size() != 3) {
            // exit out if the string is in the wrong size
            return -1;
        }
        // convert hours to minutes
        int timeLeft = localTimeStringList[0].toInt() * 3600;
        // add in the minutes
        timeLeft += localTimeStringList[1].toInt() * 60;
        // add in the seconds
        timeLeft += localTimeStringList[2].toInt();
        // subtract time since creation
        timeLeft -= creationSeconds;
        return timeLeft;
    }

    operator QString() const {
        QString result = "SHueCommand ";
        result += " name: " + name();
        result += " description: " + description();
        result += " command: " + command();
        result += " time: " + time();
        result += " localtime: " + localtime();
        result += " created: " + created();
        result += " status: " + QString(status());
        result += " index: " + QString(index());
        result += " autodelete: " + QString(autodelete());
        return result;
    }

    /// hue::Schedule equal operator
    bool operator==(const Schedule& rhs) const {
        bool result = true;
        if (name() != rhs.name()) {
            result = false;
        }
        if (description() != rhs.description()) {
            result = false;
        }
        if (index() != rhs.index()) {
            result = false;
        }
        return result;
    }

private:
    /// name of the schedule
    QString mName;

    /// description of the schedule, optional
    QString mDescription;

    /// time and date of creation
    QString mCreated;

    /// true if active and valid, false if there are any issues.
    bool mStatus;

    /// true if schedule should delete when its done, false if it should stick around
    bool mAutodelete;

    /// the time the schedule was created in localtime
    QString mLocaltime;

    /// the time the schedule will execute
    QString mTime;

    /// the command to execute when the schedule completes
    Command mCommand;

    /// the index of the schedule
    int mIndex;
};

} // namespace hue

namespace std {
template <>
struct hash<hue::Schedule> {
    size_t operator()(const hue::Schedule& k) const {
        return std::hash<std::string>{}(k.name().toStdString());
    }
};

} // namespace std

#endif // HUE_SCHEDULE_H
