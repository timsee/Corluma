/*!
 * \copyright
 * Copyright (C) 2015 - 2017.
 * Released under the GNU General Public License.
 */

#include "grouppage.h"
#include "icondata.h"
#include "corlumautils.h"

#include <QDebug>
#include <QSignalMapper>
#include <QScroller>

GroupPage::GroupPage(QWidget *parent) :
    QWidget(parent) {

    this->grabGesture(Qt::SwipeGesture);
    this->grabGesture(Qt::SwipeGesture);
    mSettings = new QSettings();

    mLayout = new QVBoxLayout(this);

    mScrollWidgetArduino = new QWidget(this);
    mScrollAreaArduino = new QScrollArea(this);
    mScrollAreaArduino->setWidget(mScrollWidgetArduino);
    mScrollAreaArduino->setStyleSheet("background-color:transparent;");
    QScroller::grabGesture(mScrollAreaArduino->viewport(), QScroller::LeftMouseButtonGesture);
    mScrollAreaArduino->setVisible(false);

    mScrollWidgetHue = new QWidget(this);
    mScrollAreaHue = new QScrollArea(this);
    mScrollAreaHue->setWidget(mScrollWidgetHue);
    mScrollAreaHue->setStyleSheet("background-color:transparent;");
    QScroller::grabGesture(mScrollAreaHue->viewport(), QScroller::LeftMouseButtonGesture);
    mScrollAreaHue->setVisible(false);

    mMoodsListWidget = new CorlumaListWidget(this);
    mMoodsListWidget->setContentsMargins(0,0,0,0);
    mMoodsListWidget->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    QScroller::grabGesture(mMoodsListWidget->viewport(), QScroller::LeftMouseButtonGesture);

    mMode = EGroupMode::eMoods;
}

GroupPage::~GroupPage() {

}


void GroupPage::setupButtons() {
    std::vector<std::string> labels = {"Water",
                                       "Frozen",
                                       "Snow",
                                       "Cool",
                                       "Warm",
                                       "Fire",
                                       "Evil",
                                       "Corrosive",
                                       "Poison",
                                       "Rose",
                                       "Pink Green",
                                       "RWB",
                                       "RGB",
                                       "CMY",
                                       "Six",
                                       "Seven",
                                       "All"};

    //---------------
    // Arduino
    //---------------

    mPresetArduinoWidgets = std::vector<PresetGroupWidget *>(labels.size(), nullptr);
    mPresetArduinoLayout = new QVBoxLayout;
    mPresetArduinoLayout->setSpacing(0);
    mPresetArduinoLayout->setContentsMargins(9, 0, 0, 0);

    int groupIndex = 0;
    for (int preset = (int)EColorGroup::eWater; preset < (int)EColorGroup::eColorGroup_MAX; preset++) {
        mPresetArduinoWidgets[groupIndex] = new PresetGroupWidget(QString(labels[groupIndex].c_str()),
                                                                  (EColorGroup)preset,
                                                                  mData->colorGroup((EColorGroup)preset),
                                                                  EPresetWidgetMode::eArduino,
                                                                  this);
        mPresetArduinoLayout->addWidget(mPresetArduinoWidgets[groupIndex], 1);
        connect(mPresetArduinoWidgets[groupIndex], SIGNAL(presetButtonClicked(int, int)), this, SLOT(multiButtonClicked(int,int)));
        groupIndex++;
    }

    mScrollAreaArduino->setWidgetResizable(true);
    mScrollAreaArduino->widget()->setLayout(mPresetArduinoLayout);
    mScrollAreaArduino->setStyleSheet("background-color:rgb(33, 32, 32);");

    //---------------
    // Hue
    //---------------

    mPresetHueWidgets = std::vector<PresetGroupWidget *>(labels.size(), nullptr);
    mPresetHueLayout = new QGridLayout;
    mPresetHueLayout->setSpacing(0);
    mPresetHueLayout->setContentsMargins(9, 0, 0, 0);

    groupIndex = 0;
    int rowIndex = 0;
    int columnIndex = 0;
    for (int preset = (int)EColorGroup::eWater; preset < (int)EColorGroup::eColorGroup_MAX; preset++) {
        mPresetHueWidgets[groupIndex] = new PresetGroupWidget(QString(labels[groupIndex].c_str()),
                                                                  (EColorGroup)preset,
                                                                  mData->colorGroup((EColorGroup)preset),
                                                                  EPresetWidgetMode::eHue,
                                                                  this);
        mPresetHueLayout->addWidget(mPresetHueWidgets[groupIndex], rowIndex, columnIndex);
        connect(mPresetHueWidgets[groupIndex], SIGNAL(presetButtonClicked(int, int)), this, SLOT(multiButtonClicked(int,int)));
        if (columnIndex == 0) {
            columnIndex = 1;
        } else {
            columnIndex = 0;
            rowIndex++;
        }
        groupIndex++;
    }

    mScrollAreaHue->setWidgetResizable(true);
    mScrollAreaHue->widget()->setLayout(mPresetHueLayout);
    mScrollAreaHue->setStyleSheet("background-color:rgb(33, 32, 32);");

    //---------------
    // Final Setup
    //---------------

    mLayout->addWidget(mMoodsListWidget, 8);
}

