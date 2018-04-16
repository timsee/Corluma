#ifndef NANOLEAFCONTROLLER_H
#define NANOLEAFCONTROLLER_H

#include <QString>
#include <QColor>
#include <vector>
#include "rhythmcontroller.h"
#include "panels.h"
#include "cor/range.h"

namespace nano
{

/*!
 * \copyright
 * Copyright (C) 2015 - 2018.
 * Released under the GNU General Public License.
 *
 *
 *
 * \brief The LeafController class holds all the data known
 *        about a NanoLeaf Auorara.
 */
class LeafController
{
public:
    /// constructor
    LeafController();

    //-----------
    // Metadata
    //-----------

    /// name of controller (defined in app data)
    QString name;

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

};

}
#endif // NANOLEAFCONTROLLER_H
