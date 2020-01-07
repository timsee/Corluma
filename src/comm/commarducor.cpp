/*!
 * \copyright
 * Copyright (C) 2015 - 2019.
 * Released under the GNU General Public License.
 */

#include "commarducor.h"

#include "comm/commhttp.h"
#include "comm/commudp.h"
#include "utils/exception.h"
#include "utils/qt.h"
#ifndef MOBILE_BUILD
#include "comm/commserial.h"
#endif

CommArduCor::CommArduCor(QObject* parent) : QObject(parent) {
    mUDP = std::make_shared<CommUDP>();
    connect(mUDP.get(),
            SIGNAL(packetReceived(QString, QString, ECommType)),
            this,
            SLOT(parsePacket(QString, QString, ECommType)));
    connect(mUDP.get(), SIGNAL(updateReceived(ECommType)), this, SLOT(receivedUpdate(ECommType)));

    mHTTP = std::make_shared<CommHTTP>();
    connect(mHTTP.get(),
            SIGNAL(packetReceived(QString, QString, ECommType)),
            this,
            SLOT(parsePacket(QString, QString, ECommType)));
    connect(mHTTP.get(), SIGNAL(updateReceived(ECommType)), this, SLOT(receivedUpdate(ECommType)));


#ifndef MOBILE_BUILD
    mSerial = std::make_shared<CommSerial>();
    connect(mSerial.get(),
            SIGNAL(packetReceived(QString, QString, ECommType)),
            this,
            SLOT(parsePacket(QString, QString, ECommType)));
    connect(mSerial.get(),
            SIGNAL(updateReceived(ECommType)),
            this,
            SLOT(receivedUpdate(ECommType)));
#endif // MOBILE_BUILD

    mDiscovery = new ArduCorDiscovery(this,
                                      mHTTP.get(),
                                      mUDP.get()
#ifndef MOBILE_BUILD
                                          ,
                                      mSerial.get()
#endif
    );

    // make list of not found devices
    for (const auto& controller : mDiscovery->undiscoveredControllers()) {
        int i = 1;
        for (const auto& name : controller.names()) {
            ArduCorMetadata metadata(name, controller, i);
            ArduCorLight light(metadata);
            ++i;
            commByType(controller.type())->addLight(light);
        }
    }

    mUDP->connectDiscovery(mDiscovery);
    mHTTP->connectDiscovery(mDiscovery);
#ifndef MOBILE_BUILD
    mSerial->connectDiscovery(mDiscovery);
#endif
}

bool CommArduCor::preparePacketForTransmission(const cor::Controller& controller, QString& packet) {
    bool shouldReset = false;
    // check if state update
    if (!(packet.at(0) == QChar('7') || packet.at(0) == QChar('8'))) {
        // if not state update, reset the state update timer.
        shouldReset = true;
    }

    // add CRC, if in use
    if (controller.isUsingCRC()) {
        packet = packet + "#" + QString::number(mCRC.calculate(packet)) + "&";
    }
    return shouldReset;
}

void CommArduCor::sendPacket(const cor::Controller& controller, QString& payload) {
    // add CRC, check if should reset state updates.
    bool shouldResetStateTimeout = preparePacketForTransmission(controller, payload);

    if (controller.type() == ECommType::HTTP) {
        mHTTP->sendPacket(controller, payload);
        if (shouldResetStateTimeout) {
            mHTTP->resetStateUpdateTimeout();
        }
    }
#ifndef MOBILE_BUILD
    else if (controller.type() == ECommType::serial) {
        mSerial->sendPacket(controller, payload);
        if (shouldResetStateTimeout) {
            mSerial->resetStateUpdateTimeout();
        }
    }
#endif
    else if (controller.type() == ECommType::UDP) {
        mUDP->sendPacket(controller, payload);
        if (shouldResetStateTimeout) {
            mUDP->resetStateUpdateTimeout();
        }
    }
}

void CommArduCor::startup() {
    mHTTP->startup();
    mUDP->startup();
#ifndef MOBILE_BUILD
    mSerial->startup();
#endif
}

void CommArduCor::shutdown() {
    mHTTP->shutdown();
    mUDP->shutdown();
#ifndef MOBILE_BUILD
    mSerial->shutdown();
#endif
}

