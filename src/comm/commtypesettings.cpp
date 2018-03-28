/*!
 * \copyright
 * Copyright (C) 2015 - 2018.
 * Released under the GNU General Public License.
 */

#include "commtypesettings.h"
#include "cor/utils.h"

CommTypeSettings::CommTypeSettings() {
    mSettings = new QSettings();
    mCommTypesInUse = std::vector<bool>(3, false);

    mCommTypeInUseSaveKeys = std::vector<QString>(3);
    mCommTypeInUseSaveKeys[0] = kUseHueKey;
    mCommTypeInUseSaveKeys[1] = kUseNanoLeafKey;
    mCommTypeInUseSaveKeys[2] = kUseArduCorKey;

    bool useAdvanceMode = mSettings->value(kAdvanceModeKey).toBool();
    for (uint32_t x = 0; x < mCommTypesInUse.size(); ++x) {
        if (!useAdvanceMode && ((ECommTypeSettings)x == ECommTypeSettings::eArduCor)) {
            // its not advance mode, don't enable this!
            mCommTypesInUse[x] = false;
            mSettings->setValue(mCommTypeInUseSaveKeys[x], QString::number((int)false));
        } else {
            if (mSettings->value(mCommTypeInUseSaveKeys[x]).toString().compare("") != 0) {
                bool shouldEnable = mSettings->value(mCommTypeInUseSaveKeys[x]).toBool();
                mCommTypesInUse[x] = shouldEnable;
            } else {
                mCommTypesInUse[x] = false;
                mSettings->setValue(mCommTypeInUseSaveKeys[x], QString::number((int)false));
            }
        }
    }
    mSettings->sync();

    //error handling, must always have at least one stream!
    if (commTypeSettingsEnabled() == 0) {
        mCommTypesInUse[1] = true; // this is Hue
        mSettings->setValue(mCommTypeInUseSaveKeys[(uint32_t)ECommTypeSettings::eHue], QString::number((int)true));
        mSettings->sync();
    }
}

bool CommTypeSettings::commTypeEnabled(ECommType type) {
    return mCommTypesInUse[(uint32_t)cor::convertCommTypeToCommTypeSettings(type)];
}

bool CommTypeSettings::commTypeSettingsEnabled(ECommTypeSettings type) {
    return mCommTypesInUse[(uint32_t)type];
}

bool CommTypeSettings::enableCommType(ECommTypeSettings type, bool shouldEnable) {
    bool useAdvanceMode = mSettings->value(kAdvanceModeKey).toBool();
    // check if trying to enable connections that can't be enabled without advanced mode
    if (!useAdvanceMode && shouldEnable && type == ECommTypeSettings::eArduCor) {
        return false;
    }
    // now that the edge case is out of the way, handle enabling or disabling the communication
    bool previouslyEnabled = commTypeSettingsEnabled(type);
    if (!shouldEnable && previouslyEnabled && commTypeSettingsEnabled() == 1) {
        qDebug() << "WARNING: one commtype must always be active! Not removing commtype.";
        return false;
    }
    mCommTypesInUse[(uint32_t)type] = shouldEnable;

    mSettings->setValue(mCommTypeInUseSaveKeys[(uint32_t)type], QString::number((int)shouldEnable));
    mSettings->sync();
    return true;
}

uint32_t CommTypeSettings::commTypeSettingsEnabled() {
   return std::count(mCommTypesInUse.begin(), mCommTypesInUse.end(), true);
}
