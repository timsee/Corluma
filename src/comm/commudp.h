#ifndef COMMUDP_H
#define COMMUDP_H

#include <QUdpSocket>
#include <QTimer>

#include "commtype.h"
#include "arducor/arducordiscovery.h"
#include "arducor/crccalculator.h"

/*!
 * \copyright
 * Copyright (C) 2015 - 2018.
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
     * \brief startup defined in CommType
     */
    void startup();

    /*!
     * \brief shutdown defined in CommType
     */
    void shutdown();

    /// connects discovery object
    void connectDiscovery(ArduCorDiscovery *discovery) { mDiscovery = discovery; }

    /*!
     * \brief sendPacket sends the packet over UDP to a specified
     *        IP addres and port. Returns immediately and buffers
     *        unsent packets.
     * \param packet the string that is going to get sent over UDP.
     */
    void sendPacket(const cor::Controller& controller, QString& packet);

    /// true is port is successfully bound, false if errors occur.
    bool portBound();

    /*!
     * \brief testForController sends a discovery packet to the currently
     *        connected serial port to test its connection.
     */
    void testForController(const cor::Controller& controller);

signals:
    /*!
     * \brief packetReceived emitted whenever a packet that is not a discovery packet is received. Contains
     *        the full packet's contents as a QString.
     */
    void packetReceived(QString, QString, ECommType);


private slots:
    /*!
     * \brief readPendingDatagrams reads and parses incoming data packets.
     */
    void readPendingDatagrams();

    /*!
     * \brief stateUpdate used by the mStateUpdateTimer to request new
     *        state updates from the currently connected lights.
     */
    void stateUpdate();

private:

    /// discovery object for storing previous connections, saving new connections, parsing discovery packets
    ArduCorDiscovery *mDiscovery;

    /// used to check CRC on incoming packets.
    CRCCalculator mCRC;

    /*!
     * \brief mSocket Qt's UDP object
     */
    QUdpSocket *mSocket;

    /*!
     * \brief mBound true if already bound, false otherwise.
     */
    bool mBound;
};

#endif // COMMUDP_H
