/*!
 * \copyright
 * Copyright (C) 2015 - 2020.
 * Released under the GNU General Public License.
 */

#include "lefthandmenu.h"

#include <QDebug>
#include <QMouseEvent>
#include <QScrollBar>
#include <QScroller>
#include <QStyleOption>

#include "cor/objects/light.h"
#include "cor/presetpalettes.h"
#include "utils/qt.h"

const QString kMiscKey = "zzzzMiscellaneous";

LeftHandMenu::LeftHandMenu(bool alwaysOpen,
                           cor::LightList* devices,
                           CommLayer* comm,
                           cor::LightList* lights,
                           GroupData* groups,
                           QWidget* parent)
    : QWidget(parent),
      mAlwaysOpen{alwaysOpen},
      mWidget{new QWidget(this)},
      mSpacer{new QWidget(this)},
      mSelectedLights{devices},
      mMainPalette{new cor::LightVectorWidget(6, 2, true, this)},
      mComm{comm},
      mData{lights},
      mGroups{groups},
      mLastRenderTime{QTime::currentTime()},
      mLastScrollValue{0} {
    auto width = int(parent->size().width() * 0.66f);
    setGeometry(width * -1, 0, width, parent->height());
    mIsIn = false;
    if (mAlwaysOpen) {
        mIsIn = true;
    }

    mSpacer->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    mSpacer->setStyleSheet("border: none; background-color:rgb(33,32,32);");

    mWidget->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    mWidget->setStyleSheet("border: none; background-color:rgba(0,0,0,0);");

    mScrollArea = new QScrollArea(this);
    mScrollArea->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    QScroller::grabGesture(mScrollArea->viewport(), QScroller::LeftMouseButtonGesture);
    mScrollArea->setWidget(mWidget);
    mScrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    mScrollArea->horizontalScrollBar()->setEnabled(false);

    // --------------
    // Setup Main Palette
    // -------------
    mMainPalette->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    mMainPalette->setFixedHeight(int(height() * 0.1));
    mMainPalette->setStyleSheet("background-color:rgb(33,32,32);");

    //---------------
    // Setup Buttons
    //---------------

    mSingleColorButton = new LeftHandButton("Single Color",
                                            EPage::colorPage,
                                            ":/images/wheels/color_wheel_hsv.png",
                                            this,
                                            this);
    mSingleColorButton->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    connect(mSingleColorButton, SIGNAL(pressed(EPage)), this, SLOT(buttonPressed(EPage)));
    mSingleColorButton->shouldHightlght(true);

    mSettingsButton = new LeftHandButton("Settings",
                                         EPage::settingsPage,
                                         ":/images/settingsgear.png",
                                         this,
                                         this);
    mSettingsButton->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    connect(mSettingsButton, SIGNAL(pressed(EPage)), this, SLOT(buttonPressed(EPage)));

    PresetPalettes palettes;
    cor::LightState state;
    state.routine(ERoutine::multiBars);
    state.palette(palettes.palette(EPalette::water));
    state.isOn(true);
    mMultiColorButton = new LeftHandButton("Multi Color", EPage::palettePage, state, this, this);
    mMultiColorButton->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    connect(mMultiColorButton, SIGNAL(pressed(EPage)), this, SLOT(buttonPressed(EPage)));

    cor::LightState moodState;
    moodState.routine(ERoutine::multiFade);
    moodState.palette(palettes.palette(EPalette::fire));
    moodState.isOn(true);
    mMoodButton = new LeftHandButton("Moods", EPage::moodPage, moodState, this, this);
    mMoodButton->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    connect(mMoodButton, SIGNAL(pressed(EPage)), this, SLOT(buttonPressed(EPage)));

    mNewGroupButton = new AddNewGroupButton(mWidget);
    mNewGroupButton->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    connect(mNewGroupButton, SIGNAL(pressed()), this, SLOT(newGroupButtonPressed()));

    mRenderThread = new QTimer(this);
    connect(mRenderThread, SIGNAL(timeout()), this, SLOT(renderUI()));
    if (mAlwaysOpen) {
        mRenderThread->start(333);
    }
    connect(this, SIGNAL(changedDeviceCount()), this, SLOT(deviceCountChanged()));
}


