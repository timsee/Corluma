#include "mooddata.h"
#include "cor/objects/group.h"

MoodData::MoodData() : QObject() {}

cor::Mood MoodData::moodFromID(const cor::UUID& ID) {
    auto moodResult = mMoodDict.item(ID.toStdString());
    // check if group is already in this list
    if (moodResult.second) {
        return moodResult.first;
    }
    return {};
}

QString MoodData::nameFromID(const cor::UUID& ID) {
    auto mood = moodFromID(ID);
    return mood.name();
}

void MoodData::saveNewMood(const cor::Mood& mood) {
    const auto& key = mood.uniqueID().toStdString();

    // check that it doesn't already exist, if it does, replace the old version
    auto dictResult = mMoodDict.item(key);
    if (dictResult.second) {
        mMoodDict.update(key, mood);
    } else {
        mMoodDict.insert(key, mood);
    }
    emit moodAdded(mood.name());
}

bool MoodData::removeLightFromMoods(const cor::LightID& uniqueID) {
    bool anyUpdates = false;
    // parse the moods, remove from dict if needed
    for (const auto& mood : mMoodDict.items()) {
        for (const auto& moodLight : mood.lights()) {
            if (mood.lights().size() == 1 && (moodLight.uniqueID() == uniqueID)) {
                // edge case where the mood only exists for this one light, remove it
                // entirely
                mMoodDict.removeKey(mood.uniqueID().toStdString());
                anyUpdates = true;
            } else if (moodLight.uniqueID() == uniqueID) {
                // standard case, make a new mood without the light
                auto newMood = mood.removeLight(moodLight.uniqueID());
                mMoodDict.update(mood.uniqueID().toStdString(), newMood);
                anyUpdates = true;
            }
        }
    }
    return anyUpdates;
}


QString MoodData::removeMood(const cor::UUID& uniqueID) {
    QString name;
    for (const auto& mood : mMoodDict.items()) {
        if (mood.uniqueID() == uniqueID) {
            auto result = mMoodDict.removeKey(uniqueID.toStdString());
            if (!result) {
                return {};
            } else {
                emit moodDeleted(mood.name());
            }
            name = mood.name();
        }
    }
    return name;
}

QJsonArray MoodData::toJsonArray() {
    QJsonArray array;

    // add all the moods
    for (const auto& mood : mMoodDict.items()) {
        array.append(mood.toJson());
    }

    return array;
}


void MoodData::loadFromJson(const QJsonArray& array) {
    for (auto value : array) {
        QJsonObject object = value.toObject();
        if (cor::Mood::isValidJson(object)) {
            cor::Mood mood(object);
            mMoodDict.insert(mood.uniqueID().toStdString(), mood);
        }
    }
}
