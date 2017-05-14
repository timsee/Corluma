#ifndef COMMHTTP_H
#define COMMHTTP_H

#include "commtype.h"

#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QTimer>

/*!
 * \copyright
 * Copyright (C) 2015 - 2017.
 * Released under the GNU General Public License.
 *
 * \brief provides a HTTP communication stream, which is
 * similar to an IP Camera. Packets are sent as HTTP
 * requests by appending them to the end of the HTTP
 * request's link.
 */

class CommHTTP : public CommType
{
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

    /*!
     * \brief sendPacket sends a packet in a way similar to am
     *        IP Camera: The packet is added to the end of the
     *        web address, and sent as an HTTP request
     * \param packet the string to be sent over HTTP.
     */
    void sendPacket(SDeviceController controller, QString packet);

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

    /*!
     * \brief discoveryRoutine sends a discovery packet to the currently
     *        connected serial port to test its connection.
     */
    void discoveryRoutine();

private:
    /*!
     * \brief mNetworkManager Qt's HTTP connection object
     */
    QNetworkAccessManager *mNetworkManager;

};

#endif // COMMHTTP_H
