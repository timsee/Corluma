#ifndef ARDUCORDISCOVERY_H
#define ARDUCORDISCOVERY_H

#include <QElapsedTimer>
#include <QObject>
#include <QTimer>

#include "comm/arducor/arducormetadata.h"
#include "comm/arducor/controller.h"
#include "cor/dictionary.h"
#include "cor/jsonsavedata.h"

class CommUDP;
class CommHTTP;
#ifdef USE_SERIAL
class CommSerial;
#endif
/*!
 * \copyright
 * Copyright (C) 2015 - 2020.
 * Released under the GNU General Public License.
 *
 *
 * \brief The ArduCorDiscovery class runs the discovery routines for ArduCor controllers.
 *        It does not actually connect to serial ports or sockets. Instead it uses existing
 *        CommHTTP, CommUDP, and CommSerial to handle the communication protocols. Instead,
 *        it stores, saves, and verifies discovery packets
 */
class ArduCorDiscovery : public QObject, public cor::JSONSaveData {
    Q_OBJECT
public:
    /// constructor
    explicit ArduCorDiscovery(QObject* parent,
                              CommHTTP* http,
                              CommUDP* udp
#ifdef USE_SERIAL
                              ,
                              CommSerial* serial
#endif
    );

    /// starts dicsovery
    void startDiscovery();

    /// ends discovery
    void stopDiscovery();

    /*!
     * \brief addManualIP attempts to connect to an IP address entered manually
     * \param ip new ip address to attempt.
     */
    void addManualIP(const QString& ip);

#ifdef USE_SERIAL
    /// adds a serial port to notFound list
    void addSerialPort(const QString& serialName);
#endif

    /// handles an incoming packet and checks if its a discovery packet. If it is, it treats it as
    /// such and parses it.
    void handleIncomingPacket(ECommType type,
                              const QString& controllerName,
                              const QString& payload);

    /// getter for list of controllers
    const cor::Dictionary<cor::Controller>& controllers() const { return mFoundControllers; }

    /// getter for list of undiscovered controllers
    const std::vector<cor::Controller>& undiscoveredControllers() const {
        return mNotFoundControllers;
    }

    /// finds the device name of based off its controller and index name.
    QString findDeviceNameByIndexAndControllerName(const QString& controllerName, uint32_t index);

    /// finds a controller based on a device name
    std::pair<cor::Controller, bool> findControllerByDeviceName(const QString& deviceName);

    /// finds a found controller based on its name
    std::pair<cor::Controller, bool> findFoundControllerByControllerName(const QString& name);

    std::pair<cor::Controller, bool> findControllerByControllerName(const QString& controllerName);

    /*!
     * \brief setupConnectionList initializes the connection list and reloads
     *        it from system memory, if needed
     * \param type the ECommType of this specific connection.
     */
    void setupConnectionList(ECommType type);

    /// removes controller from data and saved data
    bool removeController(const QString& controller);

    /// returns true if the IP already exists in either not found or found controllers.
    bool doesIPExist(const QString& ip);

    /// load the json data.
    bool loadJSON() override;

    /// string at the beginning of each discovery packet.
    const static QString kDiscoveryPacketIdentifier;

    /// the time the last arducor light was fully discovered.
    QTime lastDiscoveryTime() const noexcept { return mLastDiscoveryTime; }

private slots:

    /// slot for when the startup timer times out
    void startupTimerTimeout();

    /// runs the discovery routines.
    void handleDiscovery();

private:
    /// update json data that stores the connection and controlelr info
    void updateJSON(const cor::Controller& controller);

    /// list of not found controllers
    std::vector<cor::Controller> mNotFoundControllers;

    /// list of found controllers
    cor::Dictionary<cor::Controller> mFoundControllers;

    /// helper that moves a controller from not found to found and saves changes to JSON.
    void handleDiscoveredController(const cor::Controller& discoveredController);

    /*!
     * \brief controllerFromDiscoveryString takes a discovery string, a controller name, and
     * an empty cor::Controller as input. If parsing the string  is successful, it fills the
     * cor::Controller with the info from the discovery string. If its unsucessful, it returns
     * false.
     * \param discovery string received as discovery string
     * \param controllerName name of controller
     * \param controller filled if discovery string is valid.
     * \return true if discovery string is valid, false otherwise.
     */
    std::pair<cor::Controller, bool> controllerFromDiscoveryString(ECommType type,
                                                                   const QString& discovery,
                                                                   const QString& controllerName);

    /// pointer to object that handles the HTTP communication
    CommHTTP* mHTTP;

    /// pointer to object that handles the UDP communication
    CommUDP* mUDP;

#ifdef USE_SERIAL
    /// pointer to object that handles the serial communication
    CommSerial* mSerial;
#endif

    /*!
     * \brief mRoutineTimer single shot timer that determines when a discovery method is timing out.
     */
    QTimer* mRoutineTimer;

    /// elapse timer checks how long its been since certain updates
    QElapsedTimer* mElapsedTimer;

    /// tracks last time
    uint32_t mLastTime;

    /// flag that checks if more than 2 minutes have passed
    bool mStartupTimerFinished = false;

    /// the time the last arducor light was fully discovered.
    QTime mLastDiscoveryTime;

    /*!
     * \brief mStartupTimer in the first two minutes of the app's lifecycle, if nanoleaf is enabled
     *        it will scan for nanoleafs. This allows hardware changes to be picked up more easily
     */
    QTimer* mStartupTimer;
};

#endif // ARDUCORDISCOVERY_H