void CommArduCor::stopStateUpdates() {
    mHTTP->stopStateUpdates();
    mUDP->stopStateUpdates();
#ifndef MOBILE_BUILD
    mSerial->stopStateUpdates();
#endif
}

void CommArduCor::resetStateUpdates() {
    mHTTP->resetStateUpdateTimeout();
    mUDP->resetStateUpdateTimeout();
#ifndef MOBILE_BUILD
    mSerial->resetStateUpdateTimeout();
#endif
}


std::vector<cor::Light> CommArduCor::lights() {
    std::vector<cor::Light> lights;
    std::vector<ECommType> commTypes = {ECommType::HTTP,
#ifndef MOBILE_BUILD
                                        ECommType::serial,
#endif
                                        ECommType::UDP};
    for (auto type : commTypes) {
        for (const auto& storedLight : commByType(type)->lightDict().items()) {
            lights.push_back(storedLight);
        }
    }
    return lights;
}

void CommArduCor::deleteLight(const cor::Light& light) {
    auto result = mArduCorLights.item(light.uniqueID().toStdString());
    if (result.second) {
        auto arduCorLight = result.first;
        // remove from comm data
        commByType(arduCorLight.commType())->removeLight(arduCorLight.controller());

        // remove from JSON data
        mDiscovery->removeController(arduCorLight.controller());
    }
}

ArduCorMetadata CommArduCor::arduCorLightFromLight(const cor::Light& light) {
    auto result = mArduCorLights.item(light.uniqueID().toStdString());
    return result.first;
}

