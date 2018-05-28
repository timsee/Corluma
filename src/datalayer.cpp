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

#define MAX_SPEED 200

DataLayer::DataLayer() {
    int i = 0;

    mProtocolSettings = new ProtocolSettings();
    mColors = std::vector<std::vector<QColor> >((size_t)EPalette::ePalette_MAX);

    //==========
    // Custom Colors
    //==========
    mColors[i] =  std::vector<QColor>(10);
    i++;

    //==========
    // Water Colors
    //==========

    mColors[i] =  std::vector<QColor>(5);
    mColors[i][0] = QColor(0,   0, 255);
    mColors[i][1] = QColor(0,   25,  225);
    mColors[i][2] = QColor(0,   0,   127);
    mColors[i][3] = QColor(0,   127, 127);
    mColors[i][4] = QColor(120, 120, 255);
    i++;

    //==========
    // Frozen Colors
    //==========
    mColors[i] =  std::vector<QColor>(6);
    mColors[i][0] = QColor(0,   127, 255);
    mColors[i][1] = QColor(169, 228, 247);
    mColors[i][2] = QColor(200, 0,   255);
    mColors[i][3] = QColor(200, 200, 200);
    mColors[i][4] = QColor(127, 127, 127);
    mColors[i][5] = QColor(127, 127, 255);
    i++;

    //==========
    // Snow Colors
    //==========
    mColors[i] =  std::vector<QColor>(6);
    mColors[i][0] = QColor(255, 255, 255);
    mColors[i][1] = QColor(127, 127, 127);
    mColors[i][2] = QColor(200, 200, 200);
    mColors[i][3] = QColor(0,   0,   255);
    mColors[i][4] = QColor(0,  255,  255);
    mColors[i][5] = QColor(0,  180,   180);
    i++;

    //==========
    // Cool Colors
    //==========
    mColors[i] =  std::vector<QColor>(5);
    mColors[i][0] = QColor(0,   255, 0);
    mColors[i][1] = QColor(125, 0,   255);
    mColors[i][2] = QColor(0,   0,   255);
    mColors[i][3] = QColor(40,  127, 40);
    mColors[i][4] = QColor(60,  0,   160);
    i++;

    //==========
    // Warm Colors
    //==========
    mColors[i] =  std::vector<QColor>(5);
    mColors[i][0] = QColor(255, 255, 0);
    mColors[i][1] = QColor(255, 0,   0);
    mColors[i][2] = QColor(255, 45,  0);
    mColors[i][3] = QColor(255, 200,  0);
    mColors[i][4] = QColor(255, 127, 0);
    i++;

    //==========
    // Fire Colors
    //==========
    mColors[i] =  std::vector<QColor>(9);
    mColors[i][0] = QColor(255, 75,  0);
    mColors[i][1] = QColor(255, 20,  0);
    mColors[i][2] = QColor(255, 80,  0);
    mColors[i][3] = QColor(255, 5,   0);
    mColors[i][4] = QColor(64,  6,   0);
    mColors[i][5] = QColor(127, 127, 0);
    mColors[i][6] = QColor(255, 60,  0);
    mColors[i][7] = QColor(255, 45,  0);
    mColors[i][8] = QColor(127, 127, 0);
    i++;

    //==========
    // Evil Colors
    //==========
    mColors[i] =  std::vector<QColor>(7);
    mColors[i][0] = QColor(255, 0, 0);
    mColors[i][1] = QColor(200, 0, 0);
    mColors[i][2] = QColor(127, 0, 0);
    mColors[i][3] = QColor(20,  0, 0);
    mColors[i][4] = QColor(30,  0, 40);
    mColors[i][5] = QColor(10,  0, 0);
    mColors[i][6] = QColor(80,  0, 0);
    i++;

    //==========
    // Corrosive Colors
    //==========
    mColors[i] =  std::vector<QColor>(5);
    mColors[i][0] = QColor(0,   255, 0);
    mColors[i][1] = QColor(0,   200, 0);
    mColors[i][2] = QColor(60,  180,  60);
    mColors[i][3] = QColor(127, 135, 127);
    mColors[i][4] = QColor(10,  255,   10);
    i++;

    //==========
    // Poison Colors
    //==========
    mColors[i] =  std::vector<QColor>(9);
    mColors[i][0] = QColor(80, 0, 180);
    mColors[i][1] = QColor(120, 0, 255);
    mColors[i][2] = QColor(0, 0,   0);
    mColors[i][3] = QColor(25, 0,  25);
    mColors[i][4] = QColor(60, 60,  60);
    mColors[i][5] = QColor(120, 0, 255);
    mColors[i][6] = QColor(80,  0, 180);
    mColors[i][7] = QColor(40,  0, 90);
    mColors[i][8] = QColor(80,  0, 180);
    i++;

    //==========
    // Rose
    //==========
    mColors[i] =  std::vector<QColor>(6);
    mColors[i][0] = QColor(216, 30,  100);
    mColors[i][1] = QColor(255, 245, 251);
    mColors[i][2] = QColor(156, 62,  72);
    mColors[i][3] = QColor(127, 127, 127);
    mColors[i][4] = QColor(194, 30,  86);
    mColors[i][5] = QColor(194, 30,  30);
    i++;

    //==========
    // Pink Green
    //==========
    mColors[i] =  std::vector<QColor>(4);
    mColors[i][0]  = QColor(255, 20,  147);
    mColors[i][1]  = QColor(0,   255, 0);
    mColors[i][2]  = QColor(0,   200, 0);
    mColors[i][3]  = QColor(255, 105, 180);
    i++;

    //==========
    // Red White Blue
    //==========
    mColors[i] =  std::vector<QColor>(4);
    mColors[i][0]  = QColor(255, 255, 255);
    mColors[i][1]  = QColor(255, 0,   0);
    mColors[i][2]  = QColor(0,   0,   255);
    mColors[i][3]  = QColor(255, 255, 255);
    i++;

    //==========
    // RGB
    //==========
    mColors[i] =  std::vector<QColor>(3);
    mColors[i][0] = QColor(255, 0,   0);
    mColors[i][1] = QColor(0,   255, 0);
    mColors[i][2] = QColor(0,   0,   255);
    i++;

    //==========
    // CMY
    //==========
    mColors[i] =  std::vector<QColor>(3);
    mColors[i][0] = QColor(255, 255, 0);
    mColors[i][1] = QColor(0,   255, 255);
    mColors[i][2] = QColor(255,   0, 255);
    i++;

    //==========
    // Six Color
    //==========
    mColors[i] =  std::vector<QColor>(6);
    mColors[i][0] = QColor(255, 0,   0);
    mColors[i][1] = QColor(255, 255, 0);
    mColors[i][2] = QColor(0,   255, 0);
    mColors[i][3] = QColor(0,   255, 255);
    mColors[i][4] = QColor(0,   0,   255);
    mColors[i][5] = QColor(255, 0,   255);
    i++;

    //==========
    // Seven Color
    //==========
    mColors[i] =  std::vector<QColor>(7);
    mColors[i][0] = QColor(255, 0,   0);
    mColors[i][1] = QColor(255, 255, 0);
    mColors[i][2] = QColor(0,   255, 0);
    mColors[i][3] = QColor(0,   255, 255);
    mColors[i][4] = QColor(0,   0,   255);
    mColors[i][5] = QColor(255, 0,   255);
    mColors[i][6] = QColor(255, 255, 255);
    i++;

    mColorAverages = std::vector<QColor>((int)EPalette::ePalette_MAX);
    for (uint32_t i = 0; i < mColors.size(); ++i) {
        mColorAverages[i] = averageGroup((EPalette)i);
    }
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
    } else if ((int)currentRoutine() <= (int)cor::ERoutineSingleColorEnd) {
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
                    } else {
                        QColor color = colorsAverage(device->palette);
                        r = r + color.red();
                        g = g + color.green();
                        b = b + color.blue();
                    }
                    deviceCount++;
                }
            }
            if (deviceCount > 0) {
                r = r / deviceCount;
                g = g / deviceCount;
                b = b / deviceCount;
            }
        }
        return QColor(r,g,b);
    } else {
        return colorsAverage(palette());
    }
}

