/*!
 * \copyright
 * Copyright (C) 2015 - 2020.
 * Released under the GNU General Public License.
 */

#include "arducorpacketparser.h"

#include <QDebug>
#include <sstream>

#include "cor/protocols.h"

#define CUSTOM_COLOR_MAX 10

ArduCorPacketParser::ArduCorPacketParser(QObject* parent, PaletteData* palettes)
    : QObject(parent),
      mPalettes{palettes} {}


QString ArduCorPacketParser::turnOnPacket(int index, bool turnOn) {
    QString packet = QString("%1,%2,%3&")
                         .arg(QString::number(int(EPacketHeader::onOffChange)),
                              QString::number(index),
                              QString::number(turnOn));
    return packet;
}

QString ArduCorPacketParser::arrayColorChangePacket(int deviceIndex,
                                                    int colorIndex,
                                                    const QColor& color) {
    QString packet = QString("%1,%2,%3,%4,%5,%6&")
                         .arg(QString::number(int(EPacketHeader::customArrayColorChange)),
                              QString::number(deviceIndex),
                              QString::number(colorIndex),
                              QString::number(color.red()),
                              QString::number(color.green()),
                              QString::number(color.blue()));
    return packet;
}


QString ArduCorPacketParser::routinePacket(int index, const QJsonObject& routineObject) {
    QString packet;
    if (routineObject["routine"].isString()) {
        //------------
        // get routine
        //------------
        ERoutine routine = stringToRoutine(routineObject["routine"].toString());

        //------------
        // get either speed or palette, depending on routine type
        //------------
        QColor color;
        EPalette paletteEnum = EPalette::unknown;
        bool isValidJSON = true;
        if (routine <= cor::ERoutineSingleColorEnd) {
            if (routineObject["hue"].isDouble() && routineObject["sat"].isDouble()
                && routineObject["bri"].isDouble()) {
                color.setHsvF(routineObject["hue"].toDouble(),
                              routineObject["sat"].toDouble(),
                              routineObject["bri"].toDouble());
            } else {
                isValidJSON = false;
            }
        } else if (routineObject["palette"].isObject()) {
            QJsonObject object = routineObject["palette"].toObject();
            cor::Palette palette(object);
            paletteEnum = mPalettes->paletteToEnum(palette);
            if (paletteEnum == EPalette::unknown) {
                isValidJSON = false;
            }
        }

        //------------
        // get speed if theres a speed value
        //------------
        int speed = INT_MIN;
        if (routine != ERoutine::singleSolid) {
            if (routineObject["speed"].isDouble()) {
                speed = int(routineObject["speed"].toDouble());
            }
        }

        //------------
        // get optional parameter
        //------------
        int optionalParameter = INT_MIN;
        if (routineObject["param"].isDouble()) {
            optionalParameter = int(routineObject["param"].toDouble());
        }


        if (isValidJSON) {
            packet += QString::number(int(EPacketHeader::modeChange));
            packet += ",";
            packet += QString::number(index);
            packet += ",";
            packet += QString::number(int(routine));

            if (color.isValid()) {
                packet += ",";
                packet += QString::number(color.red());
                packet += ",";
                packet += QString::number(color.green());
                packet += ",";
                packet += QString::number(color.blue());
            } else if (paletteEnum != EPalette::unknown) {
                packet += ",";
                packet += QString::number(int(paletteEnum));
            }

            if (routine != ERoutine::singleSolid) {
                packet += ",";
                packet += QString::number(speed);
            }

            if (routine == ERoutine::singleFade || routine == ERoutine::singleSawtoothFade
                || routine == ERoutine::singleGlimmer || routine == ERoutine::multiGlimmer
                || routine == ERoutine::multiBars) {
                packet += ",";
                packet += QString::number(optionalParameter);
            }

            packet += "&";
        }
    }
    return packet;
}

QString ArduCorPacketParser::brightnessPacket(int index, int brightness) {
    QString packet = QString("%1,%2,%3&")
                         .arg(QString::number(int(EPacketHeader::brightnessChange)),
                              QString::number(index),
                              QString::number(brightness));
    return packet;
}

QString ArduCorPacketParser::changeCustomArraySizePacket(int index, int count) {
    QString packet = QString("%1,%2,%3&")
                         .arg(QString::number(int(EPacketHeader::customColorCountChange)),
                              QString::number(index),
                              QString::number(count));
    return packet;
}

QString ArduCorPacketParser::timeoutPacket(int index, int timeOut) {
    QString packet = QString("%1,%2,%3&")
                         .arg(QString::number(int(EPacketHeader::idleTimeoutChange)),
                              QString::number(index),
                              QString::number(timeOut));
    return packet;
}


