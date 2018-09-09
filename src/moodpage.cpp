/*!
 * \copyright
 * Copyright (C) 2015 - 2018.
 * Released under the GNU General Public License.
 */

#include "moodpage.h"

#include "listmoodwidget.h"
#include "listmoodgroupwidget.h"
#include "cor/utils.h"

#include <QDebug>
#include <QInputDialog>
#include <QScroller>

MoodPage::MoodPage(QWidget *parent, GroupsParser *groups) : QWidget(parent), mGroups(groups) {

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

void MoodPage::makeMoodsCollections(const std::list<cor::LightGroup>& moods,
                                    const std::list<cor::LightGroup>& roomList,
                                    const std::vector<std::pair<QString, QString>> deviceNames) {

    // pair every mood to an existing collection
    std::unordered_map<std::string, std::list<cor::LightGroup> > roomsWithMoods;
    for (auto&& mood : moods) { // for every mood
        // look at every device, mark its room
        std::list<QString> roomNames;
        for (auto&& moodDevice : mood.devices) {
            bool foundRoom = false;
            for (auto&& room : roomList) {
                for (auto&& roomDevice : room.devices) {
                    if (compareLight(roomDevice, moodDevice)) {
                        foundRoom = true;
                        auto roomIt = std::find(roomNames.begin(), roomNames.end(), room.name);
                        if (roomIt == roomNames.end()) {
                            roomNames.push_back(room.name);
                        }
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
                std::list<cor::LightGroup> newLightGroup = {mood};
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
                std::list<cor::LightGroup> newLightGroup = { mood };
                roomsWithMoods.insert(std::make_pair("Miscellaneous", newLightGroup));
            }
        }
    }

    for (auto&& room : roomsWithMoods) {
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
                 initMoodsCollectionWidget(roomName, room.second, deviceNames, roomName);
             }
        }
    }

    //TODO: remove the miscellaneous edge case by actually sorting
    for (auto&& room : roomsWithMoods) {
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
                 initMoodsCollectionWidget(roomName, room.second, deviceNames, roomName);
             }
        }
    }

    // now check for missing ones. Reiterate through widgets and remove any that can't be found in collection list
//    for (uint32_t i = 0; i < mMoodsListWidget->count(); ++i) {
//        ListCollectionWidget *item = mMoodsListWidget->widget(i);
//        if (!(item->key().compare("zzzAVAILABLE_MOODS") == 0
//                || item->key().compare("zzzUNAVAILABLE_MOODS") == 0)) {
//            bool collectionFound = false;
//            for (auto&& collection : collectionList) {
//                if (item->key().compare(collection.name) == 0) {
//                    collectionFound = true;
//                }
//            }
//            if (!collectionFound) {
//                mMoodsListWidget->removeWidget(item->key());
//            }
//        }
//    }
}



ListMoodGroupWidget* MoodPage::initMoodsCollectionWidget(const QString& name,
                                                         std::list<cor::LightGroup> moods,
                                                         const std::vector<std::pair<QString, QString>>& deviceNames,
                                                         const QString& key,
                                                         bool hideEdit) {
    for (auto&& mood : moods) {
        for (auto&& device : mood.devices) {
            for (auto&& deviceName : deviceNames) {
                if (device.uniqueID() == deviceName.first) {
                    device.name = deviceName.second;
                }
            }
//            if (device.routine > cor::ERoutineSingleColorEnd) {
//                device.palette = mPalettes.palette(device.paletteEnum);
//            }
        }
    }

    ListMoodGroupWidget *widget = new ListMoodGroupWidget(name,
                                                          moods,
                                                          key,
                                                          hideEdit,
                                                          mMoodsListWidget->mainWidget());
    connect(widget, SIGNAL(moodClicked(QString,QString)), this, SLOT(moodClicked(QString, QString)));
    connect(widget, SIGNAL(editClicked(QString, QString)), this, SLOT(editMoodClicked(QString, QString)));

    mMoodsListWidget->insertWidget(widget);
    mMoodsListWidget->resizeWidgets();

    connect(widget, SIGNAL(buttonsShown(QString, bool)), this, SLOT(shouldShowButtons(QString, bool)));
    return widget;
}

void MoodPage::editGroupClicked(QString key) {
    emit clickedEditButton(key, true);
}

void MoodPage::editMoodClicked(QString collectionKey, QString moodKey) {
    Q_UNUSED(collectionKey);
    emit clickedEditButton(moodKey, true);
}

void MoodPage::moodClicked(QString collectionKey, QString moodKey) {
    Q_UNUSED(collectionKey);
    qDebug() << "collection key:" << collectionKey
             << "mood key:" << moodKey;

    mCurrentMood = moodKey;

    emit moodUpdate(mCurrentMood);
    emit updateMainIcons();
    emit changedDeviceCount();
}

void MoodPage::resizeEvent(QResizeEvent *) {
    mMoodsListWidget->resizeWidgets();
}

void MoodPage::show(const QString& currentMood,
                    const std::list<cor::LightGroup>& moods,
                    const std::list<cor::LightGroup>& roomList,
                    const std::vector<std::pair<QString, QString>> deviceNames) {
    mMoodsListWidget->setVisible(true);
    mCurrentMood = currentMood;
    makeMoodsCollections(moods, roomList, deviceNames);
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
