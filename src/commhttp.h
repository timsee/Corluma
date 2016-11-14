#ifndef COMMHTTP_H
#define COMMHTTP_H

#include "commtype.h"
#include "commthrottle.h"

#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QNetworkReply>

/*!
 * \copyright
 * Copyright (C) 2015 - 2016.
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
     * \brief closeConnection close current connection. Doesn't do anything
     *        in HTTP but is used by most connection protocols. in HTTP, the
     *        connection is reopened and closed for every packet.
     */
    void closeConnection();

    /*!
     * \brief changeConnection change to a new connection
     * \param newConnection the new connection to use
     * \TODO refactora way
     */
    void changeConnection(QString newConnection);

    /*!
     * \brief sendPacket sends a packet in a way similar to am
     *        IP Camera: The packet is added to the end of the
     *        web address, and sent as an HTTP request
     * \param packet the string to be sent over HTTP.
     */
    void sendPacket(QString controller, QString packet);


private slots:
    /*!
     * \brief replyFinished called by the mNetworkManager, receives HTTP replies to packets
     *        sent from other methods.
     */
    void replyFinished(QNetworkReply*);

    /*!
     * \brief sendThrottleBuffer response to the throttle buffer wanting to clear itself.
     */
    void sendThrottleBuffer(QString, QString);

    /*!
     * \brief stateUpdate used by the mStateUpdateTimer to request new
     *        state updates from the currently connected lights.
     */
    void stateUpdate();

private:
    /*!
     * \brief mNetworkManager Qt's HTTP connection object
     */
    QNetworkAccessManager *mNetworkManager;

    /*!
     * \brief mThrottle used to prevent the communication stream from sending commands too frequently.
     */
    CommThrottle *mThrottle;

    /*!
     * \brief mConnection current HTTP address being used
     * \TODO refactor away
     */
    QString mConnection;


};

#endif // COMMHTTP_H
