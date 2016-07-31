/*!
 * \copyright
 * Copyright (C) 2015 - 2016.
 * Released under the GNU General Public License.
 */

#include "commtype.h"

#define DEFAULT_IP "192.168.0.125"

void CommType::setupConnectionList(ECommType type) {
    mType = type;
    mControllerListMaxSize = 5;
    mControllerIndex = 0;
    mSelectedDevice = 1;
    mControllerListCurrentSize = mSettings.value(settingsListSizeKey()).toInt();
    mIsConnected = false;

    // handles an edge case thats more likely to come up in debugging than typical use
    // but it would cause a crash either way...
    if (mControllerListCurrentSize >= mControllerListMaxSize) {
        qDebug() << "WARNING! You have too many saved settings. Reverting all.";
        mControllerListCurrentSize = 0;
    }
    // create the mList object
    mControllerList =  std::shared_ptr<std::vector<QString> > (new std::vector<QString>(mControllerListMaxSize, nullptr));
    mDeviceList =  std::shared_ptr<std::vector<SLightDevice> > (new std::vector<SLightDevice>(1));

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
           if (value.compare(QString("")) != 0) (*mControllerList.get())[i] = value;
        }
    }
}

bool CommType::addConnection(QString connection) {
    // check both that the connection is valid and that it doesn't already exist in the list
    if (checkIfConnectionIsValid(connection) && !checkIfConnectionExistsInList(connection)) {
        // when less than the max size of connections have been saved, update these values
        if (mControllerList->size() <= mControllerListMaxSize) {
            // copy the current vector
            std::vector<QString> tempCopy = *mControllerList.get();
            // add the connection to the front of the list
            (*mControllerList.get())[0] = connection;
            // move all values one right and throw out the last value of the list
            for (int i = 1; i < mControllerList->size(); ++i) {
               (*mControllerList.get())[i] = tempCopy[i - 1];
            }
            mControllerIndex = 0;
            mControllerListCurrentSize++;
            return true;
        }
    }
    return false;
}

bool CommType::selectConnection(QString connection) {
    // check if connection exists
    if (checkIfConnectionExistsInList(connection)) {
        // find the connection's index and save that as mListIndex
        for (int i = 0; i < mControllerList->size(); ++i) {
            if (connection.compare((*mControllerList.get())[i]) == 0) {
                mControllerIndex = i;
                return true;
            }
        }
    }
    return false;
}

bool CommType::selectConnection(int connectionIndex) {
    if (connectionIndex < mControllerList->size()) {
        mControllerIndex = connectionIndex;
        return true;
    }
    return false;
}


bool CommType::removeConnection(QString connection) {
    // check if connection exists in list and that its not the only connection
    if (checkIfConnectionExistsInList(connection) && (mControllerList->size() > 1)) {
        // get index of connection we're removing
        int tempIndex;
        for (int i = 0; i < mControllerList->size(); ++i) {
            if (connection.compare((*mControllerList.get())[i]) == 0) tempIndex = i;
        }
        // handle edge case
        if (tempIndex == (mControllerList->size() - 1)) {
           (*mControllerList.get())[tempIndex] = QString("");
        } else {
            // copy the vector
            std::vector<QString> tempCopy = *mControllerList.get();
            // move all values to the right of the index one left
            for (int i = tempIndex; i < mControllerList->size(); ++i) {
               (*mControllerList.get())[i] = tempCopy[i + 1];
            }
        }
        // reduce the overall size.
        if ((mControllerList->size() != 0)) mControllerListCurrentSize--;
        return true;
    }
    return false;
}

void CommType::saveConnectionList() {
     // take currently selected and reorder it to the top of the list
     if (mControllerIndex != 0) {
         // copy the vector
         std::vector<QString> tempCopy = *mControllerList.get();
         // set the selected index at the front of the vector
         (*mControllerList.get())[0] = (*mControllerList.get())[mControllerIndex];
         int j = 0;
         // move all values less than than mListIndex one index right
         // then keep all other values in the same place.
         for (int i = 1; i < mControllerList->size(); ++i) {
            (*mControllerList.get())[i] = tempCopy[j];
            j++;
            if (j == mControllerIndex) j++;
         }
     }
    // save the current size
    mSettings.setValue(settingsListSizeKey(), mControllerListCurrentSize);
    // save the values in the list
    for (int i = 0; i < mControllerListCurrentSize; ++i) {
        mSettings.setValue(settingsIndexKey(i), (*mControllerList.get())[i]);
    }
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
    for (int i = 0; i < mControllerList->size(); ++i) {
        if (connection.compare((*mControllerList.get())[i]) == 0) return true;
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


void CommType::updateDevice(SLightDevice device) {
    if ((size_t)(device.index) > mDeviceList->size()) {
        //TODO: remove this edge case
        if (device.index > 3) {
            device.index = 3;
        }
        mDeviceList->resize(device.index);
        //TODO: update individual values
        (*mDeviceList.get())[device.index - 1] = device;
    } else {
        (*mDeviceList.get())[device.index - 1] = device;
    }
}

void CommType::updateDeviceColor(int i, QColor color) {
    if ((size_t)i < mDeviceList->size()) {
        (*mDeviceList.get())[i].color = color;
    }
}