void GroupPage::highlightRoutineButton(ELightingRoutine routine, EColorGroup colorGroup) {
    int index = 0;
    for (int iteratorGroup = (int)EColorGroup::eWater; iteratorGroup < (int)EColorGroup::eColorGroup_MAX; iteratorGroup++) {
        for (int iteratorRoutine = (int)utils::ELightingRoutineSingleColorEnd + 1; iteratorRoutine < (int)ELightingRoutine::eLightingRoutine_MAX; iteratorRoutine++) {
            if (mMode == EGroupMode::eArduinoPresets) {
                if (iteratorRoutine == (int)routine && iteratorGroup == (int)colorGroup) {
                    mPresetArduinoWidgets[index]->setChecked((ELightingRoutine)iteratorRoutine, true);
                } else {
                    mPresetArduinoWidgets[index]->setChecked((ELightingRoutine)iteratorRoutine, false);
                }
            } else if (mMode == EGroupMode::eHuePresets) {
                if (iteratorRoutine == (int)routine && iteratorGroup == (int)colorGroup) {
                    mPresetHueWidgets[index]->setChecked((ELightingRoutine)iteratorRoutine, true);
                } else {
                    mPresetHueWidgets[index]->setChecked((ELightingRoutine)iteratorRoutine, false);
                }
            }
        }
        index++;
    }
}


// ----------------------------
// Slots
// ----------------------------

void GroupPage::multiButtonClicked(int routine, int colorGroup) {
    mData->updateColorGroup((EColorGroup)colorGroup);
    mData->updateRoutine((ELightingRoutine)routine);
    highlightRoutineButton((ELightingRoutine)routine, (EColorGroup)colorGroup);
    emit presetColorGroupChanged(colorGroup);
}


// ----------------------------
// Protected
// ----------------------------

void GroupPage::showEvent(QShowEvent *) {
    highlightRoutineButton(mData->currentRoutine(), mData->currentColorGroup());
    updateConnectionList();

    resize();
}

void GroupPage::hideEvent(QHideEvent *) {

}

void GroupPage::renderUI() {

}

void GroupPage::resizeEvent(QResizeEvent *) {
   // mScrollWidget->setFixedWidth(mScrollArea->viewport()->width());
    resize();

}

void GroupPage::resize() {
    mMoodsListWidget->setMaximumSize(this->size());
    updateConnectionList();
    for (uint32_t i = 0; i < mMoodsListWidget->count(); ++i) {
        ListCollectionWidget *item = mMoodsListWidget->widget(i);
        item->setListHeight(this->height());
        item->setShowButtons(mSettings->value(keyForCollection(item->key())).toBool());
    }

    mScrollAreaArduino->setFixedSize(this->size());
    for (uint32_t i = 0; i < mPresetArduinoWidgets.size(); ++i) {
        mPresetArduinoWidgets[i]->resize();
    }

    mScrollAreaHue->setFixedSize(this->size());
    for (uint32_t i = 0; i < mPresetHueWidgets.size(); ++i) {
        mPresetHueWidgets[i]->resize();
    }
}

