/*!
 * \copyright
 * Copyright (C) 2015 - 2017.
 * Released under the GNU General Public License.
 */

#include "commtypesettings.h"

CommTypeSettings::CommTypeSettings() {
    mCommTypeCount = 0;
    mSettings = new QSettings();
    mDeviceInUse = std::vector<bool>(3);

    mCommTypeInUseSaveKeys = std::vector<QString>(3);
    mCommTypeInUseSaveKeys[0] = QString("YunInUse");
    mCommTypeInUseSaveKeys[1] = QString("SerialInUse");
    mCommTypeInUseSaveKeys[2] = QString("HueInUse");

    std::vector<ECommType> usedTypes = commTypes();
    for (uint32_t x = 0; x < usedTypes.size(); ++x) {
        int i = indexOfCommTypeSettings(usedTypes[x]);
        if (mSettings->value(mCommTypeInUseSaveKeys[i]).toString().compare("") != 0) {
            bool shouldEnable = mSettings->value(mCommTypeInUseSaveKeys[i]).toBool();
            mDeviceInUse[(int)i] = shouldEnable;
            if (shouldEnable) mCommTypeCount++;
        } else {
            mDeviceInUse[(int)i] = false;
            mSettings->setValue(mCommTypeInUseSaveKeys[(int)i], QString::number((int)false));
            mSettings->sync();
        }
    }

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