void CommArduCor::parsePacket(const QString& sender, const QString& packet, ECommType type) {
    // split into vector of strings
    std::vector<std::string> packetVector;
    std::istringstream input(packet.toStdString());
    std::string parsedPacket;
    while (std::getline(input, parsedPacket, '&')) {
        packetVector.push_back(parsedPacket);
    }

    // Turn vector of strings into vector of vector of ints
    std::vector<std::vector<int>> intVectors;
    for (const auto& parsedPacket : packetVector) {
        std::vector<int> intVector;
        std::istringstream input(parsedPacket);
        std::string number;
        while (std::getline(input, number, ',')) {
            std::istringstream iss(number);
            int i;
            iss >> i;
            intVector.push_back(i);
        }
        intVectors.push_back(intVector);
    }

    //------------------
    // Check for CRC
    //------------------
    auto result = mDiscovery->findControllerByControllerName(sender);
    auto controller = result.first;
    if (result.second && (packet.length() > 0)) {
        // check if it should use CRC
        if (controller.isUsingCRC()) {
            QStringList crcPacketList = packet.split("#");
            if (crcPacketList.size() == 2) {
                // compute CRC
                uint32_t computedCRC = mCRC.calculate(crcPacketList[0]);
                uint32_t givenCRC =
                    crcPacketList[1].leftRef(crcPacketList[1].length() - 1).toUInt();
                if (givenCRC != computedCRC) {
                    // qDebug() << "INFO: failed CRC check for" << controller.name << "string:" <<
                    // crcPacketList[0] << "computed:" << QString::number(computedCRC) << "given:"
                    // << QString::number(givenCRC);
                    return;
                }
                // qDebug() << "INFO: CRC check passed!";
                // pass the CRC check
                intVectors.pop_back();
            } else {
                return;
            }
        }
        // qDebug() << "the sender: " << sender << "packet:" << packet << "type:" <<
        // commTypeToString(type);
        for (const auto& intVector : intVectors) {
            if (intVector.size() > 2) {
                if (intVector[0] < int(EPacketHeader::MAX)) {
                    auto packetHeader = EPacketHeader(intVector[0]);
                    auto index = std::size_t(intVector[1]);
                    bool isValid = true;
                    if (index < 20) {
                        // figure out devices that are getting updates
                        std::vector<ArduCorMetadata> metadataVector;
                        std::vector<cor::Light> lightVector;
                        if (index != 0) {
                            auto metadata =
                                ArduCorMetadata(controller.names()[index - 1], controller, index);
                            ArduCorLight light(metadata);
                            commByType(type)->fillDevice(light);
                            metadataVector.push_back(metadata);
                            lightVector.push_back(light);
                        } else {
                            // get a list of devices for this controller
                            auto arduCorLights = mArduCorLights.items();
                            for (const auto& arduCor : arduCorLights) {
                                if (arduCor.controller() == sender) {
                                    metadataVector.push_back(arduCor);
                                    lightVector.push_back(ArduCorLight(arduCor));
                                }
                            }
                        }

                        // check if its a valid size with the proper header for a state update
                        // packet
                        if (packetHeader == EPacketHeader::stateUpdateRequest) {
                            uint32_t x = 1;
                            // check all values fall in their proper ranges
                            if (verifyStateUpdatePacketValidity(intVector, x)) {
                                for (auto i = 0u; i < lightVector.size(); ++i) {
                                    auto light = lightVector[i];
                                    auto metadata = metadataVector[i];

                                    light.isOn(intVector[x + 1]);
                                    light.isReachable(intVector[x + 2]);

                                    double brightness = double(intVector[x + 8]) / 100.0;
                                    auto red = int(intVector[x + 3] * brightness);
                                    auto green = int(intVector[x + 4] * brightness);
                                    auto blue = int(intVector[x + 5] * brightness);
                                    QColor color(red, green, blue);
                                    color.setHsvF(color.hueF(), color.saturationF(), 1.0);
                                    color.setHsvF(color.hueF(), color.saturationF(), brightness);
                                    light.color(color);

                                    light.routine(ERoutine(intVector[x + 6]));
                                    auto palette = EPalette(intVector[x + 7]);
                                    if (palette == EPalette::custom) {
                                        light.palette(light.customPalette());
                                    } else {
                                        light.palette(mPresetPalettes.palette(palette));
                                    }
                                    light.paletteBrightness(std::uint32_t(brightness * 100.0));


                                    light.speed(intVector[x + 9]);
                                    metadata.timeout(intVector[x + 10]);
                                    metadata.minutesUntilTimeout(intVector[x + 11]);

                                    light.version(controller.majorAPI(), controller.minorAPI());

                                    commByType(type)->updateLight(light);

                                    // check that it doesn't already exist, if it does, replace the
                                    // old version
                                    auto key = light.uniqueID().toStdString();
                                    auto dictResult = mArduCorLights.item(key);
                                    if (dictResult.second) {
                                        mArduCorLights.update(key, metadata);
                                    } else {
                                        mArduCorLights.insert(key, metadata);
                                    }
                                }
                            } else {
                                // qDebug() << "WARNING: Invalid packet for light index" <<
                                // intVector[x];
                            }
                        } else if (packetHeader == EPacketHeader::customArrayUpdateRequest) {
                            if (verifyCustomColorUpdatePacket(intVector)) {
                                for (auto light : lightVector) {
                                    auto customColorCount = std::uint32_t(intVector[2]);
                                    uint32_t j = 3;
                                    std::vector<QColor> colors = light.customPalette().colors();
                                    uint32_t brightness = light.customPalette().brightness();
                                    for (std::uint32_t i = 0; i < customColorCount; ++i) {
                                        colors[i] = QColor(intVector[j],
                                                           intVector[j + 1],
                                                           intVector[j + 2]);
                                        j = j + 3;
                                    }
                                    light.customPalette(Palette(paletteToString(EPalette::custom),
                                                                colors,
                                                                brightness));
                                    if (light.palette().paletteEnum() == EPalette::custom) {
                                        light.palette(light.customPalette());
                                    }
                                    commByType(type)->updateLight(light);
                                }
                            }
                        } else if (packetHeader == EPacketHeader::brightnessChange
                                   && (intVector.size() % 3 == 0)) {
                            for (auto&& light : lightVector) {
                                QColor color;
                                color.setHsvF(light.color().hueF(),
                                              light.color().saturationF(),
                                              double(intVector[2]) / 100.0);
                                light.color(color);
                                light.paletteBrightness(std::uint32_t(intVector[2]));
                                commByType(type)->updateLight(light);
                            }
                        } else if (packetHeader == EPacketHeader::onOffChange
                                   && (intVector.size() % 3 == 0)) {
                            for (auto light : lightVector) {
                                light.isOn(intVector[2]);
                                commByType(type)->updateLight(light);
                            }
                        } else if (packetHeader == EPacketHeader::modeChange
                                   && intVector.size() > 3) {
                            for (auto light : lightVector) {
                                uint32_t tempIndex = 2;
                                light.routine(ERoutine(intVector[tempIndex]));
                                ++tempIndex;
                                // get either the color or the palette
                                if (int(light.routine()) <= int(cor::ERoutineSingleColorEnd)) {
                                    if (intVector.size() > 5) {
                                        int red = intVector[tempIndex];
                                        ++tempIndex;
                                        int green = intVector[tempIndex];
                                        ++tempIndex;
                                        int blue = intVector[tempIndex];
                                        ++tempIndex;
                                        if ((red < 0) || (red > 255)) {
                                            isValid = false;
                                        }
                                        if ((green < 0) || (green > 255)) {
                                            isValid = false;
                                        }
                                        if ((blue < 0) || (blue > 255)) {
                                            isValid = false;
                                        }
                                        light.color(QColor(red, green, blue));
                                    } else {
                                        isValid = false;
                                    }
                                } else {
                                    if (intVector.size() > 4) {
                                        // store brightness from previous data
                                        auto brightness =
                                            std::uint32_t(light.palette().brightness());
                                        auto palette = EPalette(intVector[tempIndex]);
                                        if (palette == EPalette::custom) {
                                            light.palette(light.customPalette());
                                        } else {
                                            light.palette(mPresetPalettes.palette(palette));
                                        }
                                        light.paletteBrightness(brightness);
                                        ++tempIndex;
                                        if (light.palette().paletteEnum() == EPalette::unknown) {
                                            isValid = false;
                                        }
                                    } else {
                                        isValid = false;
                                    }
                                }

                                // get speed, if it exists
                                if (intVector.size() > tempIndex) {
                                    light.speed(intVector[tempIndex]);
                                    ++tempIndex;
                                }

                                // get optional parameter
                                if (intVector.size() > tempIndex) {
                                    light.param(intVector[tempIndex]);
                                    ++tempIndex;
                                }

                                if (isValid) {
                                    commByType(type)->updateLight(light);
                                }
                            }
                        } else if (packetHeader == EPacketHeader::idleTimeoutChange
                                   && (intVector.size() % 3 == 0)) {
                            for (auto metadata : metadataVector) {
                                metadata.timeout(intVector[2]);
                                metadata.minutesUntilTimeout(intVector[2]);

                                // check that it doesn't already exist, if it does, replace the
                                // old version
                                auto key = metadata.uniqueID().toStdString();
                                auto dictResult = mArduCorLights.item(key);
                                if (dictResult.second) {
                                    mArduCorLights.update(key, metadata);
                                } else {
                                    mArduCorLights.insert(key, metadata);
                                }
                            }
                        } else if (packetHeader == EPacketHeader::customArrayColorChange
                                   && (intVector.size() % 6 == 0)) {
                            for (auto light : lightVector) {
                                if (index <= 10) {
                                    auto index = std::uint32_t(intVector[2]);
                                    if (intVector.size() > 5) {
                                        int red = intVector[3];
                                        int green = intVector[4];
                                        int blue = intVector[5];
                                        if ((red < 0) || (red > 255)) {
                                            isValid = false;
                                        }
                                        if ((green < 0) || (green > 255)) {
                                            isValid = false;
                                        }
                                        if ((blue < 0) || (blue > 255)) {
                                            isValid = false;
                                        }
                                        std::vector<QColor> colors = light.customPalette().colors();
                                        colors[index] = QColor(red, green, blue);
                                        auto brightness = light.customPalette().brightness();
                                        auto paletteString = paletteToString(EPalette::custom);
                                        light.customPalette(
                                            Palette(paletteString, colors, brightness));
                                        commByType(type)->updateLight(light);
                                    } else {
                                        isValid = false;
                                    }
                                } else {
                                    isValid = false;
                                }
                            }
                        } else if (packetHeader == EPacketHeader::customColorCountChange
                                   && (intVector.size() % 3 == 0)) {
                            for (auto light : lightVector) {
                                if (intVector[2] <= 10) {
                                    light.customCount(std::uint32_t(intVector[2]));
                                    // qDebug() << "UPDATE TO custom color count" <<
                                    // device.customColors;
                                    commByType(type)->updateLight(light);
                                } else {
                                    isValid = false;
                                }
                            }
                        } else {
                            if (!isValid) {
                                qDebug() << "WARNING: Invalid packet: " << packet;
                            } else if (intVector[0] == 7) {
                                qDebug() << "WARNING: Invalid state update packet: " << packet;
                            } else {
                                qDebug() << "WARNING: Invalid packet size: " << intVector.size()
                                         << "for packet header"
                                         << packetHeaderToString(EPacketHeader(intVector[0]));
                            }
                            isValid = false;
                        }
                        if (isValid) {
                            emit packetReceived(EProtocolType::arduCor);
                        }
                    } else {
                        qDebug() << "packet header not valid..." << sender << "packet:" << packet
                                 << "type:" << commTypeToString(type);
                    }
                }
            }
        }
    }
}