void GroupPage::showCustomMoods() {
    if (mMode != EGroupMode::eMoods) {
        // hide existing
        mLayout->removeItem(mLayout->itemAt(0));
        mScrollAreaArduino->setVisible(false);

        // add new
        mLayout->addWidget(mMoodsListWidget, 20);
        mMoodsListWidget->setVisible(true);

        mMode = EGroupMode::eMoods;
    }
}

void GroupPage::showPresetArduinoGroups() {
    if (mMode != EGroupMode::eArduinoPresets) {
        // hide existing
        mLayout->removeItem(mLayout->itemAt(0));
        mMoodsListWidget->setVisible(false);

        mScrollAreaHue->setVisible(false);
        mLayout->addWidget(mScrollAreaArduino, 20);
        mScrollAreaArduino->setVisible(true);

        mMode = EGroupMode::eArduinoPresets;
    }
}


void GroupPage::showPresetHueGroups() {
    if (mMode != EGroupMode::eHuePresets) {
        // hide existing
        mLayout->removeItem(mLayout->itemAt(0));
        mMoodsListWidget->setVisible(false);

        mScrollAreaArduino->setVisible(false);
        mLayout->addWidget(mScrollAreaHue, 20);
        mScrollAreaHue->setVisible(true);

        mMode = EGroupMode::eHuePresets;
    }
}

//--------------------
// Mood Widget
//--------------------

void GroupPage::highlightList() {
    for (uint32_t row = 0; row < mMoodsListWidget->count(); row++) {
        ListCollectionWidget *collectionWidget = mMoodsListWidget->widget(row);
        if (collectionWidget->isMoodWidget()) {
            ListMoodGroupWidget *widget = qobject_cast<ListMoodGroupWidget*>(collectionWidget);
            Q_ASSERT(widget);
            std::list<QString> connectedMoods = moodsConnected(widget->moods());
            widget->setCheckedMoods(connectedMoods);
        }
    }
}

std::list<QString> GroupPage::moodsConnected(std::list<std::pair<QString, std::list<SLightDevice> > > moods) {
    std::list<SLightDevice> deviceList = mData->currentDevices();
    std::list<QString> connectedMood;

    for (auto&& mood : moods) {
        uint32_t devicesFound = 0;
        bool moodIsConnected = true;
        // check each device in specific mood
        for (auto&& moodDevice : mood.second) {
            bool deviceMatches = false;
            // compare against all data devices
            for (auto&& device : deviceList) {
                //TODO: complete this check
                if (compareLightDevice(device, moodDevice)
                    && device.lightingRoutine == moodDevice.lightingRoutine
                    && (utils::colorDifference(device.color, moodDevice.color) <= 0.05f)
                    && (utils::brightnessDifference(device.brightness, moodDevice.brightness) <= 0.05f)
                    && device.colorGroup == moodDevice.colorGroup
                    && device.isOn == moodDevice.isOn) {
                    deviceMatches = true;
                    devicesFound++;
                 }
            }
            if (!deviceMatches) moodIsConnected = false;
        }
        if (devicesFound != mood.second.size()) moodIsConnected = false;
        if (moodIsConnected) connectedMood.push_back(mood.first);
    }
    return connectedMood;
}

void GroupPage::updateConnectionList() {
    std::list<SLightDevice> allDevices = mComm->allDevices();

    std::list<std::pair<QString, std::list<SLightDevice> > > moodList = mGroups->moodList();
    gatherAvailandAndNotReachableMoods(allDevices, moodList);
    makeMoodsCollections(moodList);
}

void GroupPage::newMoodAdded(QString mood) {
    Q_UNUSED(mood);
   // qDebug() << "mood added" << mood;
    updateConnectionList();
}


