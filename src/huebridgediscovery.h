#ifndef HUEBRIDGEDISCOVERY_H
#define HUEBRIDGEDISCOVERY_H

#include <QObject>
#include <QTimer>
#include <QUdpSocket>
#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QSettings>

#include <QJsonDocument>
#include <QJsonValue>
#include <QJsonObject>
#include <QJsonArray>

#include "hueprotocols.h"

/*!
 * \copyright
 * Copyright (C) 2015 - 2017.
 * Released under the GNU General Public License.
 *
 *
 * \brief The HueBridgeDiscovery class is an object that handles the discovery of a hue bridge by using
 *        a combination of UDP, HTTP, and JSON parsing.
 *
 *        Upon creation, it checks the persistent app data for the last bridge that was used by
 *        the application. If this passes a validity check, no extra discovery methods are called.
 *        If there is no data or if the current data is not valid, this object begins to go through
 *        the various EHueDiscoveryStates in order to achieve a valid connection. Developers only need
 *        to call startBridgeDiscovery() and stopBridgeDiscovery() in order to control it, it will
 *        automatically handle switching states as it receives and sends discovery packets.
 */
class HueBridgeDiscovery : public QObject
{
    Q_OBJECT
public:

    /*!
     * \brief HueBridgeDiscovery Constructor
     */
    explicit HueBridgeDiscovery(QObject *parent = 0);

    /*!
     * \brief Deconstructor
     */
    ~HueBridgeDiscovery();

    /*!
     * \brief startBridgeDiscovery checks if a bridge is currently discovered.
     *        If a bridge is connected, it emits a signal saying its connected.
     *        If a bridge is not connected, it checks its current data and starts
     *        the discovery process.
     */
    void startBridgeDiscovery();

    /*!
     * \brief stopBridgeDiscovery turns off discovery methods if they are currently
     *        active.
     */
    void stopBridgeDiscovery();

    /*!
     * \brief stopTimers turns off all discovery timers.
     */
    void stopTimers();

    /*!
     * \brief isConnected returns true if a bridge is connected and discovery was successful,
     *        false otherwise.
     * \return true if a bridge is connected and discovery was successful, false otherwise.
     */
    bool isConnected() { return (mDiscoveryState == EHueDiscoveryState::eBridgeConnected); }

    /*!
     * \brief bridge All currently known data about the hue bridge. This is only guarenteed to
     *        return valid data if isConnected() is returning true.
     * \return the struct that represents the current hue bridge.
     */
    SHueBridge bridge() { return mBridge; }

signals:

    /*!
     * \brief bridgeDiscoveryStateChanged int representation of the EHueDiscoveryState that
     *        the discovery object is currently in.
     */
    void bridgeDiscoveryStateChanged(int);

    /*!
     * \brief connectionStatusChanged If returning true, it has a valid Bridge that has had
     *        its connection tested. If returning false, the bridge it was using is no
     *        longer connected.
     */
    void connectionStatusChanged(bool);

private slots:

    /*!
     * \brief testBridgeIP called by a timer, sends a packet to the currently stored IP address
     *        to test for its validity and to request a username.
     */
    void testBridgeIP();

    /*!
     * \brief replyFinished called by the mNetworkManager, receives HTTP replies to packets
     *        sent from other methods.
     */
    void replyFinished(QNetworkReply*);

    /*!
     * \brief readPendingUPnPDatagrams caleld by the mUPnPSocket, receives UPnP packets and
     *        parses for a Hue Bridge.
     */
    void readPendingUPnPDatagrams();

    /*!
     * \brief handleDiscoveryTimeout Some discovery states have timeouts associated with them.
     *        This method handles when a timeout is called.
     */
    void handleDiscoveryTimeout();

private:
    /*!
     * \brief mBridge the current data for the bridge that is being discovered/has been discovered.
     */
    SHueBridge mBridge;

    /*!
     * \brief mDiscoveryState current state of discovery object.
     */
    EHueDiscoveryState mDiscoveryState;

    /*!
     * \brief mNetworkManager Qt's HTTP connection object
     */
    QNetworkAccessManager *mNetworkManager;

    /*!
     * \brief mSettings Device independent persistent application memory access
     */
    QSettings *mSettings;

    /*!
     * \brief mDiscoveryTimer timer used for repeating discovery events
     */
    QTimer *mDiscoveryTimer;
    /*!
     * \brief mTimeoutTimer single shot timer that determines when a discovery method is timing out.
     */
    QTimer *mTimeoutTimer;
    /*!
     * \brief mUPnPSocket Socket for UPnP discovery.
     */
    QUdpSocket *mUPnPSocket;

    /*!
     * \brief mHasKey true if a key has been found in the QSettings or discovered via
     *        discovery methods, false otherwise
     */
    bool mHasKey;

    /*!
     * \brief mHasKey true if an IP has been found in the QSettings or discovered via
     *        discovery methods, false otherwise
     */
    bool mHasIP;

    /*!
     * \brief mIPValid defaults to false, only gets set to true when a mesasge is
     *        received from a tested IP Address
     */
    bool mIPValid;
    /*!
     * \brief mUsernameValid defaults to false, only gets set to true when a mesasge is
     *        received that shows that the username gives access to state variables.
     */
    bool mUsernameValid;

    /*!
     * \brief attemptUPnPDiscovery attempts a UPnP connection by binding to the proper port
     *        and IP Address and waiting for packets.
     */
    void attemptUPnPDiscovery();
    /*!
     * \brief attemptNUPnPDiscovery attempts a NUPnP connection by sending a HTTP request
     *        to a phillip's server and waiting for a response.
     */
    void attemptNUPnPDiscovery();

    /*!
     * \brief attemptFinalCheck attempts a HTTP request to a given IP Address and Username and waits
     *        for an expected response to validate both the IP Address and Username.
     */
    void attemptFinalCheck();

    /*!
     * \brief attemptSearchForUsername sends an HTTP request to a discovered IP address and waits for
     *        it to respond with a new username.
     */
    void attemptSearchForUsername();

    /*!
     * \brief KPhillipsUsername Settings Key for a valid username.
     */
    const static QString kPhillipsUsername;

    /*!
     * \brief kPhillipsIPAddress Settings Key for the current Hue Bridge's IP Address.
     */
    const static QString kPhillipsIPAddress;

};

#endif // HUEBRIDGEDISCOVERY_H