void LeftHandMenu::resize() {
    auto parentSize = parentWidget()->size();
    // get if its landscape or portrait
    float preferredWidth;
    if (parentSize.width() > parentSize.height() || mAlwaysOpen) {
        preferredWidth = parentSize.width() * 2.0f / 7.0f;
    } else {
        preferredWidth = parentSize.width() * 0.66f;
    }

    auto width = int(preferredWidth);

    if (mAlwaysOpen) {
        setGeometry(0u, pos().y(), width, parentSize.height());
    } else if (mIsIn) {
        setGeometry(0u, pos().y(), width, parentSize.height());
    } else {
        setGeometry(-width, pos().y(), width, parentSize.height());
    }

    auto buttonHeight = int(height() * 0.07);
    auto yPos = int(height() * 0.02);

    mSpacer->setGeometry(0, 0, this->width(), height());

    mSingleColorButton->setGeometry(0, yPos, this->width(), buttonHeight);
    yPos += mSingleColorButton->height();

    mMultiColorButton->setGeometry(0, yPos, this->width(), buttonHeight);
    yPos += mMultiColorButton->height();

    mMoodButton->setGeometry(0, yPos, this->width(), buttonHeight);
    yPos += mMoodButton->height();

    mSettingsButton->setGeometry(0, yPos, this->width(), buttonHeight);
    yPos += mSettingsButton->height();

    mMainPalette->setGeometry(0,
                              yPos + int(height() * 0.02),
                              this->width(),
                              int(buttonHeight * 1.2));

    yPos += int(mMainPalette->height() + height() * 0.02);

    /// setting to 1.2 hides the scroll bar and avoids horizotnal scrolling
    mScrollArea->setGeometry(0, yPos, int(this->width() * 1.2), height() - yPos);
    mWidget->setFixedWidth(this->width());

    auto scrollWidgetHeight = resizeGroupWidgets();
    mNewGroupButton->setGeometry(0, scrollWidgetHeight, this->width(), buttonHeight);

    scrollWidgetHeight += mNewGroupButton->height();
    mWidget->setFixedHeight(scrollWidgetHeight);

    setFixedHeight(parentSize.height());
}

void LeftHandMenu::pushIn() {
    mIsIn = true;
    raise();
    auto transTime =
        int(((this->width() - showingWidth()) / double(this->width())) * TRANSITION_TIME_MSEC);
    cor::moveWidget(this, pos(), QPoint(0u, 0u), transTime);
    if (!mRenderThread->isActive()) {
        mRenderThread->start(333);
    }
}

void LeftHandMenu::pushOut() {
    if (!mAlwaysOpen) {
        QPoint endPoint = pos();
        endPoint.setX(size().width() * -1);
        auto transTime = int((showingWidth() / double(this->width())) * TRANSITION_TIME_MSEC);
        cor::moveWidget(this, pos(), endPoint, transTime);
        mRenderThread->stop();
        mIsIn = false;
    }
}

void LeftHandMenu::deviceCountChanged() {
    const auto& lights = mComm->commLightsFromVector(mSelectedLights->lights());

    mMainPalette->updateLights(lights);

    // loop for multi color lights
    std::uint32_t multiColorLightCount = 0u;
    for (const auto& light : lights) {
        if (light.commType() != ECommType::hue) {
            multiColorLightCount++;
            break;
        }
    }
    updateSingleColorButton();
}




std::vector<cor::Group> LeftHandMenu::gatherAllUIGroups() {
    std::vector<cor::Group> uiGroups;
    for (auto widget : mParentGroupWidgets) {
        // cast to ListDeviceGroupWidget
        auto groupWidget = qobject_cast<ParentGroupWidget*>(widget);
        uiGroups.push_back(groupWidget->group());
    }
    return uiGroups;
}

void LeftHandMenu::updateDataGroupInUI(const cor::Group& dataGroup,
                                       const std::vector<cor::Group>& uiGroups) {
    bool existsInUIGroups = false;
    for (const auto& uiGroup : uiGroups) {
        if (uiGroup.name() == dataGroup.name()) {
            existsInUIGroups = true;
            for (auto widget : mParentGroupWidgets) {
                // using name so that miscellaneous groups are deleted even though the key is
                // different
                auto groupWidget = qobject_cast<ParentGroupWidget*>(widget);
                if (groupWidget->group().name() == dataGroup.name()) {
                    auto subgroups = mGroups->subgroups().subgroupsForGroup(dataGroup.uniqueID());
                    groupWidget->updateState(dataGroup, subgroups);
                }
            }
        }
    }
    if (!existsInUIGroups) {
        // qDebug() << "this group does not exist" << dataGroup.name();
        if (dataGroup.name() == "Miscellaneous") {
            initParentGroupWidget(dataGroup, kMiscKey);
        } else {
            initParentGroupWidget(dataGroup, dataGroup.name());
        }
    }
}

