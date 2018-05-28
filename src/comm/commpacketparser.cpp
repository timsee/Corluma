/*!
 * \copyright
 * Copyright (C) 2015 - 2018.
 * Released under the GNU General Public License.
 */

#include <sstream>
#include <QDebug>

#include "commpacketparser.h"
#include "cor/protocols.h"

#define CUSTOM_COLOR_MAX 10

CommPacketParser::CommPacketParser(QWidget *parent) : QObject(parent) {

}

void CommPacketParser::parsePacket(QString packet) {
    std::list<std::vector<int> > messageList;
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


    for (auto&& intVector : messageList) {
        bool validPacket = true;
        if (intVector.size() > 1) {
            switch ((EPacketHeader)intVector[0])
            {
                case EPacketHeader::eCustomArrayColorChange:
                    if (intVector.size() == 6) {
                       if ((intVector[2] < 0) || (intVector[2] > CUSTOM_COLOR_MAX)) validPacket = false;
                       if ((intVector[3] < 0) || (intVector[3] > 255)) validPacket = false;
                       if ((intVector[4] < 0) || (intVector[4] > 255)) validPacket = false;
                       if ((intVector[5] < 0) || (intVector[5] > 255)) validPacket = false;
                       if (validPacket) {
                           emit receivedArrayColorChange(intVector[1], intVector[2], QColor(intVector[3], intVector[4], intVector[5]));
                       }
                    }
                    break;
                case EPacketHeader::eOnOffChange:
                    if (intVector.size() == 3) {
                        if (intVector[2] < 0 || intVector[2] > 1) validPacket = false;
                        if (validPacket) {
                            emit receivedOnOffChange(intVector[1], (bool)intVector[2]);
                        }
                    }
                    break;
                case EPacketHeader::eModeChange:
                    routineChange(intVector);
                    break;
                case EPacketHeader::eCustomColorCountChange:
                    if (intVector.size() == 3) {
                       if ((intVector[2] < 0) || (intVector[2] > CUSTOM_COLOR_MAX)) validPacket = false;
                       if (validPacket) {
                           emit receivedCustomArrayCount(intVector[1], intVector[2]);
                       }
                    }
                    break;

                case EPacketHeader::eBrightnessChange:
                    if (intVector.size() == 3) {
                       if ((intVector[2] < 0) || (intVector[2] > 100)) validPacket = false;
                       if (validPacket) {
                           emit receivedBrightnessChange(intVector[1], intVector[2]);
                       }
                    }
                    break;

                case EPacketHeader::eIdleTimeoutChange:
                    if (intVector.size() == 3) {
                       if ((intVector[2] < 1) || (intVector[2] > 23767)) validPacket = false;
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


void CommPacketParser::routineChange(const std::vector<int>& intVector) {
    QJsonObject routineObject;
    uint32_t tempIndex = 1;
    bool validVector = true;
    // check that theres enough information to be the smallest possible vector
    if (intVector.size() > 3) {
        // get device index
        int deviceIndex = intVector[tempIndex];
        ++tempIndex;

        // get routine
        ERoutine routine = (ERoutine)intVector[tempIndex];
        ++tempIndex;
        routineObject["routine"] = routineToString(routine);

        // get either the color or the palette
        if ((int)routine <= (int)cor::ERoutineSingleColorEnd) {
            if (intVector.size() > 5) {
                int red = intVector[tempIndex];
                ++tempIndex;
                int green = intVector[tempIndex];
                ++tempIndex;
                int blue = intVector[tempIndex];
                ++tempIndex;
                if ((red < 0)   || (red > 255))    validVector = false;
                if ((green < 0) || (green > 255))  validVector = false;
                if ((blue < 0)  || (blue > 255))   validVector = false;
                routineObject["red"]   = red;
                routineObject["green"] = green;
                routineObject["blue"]  = blue;
            } else {
                validVector = false;
            }
        } else {
            if (intVector.size() > 4) {
                EPalette palette = (EPalette)intVector[tempIndex];
                ++tempIndex;
                if (palette == EPalette::ePalette_MAX) validVector = false;
                routineObject["palette"] = paletteToString(palette);
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

