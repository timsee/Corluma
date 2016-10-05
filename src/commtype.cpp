/*!
 * \copyright
 * Copyright (C) 2015 - 2016.
 * Released under the GNU General Public License.
 */

#include "commtype.h"

#define DEFAULT_IP "192.168.0.125"

void CommType::setupConnectionList(ECommType type) {
    mType = type;
    mSelectedDevice = 0;
    mControllerIndex = 0;
    mControllerListCurrentSize = mSettings.value(settingsListSizeKey()).toInt();

    // handles an edge case thats more likely to come up in debugging than typical use
    // but it would cause a crash either way...
    if (mControllerListCurrentSize >= 10) {
        qDebug() << "WARNING! You have too many saved settings. Reverting all.";
        mControllerListCurrentSize = 0;
    }
    // create the mList object
    mControllerList = new std::deque<QString>(10, nullptr);
    mDeviceList = new std::deque<std::vector<SLightDevice> >(10, std::vector<SLightDevice>(10));
    // add in a default for specific cases
    if (mControllerListCurrentSize == 0) {
        if(mType == ECommType::eHTTP
             || mType == ECommType::eUDP ) {
            addConnection(DEFAULT_IP);
        }
    } else {
        // load preexisting settings, if they exist
        for (int i = 0; i < mControllerListCurrentSize; ++i) {
           QString value = mSettings.value(settingsIndexKey(i)).toString();
           if (value.compare(QString("")) != 0) addConnection(value, false);
        }
    }
}

bool CommType::addConnection(QString connection, bool shouldIncrement) {
    // check both that the connection is valid and that it doesn't already exist in the list
    if (checkIfConnectionIsValid(connection) && !checkIfConnectionExistsInList(connection)) {
        // when less than the max size of connections have been saved, update these values
        // add the connection to the front of the list
        (*mControllerList).push_front(connection);

        // add a default device
        SLightDevice device;
        device.index = 1;
        device.isReachable = false;

        SLightDevice invalidDevice;
        invalidDevice.isValid = false;
        invalidDevice.isReachable = false;


        std::vector<SLightDevice> tempNewList(10, invalidDevice);
        tempNewList[0] = device;
        (*mDeviceList).push_front(tempNewList);

        mControllerIndex = 0;
        if (shouldIncrement) {
            mControllerListCurrentSize++;
        }
        return true;
    }
    return false;
}

bool CommType::selectConnection(QString connection) {
    // check if connection exists
    if (checkIfConnectionExistsInList(connection)) {
        // find the connection's index and save that as mListIndex
        for (uint32_t i = 0; i < mControllerList->size(); ++i) {
            if (connection.compare((*mControllerList)[i]) == 0) {
                mControllerIndex = i;
                return true;
            }
        }
    }
    return false;
}

bool CommType::selectConnection(int connectionIndex) {
    if ((uint32_t)connectionIndex < mControllerList->size()) {
        mControllerIndex = connectionIndex;
        return true;
    }
    return false;
}


bool CommType::removeConnection(QString connection) {
    int controllerIndex = controllerIndexByName(connection);
    //qDebug()  << "contrroller index" << controllerIndex
    //          << "num of devices" << numberOfConnectedDevices(controllerIndex)
    //          << "num controllers" <<numberOfControllers()
    //          << "type" << ECommTypeToString(mType);
    // check if connection exists in list and that its not the only connection
    if (checkIfConnectionExistsInList(connection)
            && (numberOfControllers() > 1)
            && (controllerIndex > -1)) {
        // get index of connection we're removing
        int tempIndex;
        for (int i = 0; i < mControllerListCurrentSize; ++i) {
            if (connection.compare((*mControllerList)[i]) == 0) tempIndex = i;
        }
        // handle edge case
        if (tempIndex == (mControllerListCurrentSize - 1)) {
           (*mControllerList)[tempIndex] = QString("");
        } else if (mControllerListCurrentSize > 1) {
            (*mControllerList).erase((*mControllerList).begin() + tempIndex);
            (*mDeviceList).erase((*mDeviceList).begin() + tempIndex);
        }
        // reduce the overall size.
        if ((mControllerListCurrentSize != 0)) mControllerListCurrentSize--;
        return true;
    }
    return false;
}

