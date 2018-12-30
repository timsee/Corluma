#ifndef REACHABILITYUTILS_H
#define REACHABILITYUTILS_H

#include <QNetworkConfigurationManager>

namespace cor
{
/*!
 * \copyright
 * Copyright (C) 2015 - 2018.
 * Released under the GNU General Public License.
 *
 */


/*!
 * \brief wifiEnabled uses the QNetworkConfiguratyionManager to scan whether or not there is a wifi connection
 * \return true if there is a wifi connection, false otherwise.
 */
inline bool wifiEnabled() {
    QNetworkConfigurationManager mgr;
    QList<QNetworkConfiguration> activeConfigs = mgr.allConfigurations(QNetworkConfiguration::Active);
    bool hasWifiEnabled = false;
    for (const auto& config : activeConfigs) {
        // HACK: Wifi checks don't work as written for apple devices, for now do a vauge check for number of configs
#ifdef __APPLE__
        if (activeConfigs.size() > 1) {
            hasWifiEnabled = true;
        }
#endif
        if (config.bearerTypeName() == "WLAN" || config.bearerTypeName() == "Ethernet") {
            hasWifiEnabled = true;
        }
    }
    return hasWifiEnabled;
}

}
#endif // REACHABILITYUTILS_H
