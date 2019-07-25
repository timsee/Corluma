#ifndef NANO_LEAFCONTROLLER_H
#define NANO_LEAFCONTROLLER_H

#include <QColor>
#include <QJsonObject>
#include <QString>
#include <sstream>
#include <vector>

#include "cor/range.h"
#include "panels.h"
#include "rhythmcontroller.h"

namespace nano {

/*!
 * \copyright
 * Copyright (C) 2015 - 2019.
 * Released under the GNU General Public License.
 *
 * \brief The LeafController class holds all the data known
 *        about a NanoLeaf Auorara.
 */
class LeafController {
public:
    /// constructor
    LeafController();

    //-----------
    // Metadata
    //-----------

    /// name of controller (defined in app data)
    QString name;

    /// name of controller, defined by hardware
    QString hardwareName;

    /// serial number of controller
    QString serialNumber;

    /// manufacturer
    QString manufacturer;

    /// current firmware
    QString firmware;

    /// hardware model
    QString model;

    //-----------
    // Connection
    //-----------

    /// IP address for nanoleaf, empty if it doesn't exist.
    QString IP;

    /// authorization token for messages sent to nanoleaf, empty if it doesn't exist.
    QString authToken;

    /// port of nanoleaf, -1 if no nanoleaf found
    int port;

    //-----------
    // Current States
    //-----------

    /// name of current effect
    QString effect;

    /// list of names of all effects on the controller
    std::vector<QString> effectsList;

    //-----------
    // Other Hardware
    //-----------

    /// layout and information on the individual nanoleaf panels
    Panels panelLayout;

    /// information on the rhythm controller, if one is connected.
    RhythmController rhythm;

    //-----------
    // Ranges for Nanoleaf
    //-----------

    /// possible range for brightness values
    cor::Range<uint32_t> brightRange;

    /// possible range for hue values
    cor::Range<uint32_t> hueRange;

    /// possible range for saturation values
    cor::Range<uint32_t> satRange;

    /// possible range for color temperature values
    cor::Range<uint32_t> ctRange;

    operator QString() const {
        std::stringstream tempString;
        tempString << "nano::LeafController: "
                   << " name: " << name.toStdString()
                   << " hardwareName: " << hardwareName.toStdString() << " IP:" << IP.toStdString()
                   << " port: " << port << " authToken: " << authToken.toStdString()
                   << " serial: " << serialNumber.toStdString()
                   << " effect: " << authToken.toStdString();
        return QString::fromStdString(tempString.str());
    }

    /// equal operator
    bool operator==(const nano::LeafController& rhs) const {
        bool result = true;
        if (serialNumber != rhs.serialNumber)
            result = false;
        return result;
    }
};

/// converts json representation of routine to cor::Light
LeafController jsonToLeafController(const QJsonObject& object);

/// converts a cor::Light to a json representation of its routine.
QJsonObject leafControllerToJson(const LeafController& controller);

} // namespace nano

namespace std {
template <>
struct hash<::nano::LeafController> {
    size_t operator()(const ::nano::LeafController& k) const {
        return std::hash<std::string>{}(k.serialNumber.toStdString());
    }
};
} // namespace std

#endif // NANO_LEAFCONTROLLER_H
