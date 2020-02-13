/*!
 * \copyright
 * Copyright (C) 2015 - 2020.
 * Released under the GNU General Public License.
 */

#include "moodpage.h"

#include <QDebug>
#include <QInputDialog>
#include <QScroller>

#include "listmoodgroupwidget.h"
#include "listmoodpreviewwidget.h"
#include "utils/qt.h"

MoodPage::MoodPage(QWidget* parent, GroupData* groups)
    : QWidget(parent),
      mGroups(groups),
      mCurrentMood{0} {
    mMoodsListWidget = new cor::ListWidget(this, cor::EListType::linear);
    mMoodsListWidget->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    QScroller::grabGesture(mMoodsListWidget->viewport(), QScroller::LeftMouseButtonGesture);

    connect(mGroups, SIGNAL(newMoodAdded(QString)), this, SLOT(newMoodAdded(QString)));

    resize();
}

void MoodPage::newMoodAdded(const QString& mood) {
    qDebug() << "mood added" << mood;
    clearWidgets();
    makeMoodsCollections(mGroups->moods(), mGroups->rooms().items());
}

void MoodPage::makeMoodsCollections(const cor::Dictionary<cor::Mood>& moods,
                                    const std::vector<cor::Room>& roomList) {
    // pair every mood to an existing collection
    std::unordered_map<std::string, std::vector<cor::Mood>> roomsWithMoods;
    for (const auto& mood : moods.items()) { // for every mood
        // look at every device, mark its room
        std::vector<QString> roomNames;
        for (const auto& moodDevice : mood.lights()) {
            bool foundRoom = false;
            for (const auto& room : roomList) {
                for (const auto& lightID : room.lights()) {
                    if (lightID == moodDevice.uniqueID()) {
                        foundRoom = true;
                        auto roomIt = std::find(roomNames.begin(), roomNames.end(), room.name());
                        if (roomIt == roomNames.end()) {
                            roomNames.push_back(room.name());
                        }
                    }
                }
            }
            if (!foundRoom) {
                auto roomIt = std::find(roomNames.begin(), roomNames.end(), "Miscellaneous");
                if (roomIt == roomNames.end()) {
                    roomNames.emplace_back("Miscellaneous");
                }
            }
        }

        if (roomNames.size() == 1) {
            auto groupList = roomsWithMoods.find(roomNames.front().toStdString());
            if (groupList != roomsWithMoods.end()) {
                // if found, add to list
                groupList->second.push_back(mood);
            } else {
                // if not found, create entry in table
                std::vector<cor::Mood> newLightGroup = {mood};
                roomsWithMoods.insert(
                    std::make_pair(roomNames.front().toStdString(), newLightGroup));
            }
        } else {
            // if theres more than one room, put in miscellaneous
            auto groupList = roomsWithMoods.find("Miscellaneous");
            if (groupList != roomsWithMoods.end()) {
                // if found, add to list
                groupList->second.push_back(mood);
            } else {
                // if not found, create entry in table
                std::vector<cor::Mood> newLightGroup = {mood};
                roomsWithMoods.insert(std::make_pair("Miscellaneous", newLightGroup));
            }
        }
    }

    for (const auto& room : roomsWithMoods) {
        QString roomName = QString::fromStdString(room.first);
        if (roomName != "Miscellaneous") {
            // qDebug() << " room name " << roomName;
            bool roomFound = false;
            for (auto item : mMoodsListWidget->widgets()) {
                if (item->key() == roomName) {
                    roomFound = true;
                    auto moodWidget = qobject_cast<ListMoodGroupWidget*>(item);
                    Q_ASSERT(moodWidget);
                    // TODO: update mood widget
                }
            }

            if (!roomFound) {
                initMoodsCollectionWidget(roomName, room.second, roomName);
            }
        }
    }

    // TODO: remove the miscellaneous edge case by actually sorting
    for (const auto& room : roomsWithMoods) {
        QString roomName = QString::fromStdString(room.first);
        if (roomName == "Miscellaneous") {
            // qDebug() << " room name " << roomName;
            bool roomFound = false;
            for (auto item : mMoodsListWidget->widgets()) {
                if (item->key() == roomName) {
                    roomFound = true;
                    auto moodWidget = qobject_cast<ListMoodGroupWidget*>(item);
                    Q_ASSERT(moodWidget);
                    // TODO: update mood widget
                }
            }

            if (!roomFound) {
                initMoodsCollectionWidget(roomName, room.second, roomName);
            }
        }
    }
}



ListMoodGroupWidget* MoodPage::initMoodsCollectionWidget(const QString& name,
                                                         const std::vector<cor::Mood>& moods,
                                                         const QString& key,
                                                         bool hideEdit) {
    auto widget =
        new ListMoodGroupWidget(name, moods, key, hideEdit, mMoodsListWidget->mainWidget());
    connect(widget,
            SIGNAL(editClicked(QString, std::uint64_t)),
            this,
            SLOT(editMoodClicked(QString, std::uint64_t)));
    connect(widget,
            SIGNAL(moodSelected(QString, std::uint64_t)),
            this,
            SLOT(selectedMood(QString, std::uint64_t)));

    mMoodsListWidget->insertWidget(widget);
    mMoodsListWidget->resizeWidgets();

    connect(widget,
            SIGNAL(buttonsShown(QString, bool)),
            this,
            SLOT(shouldShowButtons(QString, bool)));
    return widget;
}

void MoodPage::selectedMood(const QString&, std::uint64_t moodKey) {
    mCurrentMood = moodKey;
    emit clickedSelectedMood(moodKey);
}


void MoodPage::editMoodClicked(const QString&, std::uint64_t) {
    emit clickedEditButton(true);
}

void MoodPage::resize() {
    mMoodsListWidget->setGeometry(0, 0, width(), height());
    for (auto item : mMoodsListWidget->widgets()) {
        auto moodWidget = qobject_cast<ListMoodGroupWidget*>(item);
        moodWidget->resize();
    }
    mMoodsListWidget->resizeWidgets();
}

void MoodPage::resizeEvent(QResizeEvent*) {
    resize();
}

void MoodPage::show(std::uint64_t currentMood,
                    const cor::Dictionary<cor::Mood>& moods,
                    const std::vector<cor::Room>& roomList) {
    mMoodsListWidget->setVisible(true);
    mCurrentMood = currentMood;
    makeMoodsCollections(moods, roomList);
    mMoodsListWidget->show();
}

void MoodPage::hide() {
    mMoodsListWidget->setVisible(false);
}


void MoodPage::shouldShowButtons(const QString& key, bool) {
    for (const auto& widget : mMoodsListWidget->widgets()) {
        if (widget->key() != key) {
            auto groupWidget = qobject_cast<ListMoodGroupWidget*>(widget);
            Q_ASSERT(groupWidget);
            groupWidget->closeLights();
        }
    }
    mMoodsListWidget->resizeWidgets();
}


void MoodPage::clearWidgets() {
    mCurrentMood = 0;
    mMoodsListWidget->clearAll();
}
