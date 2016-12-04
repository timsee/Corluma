#ifndef LIGHTDEVICE_H
#define LIGHTDEVICE_H

#include <QString>
#include <QColor>
#include <QDebug>
#include "lightingprotocols.h"

#include <sstream>

/*!
 * \brief The ECommType enum The connection types
 *        supported by the GUI. For mobile builds,
 *        serial is not supported.
 */
enum class ECommType {
    eHTTP,
    eUDP,
    eHue,
#ifndef MOBILE_BUILD
    eSerial,
#endif //MOBILE_BUILD
    eCommType_MAX
};

/*!
 * \brief ECommTypeToString utility function for converting a ECommType to a human readable string
 * \param type the ECommType to convert
 * \return a simple english representation of the ECommType.
 */
QString static ECommTypeToString(ECommType type) {
    if (type ==  ECommType::eHTTP) {
        return "HTTP";
    } else if (type ==  ECommType::eUDP) {
        return "UDP";
    } else if (type ==  ECommType::eHue) {
        return "Hue";
#ifndef MOBILE_BUILD
    } else if (type == ECommType::eSerial) {
        return "Serial";
#endif //MOBILE_BUILD
    } else {
        return "CommType not recognized";
    }
}


struct SLightDevice {
    /*!
     * \brief isReachable true if we can communicate with it, false otherwise
     */
    bool isReachable;
    /*!
     * \brief isOn true if the light is shining any color, false if turned
     *        off by software. By using a combination of isReachable and isOn
     *        you can determine if the light is on and shining, off by software
     *        and thus able to be turned on by software again, or off by hardware
     *        and needs the light switch to be hit in order to turn it on.
     */
    bool isOn;

    /*!
     * \brief isValid true if the device can be considered a valid packet, false
     *        otherwise.
     */
    bool isValid;

    /*!
     * \brief color color of this device.
     */
    QColor color;
    /*!
     * \brief lightingRoutine current lighting routine for this device.
     */
    ELightingRoutine lightingRoutine;
    /*!
     * \brief colorGroup color group for this device.
     */
    EColorGroup colorGroup;
    /*!
     * \brief brightness brightness for this device, between 0 and 100.
     */
    int brightness;

    //-----------------------
    // Connection Info
    //-----------------------

    /*!
     * \brief index the index of the hue, each bridge gives an index for all of the
     *        connected hues.
     */
    int index;

    /*!
     * \brief type determines whether the connection is based off of a hue, UDP, HTTP, etc.
     */
    ECommType type;
    /*!
     * \brief name the name of the connection. This varies by connection type. For example,
     *        a UDP connection will use its IP address as a name, or a serial connection
     *        will use its serial port.
     */
    QString name;


    /*!
     * \brief printLightDevice prints values of struct. used for debugging.
     */
    void printLightDevice() const {
        qDebug() << "SLight Device: "
                 << "isReachable: " << isReachable << "\n"
                 << "isOn: " << isOn << "\n"
                 << "isValid: " << isValid << "\n"
                 << "color: " << color  << "\n"
                 << "lightingRoutine: " << (int)lightingRoutine << "\n"
                 << "colorGroup: " << (int)colorGroup << "\n"
                 << "brightness: " << brightness << "\n"
                 << "index: " << index << "\n"
                 << "Type: " << ECommTypeToString(type) << "\n"
                 << "name: " << name << "\n";
    }


    /*!
     * \brief structToString converts a SLightDevice struct to a string in the format
     *        of comma delimited values.
     * \param dataStruct the struct to convert to a string
     * \return a comma delimited string that represents all values in the SLightDevice.
     */
    QString static structToString(SLightDevice dataStruct) {
        QString returnString = "";
        returnString = returnString + dataStruct.name + "," + QString::number(dataStruct.index) + "," + QString::number((int)dataStruct.type);
        return returnString;
    }

    /*!
     * \brief stringToStruct converts a string represention of a SControllerCommData
     *        back to a struct.
     * \param string the string to convert
     * \return a SLightDevice struct based on the string given. an empty struct is returned if
     *         the string is invalid.
     */
    SLightDevice static stringToStruct(QString string) {
        // first split the values from comma delimited to a vector of strings
        std::vector<std::string> valueVector;
        std::stringstream input(string.toStdString());
        while (input.good()) {
            std::string value;
            std::getline(input, value, ',');
            valueVector.push_back(value);
        }
        // check validity
        SLightDevice outputStruct;
        if (valueVector.size() == 3) {
            outputStruct.name = QString::fromStdString(valueVector[0]);
            outputStruct.index = QString::fromStdString(valueVector[1]).toInt();
            outputStruct.type = (ECommType)QString::fromStdString(valueVector[2]).toInt();
            if (outputStruct.type == ECommType::eHue) {
                outputStruct.name = "Bridge";
            }
        } else {
            qDebug() << "something went wrong with the key...";
        }
        return outputStruct;
    }

};

inline bool operator==(const SLightDevice& lhs, const SLightDevice& rhs)
{
    bool result = true;
    if (lhs.isReachable     !=  rhs.isReachable) result = false;
    if (lhs.isOn            !=  rhs.isOn) result = false;
    if (lhs.isValid         !=  rhs.isValid) result = false;
    if (lhs.color           !=  rhs.color) result = false;
    if (lhs.lightingRoutine !=  rhs.lightingRoutine) result = false;
    if (lhs.colorGroup      !=  rhs.colorGroup) result = false;
    if (lhs.brightness      !=  rhs.brightness) result = false;
    if (lhs.index           !=  rhs.index) result = false;
    if (lhs.type            !=  rhs.type) result = false;
    if (lhs.name.compare(rhs.name)) result = false;

    return result;
}

#endif // LIGHTDEVICE_H