void GroupPage::groupDeleted(QString group) {
    qDebug() << "group deleted" << group;
    for (uint32_t i = 0; i < mMoodsListWidget->count(); ++i) {
        ListCollectionWidget *widget = mMoodsListWidget->widget(i);
        Q_ASSERT(widget);
        ListMoodGroupWidget *moodWidget = qobject_cast<ListMoodGroupWidget*>(widget);
        for (auto&& widgetMood : moodWidget->moods()) {
            if (widgetMood.first.compare(group) == 0) {
                moodWidget->removeMood(widgetMood.first);
            }
        }
        if (moodWidget->moods().size() == 0) {
            qDebug() << " group deleted" << widget->key();
            mMoodsListWidget->removeWidget(widget->key());
        }
    }

}

void GroupPage::connectGroupsParser(GroupsParser *parser) {
    mGroups = parser;
    connect(mGroups, SIGNAL(groupDeleted(QString)), this, SLOT(groupDeleted(QString)));
    connect(mGroups, SIGNAL(newMoodAdded(QString)), this, SLOT(newMoodAdded(QString)));
}


void GroupPage::makeMoodsCollections(const std::list<std::pair<QString, std::list<SLightDevice> > >& moods) {
    std::list<std::pair<QString, std::list<SLightDevice> > > collectionList = mGroups->collectionList();
    for (auto&& collection : collectionList) {
        bool collectionFound = false;
        for (uint32_t i = 0; i < mMoodsListWidget->count(); ++i) {
            ListCollectionWidget *item = mMoodsListWidget->widget(i);
            if (item->key().compare(collection.first) == 0) {
                collectionFound = true;
                ListMoodGroupWidget *moodWidget = qobject_cast<ListMoodGroupWidget*>(item);
                Q_ASSERT(moodWidget);
               // moodWidget->updateMoods(moods, mData->colors());
            }
        }

        if (!collectionFound) {
            std::list<std::pair<QString, std::list<SLightDevice> > > allCollectionMoods;
            for (auto&& mood : moods) {
                int devicesToTest = mood.second.size();
                int devicesFound = 0;
                for (auto&& moodDevice : mood.second) {
                    for (auto&& deviceData : collection.second) {
                        if (compareLightDevice(moodDevice, deviceData)) {
                            devicesFound++;
                        }
                    }
                }
                if (devicesFound == devicesToTest) {
                    allCollectionMoods.push_back(mood);
                }
            }
            if (allCollectionMoods.size() > 0) {
                initMoodsCollectionWidget(collection.first, allCollectionMoods, collection.first);
            }
        }
    }

    // now check for missing ones. Reiterate through widgets and remove any that can't be found in collection list
    for (uint32_t i = 0; i < mMoodsListWidget->count(); ++i) {
        ListCollectionWidget *item = mMoodsListWidget->widget(i);
        if (!(item->key().compare("zzzAVAILABLE_MOODS") == 0
                || item->key().compare("zzzUNAVAILABLE_MOODS") == 0)) {
            bool collectionFound = false;
            for (auto&& collection : collectionList) {
                if (item->key().compare(collection.first) == 0) {
                    collectionFound = true;
                }
            }
            if (!collectionFound) {
                mMoodsListWidget->removeWidget(item->key());
            }
        }
    }
}



ListMoodGroupWidget* GroupPage::initMoodsCollectionWidget(const QString& name,
                                                                std::list<std::pair<QString, std::list<SLightDevice> > > moods,
                                                                const QString& key,
                                                                bool hideEdit) {
    ListMoodGroupWidget *widget = new ListMoodGroupWidget(name,
                                                            moods,
                                                            mData->colors(),
                                                            key,
                                                            hideEdit,
                                                            mMoodsListWidget);
    connect(widget, SIGNAL(moodClicked(QString,QString)), this, SLOT(moodClicked(QString, QString)));
    connect(widget, SIGNAL(editClicked(QString)), this, SLOT(editGroupClicked(QString)));
    connect(widget, SIGNAL(editClicked(QString, QString)), this, SLOT(editMoodClicked(QString, QString)));
    connect(widget, SIGNAL(buttonsShown(QString, bool)), this, SLOT(shouldShowButtons(QString, bool)));

    widget->setShowButtons(mSettings->value(keyForCollection(widget->key())).toBool());

    ListCollectionWidget *collectionWidget = qobject_cast<ListCollectionWidget*>(widget);
    mMoodsListWidget->addWidget(collectionWidget);
    return widget;
}

