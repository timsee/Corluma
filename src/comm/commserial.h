#ifndef SERIALCOMM_H
#define SERIALCOMM_H

#include "arducor/arducordiscovery.h"
#include "arducor/crccalculator.h"
#include "commtype.h"

#include <QSerialPort>
#include <QSerialPortInfo>
#include <QTimer>
#include <memory>

/*!
 * \copyright
 * Copyright (C) 2015 - 2019.
 * Released under the GNU General Public License.
 *
 *
 * \brief A serial conenction using QSerialPort. This connection
 * will not work in mobile devices since QSerialPort is
 * unimplemented (for pretty obvious reasons :P). It is the
 * fastest and most stable connection on PCs.
 */

class CommSerial : public CommType {
    Q_OBJECT
public:
    /*!
     * \brief CommSerial Constructor
     */
    CommSerial();
    /*!
     * \brief CommSerial Deconstructor
     */
    ~CommSerial();

    /*!
     * \brief discoverSerialPorts looks for new serial ports and adds it them to
     *        the connections page, if they are found.
     */
    void discoverSerialPorts();

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
     * \brief sendPacket sends a string over serial
     * \param packet the string that is going to be sent over serial.
     */
    void sendPacket(const cor::Controller& controller, QString& packet);

    /*!
     * \brief lookingForActivePorts true if looking for active ports, false otherwise.
     * \return true if looking for active ports, false otherwise.
     */
    bool lookingForActivePorts() { return mLookingForActivePorts; }

    /// returns true if a serial port has failed.
    bool serialPortErrorsExist() { return mSerialPortFailed; }

    /*!
     * \brief testForController sends a discovery packet to the currently
     *        connected serial port to test its connection.
     */
    void testForController(const cor::Controller& controller);

signals:
    /*!
     * \brief packetReceived emitted whenever a packet that is not a discovery packet is received.
     * Contains
     *        the full packet's contents as a QString.
     */
    void packetReceived(QString, QString, ECommType);

private slots:

    /*!
     * \brief handleReadyRead parses incoming packets
     */
    void handleReadyRead();
    /*!
     * \brief handleError handles errors from the QSerialPort.
     */
    void handleError(QSerialPort::SerialPortError);

    /*!
     * \brief stateUpdate used by the mStateUpdateTimer to request new
     *        state updates from the currently connected lights.
     */
    void stateUpdate();

private:
    /// discovery object for storing previous connections, saving new connections, parsing discovery
    /// packets
    ArduCorDiscovery* mDiscovery;

    /// used to check CRC on incoming packets.
    CRCCalculator mCRC;

    /*!
     * \brief connectSerialPort connect to a specific serial port, if possible.
     * \param serialPortName The name of the serial port that you want
     *        to connect to.
     * \return true if connection is successful, false otherwise.
     */
    bool connectSerialPort(const QSerialPortInfo& info);

    /*!
     * \brief serialPortByName pointer to QSerialPort instance that is connectd
     *        to the given serial port. Gives back a nullptr if no QSerialPort is found
     * \param name name fo QSerialPort
     * \return pointer to QSerialPort
     */
    QSerialPort* serialPortByName(const QString& name);

    /*!
     * \brief mSerialList list of possible serial ports
     *        for connection
     */
    std::list<QSerialPortInfo> mSerialInfoList;

    /*!
     * \brief mSerialPorts list of serial ports in use. The second element in the pair is a buffer
     * that contains the most recent characters sent by the serial port.
     */
    std::list<std::pair<QSerialPort*, QString>> mSerialPorts;

    /// set to true when looking for active ports, used on discovery page.
    bool mLookingForActivePorts;

    /// true if any serial port has failed, false otherwise
    bool mSerialPortFailed;
};

#endif // SERIALCOMM_H
