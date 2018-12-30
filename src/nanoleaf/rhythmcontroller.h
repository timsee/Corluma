#ifndef RHYTHMCONTROLLER_H
#define RHYTHMCONTROLLER_H

#include <QString>

namespace nano
{
/*!
 * \copyright
 * Copyright (C) 2015 - 2019.
 * Released under the GNU General Public License.
 */

/*!
 * \brief The RhythmController class contains all known information about the
 *        connected Rhythm Controller.
 */
class RhythmController
{
public:
    /// Constructor
    RhythmController();

    /// true if connected, false otherwise. If this is false, other values may be null.
    bool isConnected;

    /// true if active, false otherwise
    bool isActive;

    /// ID for the Rhythm Controller
    QString ID;

    /// hardware version
    QString hardwareVersion;

    /// current firmware version
    QString firmwareVersion;

    /// true if an auxiliary input is available, false otherwise
    bool auxAvailable;

    /// current mode of Rhythm controller
    QString mode;

    /// current position of the Rhythm controller
    QString position;
};

}

#endif // RHYTHMCONTROLLER_H
