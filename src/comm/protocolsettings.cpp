/*!
 * \copyright
 * Copyright (C) 2015 - 2018.
 * Released under the GNU General Public License.
 */

#include "protocolsettings.h"
#include "cor/utils.h"

ProtocolSettings::ProtocolSettings() {
    mSettings = new QSettings();
    mProtocolsInUse = std::vector<bool>((size_t)EProtocolType::eProtocolType_MAX, false);

    std::vector<QString> keys = protocolKeys();

    for (uint32_t x = 0; x < mProtocolsInUse.size(); ++x) {
        if (mSettings->value(keys[x]).toString().compare("") != 0) {
            bool shouldEnable = mSettings->value(keys[x]).toBool();
            mProtocolsInUse[x] = shouldEnable;
        } else {
            mProtocolsInUse[x] = false;
            mSettings->setValue(keys[x], QString::number((int)false));
        }
    }
    mSettings->sync();

    //error handling, must always have at least one stream!
    if (numberEnabled() == 0) {
        mProtocolsInUse[(size_t)EProtocolType::eArduCor] = true;
        mSettings->setValue(keys[(uint32_t)EProtocolType::eHue], QString::number((int)true));
        mSettings->sync();
    }
}

bool ProtocolSettings::enabled(EProtocolType type) {
    return mProtocolsInUse[(uint32_t)type];
}

bool ProtocolSettings::enable(EProtocolType type, bool shouldEnable) {
    // now that the edge case is out of the way, handle enabling or disabling the communication
    bool previouslyEnabled = enabled(type);
    if (!shouldEnable && previouslyEnabled && numberEnabled() == 1) {
        qDebug() << "WARNING: one commtype must always be active! Not removing commtype.";
        return false;
    }
    mProtocolsInUse[(uint32_t)type] = shouldEnable;

    mSettings->setValue(protocolKeys()[(uint32_t)type], QString::number((int)shouldEnable));
    mSettings->sync();
    return true;
}

std::vector<QString> ProtocolSettings::protocolKeys() {
    std::vector<QString> keys((size_t)EProtocolType::eProtocolType_MAX);
    for (uint32_t i = 0; i < keys.size(); ++i) {
        QString key = protocolToString((EProtocolType)i);
        key += "InUse";
        keys[i] = key;
    }
    return keys;
}

uint32_t ProtocolSettings::numberEnabled() {
   return std::count(mProtocolsInUse.begin(), mProtocolsInUse.end(), true);
}
