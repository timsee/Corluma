#ifndef LEAFACTION_H
#define LEAFACTION_H

#include <QDebug>
#include <QJsonObject>

#include "utils/exception.h"

namespace nano {

/// enum for basic action types
enum class EActionType { on, off };

/*!
 * \copyright
 * Copyright (C) 2015 - 2019.
 * Released under the GNU General Public License.
 */


/*!
 * \brief The LeafAction class stores an action that a leaf can execute. This is used for schedules
 * and the like to store an action for the lights.
 *
 */
class LeafAction {
public:
    LeafAction(EActionType action) {
        switch (action) {
            case EActionType::off:
                mIsOn = false;
                break;
            case EActionType::on:
                mIsOn = true;
                break;
        }
    }

    LeafAction() {}

    LeafAction(const QJsonObject& object) {
        GUARD_EXCEPTION(isValidJSON(object), "Invalid JSON passed to LeafAction");
        mIsOn = bool(object["on"].toDouble());
    }

    /// checks if an action is valid JSON
    /// TODO: support more than just on and off.
    static bool isValidJSON(const QJsonObject& object) { return (object["on"].isDouble()); }

    /// converts an action to valid json.
    QJsonObject toJSON() const {
        QJsonObject object;
        object["on"] = double(mIsOn);
        QJsonObject durationObject;
        durationObject["duration"] = 0;
        durationObject["value"] = 0;
        object["brightness"] = durationObject;
        return object;
    }

private:
    /// true if on, false otherwise
    bool mIsOn;
};


} // namespace nano

#endif // LEAFACTION_H
