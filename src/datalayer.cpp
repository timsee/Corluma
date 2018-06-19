/*!
 * \copyright
 * Copyright (C) 2015 - 2018.
 * Released under the GNU General Public License.
 */


#include "datalayer.h"
#include <QDebug>
#include <algorithm>
#include <vector>
#include "cor/utils.h"
#include "cor/presetpalettes.h"
#define MAX_SPEED 200

DataLayer::DataLayer(QObject *parent) : QObject(parent) {
}

DataLayer::~DataLayer() {

}

int DataLayer::brightness() {
    int brightness = 0;
    size_t deviceCount = 0;
    for (auto&& device = mCurrentDevices.begin(); device != mCurrentDevices.end(); ++device) {
        if (device->isReachable) {
            brightness = brightness + device->brightness;
            deviceCount++;
        }
    }
    if (mCurrentDevices.size() > 1 && (deviceCount > 0)) {
        brightness = brightness / deviceCount;
    }

    return brightness;
}

int DataLayer::speed() {
    int speed = 0;
    size_t deviceCount = 0;
    for (auto&& device = mCurrentDevices.begin(); device != mCurrentDevices.end(); ++device) {
        if (device->isReachable) {
            speed = speed + device->speed;
            deviceCount++;
        }
    }
    if (mCurrentDevices.size() > 1 && (deviceCount > 0)) {
        speed = speed / deviceCount;
    }

    return speed;
}

bool DataLayer::isOn() {
    bool anyOn = false;
    for (auto&& device = mCurrentDevices.begin(); device != mCurrentDevices.end(); ++device) {
        if (device->isOn) anyOn = true;
    }
    return anyOn;
}

QColor DataLayer::mainColor() {
    if (!anyDevicesReachable()) {
        return QColor(0,0,0);
    } else {
        int r = 0;
        int g = 0;
        int b = 0;
        size_t deviceCount = 0;
        if (mCurrentDevices.size() > 0) {
            for (auto&& device = mCurrentDevices.begin(); device != mCurrentDevices.end(); ++device) {
                if (device->isReachable) {
                    if ((int)device->routine <= (int)cor::ERoutineSingleColorEnd) {
                        r = r + device->color.red();
                        g = g + device->color.green();
                        b = b + device->color.blue();
                        deviceCount++;
                    } else {
                        for (auto&& color : device->palette.colors()) {
                            r = r + color.red();
                            g = g + color.green();
                            b = b + color.blue();
                            deviceCount++;
                        }
                    }
                }
            }
            if (deviceCount > 0) {
                r = r / deviceCount;
                g = g / deviceCount;
                b = b / deviceCount;
            } else {
                return QColor(0,0,0);
            }
        }
        return QColor(r,g,b);
    }
}

bool DataLayer::hasHueDevices() {
    int hueCount = 0;
    for (auto&& device = mCurrentDevices.begin(); device != mCurrentDevices.end(); ++device) {
        if (device->commType() == ECommType::hue) hueCount++;
    }
    return (hueCount > 0);
}

bool DataLayer::hasNanoLeafDevices() {
    int nanoLeafCount = 0;
    for (auto&& device = mCurrentDevices.begin(); device != mCurrentDevices.end(); ++device) {
        if (device->commType() == ECommType::nanoleaf) nanoLeafCount++;
    }
    return (nanoLeafCount > 0);
}

bool DataLayer::hasArduinoDevices() {
    int arduinoCount = 0;
    for (auto&& device = mCurrentDevices.begin(); device != mCurrentDevices.end(); ++device) {
        if (device->protocol() == EProtocolType::arduCor) arduinoCount++;
    }
    return (arduinoCount > 0);
}

ERoutine DataLayer::currentRoutine() {
    std::vector<int> routineCount((int)ERoutine::MAX, 0);
    for (auto&& device = mCurrentDevices.begin(); device != mCurrentDevices.end(); ++device) {
        if (device->isReachable) {
            routineCount[(int)device->routine] = routineCount[(int)device->routine] + 1;
        }
    }
    std::vector<int>::iterator result = std::max_element(routineCount.begin(), routineCount.end());
    ERoutine modeRoutine = (ERoutine)std::distance(routineCount.begin(), result);
    return modeRoutine;
}

