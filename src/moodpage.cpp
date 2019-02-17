/*!
 * \copyright
 * Copyright (C) 2015 - 2019.
 * Released under the GNU General Public License.
 */

#include "moodpage.h"

#include "listmoodpreviewwidget.h"
#include "listmoodgroupwidget.h"
#include "cor/utils.h"

#include <QDebug>
#include <QInputDialog>
#include <QScroller>

MoodPage::MoodPage(QWidget *parent, GroupData *groups) : QWidget(parent), mGroups(groups) {

    mMoodsListWidget = new cor::ListWidget(this, cor::EListType::linear);
    mMoodsListWidget->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    QScroller::grabGesture(mMoodsListWidget->viewport(), QScroller::LeftMouseButtonGesture);

    mLayout = new QVBoxLayout(this);
    mLayout->addWidget(mMoodsListWidget);

    setLayout(mLayout);

    connect(mGroups, SIGNAL(newMoodAdded(QString)), this, SLOT(newMoodAdded(QString)));

    mRenderInterval = 1000;
}



void MoodPage::newMoodAdded(QString mood) {
    Q_UNUSED(mood);
    qDebug() << "mood added" << mood;
  //  updateConnectionList();
}

void MoodPage::makeMoodsCollections(const cor::Dictionary<cor::Mood>& moods,
                                    const std::list<cor::Group>& roomList) {

    // pair every mood to an existing collection
    std::unordered_map<std::string, std::list<cor::Mood>> roomsWithMoods;
    for (const auto& mood : moods.itemVector()) { // for every mood
        // look at every device, mark its room
        std::list<QString> roomNames;
        for (const auto& moodDevice : mood.lights) {
            bool foundRoom = false;
            for (const auto& room : roomList) {
                for (const auto& lightID : room.lights) {
                    try {
                        if (lightID == moodDevice.uniqueID()) {
                            foundRoom = true;
                            auto roomIt = std::find(roomNames.begin(), roomNames.end(), room.name());
                            if (roomIt == roomNames.end()) {
                                roomNames.push_back(room.name());
                            }
                        }

                    } catch (...) {
                    }
                }
            }
            if (!foundRoom) {
                auto roomIt = std::find(roomNames.begin(), roomNames.end(), "Miscellaneous");
                if (roomIt == roomNames.end()) {
                    roomNames.push_back("Miscellaneous");
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
                std::list<cor::Mood> newLightGroup = {mood};
                roomsWithMoods.insert(std::make_pair(roomNames.front().toStdString(), newLightGroup));
            }
        } else {
            // if theres more than one room, put in miscellaneous
            auto groupList = roomsWithMoods.find("Miscellaneous");
            if (groupList != roomsWithMoods.end()) {
                // if found, add to list
                groupList->second.push_back(mood);
            } else {
                // if not found, create entry in table
                std::list<cor::Mood> newLightGroup = { mood };
                roomsWithMoods.insert(std::make_pair("Miscellaneous", newLightGroup));
            }
        }
    }

    for (const auto& room : roomsWithMoods) {
        QString roomName = QString::fromStdString(room.first);
        if (roomName.compare("Miscellaneous") != 0 ) {
            // qDebug() << " room name " << roomName;
             bool roomFound = false;
             for (auto item : mMoodsListWidget->widgets()) {
                 if (item->key().compare(roomName) == 0) {
                     roomFound = true;
                     ListMoodGroupWidget *moodWidget = qobject_cast<ListMoodGroupWidget*>(item);
                     Q_ASSERT(moodWidget);
                     //TODO: update mood widget
                 }
             }

             if (!roomFound) {
                 initMoodsCollectionWidget(roomName, room.second, roomName);
             }
        }
    }

    //TODO: remove the miscellaneous edge case by actually sorting
    for (const auto& room : roomsWithMoods) {
        QString roomName = QString::fromStdString(room.first);
        if (roomName.compare("Miscellaneous") == 0 ) {
            // qDebug() << " room name " << roomName;
             bool roomFound = false;
             for (auto item : mMoodsListWidget->widgets()) {
                 if (item->key().compare(roomName) == 0) {
                     roomFound = true;
                     ListMoodGroupWidget *moodWidget = qobject_cast<ListMoodGroupWidget*>(item);
                     Q_ASSERT(moodWidget);
                     //TODO: update mood widget
                 }
             }

             if (!roomFound) {
                 initMoodsCollectionWidget(roomName, room.second, roomName);
             }
        }
    }
}



ListMoodGroupWidget* MoodPage::initMoodsCollectionWidget(const QString& name,
                                                         const std::list<cor::Mood>& moods,
                                                         const QString& key,
                                                         bool hideEdit) {

    ListMoodGroupWidget *widget = new ListMoodGroupWidget(name,
                                                          moods,
                                                          key,
                                                          hideEdit,
                                                          mMoodsListWidget->mainWidget());
    connect(widget, SIGNAL(moodClicked(QString, std::uint64_t)), this, SLOT(moodClicked(QString, std::uint64_t)));
    connect(widget, SIGNAL(editClicked(QString, std::uint64_t)), this, SLOT(editMoodClicked(QString, std::uint64_t)));
    connect(widget, SIGNAL(moodSelected(QString, std::uint64_t)), this, SLOT(selectedMood(QString, std::uint64_t)));

    mMoodsListWidget->insertWidget(widget);
    mMoodsListWidget->resizeWidgets();

    connect(widget, SIGNAL(buttonsShown(QString, bool)), this, SLOT(shouldShowButtons(QString, bool)));
    return widget;
}

void MoodPage::editGroupClicked(std::uint64_t key) {
    emit clickedEditButton(key, true);
}


void MoodPage::selectedMood(QString, std::uint64_t moodKey) {
    mSelectedMood = moodKey;
    emit clickedSelectedMood(moodKey);
}


void MoodPage::editMoodClicked(QString collectionKey, std::uint64_t moodKey) {
    Q_UNUSED(collectionKey);
    emit clickedEditButton(moodKey, true);
}

void MoodPage::moodClicked(QString collectionKey, std::uint64_t moodKey) {
    Q_UNUSED(collectionKey);
//    qDebug() << "collection key:" << collectionKey
//             << "mood key:" << moodKey;

    mCurrentMood = moodKey;

    emit moodUpdate(mCurrentMood);
    emit updateMainIcons();
    emit changedDeviceCount();
}

void MoodPage::resizeEvent(QResizeEvent *) {
    mMoodsListWidget->resizeWidgets();
}

void MoodPage::show(std::uint64_t currentMood,
                    const cor::Dictionary<cor::Mood>& moods,
                    const std::list<cor::Group>& roomList) {
    mMoodsListWidget->setVisible(true);
    mCurrentMood = currentMood;
    makeMoodsCollections(moods, roomList);
    mMoodsListWidget->show();
}

void MoodPage::hide() {
    mMoodsListWidget->setVisible(false);
}


void MoodPage::shouldShowButtons(QString key, bool) {
    for (const auto& widget : mMoodsListWidget->widgets()) {
        if (widget->key() != key) {
            ListMoodGroupWidget *groupWidget = qobject_cast<ListMoodGroupWidget*>(widget);
            Q_ASSERT(groupWidget);
            groupWidget->closeLights();
        }
    }
    mMoodsListWidget->resizeWidgets();
}
