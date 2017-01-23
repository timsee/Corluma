/*!
 * \copyright
 * Copyright (C) 2015 - 2017.
 * Released under the GNU General Public License.
 */

#include <sstream>

#include "commpacketparser.h"

#define CUSTOM_COLOR_MAX 10

CommPacketParser::CommPacketParser(QWidget *parent) : QWidget(parent)
{

}

void CommPacketParser::parsePacket(QString packet) {
    std::vector<int> intVector;
    std::istringstream input(packet.toStdString());
    std::string number;
    while (std::getline(input, number, ',')) {
        std::istringstream iss(number);
        int i;
        iss >> i;
        intVector.push_back(i);
    }

    bool validPacket = true;
    if (intVector.size() > 1) {
        switch ((EPacketHeader)intVector[0])
        {
            case EPacketHeader::eMainColorChange:
                if (intVector.size() == 5) {
                   if ((intVector[2] < 0) || (intVector[2] > 255)) validPacket = false;
                   if ((intVector[3] < 0) || (intVector[3] > 255)) validPacket = false;
                   if ((intVector[4] < 0) || (intVector[4] > 255)) validPacket = false;
                   if (validPacket) {
                       emit receivedMainColorChange(intVector[1], QColor(intVector[2], intVector[3], intVector[4]));
                   }
                }
                break;
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
            case EPacketHeader::eModeChange:
                if (intVector.size() == 3 || intVector.size() == 4) {
                   if ((intVector[2] < 0) || (intVector[2] >= (int)ELightingRoutine::eLightingRoutine_MAX)) validPacket = false;
                   if (intVector.size() == 4) {
                       if ((intVector[3] < 0) || (intVector[3] >= (int)EColorGroup::eColorGroup_MAX)) validPacket = false;
                       if (validPacket) {
                           emit receivedRoutineChange(intVector[1], intVector[2], intVector[3]);
                       }
                   } else {
                       if (validPacket) {
                           emit receivedRoutineChange(intVector[1],intVector[2], -1);
                       }
                   }
                }
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

            case EPacketHeader::eSpeedChange:
                if (intVector.size() == 3) {
                   if ((intVector[2] < 0) || (intVector[2] > 2000)) validPacket = false;
                   if (validPacket) {
                       emit receivedSpeedChange(intVector[1], intVector[2]);
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

            case EPacketHeader::eResetSettingsToDefaults:
                if (intVector.size() == 3) {
                   if (intVector[1] == 42) validPacket = false;
                   if (intVector[2] == 71) validPacket = false;
                   if (validPacket) {
                       emit receivedReset();
                   }
                }
                break;
            default:
                break;
        }
    }

}