EPalette DataLayer::closestColorGroupToColor(QColor color) {
    EPalette closestMatch = EPalette::eWater;
    int currentDifference = INT_MAX;
    //TODO: do a much less naive check for this
    for (uint32_t i = 1; i < mColorAverages.size() - 1; ++i) {
        int difference = 0;
        difference += std::abs(mColorAverages[i].red() - color.red());
        difference += std::abs(mColorAverages[i].green() - color.green());
        difference += std::abs(mColorAverages[i].blue() - color.blue());
        if (difference < currentDifference) {
            currentDifference = difference;
            closestMatch = (EPalette)i;
        }
    }
    return closestMatch;
}

uint8_t DataLayer::maxColorGroupSize() {
    return 10;
}

const std::vector<QColor>& DataLayer::palette(EPalette palette) {
    if (palette == EPalette::eCustom && mCurrentDevices.size() > 0) {
        return mCurrentDevices.begin()->customColorArray;
    } else if ((int)palette < (int)EPalette::ePalette_MAX) {
        return mColors[(int)palette];
    } else {
        throw "Color Group not valid!";
        return mColors[0];
    }
}

QColor DataLayer::averageGroup(EPalette palette) {
    int r = 0;
    int g = 0;
    int b = 0;

    std::vector<QColor> colorGroup;
    uint8_t count;
    if (palette == EPalette::eCustom && mCurrentDevices.size() > 0) {
        colorGroup = mCurrentDevices.begin()->customColorArray;
        count = mCurrentDevices.begin()->customColorCount;
    } else {
        colorGroup = mColors[(uint32_t)palette];
        count = colorGroup.size();
    }
    for (int i = 0; i < count; ++i) {
       r = r + colorGroup[i].red();
       g = g + colorGroup[i].green();
       b = b + colorGroup[i].blue();
    }
    if (count > 0) {
        return QColor(r / count,
                      g / count,
                      b / count);
        return QColor();
    } else {
        return QColor();
    }
}


