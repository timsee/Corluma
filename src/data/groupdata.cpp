#include "groupdata.h"

GroupData::GroupData(QObject* parent) : QObject(parent) {}

void GroupData::saveNewGroup(const cor::Group& group) {
    auto groupObject = group.toJson();
    if (!group.lights().empty()) {
        // check that it doesn't already exist, if it does, replace the old version
        auto key = group.uniqueID().toStdString();
        auto dictResult = mGroupDict.item(key);
        if (dictResult.second) {
            mGroupDict.update(key, group);
        } else {
            mGroupDict.insert(key, group);
        }

        emit groupAdded(group.name());
    }
}

QString GroupData::nameFromID(const cor::UUID& ID) {
    if (ID == cor::kMiscGroupKey) {
        return "Miscellaneous";
    }
    auto group = groupFromID(ID);
    return group.name();
}

std::vector<QString> GroupData::groupNamesFromIDs(const std::vector<cor::UUID>& IDs) {
    std::vector<QString> nameVector;
    nameVector.reserve(IDs.size());
    for (const auto& subGroupID : IDs) {
        auto groupResult = mGroupDict.item(subGroupID.toStdString());
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

cor::Group GroupData::groupFromID(const cor::UUID& ID) {
    auto groupResult = mGroupDict.item(ID.toStdString());
    // check if group is already in this list
    if (groupResult.second) {
        return groupResult.first;
    }
    return {};
}


std::vector<cor::Group> GroupData::groupsFromIDs(const std::vector<cor::UUID>& IDs) {
    std::vector<cor::Group> retVector;
    retVector.reserve(IDs.size());
    for (const auto& subGroupID : IDs) {
        auto groupResult = mGroupDict.item(subGroupID.toStdString());
        // check if group is already in this list
        if (groupResult.second) {
            retVector.emplace_back(groupResult.first);
        }
    }
    return retVector;
}


cor::UUID GroupData::groupNameToID(const QString name) {
    for (const auto& group : mGroupDict.items()) {
        if (name == group.name()) {
            return group.uniqueID();
        }
    }

    return cor::UUID::invalidID();
}

QString GroupData::removeGroup(cor::UUID uniqueID) {
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

bool GroupData::removeLightFromGroups(const cor::LightID& uniqueID) {
    bool anyUpdates = false;

    // parse the groups, remove from dict if needed
    for (const auto& group : mGroupDict.items()) {
        for (const auto& groupLight : group.lights()) {
            if (group.lights().size() == 1 && (groupLight == uniqueID)) {
                // edge case where the mood only exists for this one light, remove it
                // entirely
                mGroupDict.removeKey(group.uniqueID().toStdString());
                anyUpdates = true;
            } else if (groupLight == uniqueID) {
                // standard case, make a new mood without the light
                auto newGroup = group.removeLight(uniqueID);
                mGroupDict.update(group.uniqueID().toStdString(), newGroup);
                anyUpdates = true;
            }
        }
    }
    return anyUpdates;
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
        const auto& key = externalGroup.uniqueID().toStdString();
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
                    mGroupDict.update(internalGroup.uniqueID().toStdString(), groupCopy);
                    foundGroup = true;
                }
            }
            if (!foundGroup) {
                qDebug() << "INFO: added external group" << externalGroup.toJson();
                auto result = mGroupDict.insert(key, externalGroup);
                if (!result) {
                    qDebug() << " insert failed" << externalGroup.name();
                }
            }
        }
    }
}
