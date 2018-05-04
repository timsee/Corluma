/*!
 * \copyright
 * Copyright (C) 2015 - 2018.
 * Released under the GNU General Public License.
 */

#include "moodspage.h"

#include "listmoodwidget.h"
#include "listmoodgroupwidget.h"
#include "cor/utils.h"

#include <QDebug>
#include <QInputDialog>
#include <QMessageBox>
#include <QScroller>

MoodsPage::MoodsPage(QWidget *parent) : QWidget(parent) {
    mMoodsListWidget = new cor::ListWidget(this);
    mMoodsListWidget->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    QScroller::grabGesture(mMoodsListWidget->viewport(), QScroller::LeftMouseButtonGesture);

    mLayout = new QVBoxLayout(this);
    mLayout->addWidget(mMoodsListWidget);

    setLayout(mLayout);

    mRenderInterval = 1000;
}



void MoodsPage::newMoodAdded(QString mood) {
    Q_UNUSED(mood);
    qDebug() << "mood added" << mood;
  //  updateConnectionList();
}

void MoodsPage::makeMoodsCollections(const std::list<cor::LightGroup>& moods) {
    std::list<cor::LightGroup> roomList = mComm->roomList();

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
             for (uint32_t i = 0; i < mMoodsListWidget->count(); ++i) {
                 ListCollectionWidget *item = mMoodsListWidget->widget(i);
                 if (item->key().compare(roomName) == 0) {
                     roomFound = true;
                     ListMoodGroupWidget *moodWidget = qobject_cast<ListMoodGroupWidget*>(item);
                     Q_ASSERT(moodWidget);
                    // moodWidget->updateMoods(moods, mData->colors());
                 }
             }

             if (!roomFound) {
                 initMoodsCollectionWidget(roomName, room.second, roomName);
             }
        }
    }

    //TODO: remove the miscellaneous edge case by actually sorting
    for (auto&& room : roomsWithMoods) {
        QString roomName = QString::fromStdString(room.first);
        if (roomName.compare("Miscellaneous") == 0 ) {
            // qDebug() << " room name " << roomName;
             bool roomFound = false;
             for (uint32_t i = 0; i < mMoodsListWidget->count(); ++i) {
                 ListCollectionWidget *item = mMoodsListWidget->widget(i);
                 if (item->key().compare(roomName) == 0) {
                     roomFound = true;
                     ListMoodGroupWidget *moodWidget = qobject_cast<ListMoodGroupWidget*>(item);
                     Q_ASSERT(moodWidget);
                    // moodWidget->updateMoods(moods, mData->colors());
                 }
             }

             if (!roomFound) {
                 initMoodsCollectionWidget(roomName, room.second, roomName);
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



ListMoodGroupWidget* MoodsPage::initMoodsCollectionWidget(const QString& name,
                                                                std::list<cor::LightGroup> moods,
                                                                const QString& key,
                                                                bool hideEdit) {
    // TODO: add names into lights
    for (auto&& mood : moods) {
        for (auto&& device : mood.devices) {
            cor::Light deviceCopy = device;
            mComm->fillDevice(deviceCopy);
            device.name = deviceCopy.name;
        }
    }

    ListMoodGroupWidget *widget = new ListMoodGroupWidget(name,
                                                          moods,
                                                          mData->colors(),
                                                          key,
                                                          hideEdit,
                                                          mMoodsListWidget);
    connect(widget, SIGNAL(moodClicked(QString,QString)), this, SLOT(moodClicked(QString, QString)));
    connect(widget, SIGNAL(editClicked(QString)), this, SLOT(editGroupClicked(QString)));
    connect(widget, SIGNAL(editClicked(QString, QString)), this, SLOT(editMoodClicked(QString, QString)));

    ListCollectionWidget *collectionWidget = qobject_cast<ListCollectionWidget*>(widget);
    mMoodsListWidget->addWidget(collectionWidget);
    return widget;
}

void MoodsPage::editGroupClicked(QString key) {
    qDebug()  << " edit group " << key;
    emit clickedEditButton(key, true);
}

void MoodsPage::editMoodClicked(QString collectionKey, QString moodKey) {
    Q_UNUSED(collectionKey);
    emit clickedEditButton(moodKey, true);
}

void MoodsPage::moodClicked(QString collectionKey, QString moodKey) {
    Q_UNUSED(collectionKey);
    qDebug() << "collection key:" << collectionKey
             << "mood key:" << moodKey;

    for (auto&& group :  mComm->groups()->moodList()) {
        if (group.name.compare(moodKey) == 0) {
            mData->clearDevices();
            mData->addDeviceList(group.devices);
        }
    }

    // update UI
    emit updateMainIcons();
    emit changedDeviceCount();
}

void MoodsPage::resizeEvent(QResizeEvent *) {
    mMoodsListWidget->setMaximumSize(this->size());
    mMoodsListWidget->resizeWidgets();
}

void MoodsPage::show() {
    mMoodsListWidget->setMaximumSize(this->size());
    mMoodsListWidget->resizeWidgets();
    mMoodsListWidget->setVisible(true);
    std::list<cor::LightGroup> moodList = mComm->groups()->moodList();
    makeMoodsCollections(moodList);
}

void MoodsPage::hide() {
    mMoodsListWidget->setVisible(false);
}


void MoodsPage::setupUI() {
    connect(mComm->groups(), SIGNAL(newMoodAdded(QString)), this, SLOT(newMoodAdded(QString)));
}
