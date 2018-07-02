#ifndef BRIDGE_H
#define BRIDGE_H

#include <QString>
#include <QJsonObject>
#include <sstream>

namespace hue
{

/*!
 * \copyright
 * Copyright (C) 2015 - 2018.
 * Released under the GNU General Public License.
 *
 * \brief The Bridge class stores useful information about a hue::Bridge for discovery and for connection purposes.
 */
class Bridge
{
public:
    /// constructor
    Bridge();

    /*!
     * \brief IP The IP address of the current bridge
     */
    QString IP = "";
    /*!
     * \brief username the username assigned by the bridge. Received
     *        by sending a request packet ot the bridge.
     */
    QString username = "";

    /// unique key
    QString id = "";

    /// name of bridge
    QString name = "";

    /// mac address for the bridge
    QString macaddress = "";

    operator QString() const {
        std::stringstream tempString;
        tempString << "hue::Bridge: "
                   << " IP: " << IP.toStdString()
                   << " username: " << username.toStdString()
                   << " id:" <<  id.toStdString()
                   << " name:" <<  name.toStdString()
                   << " macaddress:" <<  macaddress.toStdString();
        return QString::fromStdString(tempString.str());
    }

    /// equal operator
    bool operator==(const hue::Bridge& rhs) const {
        bool result = true;
        if (IP        !=  rhs.IP)       result = false;
        if (username  !=  rhs.username) result = false;
        if (id        !=  rhs.id)       result = false;
        return result;
    }
};

/// converts a json object to a Bridge
Bridge jsonToBridge(const QJsonObject& object);

/// converts a bridge to a json object
QJsonObject bridgeToJson(const Bridge& controller);

}


#endif // BRIDGE_H