QColor DataLayer::colorsAverage(EPalette palette) {
    if (palette == EPalette::eCustom) {
        return averageGroup(palette);
    } else {
        return mColorAverages[(uint32_t)palette];
    }
}



bool DataLayer::hasHueDevices() {
    int hueCount = 0;
    for (auto&& device = mCurrentDevices.begin(); device != mCurrentDevices.end(); ++device) {
        if (device->commType() == ECommType::eHue) hueCount++;
    }
    return (hueCount > 0);
}

bool DataLayer::hasNanoLeafDevices() {
    int nanoLeafCount = 0;
    for (auto&& device = mCurrentDevices.begin(); device != mCurrentDevices.end(); ++device) {
        if (device->commType() == ECommType::eNanoleaf) nanoLeafCount++;
    }
    return (nanoLeafCount > 0);
}

bool DataLayer::hasArduinoDevices() {
    int arduinoCount = 0;
    for (auto&& device = mCurrentDevices.begin(); device != mCurrentDevices.end(); ++device) {
        if (device->protocol() == EProtocolType::eArduCor) arduinoCount++;
    }
    return (arduinoCount > 0);
}

ERoutine DataLayer::currentRoutine() {
    std::vector<int> routineCount((int)ERoutine::eRoutine_MAX, 0);
    for (auto&& device = mCurrentDevices.begin(); device != mCurrentDevices.end(); ++device) {
        if (device->isReachable) {
            routineCount[(int)device->routine] = routineCount[(int)device->routine] + 1;
        }
    }
    std::vector<int>::iterator result = std::max_element(routineCount.begin(), routineCount.end());
    ERoutine modeRoutine = (ERoutine)std::distance(routineCount.begin(), result);
    return modeRoutine;
}

QJsonObject DataLayer::currentRoutineObject() {
    cor::Light light;
    light.routine = currentRoutine();
    light.color = mainColor();
    light.palette = palette();
    light.speed = speed();
    return lightToJson(light);
}

EPalette DataLayer::palette() {
    // count number of times each color group occurs
    std::vector<int> paletteCount((int)EPalette::ePalette_MAX, 0);
    for (auto&& device = mCurrentDevices.begin(); device != mCurrentDevices.end(); ++device) {
        if (device->isReachable) {
            paletteCount[(int)device->palette] = paletteCount[(int)device->palette] + 1;
        }
    }
    // find the most frequent color group occurence, return its index.
    std::vector<int>::iterator result = std::max_element(paletteCount.begin(), paletteCount.end());
    return (EPalette)std::distance(paletteCount.begin(), result);
}

const std::vector<QColor>& DataLayer::paletteColors() {
    if (palette() == EPalette::eCustom) {
        return mCurrentDevices.begin()->customColorArray;
    } else {
        return mColors[(int)palette()];
    }
}

