#ifndef UPNPDISCOVERY_H
#define UPNPDISCOVERY_H

#include <QUdpSocket>
#include <QObject>

/*!
 * \copyright
 * Copyright (C) 2015 - 2019.
 * Released under the GNU General Public License.
 *
 *
 * \brief The UPnPDiscovery class listens for UPnP packets and signals them out when they are received.
 *        Rather than using the standard startup/shutdown methods that most of the backend classes use in this app,
 *        this object automatically turns on and off its socket based on how many listeners are requesting that it
 *        sends UPnP packets.
 */
class UPnPDiscovery: public QObject
{
    Q_OBJECT
public:
    /*!
     * \brief Constructor
     */
    UPnPDiscovery(QObject *parent);

    /*!
     * \brief addListener increments the listener count. If the count was at zero, it
     *        also starts up the UPnP socket.
     */
    void addListener();

    /*!
     * \brief removeListener decements the listner count. If the count reaches zero, it
     *         also shuts down the UPnP socket.
     */
    void removeListener();

signals:
    /*!
     * \brief UPnPPacketReceived sends ouit the QHostAddress and the payload associated
     *        with the UPnP packet received.
     */
    void UPnPPacketReceived(QHostAddress, QString);

private slots:

    /*!
     * \brief readPendingUPnPDatagrams caleld by the mUPnPSocket, receives UPnP packets and
     *        parses for a Hue Bridge.
     */
    void readPendingUPnPDatagrams();

private:
    /*!
     * \brief mSocket Qt's UDP object
     */
    QUdpSocket *mSocket;

    /// startup the UDP socket for listening for packets
    void startup();

    /// shutdown listening for UPnP packets
    void shutdown();

    /// counts the number of listeners for UPnP packets.
    uint32_t mListenerCount;
};

#endif // UPNPDISCOVERY_H
