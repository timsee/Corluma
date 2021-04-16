/*!
 * \copyright
 * Copyright (C) 2015 - 2020.
 * Released under the GNU General Public License.
 */

#include "appsettings.h"

#include "utils/qt.h"

AppSettings::AppSettings() {
    mSettings = new QSettings();
    mTimeout = mSettings->value(cor::kTimeoutValue).toInt();
    mTimeoutEnabled = mSettings->value(cor::kUseTimeoutKey).toBool();
    mProtocolsInUse = std::vector<bool>(std::size_t(EProtocolType::MAX), false);

    std::vector<QString> keys = protocolKeys();
    for (std::size_t x = 0; x < mProtocolsInUse.size(); ++x) {
        if (mSettings->value(keys[x]).isValid()) {
            bool shouldEnable = mSettings->value(keys[x]).toBool();
            mProtocolsInUse[x] = shouldEnable;
        } else {
            mProtocolsInUse[x] = false;
            mSettings->setValue(keys[x], QString::number(int(false)));
        }
    }
    mSettings->sync();

    // error handling, if nothings enabled, enable hue, nanoleaf, arduCor
    if (numberEnabled() == 0) {
        mProtocolsInUse[std::size_t(EProtocolType::hue)] = true;
        mProtocolsInUse[std::size_t(EProtocolType::nanoleaf)] = true;
        mProtocolsInUse[std::size_t(EProtocolType::arduCor)] = true;
        mSettings->setValue(keys[std::size_t(EProtocolType::hue)], QString::number(int(true)));
        mSettings->setValue(keys[std::size_t(EProtocolType::nanoleaf)], QString::number(int(true)));
        mSettings->setValue(keys[std::size_t(EProtocolType::arduCor)], QString::number(int(true)));
        mSettings->sync();
    }
}

bool AppSettings::enabled(EProtocolType type) {
    return mProtocolsInUse[std::uint32_t(type)];
}

bool AppSettings::enable(EProtocolType type, bool shouldEnable) {
    // now that the edge case is out of the way, handle enabling or disabling the communication
    bool previouslyEnabled = enabled(type);
    if (!shouldEnable && previouslyEnabled && numberEnabled() == 1) {
        qDebug() << "WARNING: one commtype must always be active! Not removing commtype.";
        return false;
    }
    mProtocolsInUse[std::uint32_t(type)] = shouldEnable;

    mSettings->setValue(protocolKeys()[std::uint32_t(type)], QString::number(int(shouldEnable)));
    mSettings->sync();
    return true;
}

std::vector<QString> AppSettings::protocolKeys() {
    std::vector<QString> keys(std::size_t(EProtocolType::MAX), QString(""));
    for (std::size_t i = 0; i < keys.size(); ++i) {
        QString key = protocolToString(EProtocolType(i));
        key += "InUse";
        keys[i] = key;
    }
    return keys;
}

uint32_t AppSettings::numberEnabled() {
    return std::uint32_t(std::count(mProtocolsInUse.begin(), mProtocolsInUse.end(), true));
}


void AppSettings::updateTimeout(int timeout) {
    mTimeout = timeout;
    mSettings->setValue(cor::kTimeoutValue, QString::number(timeout));
    emit settingsUpdate();
    emit timeoutUpdate();
}

void AppSettings::enableTimeout(bool timeout) {
    mTimeoutEnabled = timeout;
    mSettings->setValue(cor::kUseTimeoutKey, QString::number(int(timeout)));
    emit settingsUpdate();
    emit timeoutUpdate();
}


void AppSettings::setToDefaults() {
    QSettings settings;
    settings.setValue(cor::kUseTimeoutKey, QString::number(int(false)));
    settings.setValue(cor::kTimeoutValue, QString::number(120));
}