void CommType::saveConnectionList() {
    // save the values in the list
    int testCount = mControllerListCurrentSize;
    int x = 0;
    for (int i = 0; i < testCount; ++i) {
        if((*mControllerList)[i].compare(QString("")) == 0) {
            testCount--;
        } else {
            mSettings.setValue(settingsIndexKey(x), (*mControllerList)[x]);
            x++;
        }
    }
    // save the current size
    mSettings.setValue(settingsListSizeKey(), testCount);
    // write settings to disk
    mSettings.sync();
}


QString CommType::settingsIndexKey(int index) {
    QString typeID;
    if (mType == ECommType::eHTTP) {
        typeID = QString("HTTP");
    } else if (mType == ECommType::eUDP) {
        typeID = QString("UDP");
    } else if (mType == ECommType::eHue) {
        typeID = QString("HUE");
    }
#ifndef MOBILE_BUILD
    else if (mType == ECommType::eSerial) {
        typeID = QString("SERIAL");
    }
#endif //MOBILE_BUILD
    return (QString("CommList_%1_%2_Key").arg(typeID,
                                              QString::number(index)));
}

QString CommType::settingsListSizeKey() {
    QString typeID;
    if (mType == ECommType::eHTTP) {
        typeID = QString("HTTP");
    } else if (mType == ECommType::eUDP) {
        typeID = QString("UDP");
    } else if (mType == ECommType::eHue) {
        typeID = QString("HUE");
    }
#ifndef MOBILE_BUILD
    else if (mType == ECommType::eSerial) {
        typeID = QString("SERIAL");
    }
#endif //MOBILE_BUILD
    return (QString("CommList_%1_Size_Key").arg(typeID));
}


bool CommType::checkIfConnectionExistsInList(QString connection) {
    for (int i = 0; i < mControllerListCurrentSize; ++i) {
        if (connection.compare((*mControllerList)[i]) == 0) return true;
    }
    return false;
}


bool CommType::checkIfConnectionIsValid(QString connection) {
    //TODO: write a stronger check...
    if (mType == ECommType::eHTTP || mType == ECommType::eUDP) {
        if (connection.count(QLatin1Char('.') != 3)) return false;
    }
    return true;
}


void CommType::updateDevice(int controller, SLightDevice device) {
    //qDebug() << "update device" << controller << "indevx" << device.index << "for type" << ECommTypeToString(mType);
    if ((size_t)device.index > ((*mDeviceList)[controller]).size()) {
        (*mDeviceList)[controller].resize(device.index);
        (*mDeviceList)[controller][device.index - 1] = device;
    } else {
        //qDebug() << "adding this" << device.index;
        ((*mDeviceList)[controller])[device.index - 1] = device;
    }
}

void CommType::updateDeviceColor(int i, QColor color) {
    if ((size_t)i < mDeviceList[mControllerIndex].size()) {
        (*mDeviceList)[mControllerIndex][i].color = color;
    } else {

    }
}

int CommType::controllerIndexByName(QString name) {
    int returnValue = -1;
    for (uint32_t i = 0; i < mControllerList->size(); ++i) {
        if(!QString::compare((*mControllerList)[i], name)
                && QString::compare(QString(""), name)) {
            returnValue = i;
        }
    }
    return returnValue;
}


void CommType::selectDevice(int controller, uint32_t i) {
    if (i - 1 <= mDeviceList[controller].size()) {
         mSelectedDevice = i - 1;
    }
}

int CommType::numberOfConnectedDevices(uint32_t controllerIndex)
{
    if (controllerIndex < (*mControllerList).size()) {
        // return only valid device count
        int x = 0;
        for (uint32_t i = 0; i < (*mControllerList).size(); ++i) {
            if ((*mDeviceList)[controllerIndex][i].isValid) {
                x++;
            }
        }
        return x;
    } else {
        qDebug() << "WARNING: looking for a connected device that doesn't exist!";
        return -1;
    }
}

bool CommType::deviceByControllerAndIndex(SLightDevice& device, int controllerIndex, int deviceIndex)
{
    if (controllerIndex < (int)mDeviceList->size()) {
        if(deviceIndex < (int)((*mDeviceList)[controllerIndex]).size()) {
            device = ((*mDeviceList)[controllerIndex])[deviceIndex];
            // check if null
            return !(device.index == 0);
        }
    }
    return false;
}

QString CommType::currentConnection() {
    return (*mControllerList)[mControllerIndex];
}
