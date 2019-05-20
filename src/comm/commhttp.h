#ifndef COMMHTTP_H
#define COMMHTTP_H

#include "comm/arducor/arducordiscovery.h"
#include "comm/arducor/crccalculator.h"
#include "commtype.h"

#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QTimer>

/*!
 * \copyright
 * Copyright (C) 2015 - 2019.
 * Released under the GNU General Public License.
 *
 * \brief provides a HTTP communication stream, which is
 * similar to an IP Camera. Packets are sent as HTTP
 * requests by appending them to the end of the HTTP
 * request's link.
 */
class CommHTTP : public CommType {
    Q_OBJECT
public:
    /*!
     * \brief CommHTTP Constructor
     */
    CommHTTP();
    /*!
     * \brief CommHTTP Deconstructor
     */
    ~CommHTTP();

    /*!
     * \brief startup defined in CommType
     */
    void startup();

    /*!
     * \brief shutdown defined in CommType
     */
    void shutdown();

    /// connects discovery object
    void connectDiscovery(ArduCorDiscovery* discovery) { mDiscovery = discovery; }

    /*!
     * \brief sendPacket sends a packet in a way similar to am
     *        IP Camera: The packet is added to the end of the
     *        web address, and sent as an HTTP request
     * \param packet the string to be sent over HTTP.
     */
    void sendPacket(const cor::Controller& controller, QString& packet);

    /*!
     * \brief testForController sends a discovery packet to the currently
     *        connected serial port to test its connection.
     */
    void testForController(const cor::Controller& controller);

signals:
    /*!
     * \brief packetReceived emitted whenever a packet that is not a discovery packet is received.
     * Contains the full packet's contents as a QString.
     */
    void packetReceived(QString, QString, ECommType);

private slots:
    /*!
     * \brief replyFinished called by the mNetworkManager, receives HTTP replies to packets
     *        sent from other methods.
     */
    void replyFinished(QNetworkReply*);

    /*!
     * \brief stateUpdate used by the mStateUpdateTimer to request new
     *        state updates from the currently connected lights.
     */
    void stateUpdate();

private:
    /*!
     * \brief mNetworkManager Qt's HTTP connection object
     */
    QNetworkAccessManager* mNetworkManager;

    /// used to check CRC on incoming packets.
    CRCCalculator mCRC;

    /// discovery object for storing previous connections, saving new connections, parsing discovery
    /// packets
    ArduCorDiscovery* mDiscovery;
};

#endif // COMMHTTP_H