void LeftHandMenu::updateLights() {
    mLastRenderTime = QTime::currentTime();
    mGroups->updateSubgroups();

    // get all rooms
    auto parentGroups = mGroups->parents();
    std::vector<cor::Group> groupData = mGroups->groupsFromIDs(parentGroups);
    if (!mGroups->orphanLights().empty()) {
        groupData.push_back(mGroups->orphanGroup());
    }

    // update ui groups
    const auto& uiGroups = gatherAllUIGroups();
    for (const auto& group : groupData) {
        updateDataGroupInUI(group, uiGroups);
    }

    // TODO: remove any groups that should no longer be shown

    // get the number of lights shown
    auto lightIDs = cor::lightVectorToIDs(mSelectedLights->lights());
    for (const auto& room : mParentGroupWidgets) {
        room->setCheckedLights(lightIDs);
    }

    auto filledDataLights = mComm->commLightsFromVector(mData->lights());
    if (filledDataLights != mLastDataLights) {
        // take the lights being used as the app's expectation, and get their current state by
        // polling the CommLayer for its understanding of these lights
        mLastDataLights = filledDataLights;
        mMainPalette->updateLights(filledDataLights);
    }
}


void LeftHandMenu::buttonPressed(EPage page) {
    if (alwaysOpen()) {
        mSingleColorButton->shouldHightlght(false);
        mMultiColorButton->shouldHightlght(false);
        mMoodButton->shouldHightlght(false);
        mSettingsButton->shouldHightlght(false);
        if (page == EPage::colorPage) {
            mSingleColorButton->shouldHightlght(true);
        } else if (page == EPage::palettePage) {
            mMultiColorButton->shouldHightlght(true);
        } else if (page == EPage::moodPage) {
            mMoodButton->shouldHightlght(true);
        } else if (page == EPage::settingsPage) {
            mSettingsButton->shouldHightlght(true);
        } else {
            qDebug() << "Do not recognize key " << pageToString(page);
        }
    } else {
        if (page != EPage::settingsPage) {
            mSingleColorButton->shouldHightlght(false);
            mMultiColorButton->shouldHightlght(false);
            mMoodButton->shouldHightlght(false);
            if (page == EPage::colorPage) {
                mSingleColorButton->shouldHightlght(true);
            } else if (page == EPage::palettePage) {
                mMultiColorButton->shouldHightlght(true);
            } else if (page == EPage::moodPage) {
                mMoodButton->shouldHightlght(true);
            } else {
                qDebug() << "Do not recognize key " << pageToString(page);
            }
        }
    }

    emit pressedButton(page);
}

void LeftHandMenu::updateSingleColorButton() {
    bool hasHue = mSelectedLights->hasLightWithProtocol(EProtocolType::hue);
    bool hasArduino = mSelectedLights->hasLightWithProtocol(EProtocolType::arduCor);
    if (hasHue && !hasArduino) {
        auto devices = mSelectedLights->lights();
        std::vector<HueMetadata> hues;
        for (auto& device : devices) {
            if (device.protocol() == EProtocolType::hue) {
                hues.push_back(mComm->hue()->hueLightFromLight(device));
            }
        }

        EHueType bestHueType = checkForHueWithMostFeatures(hues);
        // get a vector of all the possible hue types for a check.
        if (bestHueType == EHueType::ambient) {
            mSingleColorButton->updateIcon(":images/wheels/color_wheel_ct.png");
        } else {
            mSingleColorButton->updateIcon(":images/wheels/color_wheel_hsv.png");
        }
    } else {
        mSingleColorButton->updateIcon(":images/wheels/color_wheel_hsv.png");
    }
}


ParentGroupWidget* LeftHandMenu::initParentGroupWidget(const cor::Group& group,
                                                       const QString& key) {
    auto subgroups = mGroups->subgroups().subgroupsForGroup(group.uniqueID());
    auto widget = new ParentGroupWidget(group,
                                        subgroups,
                                        mComm,
                                        mGroups,
                                        key,
                                        cor::EListType::linear,
                                        cor::EWidgetType::condensed,
                                        mWidget);

    QScroller::grabGesture(widget, QScroller::LeftMouseButtonGesture);
    connect(widget,
            SIGNAL(deviceClicked(std::uint64_t, QString)),
            this,
            SLOT(lightClicked(std::uint64_t, QString)));
    connect(widget,
            SIGNAL(allButtonPressed(std::uint64_t, bool)),
            this,
            SLOT(groupSelected(std::uint64_t, bool)));
    connect(widget,
            SIGNAL(buttonsShown(std::uint64_t, bool)),
            this,
            SLOT(shouldShowButtons(std::uint64_t, bool)));
    connect(widget, SIGNAL(groupChanged(std::uint64_t)), this, SLOT(changedGroup(std::uint64_t)));

    mParentGroupWidgets.push_back(widget);
    resizeGroupWidgets();
    return widget;
}

