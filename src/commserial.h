#ifndef SERIALCOMM_H
#define SERIALCOMM_H

#ifndef MOBILE_BUILD

#include "commtype.h"
#include "commthrottle.h"

#include <QSerialPort>
#include <QSerialPortInfo>
#include <QTimer>
#include <memory>

/*!
 * \copyright
 * Copyright (C) 2015 - 2017.
 * Released under the GNU General Public License.
 *
 *
 * \brief A serial conenction using QSerialPort. This connection
 * will not work in mobile devices since QSerialPort is
 * unimplemented (for pretty obvious reasons :P). It is the
 * fastest and most stable connection on PCs.
 */

class CommSerial : public CommType
{
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

    /*!
     * \brief sendPacket sends a string over serial
     * \param packet the string that is going to be sent over
     *        serial.
     */
    void sendPacket(QString controller, QString packet);

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
     * \brief discoveryRoutine sends a discovery packet to the currently
     *        connected serial port to test its connection.
     */
    void discoveryRoutine();

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
    QSerialPort *serialPortByName(QString name);

    /*!
     * \brief mSerialList list of possible serial ports
     *        for connection
     */
    std::list<QSerialPortInfo> mSerialInfoList;

    /*!
     * \brief mSerialPorts list of serial ports in use. The second element in the pair is a buffer that contains the
     *        most recent characters sent by the serial port.
     */
    std::list<std::pair<QSerialPort*, QString> > mSerialPorts;
};

#endif // MOBILE_BUILD
#endif // SERIALCOMM_H
