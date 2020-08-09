#ifndef DEBUGCONNECTIONSPOOFER_H
#define DEBUGCONNECTIONSPOOFER_H

#include <QDebug>
#include "comm/commlayer.h"

/// struct that stores the data needed to spoof a connection
struct DebugConnection {
    /// IP to spoof
    QString IP;
    /// discovery packet for spoofing
    QString discovery;
    /// initial state for the spoofed light
    QString stateUpdate;
    /// all connections currently spoof as UDP packets
    ECommType type = ECommType::UDP;
};

/*!
 * \copyright
 * Copyright (C) 2015 - 2020.
 * Released under the GNU General Public License.
 *
 *
 * \brief The DebugConnectionSpoofer class is used for debugging the application without explicit
 * lights to test on. It will send packets that pretend to be ArduCor devices, which then show up as
 * reachable in the application. This allows you to get into the application to debug UI elements.
 */
class DebugConnectionSpoofer {
private:
    /// pointer to comm layer
    CommLayer* mComm;

    /// spoofs a connection by sending the the right packets from a DebugConnection struct to the
    /// CommLayer
    void spoofConnection(const DebugConnection& connection);

public:
    DebugConnectionSpoofer(CommLayer* comm) : mComm{comm} {}

    /// spoofs all connections
    void initiateSpoofedConnections();
};

#endif // DEBUGCONNECTIONSPOOFER_H