bool CommArduCor::verifyStateUpdatePacketValidity(const std::vector<int>& packetIntVector,
                                                  uint32_t x) {
    while (x < packetIntVector.size()) {
        if (!(packetIntVector[x] > 0 && packetIntVector[x] < 15)) {
            return false;
        }
        if (!(packetIntVector[x + 1] == 1 || packetIntVector[x + 1] == 0)) {
            return false;
        }
        if (!(packetIntVector[x + 2] == 1 || packetIntVector[x + 2] == 0)) {
            return false;
        }
        if (!(packetIntVector[x + 3] >= 0 && packetIntVector[x + 3] <= 255)) {
            return false;
        }
        if (!(packetIntVector[x + 4] >= 0 && packetIntVector[x + 4] <= 255)) {
            return false;
        }
        if (!(packetIntVector[x + 5] >= 0 && packetIntVector[x + 5] <= 255)) {
            return false;
        }
        if (!(packetIntVector[x + 6] >= 0 && packetIntVector[x + 6] < int(ERoutine::MAX))) {
            return false;
        }
        if (!(packetIntVector[x + 7] >= 0 && packetIntVector[x + 7] < int(EPalette::unknown))) {
            return false;
        }
        if (!(packetIntVector[x + 8] >= 0 && packetIntVector[x + 8] <= 100)) {
            return false;
        }
        if (!(packetIntVector[x + 9] >= 0 && packetIntVector[x + 9] <= 2000)) {
            return false;
        }
        if (!(packetIntVector[x + 10] >= 0 && packetIntVector[x + 10] <= 1000)) {
            return false;
        }
        if (!(packetIntVector[x + 11] >= 0 && packetIntVector[x + 11] <= 1000)) {
            return false;
        }
        x += 12;
    }
    return true;
}