void DataLayer::updateRoutine(const QJsonObject& routineObject) {
    std::list<cor::Light>::iterator iterator;
    ERoutine routine = stringToRoutine(routineObject["routine"].toString());
    int speed = INT_MIN;
    if (routine != ERoutine::eSingleSolid) {
        speed = routineObject["speed"].toDouble();
    }
    int hueCount = 0;
    for (iterator = mCurrentDevices.begin(); iterator != mCurrentDevices.end(); ++iterator) {
        iterator->routine = routine;
        if (routine != ERoutine::eSingleSolid) {
            iterator->speed = speed;
            if (iterator->protocol() == EProtocolType::eNanoleaf) {
                iterator->speed = MAX_SPEED - speed;
            }
        }

        if (routine <= cor::ERoutineSingleColorEnd) {
            QColor color(routineObject["red"].toDouble(),
                         routineObject["green"].toDouble(),
                         routineObject["blue"].toDouble());
            iterator->color = color;
        } else {
            EPalette palette = stringToPalette(routineObject["palette"].toString());
            iterator->palette = palette;

            /*!
             * hues are not individually addressable, mock a color group by setting
             * each individual light as a color
             */
            std::vector<QColor> colors = this->palette(palette);
            if (iterator->commType() == ECommType::eHue) {
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
        if (iterator->protocol() == EProtocolType::eArduCor) {
            finalSpeed = speed;
        } else if (iterator->commType() == ECommType::eNanoleaf) {
            finalSpeed = MAX_SPEED - speed;
        }
        iterator->speed = finalSpeed;
    }
    emit dataUpdate();
}

void DataLayer::updatePalette(EPalette palette) {
    std::list<cor::Light>::iterator iterator;
    int hueCount = 0;
    for (iterator = mCurrentDevices.begin(); iterator != mCurrentDevices.end(); ++iterator) {
        /*!
         * hues are not individually addressable, mock a color group by setting
         * each individual light as a color
         */
        std::vector<QColor> colors = this->palette(palette);
        if (iterator->commType() == ECommType::eHue) {
            int colorIndex = hueCount % colors.size();
            iterator->color = colors[colorIndex];
            // update the brightness to match the brightness of the color
            int brightness = iterator->color.toHsv().valueF() * 100;
            iterator->brightness = brightness;
            hueCount++;
        }
        iterator->palette = palette;
    }
    emit dataUpdate();
}

void DataLayer::updateCt(int ct) {
    std::list<cor::Light>::iterator iterator;
    for (iterator = mCurrentDevices.begin(); iterator != mCurrentDevices.end(); ++iterator) {
        iterator->color = cor::colorTemperatureToRGB(ct);
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
        if (iterator->commType() == ECommType::eHue) {
            iterator->colorMode = EColorMode::eHSV;
        } else {
            iterator->colorMode = EColorMode::eRGB;
        }

        i++;
        i = i % colors.size();
    }
    emit dataUpdate();
}

void DataLayer::updateBrightness(int brightness, std::list<cor::Light> specialCaseDevices) {
    std::list<cor::Light>::iterator iterator;
    for (iterator = mCurrentDevices.begin(); iterator != mCurrentDevices.end(); ++iterator) {
        iterator->brightness = brightness;
        if (iterator->commType() == ECommType::eHue
                || iterator->commType() == ECommType::eNanoleaf) {
            bool isSpecialCase = false;
            for (auto&& specialDevice : specialCaseDevices) {
                if (compareLight(specialDevice, *iterator)) {
                    isSpecialCase = true;
                }
            }
            if (!isSpecialCase) {
                // get the color as a HSV
                QColor hsv = iterator->color.toHsv();
                // update to new version of color
                iterator->color.setHsv(hsv.hue(), hsv.saturation(), (int)(255.0f * (brightness / 100.0f)));
            }
        }
        iterator->isOn = true;
    }
    emit dataUpdate();
}

void DataLayer::updateTimeout(int timeout) {
    mTimeout = timeout;
    emit settingsUpdate();
}

void DataLayer::updateCustomColorCount(uint32_t count) {
    std::list<cor::Light>::iterator iterator;
    for (iterator = mCurrentDevices.begin(); iterator != mCurrentDevices.end(); ++iterator) {
        iterator->customColorCount = count;
    }
    emit dataUpdate();
}

void DataLayer::updateCustomColorArray(int index, QColor color) {
    if (index <= 10) {
        std::list<cor::Light>::iterator iterator;
        for (iterator = mCurrentDevices.begin(); iterator != mCurrentDevices.end(); ++iterator) {
            if (iterator->commType() != ECommType::eHue) { // hues dont use this custom color array
                iterator->customColorArray[index] = color;
            }
        }
        emit dataUpdate();
    }
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


bool DataLayer::customColor(uint32_t index, QColor color) {
    if (index < mColors[0].size()) {
        mColors[0][index] = color;
        return true;
    }
    return false;
}


void DataLayer::enableTimeout(bool timeout) {
    mTimeoutEnabled = timeout;
    emit settingsUpdate();
}

uint32_t DataLayer::customColorsUsed() {
    uint32_t customColorCount = 0;
    for (auto&& device : mCurrentDevices) {
        customColorCount = customColorCount + device.customColorCount;
    }
    if (mCurrentDevices.size() > 1) {
        customColorCount = customColorCount / mCurrentDevices.size();
    }
    return customColorCount;
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
        if (!addDevice(device)) hasFailed = true;
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