void GroupPage::gatherAvailandAndNotReachableMoods(const std::list<SLightDevice>& allDevices,
                                                   const std::list<std::pair<QString, std::list<SLightDevice> > >& moodList) {

    std::list<std::pair<QString, std::list<SLightDevice> > > availableMoods;
    std::list<std::pair<QString, std::list<SLightDevice> > > unavailableMoods;
    QString kAvailableMoodsKey = "zzzAVAILABLE_MOODS";
    QString kUnavailableMoodsKey = "zzzUNAVAILABLE_MOODS";

    for (auto&& mood : moodList) {
        int devicesToTest = mood.second.size();
        int devicesReachable = 0;
        for (auto&& moodDevice : mood.second) {
            for (auto&& deviceData : allDevices) {
                if (compareLightDevice(moodDevice, deviceData)
                        && deviceData.isReachable) {
                    devicesReachable++;
                }
            }
        }
        if (devicesToTest == devicesReachable) {
            availableMoods.push_back(mood);
        } else {
            unavailableMoods.push_back(mood);
        }
    }

    // ------------------------------------
    // add available and not reachable collections
    // ------------------------------------

    bool foundAvailable = false;
    bool foundUnavailable = false;
    for (uint32_t i = 0; i < mMoodsListWidget->count(); ++i) {
        ListCollectionWidget *item = mMoodsListWidget->widget(i);
        if (item->key().compare(kAvailableMoodsKey) == 0) {
            foundAvailable = true;
            // update
            ListMoodGroupWidget *widget = qobject_cast<ListMoodGroupWidget*>(item);
            Q_ASSERT(widget);
            widget->updateMoods(availableMoods, mData->colors());
        }
        if (item->key().compare(kUnavailableMoodsKey) == 0) {
            foundUnavailable = true;
            // update
            ListMoodGroupWidget *widget = qobject_cast<ListMoodGroupWidget*>(item);
            Q_ASSERT(widget);
            widget->updateMoods(unavailableMoods, mData->colors());
        }
    }


    if (!foundAvailable) {
        initMoodsCollectionWidget("Available", availableMoods, kAvailableMoodsKey, true);
    }

    if (!foundUnavailable) {
        initMoodsCollectionWidget("Not Reachable", unavailableMoods, kUnavailableMoodsKey, true);
    }
}


QString GroupPage::keyForCollection(const QString& key) {
    return (QString("COLLECTION_" + key));
}


void GroupPage::shouldShowButtons(QString key, bool isShowing) {
    mSettings->setValue(keyForCollection(key), QString::number((int)isShowing));
    mSettings->sync();
    mMoodsListWidget->resizeWidgets();
}

void GroupPage::editMoodClicked(QString collectionKey, QString moodKey) {
    Q_UNUSED(collectionKey);
    for (auto&& group :  mGroups->moodList()) {
        if (group.first.compare(moodKey) == 0) {
            mData->clearDevices();
            mData->addDeviceList(group.second);
        }
    }
    emit clickedEditButton(moodKey, true);
}

void GroupPage::editGroupClicked(QString key) {
//    mGreyOut->setVisible(true);
//    mEditPage->setVisible(true);
    emit clickedEditButton(key, true);
}


void GroupPage::moodClicked(QString collectionKey, QString moodKey) {
    Q_UNUSED(collectionKey);
    qDebug() << "collection key:" << collectionKey
             << "mood key:" << moodKey;

    for (auto&& group :  mGroups->moodList()) {
        if (group.first.compare(moodKey) == 0) {
            mData->clearDevices();
            mData->addDeviceList(group.second);
        }
    }

    // update UI
    emit updateMainIcons();
    emit deviceCountChanged();
    highlightList();
}
