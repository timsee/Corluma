/*!
 * \copyright
 * Copyright (C) 2015 - 2017.
 * Released under the GNU General Public License.
 */

#include "commtypesettings.h"
#include "corlumautils.h"

CommTypeSettings::CommTypeSettings() {
    mCommTypeCount = 0;
    mSettings = new QSettings();
    mDeviceInUse = std::vector<bool>(3);

    mCommTypeInUseSaveKeys = std::vector<QString>(3);
    mCommTypeInUseSaveKeys[0] = kUseYunKey;
    mCommTypeInUseSaveKeys[1] = kUseSerialKey;
    mCommTypeInUseSaveKeys[2] = kUseHueKey;

    std::vector<ECommType> usedTypes = commTypes();
    bool useAdvanceMode = mSettings->value(kAdvanceModeKey).toBool();
    for (uint32_t x = 0; x < usedTypes.size(); ++x) {
        int i = indexOfCommTypeSettings(usedTypes[x]);
        if (!useAdvanceMode
                && (usedTypes[x] == ECommType::eHTTP
    #ifndef MOBILE_BUILD
                || usedTypes[x] == ECommType::eSerial
    #endif //MOBILE_BUILD
                || usedTypes[x] == ECommType::eUDP)) {
            // its not advance mode, don't enable this!
            mDeviceInUse[(int)i] = false;
            mSettings->setValue(mCommTypeInUseSaveKeys[(int)i], QString::number((int)false));
        } else {
            if (mSettings->value(mCommTypeInUseSaveKeys[i]).toString().compare("") != 0) {
                bool shouldEnable = mSettings->value(mCommTypeInUseSaveKeys[i]).toBool();
                mDeviceInUse[(int)i] = shouldEnable;
                if (shouldEnable) mCommTypeCount++;
            } else {
                mDeviceInUse[(int)i] = false;
                mSettings->setValue(mCommTypeInUseSaveKeys[(int)i], QString::number((int)false));
            }
        }
    }
    mSettings->sync();

    //error handling, must always have at least one stream!
    if (mCommTypeCount == 0) {
        mDeviceInUse[indexOfCommTypeSettings(ECommType::eHue)] = true;
        mSettings->setValue(mCommTypeInUseSaveKeys[indexOfCommTypeSettings(ECommType::eHue)], QString::number((int)true));
        mSettings->sync();
        mCommTypeCount++;
    }
}

bool CommTypeSettings::commTypeEnabled(ECommType type) {
    return mDeviceInUse[indexOfCommTypeSettings(type)];
}

int CommTypeSettings::indexOfCommTypeSettings(ECommType type) {
    int index = -1;
    switch (type) {
        case ECommType::eHTTP:
        case ECommType::eUDP:
            index = 0;
            break;
#ifndef MOBILE_BUILD
        case ECommType::eSerial:
            index = 1;
            break;
#endif //MOBILE_BUILD
        case ECommType::eHue:
#ifndef MOBILE_BUILD
            index = 2;
#else
            index = 1;
#endif
            break;
        default:
            throw "got invalid commtype in commtypesettings";
            index = 0;
            break;
    }
    return index;
}

std::vector<ECommType> CommTypeSettings::commTypes() {
    std::vector<ECommType> usedTypes = {ECommType::eUDP,
                                    #ifndef MOBILE_BUILD
                                        ECommType::eSerial,
                                    #endif //MOBILE_BUILD
                                        ECommType::eHue };
    return usedTypes;
}

bool CommTypeSettings::enableCommType(ECommType type, bool shouldEnable) {
    bool useAdvanceMode = mSettings->value(kAdvanceModeKey).toBool();
    // check if trying to enable connections that can't be enabled without advanced mode
    if (!useAdvanceMode && shouldEnable
            && (type == ECommType::eHTTP
#ifndef MOBILE_BUILD
            || type == ECommType::eSerial
#endif //MOBILE_BUILD
            || type == ECommType::eUDP)) {
        return false;
    }
    // now that the edge case is out of the way, handle enabling or disabling the communication
    bool previouslyEnabled = commTypeEnabled(type);
    if (!shouldEnable && previouslyEnabled && mCommTypeCount == 1) {
        qDebug() << "WARNING: one commtype must always be active! Not removing commtype." << mCommTypeCount;
        return false;
    }
    if (shouldEnable && !previouslyEnabled) {
        mCommTypeCount++;
    }
    if (!shouldEnable && previouslyEnabled) {
        mCommTypeCount--;
    }
    mDeviceInUse[indexOfCommTypeSettings(type)] = shouldEnable;

    mSettings->setValue(mCommTypeInUseSaveKeys[indexOfCommTypeSettings(type)], QString::number((int)shouldEnable));
    mSettings->sync();
    return true;
}