#ifndef HUE_COMMAND_H
#define HUE_COMMAND_H

#include <QJsonDocument>
#include <QJsonObject>
#include <QString>

namespace hue {
/*!
 * \copyright
 * Copyright (C) 2015 - 2020.
 * Released under the GNU General Public License.
 *
 *
 *
 * \brief The hue::Command is a simple object that represents a command for a Hue. It is used by
 * schedules and similar systems to know what to do when certain conditions are met. For example,
 * for schedules, the command executes when the schedule timer is up.
 */
class Command {
public:
    /// default constructor
    Command() = default;

    /// json based constructor
    Command(const QJsonObject& object) {
        mAddress = object["address"].toString();
        mMethod = object["method"].toString();
        mRoutineObject = object["body"].toObject();
    }

    /// getter for address
    const QString& address() const noexcept { return mAddress; }

    /// getter for method
    const QString& method() const noexcept { return mMethod; }

    /// getter for routine for the command
    const QJsonObject& routineObject() const noexcept { return mRoutineObject; }

    /// creates a JSON representation of the command
    QJsonObject toJson() {
        QJsonObject object;
        object["address"] = address();
        object["method"] = method();
        QJsonObject body;
        object["body"] = routineObject();
        return body;
    }

    operator QString() const {
        QString result = "SHueCommand ";
        result += " address: " + address();
        result += " method: " + method();
        QJsonDocument doc(routineObject());
        result += " routineObject: " + doc.toJson(QJsonDocument::Compact);
        return result;
    }

private:
    /// the address of the light to send the command to.
    QString mAddress;

    /// the method to do for the command
    QString mMethod;

    /// the full routine that the light is expected to do with this command.
    QJsonObject mRoutineObject;
};

} // namespace hue

#endif // HUE_COMMAND_H
