#ifndef COMMPACKETPARSER_H
#define COMMPACKETPARSER_H

#include <QWidget>
#include <QColor>

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
 *        functionality of the parser that is embedded on the arduino platforms on devices like the Phillips Hue.
 */
class CommPacketParser: public QWidget
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
     * \brief receivedMainColorChange signal that requests a main color change for a device.
     * \param deviceIndex 0 for all indices, a specific index for a specific light.
     *        Will do nothing if index doesn't exist.
     * \param color the new color for the device.
     */
    void receivedMainColorChange(int deviceIndex, QColor color);

    /*!
     * \brief receivedArrayColorChange signal that requests that the custom array changes a color.
     * \param deviceIndex 0 for all indices, a specific index for a specific light.
     *        Will do nothing if index doesn't exist.
     * \param index the custom array color index. must be less than 10.
     * \param color the new color for the device's custom color index.
     */
    void receivedArrayColorChange(int deviceIndex, int index, QColor color);

    /*!
     * \brief receivedRoutineChange signal that requests a routine change for a device.
     * \param deviceIndex 0 for all indices, a specific index for a specific light.
     *        Will do nothing if index doesn't exist.
     * \param routine the new color routine.
     * \param colorGroupUsed the color group to use for the custom routine. If its a single
     *        color routine, this value will be -1.
     */
    void receivedRoutineChange(int deviceIndex, int routine, int colorGroupUsed);

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
     * \brief receivedSpeedChange signal that requests a light speed change.
     * \param deviceIndex 0 for all indices, a specific index for a specific light.
     *        Will do nothing if index doesn't exist.
     * \param speed a value between 1 and 2000. To get the FPS, take this value and
     *        divide it by 100. For example, 500 would be 5 FPS.
     */
    void receivedSpeedChange(int deviceIndex, int speed);

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

    /*!
     * \brief receivedReset reset all lights to their default settings.
     */
    void receivedReset();
};

#endif // COMMPACKETPARSER_H
