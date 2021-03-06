#ifndef UPNPDISCOVERY_H
#define UPNPDISCOVERY_H

#include <QObject>
#include <QUdpSocket>

/*!
 * \copyright
 * Copyright (C) 2015 - 2020.
 * Released under the GNU General Public License.
 *
 *
 * \brief The UPnPDiscovery class listens for UPnP packets and signals them out when they are
 * received. Rather than using the standard startup/shutdown methods that most of the backend
 * classes use in this app, this object automatically turns on and off its socket based on how many
 * listeners are requesting that it sends UPnP packets.
 */
class UPnPDiscovery : public QObject {
    Q_OBJECT
public:
    /*!
     * \brief Constructor
     */
    UPnPDiscovery(QObject* parent);

    ~UPnPDiscovery();

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

    /// true if UPnP is connected, false otherwise
    bool isStateConnected() { return mSocket->state() == QAbstractSocket::BoundState; }

    /// true if it has received any traffic
    bool hasReceivedTraffic() { return mHasReceivedTraffic; }

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
    QUdpSocket* mSocket;

    /// startup the UDP socket for listening for packets
    void startup();

    /// shutdown listening for UPnP packets
    void shutdown();

    /// counts the number of listeners for UPnP packets.
    std::uint32_t mListenerCount;

    /// true if it has received any traffic
    bool mHasReceivedTraffic;
};

#endif // UPNPDISCOVERY_H
