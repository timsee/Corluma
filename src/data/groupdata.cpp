#include "groupdata.h"

GroupData::GroupData(QObject* parent) : QObject(parent) {}

void GroupData::saveNewGroup(const cor::Group& group) {
    auto groupObject = group.toJson();
    if (!group.lights().empty()) {
        // check that it doesn't already exist, if it does, replace the old version
        auto key = QString::number(group.uniqueID()).toStdString();
        auto dictResult = mGroupDict.item(key);
        if (dictResult.second) {
            mGroupDict.update(key, group);
        } else {
            mGroupDict.insert(key, group);
        }

        emit groupAdded(group.name());
    }
}

QString GroupData::nameFromID(std::uint64_t ID) {
    auto group = groupFromID(ID);
    return group.name();
}

std::vector<QString> GroupData::groupNamesFromIDs(std::vector<std::uint64_t> IDs) {
    std::vector<QString> nameVector;
    nameVector.reserve(IDs.size());
    for (const auto& subGroupID : IDs) {
        auto groupResult = mGroupDict.item(QString::number(subGroupID).toStdString());
        // check if group is already in this list
        if (groupResult.second) {
            // check if its already in the sub group names
            auto widgetResult =
                std::find(nameVector.begin(), nameVector.end(), groupResult.first.name());
            if (widgetResult == nameVector.end()) {
                nameVector.emplace_back(groupResult.first.name());
            }
        }
    }
    return nameVector;
}

cor::Group GroupData::groupFromID(std::uint64_t ID) {
    auto groupResult = mGroupDict.item(QString::number(ID).toStdString());
    // check if group is already in this list
    if (groupResult.second) {
        return groupResult.first;
    }
    return {};
}


std::vector<cor::Group> GroupData::groupsFromIDs(std::vector<std::uint64_t> IDs) {
    std::vector<cor::Group> retVector;
    retVector.reserve(IDs.size());
    for (const auto& subGroupID : IDs) {
        auto groupResult = mGroupDict.item(QString::number(subGroupID).toStdString());
        // check if group is already in this list
        if (groupResult.second) {
            retVector.emplace_back(groupResult.first);
        }
    }
    return retVector;
}


std::uint64_t GroupData::groupNameToID(const QString name) {
    for (const auto& group : mGroupDict.items()) {
        if (name == group.name()) {
            return group.uniqueID();
        }
    }

    return std::numeric_limits<std::uint64_t>::max();
}

QString GroupData::removeGroup(std::uint64_t uniqueID) {
    QString name;
    for (const auto& group : mGroupDict.items()) {
        if (group.uniqueID() == uniqueID) {
            mGroupDict.remove(group);
            name = group.name();
        }
    }
    return name;
}

std::vector<QString> GroupData::groupNames() {
    auto groups = mGroupDict.items();
    std::vector<QString> retVector;
    retVector.reserve(groups.size());
    for (const auto& group : groups) {
        retVector.emplace_back(group.name());
    }
    return retVector;
}

bool GroupData::removeLightFromGroups(const QString& uniqueID) {
    bool anyUpdates = false;

    // parse the groups, remove from dict if needed
    for (const auto& group : mGroupDict.items()) {
        for (const auto& groupLight : group.lights()) {
            if (group.lights().size() == 1 && (groupLight == uniqueID)) {
                // edge case where the mood only exists for this one light, remove it
                // entirely
                mGroupDict.removeKey(QString::number(group.uniqueID()).toStdString());
                anyUpdates = true;
            } else if (groupLight == uniqueID) {
                // standard case, make a new mood without the light
                auto newGroup = group.removeLight(uniqueID);
                mGroupDict.update(QString::number(group.uniqueID()).toStdString(), newGroup);
                anyUpdates = true;
            }
        }
    }
    return anyUpdates;
}


std::uint64_t GroupData::generateNewUniqueKey() {
    std::uint64_t maxKey = 0u;
    for (const auto& group : mGroupDict.items()) {
        if (group.uniqueID() > maxKey
            && (group.uniqueID() < std::numeric_limits<unsigned long>::max() / 2)) {
            maxKey = group.uniqueID();
        }
    }
    return maxKey + 1;
}

namespace {

void updateGroup(cor::Group& group, const cor::Group& externalGroup) {
    // merge new lights into it
    for (const auto& externalLightID : externalGroup.lights()) {
        // search for old light
        auto result = std::find(group.lights().begin(), group.lights().end(), externalLightID);
        // add if not found
        if (result == group.lights().end()) {
            auto lights = group.lights();
            lights.push_back(externalLightID);
            group.lights(lights);
        }
    }
}

} // namespace

void GroupData::updateExternallyStoredGroups(const std::vector<cor::Group>& externalGroups) {
    // fill in subgroups for each room
    for (const auto& externalGroup : externalGroups) {
        const auto& key = QString::number(externalGroup.uniqueID()).toStdString();
        // check if it exists in group already
        auto lookupResult = mGroupDict.item(key);
        if (lookupResult.second) {
            // get existing group
            auto groupCopy = lookupResult.first;
            updateGroup(groupCopy, externalGroup);
            mGroupDict.update(key, groupCopy);
        } else {
            bool foundGroup = false;
            for (const auto& internalGroup : mGroupDict.items()) {
                if (internalGroup.name() == externalGroup.name()) {
                    auto groupCopy = internalGroup;
                    updateGroup(groupCopy, externalGroup);
                    mGroupDict.update(QString::number(internalGroup.uniqueID()).toStdString(),
                                      groupCopy);
                    foundGroup = true;
                }
            }
            if (!foundGroup) {
                auto result = mGroupDict.insert(key, externalGroup);
                if (!result) {
                    qDebug() << " insert failed" << externalGroup.name();
                }
            }
        }
    }
}
