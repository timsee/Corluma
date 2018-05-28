#ifndef COMMPACKETPARSER_H
#define COMMPACKETPARSER_H

#include <QWidget>
#include <QColor>
#include <QJsonObject>
#include "lightingprotocols.h"

/*!
 * \copyright
 * Copyright (C) 2015 - 2018.
 * Released under the GNU General Public License.
 *
 *
 * \brief The CommPacketParser class takes the packets used for communication with other devices as input,
 *        parses them, and emits signals based on the contents of a packet. For example, a main color change
 *        packet will emit a `receivedMainColorChange(int, QColor)`  signal. This class is used to emulate the
 *        functionality of the parser that is embedded on the arduino platforms on devices like the Philips Hue.
 */
class CommPacketParser: public QObject
{
    Q_OBJECT

public:
    /*!
     * \brief CommPacketParser constructor
     */
    CommPacketParser(QWidget *parent = 0);

    /*!
     * \brief parsePacket take a packet that is assumed to be a properly formatted command packet
     *        and attempts to parse it. If the command is valid, it emits a signal. If the command is invalid,
     *        nothing happens.
     * \param packet the command packet being used as input
     */
    void parsePacket(QString packet);

signals:

    /*!
     * \brief receivedOnOffChange received a message to turn light on or off.
     * \param deviceIndex index of device to control
     * \param turnOn true to turn on, false to turn off
     */
    void receivedOnOffChange(int deviceIndex, bool turnOn);

    /*!
     * \brief receivedArrayColorChange signal that requests that the custom array changes a color.
     * \param deviceIndex 0 for all indices, a specific index for a specific light.
     *        Will do nothing if index doesn't exist.
     * \param index the custom array color index. must be less than 10.
     * \param color the new color for the device's custom color index.
     */
    void receivedArrayColorChange(int deviceIndex, int index, QColor color);

    /*!
     * \brief receivedRoutineChange signal that requests that the lighting routine change.
     * \param deviceIndex 0 for all indices, a specific index for a specific light.
     *        Will do nothing if index doesn't exist.
     * \param routineObject json object representing the new routine for the device
     */
    void receivedRoutineChange(int deviceIndex, QJsonObject routineObject);

    /*!
     * \brief receivedCustomArrayCount signal that requests change of te count of colors
     *        used from the custom color array.
     * \param deviceIndex 0 for all indices, a specific index for a specific light.
     *        Will do nothing if index doesn't exist.
     * \param count the new count for the custom color array.
     */
    void receivedCustomArrayCount(int deviceIndex, int count);

    /*!
     * \brief receivedBrightnessChange signal that requests a light brightness change
     * \param deviceIndex 0 for all indices, a specific index for a specific light.
     *        Will do nothing if index doesn't exist.
     * \param brightness a value between 0 and 100, 0 is off, 100 is full brightness
     */
    void receivedBrightnessChange(int deviceIndex, int brightness);

    /*!
     * \brief receivedTimeOutChange signal that requests that the idle timeout of the hardware
     *        changes. A value of 0 will disable the timeout, each other value will be the number
     *        of minutes it takes to time out.
     * \param deviceIndex 0 for all indices, a specific index for a specific light.
     *        Will do nothing if index doesn't exist.
     * \param timeOut value between 0 and 1000. number represents number of minutes before
     *        lights automatically turn off. 0 disables this feature.
     */
    void receivedTimeOutChange(int deviceIndex, int timeOut);

private:

    /// parses an int vector representation of a routine object.
    void routineChange(const std::vector<int>& intVector);
};

#endif // COMMPACKETPARSER_H
