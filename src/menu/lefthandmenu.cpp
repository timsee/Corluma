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

/// disables the light menu for debugging light menus in other places.
//#define DISABLE_LIGHTS_MENU

LeftHandMenu::LeftHandMenu(bool alwaysOpen,
                           cor::LightList* devices,
                           CommLayer* comm,
                           cor::LightList* lights,
                           GroupData* groups,
                           QWidget* parent)
    : QWidget(parent),
      mAlwaysOpen{alwaysOpen},
      mSpacer{new QWidget(this)},
      mSelectedLights{devices},
      mMainPalette{new cor::LightVectorWidget(6, 2, true, this)},
      mComm{comm},
      mData{lights},
      mGroups{groups},
      mLastRenderTime{QTime::currentTime()} {
    auto width = int(parent->size().width() * 0.66f);
    setGeometry(width * -1, 0, width, parent->height());
    mIsIn = false;
    if (mAlwaysOpen) {
        mIsIn = true;
    }

    mSpacer->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    mSpacer->setStyleSheet("border: none; background-color:rgb(33,32,32);");

#ifndef DISABLE_LIGHTS_MENU
    mLightMenu = new StandardLightsMenu(this, mComm, mGroups);
    connect(mLightMenu, SIGNAL(clickedLight(QString)), this, SLOT(lightClicked(QString)));
    connect(mLightMenu,
            SIGNAL(clickedGroupSelectAll(std::uint64_t, bool)),
            this,
            SLOT(selectAllToggled(std::uint64_t, bool)));
#endif

    // --------------
    // Setup Main Palette
    // -------------
    mMainPalette->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    mMainPalette->setFixedHeight(int(height() * 0.1));
    mMainPalette->setStyleSheet("background-color:rgb(33,32,32);");

    //---------------
    // Setup Buttons
    //---------------

    mLightsButton =
        new LeftHandButton("Lights", EPage::discoveryPage, ":/images/connectionIcon.png", this);
    mLightsButton->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    connect(mLightsButton, SIGNAL(pressed(EPage)), this, SLOT(buttonPressed(EPage)));

    mSingleColorButton = new LeftHandButton("Single Color",
                                            EPage::colorPage,
                                            ":/images/wheels/color_wheel_hsv.png",
                                            this);
    mSingleColorButton->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    connect(mSingleColorButton, SIGNAL(pressed(EPage)), this, SLOT(buttonPressed(EPage)));
    mSingleColorButton->shouldHightlght(true);

    mSettingsButton =
        new LeftHandButton("Settings", EPage::settingsPage, ":/images/settingsgear.png", this);
    mSettingsButton->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    connect(mSettingsButton, SIGNAL(pressed(EPage)), this, SLOT(buttonPressed(EPage)));


    mTimeoutButton =
        new TimeoutButton("Timeouts", EPage::timeoutPage, ":/images/timer.png", mComm, mData, this);
    mTimeoutButton->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    connect(mTimeoutButton, SIGNAL(pressed(EPage)), this, SLOT(buttonPressed(EPage)));


    PresetPalettes palettes;
    cor::LightState state;
    state.routine(ERoutine::multiBars);
    state.palette(palettes.palette(EPalette::water));
    state.isOn(true);
    mMultiColorButton = new LeftHandButton("Multi Color", EPage::palettePage, state, this);
    mMultiColorButton->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    connect(mMultiColorButton, SIGNAL(pressed(EPage)), this, SLOT(buttonPressed(EPage)));

    cor::LightState moodState;
    moodState.routine(ERoutine::multiFade);
    moodState.palette(palettes.palette(EPalette::fire));
    moodState.isOn(true);
    mMoodButton = new LeftHandButton("Moods", EPage::moodPage, moodState, this);
    mMoodButton->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    connect(mMoodButton, SIGNAL(pressed(EPage)), this, SLOT(buttonPressed(EPage)));

    mRenderThread = new QTimer(this);
    connect(mRenderThread, SIGNAL(timeout()), this, SLOT(renderUI()));
    if (mAlwaysOpen) {
        mRenderThread->start(333);
    }
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

    auto buttonHeight = mRowHeight;
    auto yPos = int(height() * 0.02);

    mSpacer->setGeometry(0, 0, this->width(), height());

    mSingleColorButton->setGeometry(0, yPos, this->width(), buttonHeight);
    yPos += mSingleColorButton->height();

    mMultiColorButton->setGeometry(0, yPos, this->width(), buttonHeight);
    yPos += mMultiColorButton->height();

    mMoodButton->setGeometry(0, yPos, this->width(), buttonHeight);
    yPos += mMoodButton->height();

    mLightsButton->setGeometry(0, yPos, this->width(), buttonHeight);
    yPos += mLightsButton->height();

    mTimeoutButton->setGeometry(0, yPos, this->width(), buttonHeight);
    yPos += mTimeoutButton->height();

    mSettingsButton->setGeometry(0, yPos, this->width(), buttonHeight);
    yPos += mSettingsButton->height();

    mMainPalette->setGeometry(0, yPos, this->width(), int(buttonHeight * 1.2));

    yPos += int(mMainPalette->height() + height() * 0.02);

#ifndef DISABLE_LIGHTS_MENU
    QRect lightMenuRect(0, yPos, this->width(), this->height() - yPos);
    mLightMenu->resize(lightMenuRect, buttonHeight * 1.2);
#endif
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

void LeftHandMenu::lightCountChanged() {
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



void LeftHandMenu::updateLights() {
    mLastRenderTime = QTime::currentTime();
#ifndef DISABLE_LIGHTS_MENU
    mLightMenu->updateLights(cor::lightVectorToIDs(mData->lights()));
#endif
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
        mLightsButton->shouldHightlght(false);
        mSingleColorButton->shouldHightlght(false);
        mMultiColorButton->shouldHightlght(false);
        mMoodButton->shouldHightlght(false);
        mSettingsButton->shouldHightlght(false);
        mTimeoutButton->shouldHightlght(false);
        if (page == EPage::discoveryPage) {
            mLightsButton->shouldHightlght(true);
        } else if (page == EPage::colorPage) {
            mSingleColorButton->shouldHightlght(true);
        } else if (page == EPage::palettePage) {
            mMultiColorButton->shouldHightlght(true);
        } else if (page == EPage::moodPage) {
            mMoodButton->shouldHightlght(true);
        } else if (page == EPage::settingsPage) {
            mSettingsButton->shouldHightlght(true);
        } else if (page == EPage::timeoutPage) {
            mTimeoutButton->shouldHightlght(true);
        } else {
            qDebug() << "Do not recognize key " << pageToString(page);
        }
    } else {
        if (page != EPage::settingsPage) {
            mLightsButton->shouldHightlght(false);
            mSingleColorButton->shouldHightlght(false);
            mMultiColorButton->shouldHightlght(false);
            mMoodButton->shouldHightlght(false);
            mTimeoutButton->shouldHightlght(false);
            if (page == EPage::discoveryPage) {
                mLightsButton->shouldHightlght(true);
            } else if (page == EPage::colorPage) {
                mSingleColorButton->shouldHightlght(true);
            } else if (page == EPage::palettePage) {
                mMultiColorButton->shouldHightlght(true);
            } else if (page == EPage::moodPage) {
                mMoodButton->shouldHightlght(true);
            } else if (page == EPage::timeoutPage) {
                mTimeoutButton->shouldHightlght(true);
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


void LeftHandMenu::lightClicked(const QString& lightKey) {
    auto light = mComm->lightByID(lightKey);
    auto state = light.state();
    if (light.isReachable()) {
        if (mSelectedLights->doesLightExist(light)) {
            mSelectedLights->removeLight(light);
        } else {
            mSelectedLights->addLight(light);
        }

        // update UI
        emit changedLightCount();
        lightCountChanged();
        updateLights();
    }
}


void LeftHandMenu::selectAllToggled(std::uint64_t ID, bool shouldSelect) {
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
        emit changedLightCount();
        lightCountChanged();
    } else {
        qDebug() << " group not found " << ID;
    }
}

void LeftHandMenu::renderUI() {
    if (mIsIn && (mLastRenderTime < mComm->lastUpdateTime())) {
        updateLights();
    }
}

void LeftHandMenu::newGroupButtonPressed() {
    if (!cor::leftHandMenuMoving()) {
        emit createNewGroup();
    } else {
        pushIn();
    }
}


void LeftHandMenu::clearWidgets() {
#ifndef DISABLE_LIGHTS_MENU
    mLightMenu->reset();
#endif
    resize();
}

QWidget* LeftHandMenu::selectedButton() {
    if (mLightsButton->highlighted()) {
        return mLightsButton;
    }
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
    if (mTimeoutButton->highlighted()) {
        return mTimeoutButton;
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


void LeftHandMenu::updateTimeoutButton(bool enabled, std::uint32_t value) {
    mTimeoutButton->update(enabled, value);
}