bool CommArduCor::verifyCustomColorUpdatePacket(const std::vector<int>& packetIntVector) {
    std::uint32_t x = 2u;
    if (packetIntVector.size() < 3) {
        return false;
    }
    auto customColorCount = std::uint32_t(packetIntVector[x]);
    if (!(customColorCount > 0 && customColorCount < 11)) {
        return false;
    }
    if (!(packetIntVector.size() >= customColorCount * 3 + 3)) {
        return false;
    }
    for (; x < packetIntVector.size(); x = x + 3) {
        if (!(packetIntVector[x] >= 0 && packetIntVector[x] <= 255)) {
            return false;
        }
        if (!(packetIntVector[x + 1] >= 0 && packetIntVector[x + 1] <= 255)) {
            return false;
        }
        if (!(packetIntVector[x + 2] >= 0 && packetIntVector[x + 2] <= 255)) {
            return false;
        }
    }
    return true;
}

CommType* CommArduCor::commByType(ECommType type) {
    CommType* ptr;
    switch (type) {
#ifndef MOBILE_BUILD
        case ECommType::serial:
            ptr = static_cast<CommType*>(mSerial.get());
            break;
#endif // MOBILE_BUILD
        case ECommType::HTTP:
            ptr = static_cast<CommType*>(mHTTP.get());
            break;
        case ECommType::UDP:
            ptr = static_cast<CommType*>(mUDP.get());
            break;
        default:
            THROW_EXCEPTION("no type for this commtype");
    }
    return ptr;
}
