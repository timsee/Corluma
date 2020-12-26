#ifndef COR_UTILS_REACHABILITY_H
#define COR_UTILS_REACHABILITY_H

/*!
 * \copyright
 * Copyright (C) 2015 - 2020.
 * Released under the GNU General Public License.
 */


#include <QHostAddress>
#include <QNetworkInterface>


//#define DEBUG_REACHABILITY

namespace cor {

/*!
 * \brief wifiEnabled uses the QNetworkConfiguratyionManager to scan whether or not there is a wifi
 * connection
 *
 * \return true if there is a wifi connection, false otherwise.
 */
inline bool wifiEnabled() {
    bool hasWifiEnabled = false;
    for (const QNetworkInterface& iface : QNetworkInterface::allInterfaces()) {
        // HACK: Wifi checks don't work as written for apple devices, for now do a vauge check for
        // number of configs
        //#ifdef __APPLE__
        //        if (activeConfigs.size() > 1) {
        //            hasWifiEnabled = true;
        //        }
        //#endif
        if ((iface.type() == QNetworkInterface::Wifi || iface.type() == QNetworkInterface::Ethernet)
            && iface.flags().testFlag(QNetworkInterface::IsUp)
            && iface.flags().testFlag(QNetworkInterface::IsRunning)
            && !iface.humanReadableName().contains("dummy")) {
            hasWifiEnabled = true;
#ifdef DEBUG_REACHABILITY
            qDebug() << iface.humanReadableName() << "(" << iface.name() << ")"
                          << "is up:" << iface.flags().testFlag(QNetworkInterface::IsUp)
                          << "is running:" << iface.flags().testFlag(QNetworkInterface::IsRunning);
#endif
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

} // namespace cor
#endif // COR_UTILS_REACHABILITY_H