void ArduCorPacketParser::parsePacket(const QString& packet) {
    std::vector<std::vector<int>> messageList;
    std::istringstream input(packet.toStdString());
    std::string number;
    std::string message;

    while (std::getline(input, message, '&')) {
        std::vector<int> intVector;
        std::istringstream input2(message);
        while (std::getline(input2, number, ',')) {
            std::istringstream iss(number);
            int i;
            iss >> i;
            intVector.push_back(i);
        }
        messageList.push_back(intVector);
    }


    for (const auto& intVector : messageList) {
        bool validPacket = true;
        if (intVector.size() > 1) {
            switch (EPacketHeader(intVector[0])) {
                case EPacketHeader::customArrayColorChange:
                    if (intVector.size() == 6) {
                        if ((intVector[2] < 0) || (intVector[2] > CUSTOM_COLOR_MAX)) {
                            validPacket = false;
                        }
                        if ((intVector[3] < 0) || (intVector[3] > 255)) {
                            validPacket = false;
                        }
                        if ((intVector[4] < 0) || (intVector[4] > 255)) {
                            validPacket = false;
                        }
                        if ((intVector[5] < 0) || (intVector[5] > 255)) {
                            validPacket = false;
                        }
                        if (validPacket) {
                            emit receivedArrayColorChange(
                                intVector[1],
                                intVector[2],
                                QColor(intVector[3], intVector[4], intVector[5]));
                        }
                    }
                    break;
                case EPacketHeader::onOffChange:
                    if (intVector.size() == 3) {
                        if (intVector[2] < 0 || intVector[2] > 1) {
                            validPacket = false;
                        }
                        if (validPacket) {
                            emit receivedOnOffChange(intVector[1], bool(intVector[2]));
                        }
                    }
                    break;
                case EPacketHeader::modeChange:
                    routineChange(intVector);
                    break;
                case EPacketHeader::customColorCountChange:
                    if (intVector.size() == 3) {
                        if ((intVector[2] < 0) || (intVector[2] > CUSTOM_COLOR_MAX)) {
                            validPacket = false;
                        }
                        if (validPacket) {
                            emit receivedCustomArrayCount(intVector[1], intVector[2]);
                        }
                    }
                    break;

                case EPacketHeader::brightnessChange:
                    if (intVector.size() == 3) {
                        if ((intVector[2] < 0) || (intVector[2] > 100)) {
                            validPacket = false;
                        }
                        if (validPacket) {
                            emit receivedBrightnessChange(intVector[1], intVector[2]);
                        }
                    }
                    break;

                case EPacketHeader::idleTimeoutChange:
                    if (intVector.size() == 3) {
                        if ((intVector[2] < 1) || (intVector[2] > 23767)) {
                            validPacket = false;
                        }
                        if (validPacket) {
                            emit receivedTimeOutChange(intVector[1], intVector[2]);
                        }
                    }
                    break;
                default:
                    break;
            }
        }
    }
}


void ArduCorPacketParser::routineChange(const std::vector<int>& intVector) {
    QJsonObject routineObject;
    uint32_t tempIndex = 1;
    bool validVector = true;
    // check that theres enough information to be the smallest possible vector
    if (intVector.size() > 3) {
        // get device index
        int deviceIndex = intVector[tempIndex];
        ++tempIndex;

        // get routine
        auto routine = ERoutine(intVector[tempIndex]);
        ++tempIndex;
        routineObject["routine"] = routineToString(routine);

        // get either the color or the palette
        if (int(routine) <= int(cor::ERoutineSingleColorEnd)) {
            if (intVector.size() > 5) {
                int red = intVector[tempIndex];
                ++tempIndex;
                int green = intVector[tempIndex];
                ++tempIndex;
                int blue = intVector[tempIndex];
                ++tempIndex;
                if ((red < 0) || (red > 255)) {
                    validVector = false;
                }
                if ((green < 0) || (green > 255)) {
                    validVector = false;
                }
                if ((blue < 0) || (blue > 255)) {
                    validVector = false;
                }
                routineObject["red"] = red;
                routineObject["green"] = green;
                routineObject["blue"] = blue;
            } else {
                validVector = false;
            }
        } else {
            if (intVector.size() > 4) {
                auto palette = EPalette(intVector[tempIndex]);
                ++tempIndex;
                if (palette == EPalette::unknown) {
                    validVector = false;
                } else {
                    routineObject["paletteEnum"] = paletteToString(palette);
                }
            } else {
                validVector = false;
            }
        }

        // get speed, if it exists
        if (intVector.size() > tempIndex) {
            routineObject["speed"] = intVector[tempIndex];
            ++tempIndex;
        }

        // get optional parameter
        if (intVector.size() > tempIndex) {
            routineObject["param"] = intVector[tempIndex];
            ++tempIndex;
        }

        if (validVector) {
            emit receivedRoutineChange(deviceIndex, routineObject);
        }
    }
}
