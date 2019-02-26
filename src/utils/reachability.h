#ifndef COR_UTILS_REACHABILITY_H
#define COR_UTILS_REACHABILITY_H

/*!
 * \copyright
 * Copyright (C) 2015 - 2019.
 * Released under the GNU General Public License.
 */


#include <QNetworkConfigurationManager>
#include <QHostAddress>

namespace cor
{

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
// Windows doesn't really work at all... this is a bad check if its not a mobile phone.
#ifdef Q_OS_WIN
    hasWifiEnabled = true;
#endif

    return hasWifiEnabled;
}

/// hacky check as to whether or not an IP address is valid
inline bool checkIfValidIP(const QString& ip) {
    QHostAddress address(ip);
    if (QAbstractSocket::IPv4Protocol == address.protocol()) {
        return true;
    } else if (QAbstractSocket::IPv6Protocol == address.protocol()) {
        return true;
    } else {
        return false;
    }
}

}
#endif // COR_UTILS_REACHABILITY_H
