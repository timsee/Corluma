#ifndef COMMUDP_H
#define COMMUDP_H

#include <QUdpSocket>
#include <QTimer>

#include "commtype.h"

/*!
 * \copyright
 * Copyright (C) 2015 - 2017.
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

    /*!
     * \brief sendPacket sends the packet over UDP to a specified
     *        IP addres and port. Returns immediately and buffers
     *        unsent packets.
     * \param packet the string that is going to get sent over UDP.
     */
    void sendPacket(QString controller, QString packet);

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
     * \brief mBound true if already bound, false otherwise.
     */
    bool mBound;
};

#endif // COMMUDP_H
