/*!
 * \copyright
 * Copyright (C) 2015 - 2020.
 * Released under the GNU General Public License.
 */
#include "debugconnectionspoofer.h"
#include "comm/commarducor.h"

void DebugConnectionSpoofer::initiateSpoofedConnections() {
    std::vector<DebugConnection> lightsToSpoof;

    DebugConnection light1;
    light1.IP = "192.168.0.420";
    light1.discovery = "DISCOVERY_PACKET,3,3,0,0,200,1@DEBUG 1,1,1&";
    light1.stateUpdate = "6,1,1,1,255,127,0,3,1,80,200,120,60&";
    lightsToSpoof.push_back(light1);

    DebugConnection light2;
    light2.IP = "192.168.0.710";
    light2.discovery = "DISCOVERY_PACKET,3,3,0,0,200,1@Cool Light,4,1&";
    light2.stateUpdate = "6,1,1,1,0,255,0,2,1,80,200,120,60&";
    lightsToSpoof.push_back(light2);

    DebugConnection light3;
    light3.IP = "192.168.0.500";
    light3.discovery = "DISCOVERY_PACKET,3,3,0,0,200,1@Just a Light,3,1&";
    light3.stateUpdate = "6,1,1,1,0,0,255,1,1,80,200,120,60&";
    lightsToSpoof.push_back(light3);

    DebugConnection light4;
    light4.IP = "192.168.0.123";
    light4.discovery = "DISCOVERY_PACKET,3,3,0,0,200,1@Floop Lamp,2,1&";
    light4.stateUpdate = "6,1,1,1,0,255,255,4,1,80,200,120,60&";
    lightsToSpoof.push_back(light4);

    DebugConnection light5;
    light5.IP = "192.168.0.321";
    light5.discovery = "DISCOVERY_PACKET,3,3,0,0,200,1@Floop Lamp Jr,0,1&";
    light5.stateUpdate = "6,1,1,1,0,255,127,3,1,80,200,120,60&";
    lightsToSpoof.push_back(light5);

    for (const auto& light : lightsToSpoof) {
        spoofConnection(light);
    }
}

void DebugConnectionSpoofer::spoofConnection(const DebugConnection& connection) {
    mComm->arducor()->discovery()->addManualIP(connection.IP);
    mComm->arducor()->discovery()->handleIncomingPacket(connection.type,
                                                        connection.IP,
                                                        connection.discovery);
    mComm->arducor()->parsePacket(connection.IP, connection.stateUpdate, connection.type);
}
