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
            mDeviceInUse[(int)i] = true;
        }
    }

    //error handling, must always have at least one stream!
    if (mCommTypeCount == 0) {
        mDeviceInUse[(int)ECommType::eHue] = true;
        mCommTypeCount++;
    }

    // check if a default commtype already exists
    ECommType previousType = ECommType::eCommType_MAX;
    if (mSettings->value(kCommDefaultType).toString().compare("") != 0) {
        previousType = (ECommType)mSettings->value(kCommDefaultType).toInt();
    }

    // set the applications commtype based off of the enabled commtypes and the
    // previous settings.
    if (mCommTypeCount == 1) {
        int x = 0;
        for (int i = 0; i <  mDeviceInUse.size(); ++i) {
            if (mDeviceInUse[i]) x = i;
        }
        mDefaultCommType = (ECommType)x;
    } else if (previousType == ECommType::eCommType_MAX) {
        // no previous type found, so find last connection.
        int x = 0;
        for (int i = 0; i <  mDeviceInUse.size(); ++i) {
            if (mDeviceInUse[i]) x = i;
        }
        mDefaultCommType = (ECommType)x;
    } else {
        //no default comm type, default to first commtype connected
        mDefaultCommType = previousType;
    }
}

bool CommTypeSettings::commTypeEnabled(ECommType type) {
    return mDeviceInUse[(int)type];
}

void CommTypeSettings::changeDefaultCommType(ECommType type) {
    mDefaultCommType = type;
    mSettings->setValue(kCommDefaultType, QString::number((int)type));
    mSettings->sync();
}

ECommType CommTypeSettings::defaultCommType() {
    return mDefaultCommType;
}

bool CommTypeSettings::enableCommType(ECommType type, bool shouldEnable) {
    bool previouslyEnabled = commTypeEnabled(type);
    if (!shouldEnable && previouslyEnabled && mCommTypeCount == 1) {
        qDebug() << "WARNING: one commtype must always be active! Not removing commtype.";
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

const QString CommTypeSettings::kCommDefaultType = QString("CommDefaultType");