int LeftHandMenu::resizeGroupWidgets() {
    int yPos = 0u;
    // check if any is open
    bool isAnyOpen = false;
    for (auto widget : mParentGroupWidgets) {
        if (widget->isOpen()) {
            isAnyOpen = true;
        }
    }
    std::sort(mParentGroupWidgets.begin(),
              mParentGroupWidgets.end(),
              [](ParentGroupWidget* a, ParentGroupWidget* b) { return a->key() < b->key(); });
    if (!isAnyOpen) {
        for (auto widget : mParentGroupWidgets) {
            widget->setVisible(true);
            widget->setFixedWidth(width());
            widget->setGeometry(0, yPos, width(), widget->height());
            yPos += widget->height();
        }
    } else {
        for (auto widget : mParentGroupWidgets) {
            if (widget->isOpen()) {
                widget->setVisible(true);
                widget->setFixedWidth(width());
                widget->setGeometry(0, yPos, width(), widget->height());
                yPos += widget->height();
            } else {
                widget->setVisible(false);
            }
        }
    }
    return yPos;
}

void LeftHandMenu::lightClicked(std::uint64_t, const QString& deviceKey) {
    //    qDebug() << "collection key:" << collectionKey
    //             << "device key:" << deviceKey;

    auto light = mComm->lightByID(deviceKey);
    auto state = light.state();
    if (light.isReachable()) {
        if (mSelectedLights->doesLightExist(light)) {
            mSelectedLights->removeLight(light);
        } else {
            mSelectedLights->addLight(light);
        }
        // update UI
        emit changedDeviceCount();
    }
}

void LeftHandMenu::groupSelected(std::uint64_t ID, bool shouldSelect) {
    // convert the group ID to a group
    auto groupResult = mGroups->groupDict().item(QString::number(ID).toStdString());
    bool groupFound = groupResult.second;
    cor::Group group = groupResult.first;
    if (groupFound) {
        auto lightIDs = group.lights();
        auto lights = mComm->lightsByIDs(group.lights());
        // if the group selected is found, either select or deselect it
        if (shouldSelect) {
            mSelectedLights->addLights(lights);
        } else {
            mSelectedLights->removeLights(lights);
        }
        updateLights();

        auto selectedLightIDs = cor::lightVectorToIDs(mSelectedLights->lights());
        for (const auto& widget : mParentGroupWidgets) {
            widget->setCheckedLights(selectedLightIDs);
        }
        emit changedDeviceCount();
    }
}

void LeftHandMenu::shouldShowButtons(std::uint64_t key, bool) {
    auto name = mGroups->nameFromID(key);
    // miscellaneous group isn't part of GroupData so its key can't be converted properly.
    if (key == 0u) {
        name = kMiscKey;
    }
    for (const auto& widget : mParentGroupWidgets) {
        if (widget->key() != name) {
            auto groupWidget = qobject_cast<ParentGroupWidget*>(widget);
            Q_ASSERT(groupWidget);
            groupWidget->closeWidget();
        }
    }
    resize();
}


void LeftHandMenu::changedGroup(std::uint64_t) {
    resize();
}

void LeftHandMenu::renderUI() {
    auto scrollValue = mScrollArea->verticalScrollBar()->value();
    if (mIsIn && (mLastRenderTime < mComm->lastUpdateTime()) && (scrollValue == mLastScrollValue)) {
        updateLights();
    } else {
        mLastScrollValue = scrollValue;
    }
}

void LeftHandMenu::newGroupButtonPressed() {
    if (!cor::leftHandMenuMoving()) {
        emit createNewGroup();
    } else {
        pushIn();
    }
}

void LeftHandMenu::removeParentGroup(const QString& name) {
    // find parent group in vector
    for (auto widget : mParentGroupWidgets) {
        if (widget->key() == name) {
            auto it = std::find(mParentGroupWidgets.begin(), mParentGroupWidgets.end(), widget);
            mParentGroupWidgets.erase(it);
            delete widget;
            return;
        }
    }
}


void LeftHandMenu::clearWidgets() {
    for (auto widget : mParentGroupWidgets) {
        delete widget;
    }
    mParentGroupWidgets.clear();
    resize();
}

QWidget* LeftHandMenu::selectedButton() {
    if (mSingleColorButton->highlighted()) {
        return mSingleColorButton;
    }
    if (mMultiColorButton->highlighted()) {
        return mMultiColorButton;
    }
    if (mMoodButton->highlighted()) {
        return mMoodButton;
    }
    if (mSettingsButton->highlighted()) {
        return mSettingsButton;
    }
    return nullptr;
}

void LeftHandMenu::mouseReleaseEvent(QMouseEvent* event) {
    if (!cor::leftHandMenuMoving()) {
        event->accept();
    } else {
        event->ignore();
    }
}

int LeftHandMenu::showingWidth() {
    auto width = this->width() + geometry().x();
    if (width < 0) {
        width = 0u;
    }
    if (width > this->width()) {
        width = this->width();
    }
    return width;
}