EPalette DataLayer::palette() {
    // count number of times each color group occurs
    std::vector<int> paletteCount((int)EPalette::unknown, 0);
    for (auto&& device = mCurrentDevices.begin(); device != mCurrentDevices.end(); ++device) {
        if (device->isReachable) {
            paletteCount[(int)device->palette.paletteEnum()] = paletteCount[(int)device->palette.paletteEnum()] + 1;
        }
    }
    // find the most frequent color group occurence, return its index.
    std::vector<int>::iterator result = std::max_element(paletteCount.begin(), paletteCount.end());
    return (EPalette)std::distance(paletteCount.begin(), result);
}

void DataLayer::updateRoutine(const QJsonObject& routineObject) {
    std::list<cor::Light>::iterator iterator;
    ERoutine routine = stringToRoutine(routineObject["routine"].toString());
    bool isOn = routineObject["isOn"].toBool();

    int speed = INT_MIN;
    if (routine != ERoutine::singleSolid) {
        speed = routineObject["speed"].toDouble();
    }
    int hueCount = 0;
    for (iterator = mCurrentDevices.begin(); iterator != mCurrentDevices.end(); ++iterator) {
        iterator->routine = routine;
        iterator->isOn = isOn;

        if (routine != ERoutine::singleSolid) {
            iterator->speed = speed;
            if (iterator->protocol() == EProtocolType::nanoleaf) {
                iterator->speed = MAX_SPEED - speed;
            }
        }

        if (routine <= cor::ERoutineSingleColorEnd) {
            // check for edge case where ambient color values are used
            if (routineObject["temperature"].isDouble()) {
                iterator->color = cor::colorTemperatureToRGB(routineObject["temperature"].toDouble());
                if (iterator->commType() == ECommType::hue) {

                }
            } else {
                iterator->color = QColor(routineObject["red"].toDouble(),
                                         routineObject["green"].toDouble(),
                                         routineObject["blue"].toDouble());
            }
        } else {
            Palette palette = Palette(routineObject["palette"].toObject());
            iterator->palette = palette;

            /*!
             * hues are not individually addressable, mock a color group by setting
             * each individual light as a color
             */
            std::vector<QColor> colors = palette.colors();
            if (iterator->commType() == ECommType::hue) {
                int colorIndex = hueCount % colors.size();
                iterator->color = colors[colorIndex];
                // update the brightness to match the brightness of the color
                int brightness = iterator->color.toHsv().valueF() * 100;
                iterator->brightness = brightness;
                hueCount++;
            }
        }

        if (routineObject["param"].isDouble()) {
            iterator->param = routineObject["param"].toDouble();
        }

    }
    emit dataUpdate();
}

void DataLayer::updateSpeed(int speed) {
    std::list<cor::Light>::iterator iterator;
    for (iterator = mCurrentDevices.begin(); iterator != mCurrentDevices.end(); ++iterator) {
        int finalSpeed = 0;
        if (iterator->protocol() == EProtocolType::arduCor) {
            finalSpeed = speed;
        } else if (iterator->commType() == ECommType::nanoleaf) {
            finalSpeed = MAX_SPEED - speed;
        }
        iterator->speed = finalSpeed;
    }
    emit dataUpdate();
}

void DataLayer::turnOn(bool on) {
    std::list<cor::Light>::iterator iterator;
    for (iterator = mCurrentDevices.begin(); iterator != mCurrentDevices.end(); ++iterator) {
        iterator->isOn = on;
    }
    emit dataUpdate();
}

