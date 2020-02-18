#ifndef HUE_SCHEDULE_H
#define HUE_SCHEDULE_H

#include <QJsonObject>
#include <QString>

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

    /// true if schedule should delete when its done, false if it should stick around
    bool autodelete() const noexcept { return mAutodelete; }

    /// the command to execute when the schedule completes
    const Command& command() const noexcept { return mCommand; }

    /// the time the schedule will execute
    const QString& time() const noexcept { return mTime; }

    /// the time the schedule was created in localtime
    const QString& localtime() const noexcept { return mLocaltime; }

    /// the index of the schedule
    int index() const noexcept { return mIndex; }

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
