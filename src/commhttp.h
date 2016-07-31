#ifndef COMMHTTP_H
#define COMMHTTP_H

#include "commtype.h"

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
     * \brief sendPacket sends a packet in a way similar to am
     *        IP Camera: The packet is added to the end of the
     *        web address, and sent as an HTTP request
     * \param packet the string to be sent over HTTP.
     */
    void sendPacket(QString packet);

private slots:
    /*!
     * \brief replyFinished called by the mNetworkManager, receives HTTP replies to packets
     *        sent from other methods.
     */
    void replyFinished(QNetworkReply*);

private:
    /*!
     * \brief mNetworkManager Qt's HTTP connection object
     */
    QNetworkAccessManager *mNetworkManager;

};

#endif // COMMHTTP_H
