#ifndef LEAFSCHEDULE_H
#define LEAFSCHEDULE_H

#include <QDebug>
#include <QJsonObject>
#include <string>

#include "leafaction.h"
#include "leafdate.h"
#include "utils/exception.h"

namespace nano {

/// types of repeat behavior
enum class ERepeat { once = -1, minute = 0, hourly = 1, weekly = 3, monthly = 4 };

/// converts a double to a repeat enum
inline ERepeat doubleToRepeatEnum(double i) {
    GUARD_EXCEPTION(i <= double(ERepeat::monthly) && i >= double(ERepeat::once),
                    "cannot convert given value to repeat type");
    return ERepeat(i);
}

const int kTimeoutID = 51;

/*!
 * \copyright
 * Copyright (C) 2015 - 2020.
 * Released under the GNU General Public License.
 */


/*!
 * \brief The LeafSchedule class stores a schedule for a nonaleaf
 */
class LeafSchedule {
public:
    LeafSchedule() {}

    LeafSchedule(bool enabled,
                 ERepeat type,
                 int intervalType,
                 LeafDate startDate,
                 LeafAction action)
        : mRepeatType{type},
          mIsEnabled{enabled},
          mRepeatValue{intervalType},
          mStartDate{startDate},
          mAction{action} {}

    LeafSchedule(const QJsonObject& object) {
        GUARD_EXCEPTION(isValidJson(object), "LeafSchedule passed invalid JSON");

        mIsEnabled = object["enabled"].toBool();
        const auto& repeat = object["repeat"].toObject();
        mRepeatType = doubleToRepeatEnum(repeat["interval_type"].toDouble());

        if (repeat["end_time"].isObject()) {
            mEndDate = LeafDate(repeat["end_time"].toObject());
        }

        if (object["start_time"].isObject()) {
            mStartDate = LeafDate(object["start_time"].toObject());
        }

        mAction = LeafAction(object["action"].toObject());
        mSetID = object["set_id"].toString();

        mID = int(object["id"].toDouble());
    }

    /// returns whether or not a json object represents a leaf schedule
    bool isValidJson(const QJsonObject& object) {
        return (object["enabled"].isBool() && object["repeat"].isObject()
                && object["start_time"].isObject() && object["set_id"].isString()
                && object["id"].isDouble());
    }

    /// converts a schedule to json
    QJsonObject toJson() const {
        QJsonObject object;
        object["action"] = mAction.toJSON();
        object["enabled"] = mIsEnabled;
        object["start_time"] = mStartDate.toJson();
        object["id"] = mID;
        QJsonObject repeatObject;
        repeatObject["interval_type"] = double(mRepeatType);
        repeatObject["interval_value"] = double(mRepeatValue);
        // repeatObject["end_time"] = mEndDate.toJSON();
        object["repeat"] = repeatObject;
        return object;
    }

    /// getter for repeat type
    const ERepeat& repeat() const noexcept { return mRepeatType; }

    /// true if enabled, false otherwise
    bool enabled() const noexcept { return mIsEnabled; }

    /// UUID for schedule
    const QString& setID() const noexcept { return mSetID; }

    /// ID number for a schedule
    int ID() const noexcept { return mID; }

    /// getter for start date
    const nano::LeafDate& startDate() const noexcept { return mStartDate; }

    /// returns the number of seconds until a schedule executes.
    double secondsUntilExecution() const {
        auto curTime = LeafDate::currentTime().date();
        auto executionTime = mStartDate.date();
        return std::difftime(std::mktime(&executionTime), std::mktime(&curTime));
    }

    bool operator==(const LeafSchedule& rhs) const {
        bool result = true;
        if (mID != rhs.ID()) {
            result = false;
        }
        return result;
    }

private:
    /// type of repeat behavior
    ERepeat mRepeatType;

    /// true if enabled false otherwise
    bool mIsEnabled;

    /// UUID for schedule
    QString mSetID;

    /// integer ID for schedule
    int mID = kTimeoutID;

    /// repeat value
    int mRepeatValue = 1u;

    /// time the schedule ends
    LeafDate mEndDate;

    /// time the schedule should start executing
    LeafDate mStartDate;

    /// action the schedule should execute
    LeafAction mAction;
};

} // namespace nano

namespace std {
template <>
struct hash<::nano::LeafSchedule> {
    size_t operator()(const ::nano::LeafSchedule& k) const {
        return std::hash<std::string>{}(QString::number(k.ID()).toStdString());
    }
};
} // namespace std

#endif // LEAFSCHEDULE_H
