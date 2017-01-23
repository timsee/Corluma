/*!
 * \copyright
 * Copyright (C) 2015 - 2017.
 * Released under the GNU General Public License.
 */

#include "commtypesettings.h"

CommTypeSettings::CommTypeSettings() {
    mCommTypeCount = 0;
    mSettings = new QSettings();
    mDeviceInUse = std::vector<bool>((int)ECommType::eCommType_MAX);

    mCommTypeInUseSaveKeys = std::vector<QString>(4);
    mCommTypeInUseSaveKeys[0] = QString("HTTPInUse");
    mCommTypeInUseSaveKeys[1] = QString("UDPInUse");
    mCommTypeInUseSaveKeys[2] = QString("HueInUse");
    mCommTypeInUseSaveKeys[3] = QString("SerialInUse");

    for (int i = 0; i < (int)ECommType::eCommType_MAX; ++i) {
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
        mDeviceInUse[(int)ECommType::eHue] = true;
        mSettings->setValue(mCommTypeInUseSaveKeys[(int)ECommType::eHue], QString::number((int)false));
        mSettings->sync();
        mCommTypeCount++;
    }
}

bool CommTypeSettings::commTypeEnabled(ECommType type) {
    return mDeviceInUse[(int)type];
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
    mDeviceInUse[(int)type] = shouldEnable;

    mSettings->setValue(mCommTypeInUseSaveKeys[(int)type], QString::number((int)shouldEnable));
    mSettings->sync();
    return true;
}