void DataLayer::updateColorScheme(std::vector<QColor> colors) {
    int i = 0;
    std::list<cor::Light>::iterator iterator;
    for (iterator = mCurrentDevices.begin(); iterator != mCurrentDevices.end(); ++iterator) {
        iterator->color = colors[i];
        iterator->isOn = true;

        // handle Hue special case
        if (iterator->commType() == ECommType::hue) {
            iterator->colorMode = EColorMode::HSV;
        } else {
            iterator->colorMode = EColorMode::RGB;
        }

        i++;
        i = i % colors.size();
    }
    emit dataUpdate();
}

void DataLayer::updateBrightness(int brightness) {
    std::list<cor::Light>::iterator iterator;
    for (iterator = mCurrentDevices.begin(); iterator != mCurrentDevices.end(); ++iterator) {
        iterator->brightness = brightness;
        if (iterator->commType() == ECommType::hue
                || iterator->commType() == ECommType::nanoleaf) {
            // get the color as a HSV
            QColor hsv = iterator->color.toHsv();
            // update to new version of color
            iterator->color.setHsv(hsv.hue(), hsv.saturation(), (int)(255.0f * (brightness / 100.0f)));
        }
        iterator->isOn = true;
    }
    emit dataUpdate();
}

void DataLayer::updateTimeout(int timeout) {
    mTimeout = timeout;
    emit settingsUpdate();
}

bool DataLayer::devicesContainCommType(ECommType type) {
    std::list<cor::Light>::iterator iterator;
    bool foundCommType = false;
    for (iterator = mCurrentDevices.begin(); iterator != mCurrentDevices.end(); ++iterator) {
        if (iterator->commType() == type) {
            foundCommType = true;
        }
    }
    return foundCommType;
}

void DataLayer::enableTimeout(bool timeout) {
    mTimeoutEnabled = timeout;
    emit settingsUpdate();
}


bool DataLayer::clearDevices() {
    if (mCurrentDevices.size()) {
        mCurrentDevices.clear();
    }
    return true;
}

bool DataLayer::removeDevice(cor::Light device){
    bool foundDevice = false;
    cor::Light deviceFound;
    std::list<cor::Light>::const_iterator iterator;
    for (iterator = mCurrentDevices.begin(); iterator != mCurrentDevices.end(); ++iterator) {
        bool deviceExists = compareLight(device, *iterator);
        if (deviceExists) {
            deviceFound = (*iterator);
            foundDevice = true;
        }
    }
    if (foundDevice) {
        mCurrentDevices.remove(deviceFound);
        if (mCurrentDevices.size() == 0) {
            emit devicesEmpty();
        }
        return true;
    } else {
        return false;
    }
}



bool DataLayer::addDeviceList(const std::list<cor::Light>& list) {
    bool hasFailed = false;
    for (auto&& device : list) {
        if (device.isReachable) {
            if (!addDevice(device)) hasFailed = true;
        }
    }

    //TODO: remove this edge case that always autofills the timeout and speed to default values
    std::list<cor::Light>::iterator iterator;
    for (iterator = mCurrentDevices.begin(); iterator != mCurrentDevices.end(); ++iterator) {
        iterator->timeout = 120;
    }

    emit dataUpdate();
    return hasFailed;
}

bool DataLayer::removeDeviceList(const std::list<cor::Light>& list) {
    bool hasFailed = false;
    for (auto&& device : list) {
        if (!removeDevice(device)) hasFailed = true;
    }

    emit dataUpdate();
    return hasFailed;
}



bool DataLayer::addDevice(cor::Light device) {
    std::list<cor::Light>::iterator iterator;
    for (iterator = mCurrentDevices.begin(); iterator != mCurrentDevices.end(); ++iterator) {
        bool deviceExists = compareLight(device, *iterator);
        if (deviceExists) {
            *iterator = device;
            emit dataUpdate();
            return true;
        }
    }
    // device doesn't exist, add it to the device
    mCurrentDevices.push_back(device);
    emit dataUpdate();
    return false;
}


bool DataLayer::doesDeviceExist(cor::Light device) {
    bool foundDevice = false;
    std::list<cor::Light>::const_iterator iterator;
    for (iterator = mCurrentDevices.begin(); iterator != mCurrentDevices.end(); ++iterator) {
        bool deviceExists = compareLight(device, *iterator);
        if (deviceExists) {
            foundDevice = true;
        }
    }
    return foundDevice;
}

