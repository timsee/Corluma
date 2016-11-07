#ifndef COMMUDP_H
#define COMMUDP_H

#include "commtype.h"
#include "commthrottle.h"

#include <QUdpSocket>
#include <QTimer>

/*!
 * \copyright
 * Copyright (C) 2015 - 2016.
 * Released under the GNU General Public License.
 *
 * \brief Provides a UDP communication stream which allows
 * the sending of UDP datagrams to a specific IP Address
 * and port.
 */

class CommUDP : public CommType
{
    Q_OBJECT
public:
    /*!
     * \brief CommUDP Constructor
     */
    CommUDP();
    /*!
     * \brief CommUDP Deconstructor
     */
    ~CommUDP();

    /*!
     * \brief changeConnection changes the IP address that is currently connected.
     * \param connectionName the IP address that you want to connect to.
     */
    void changeConnection(QString connectionName);

    /*!
     * \brief sendPacket sends the packet over UDP to a specified
     *        IP addres and port. Returns immediately and buffers
     *        unsent packets.
     * \param packet the string that is going to get sent over UDP.
     */
    void sendPacket(QString packet);

    /*!
     * \brief closeConnection unnecesary, implemented
     *        only to comply with being a CommType, since
     *        UDP is connectionless.
     */
    void closeConnection();

private slots:
    /*!
     * \brief readPendingDatagrams reads and parses incoming data packets.
     */
    void readPendingDatagrams();
    /*!
     * \brief discoveryRoutine sends a discovery packet in order to test a connection.
     */
    void discoveryRoutine();

    /*!
     * \brief sendThrottleBuffer response to the throttle buffer wanting to clear itself.
     */
    void sendThrottleBuffer(QString);

    /*!
     * \brief stateUpdate used by the mStateUpdateTimer to request new
     *        state updates from the currently connected lights.
     */
    void stateUpdate();

private:
    /*!
     * \brief mSocket Qt's UDP object
     */
    QUdpSocket *mSocket;

    /*!
     * \brief mDiscoveryTimer used during discovery to poll the device every few seconds.
     */
    QTimer *mDiscoveryTimer;

    /*!
     * \brief mBound true if already bound, false otherwise.
     */
    bool mBound;

    /*!
     * \brief mThrottle used to prevent the communication stream from sending commands too frequently.
     */
    CommThrottle *mThrottle;

    /*!
     * \brief mBufferedConnection buffer of last UDP IP address used.
     */
    QString mBufferedConnection;
};

#endif // COMMUDP_H