bool DataLayer::anyDevicesReachable() {
    // check for not reachable devices
    uint32_t numberOfNotReachableDevices = 0;
    for (auto&& device : mCurrentDevices) {
        if (!device.isReachable) {
            numberOfNotReachableDevices++;
        }
    }
    bool allDevicesNotReachable = false;
    if (mCurrentDevices.size() == numberOfNotReachableDevices) {
        allDevicesNotReachable = true;
    }
    return !allDevicesNotReachable;
}

int DataLayer::removeDevicesOfType(ECommType type) {
    std::list<cor::Light>::const_iterator iterator;
    std::list<cor::Light> removeList;
    for (iterator = mCurrentDevices.begin(); iterator != mCurrentDevices.end(); ++iterator) {
        if (type == iterator->commType()) {
            removeList.push_back((*iterator));
        }
    }
    for (auto&& device : removeList) {
        removeDevice(device);
    }
    return mCurrentDevices.size();
}

int DataLayer::countDevicesOfType(ECommType type) {
    int count = 0;
    std::list<cor::Light>::const_iterator iterator;
    for (iterator = mCurrentDevices.begin(); iterator != mCurrentDevices.end(); ++iterator) {
        if (type == iterator->commType()) {
            count++;
        }
    }
    return count;
}

QString DataLayer::findCurrentCollection(const std::list<cor::LightGroup>& collections, bool allowLights) {
    // count number of lights in each collection currently selected
    std::vector<uint32_t> lightCount(collections.size(), 0);
    uint32_t index = 0;
    for (auto&& collection : collections) {
        for (auto&& device : mCurrentDevices) {
            for (auto&& collectionDevice : collection.devices) {
                if (compareLight(device, collectionDevice)) {
                    ++lightCount[index];
                }
            }
        }
        ++index;
    }

    // check how many collections are currently fully selected
    std::vector<bool> allLightsFound(collections.size(), false);
    index = 0;
    uint32_t completeGroupCount = 0;
    for (auto&& collection : collections) {
        // only count reachable devices
        std::list<cor::Light> reachableDevices;
        for (auto&& device : collection.devices) {
            if (device.isReachable) {
                reachableDevices.push_back(device);
            }
        }
        if (lightCount[index] == reachableDevices.size() && reachableDevices.size() > 0) {
            allLightsFound[index] = true;
            ++completeGroupCount;
        }
        ++index;
    }

    // if count is higher than 1, check if any have too many
    if (completeGroupCount > 1) {
        QString name;
        uint32_t biggestSize = 0;
        bool foundNonZeroGroup = false;
        index = 0;
        for (auto&& collection : collections) {
            if (allLightsFound[index]) {
                if (collection.devices.size() > biggestSize) {
                    name = collection.name;
                    foundNonZeroGroup = true;
                    biggestSize = collection.devices.size();
                }
            }
            ++index;
        }
        if (foundNonZeroGroup) {
            return name;
        }
    }

    // if only one group is connected, this is easy, return that group.
    if (completeGroupCount == 1) {
        auto result = std::find(allLightsFound.begin(), allLightsFound.end(), true);
        int allLightsIndex = std::distance(allLightsFound.begin(), result);
        int currentIndex = 0;
        for (auto collection : collections) {
            if (allLightsIndex == currentIndex) {
                return collection.name;
            }
            ++currentIndex;
        }
    }


    if (allowLights) {
        if (mCurrentDevices.size() == 1) {
            return mCurrentDevices.front().name;
        }
        if (mCurrentDevices.size() == 2) {
            return QString(mCurrentDevices.front().name + ", " + mCurrentDevices.back().name);
        }
    }

    return "";
}

QString DataLayer::findCurrentMood(const std::list<cor::LightGroup>& moods) {
    for (auto mood : moods) {
        if (mCurrentDevices == mood.devices) {
            return mood.name;
        }
    }
    return "";
}
