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
      mButtonsEnabled{false},
      mIsIn{false},
      mSpacer{new QWidget(this)},
      mStateWidget{new GlobalStateWidget(this)},
      mSelectedLights{devices},
      mComm{comm},
      mData{lights},
      mGroups{groups},
      mLastRenderTime{QTime::currentTime()},
      mRowHeight{10} {
    auto width = int(parent->size().width() * 0.66f);
    setGeometry(width * -1, 0, width, parent->height());
    mIsIn = false;
    if (mAlwaysOpen) {
        mIsIn = true;
    }

    mSpacer->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    mSpacer->setStyleSheet(cor::kDarkerGreyBackground);

#ifndef DISABLE_LIGHTS_MENU
    mLightMenu = new StandardLightsMenu(this, mComm, mGroups, "LeftHandMenu");
    connect(mLightMenu, SIGNAL(clickedLight(QString)), this, SLOT(lightClicked(QString)));
    connect(mLightMenu,
            SIGNAL(clickedGroupSelectAll(std::uint64_t, bool)),
            this,
            SLOT(selectAllToggled(std::uint64_t, bool)));
#endif

    //---------------
    // Setup Buttons
    //---------------

    mLightsButton =
        new LeftHandButton("Lights", EPage::lightsPage, ":/images/lights_icon.png", this);
    mLightsButton->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    connect(mLightsButton, SIGNAL(pressed(EPage)), this, SLOT(buttonPressed(EPage)));
    mLightsButton->shouldHightlght(true);

    mSingleColorButton = new LeftHandButton("Single Color",
                                            EPage::colorPage,
                                            ":/images/single_color_icon.png",
                                            this);
    mSingleColorButton->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    connect(mSingleColorButton, SIGNAL(pressed(EPage)), this, SLOT(buttonPressed(EPage)));

    mMultiColorButton = new LeftHandButton("Multi Color",
                                           EPage::palettePage,
                                           ":/images/multi_color_icon.png",
                                           this);
    mMultiColorButton->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    connect(mMultiColorButton, SIGNAL(pressed(EPage)), this, SLOT(buttonPressed(EPage)));

    mMoodButton = new LeftHandButton("Moods", EPage::moodPage, ":/images/mood_icon.png", this);
    mMoodButton->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    connect(mMoodButton, SIGNAL(pressed(EPage)), this, SLOT(buttonPressed(EPage)));

    mSettingsButton =
        new LeftHandButton("Settings", EPage::settingsPage, ":/images/settingsgear.png", this);
    mSettingsButton->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    connect(mSettingsButton, SIGNAL(pressed(EPage)), this, SLOT(buttonPressed(EPage)));

#ifdef USE_EXPERIMENTAL_FEATURES
    mTimeoutButton = new TimeoutButton("Timeouts",
                                       EPage::timeoutPage,
                                       ":/images/timeout_icon.png",
                                       mComm,
                                       mData,
                                       this);
    mTimeoutButton->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    connect(mTimeoutButton, SIGNAL(pressed(EPage)), this, SLOT(buttonPressed(EPage)));
#endif

    // by default, initialize with certain buttons disabled until a connection to any light is made
    enableButtons(false);

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

    mStateWidget->setGeometry(0, 0, width, buttonHeight * 0.2);

    auto yPos = mStateWidget->height() + int(height() * 0.02);

    mSpacer->setGeometry(0, 0, width, height());

    mLightsButton->setGeometry(0, yPos, width, buttonHeight);
    yPos += mLightsButton->height();

    mSingleColorButton->setGeometry(0, yPos, width, buttonHeight);
    yPos += mSingleColorButton->height();

    mMultiColorButton->setGeometry(0, yPos, width, buttonHeight);
    yPos += mMultiColorButton->height();

    mMoodButton->setGeometry(0, yPos, width, buttonHeight);
    yPos += mMoodButton->height();

#ifdef USE_EXPERIMENTAL_FEATURES
    mTimeoutButton->setGeometry(0, yPos, width, buttonHeight);
    yPos += mTimeoutButton->height();
#endif

    mSettingsButton->setGeometry(0, yPos, width, buttonHeight);
    yPos += mSettingsButton->height();

    // second time to use as a spacer
    yPos += mSettingsButton->height();

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



void LeftHandMenu::updateLights() {
    mLastRenderTime = QTime::currentTime();
#ifndef DISABLE_LIGHTS_MENU
    mLightMenu->updateMenu();
    mLightMenu->selectLights(cor::lightVectorToIDs(mData->lights()));
#endif
    if (!alwaysOpen() && isIn()) {
        mStateWidget->update(cor::lightStatesFromLights(mData->lights(), true));
    }
}


void LeftHandMenu::buttonPressed(EPage page) {
    if (alwaysOpen()) {
        mLightsButton->shouldHightlght(false);
        mSingleColorButton->shouldHightlght(false);
        mMultiColorButton->shouldHightlght(false);
        mMoodButton->shouldHightlght(false);
        mSettingsButton->shouldHightlght(false);
#ifdef USE_EXPERIMENTAL_FEATURES
        mTimeoutButton->shouldHightlght(false);
#endif
        if (page == EPage::lightsPage) {
            mLightsButton->shouldHightlght(true);
        } else if (page == EPage::colorPage) {
            mSingleColorButton->shouldHightlght(true);
        } else if (page == EPage::palettePage) {
            mMultiColorButton->shouldHightlght(true);
        } else if (page == EPage::moodPage) {
            mMoodButton->shouldHightlght(true);
        } else if (page == EPage::settingsPage) {
            mSettingsButton->shouldHightlght(true);
        }
#ifdef USE_EXPERIMENTAL_FEATURES
        else if (page == EPage::timeoutPage) {
            mTimeoutButton->shouldHightlght(true);
        }
#endif
        else {
            qDebug() << "Do not recognize key " << pageToString(page);
        }
    } else {
        mLightsButton->shouldHightlght(false);
        mSingleColorButton->shouldHightlght(false);
        mMultiColorButton->shouldHightlght(false);
        mMoodButton->shouldHightlght(false);
#ifdef USE_EXPERIMENTAL_FEATURES
        mTimeoutButton->shouldHightlght(false);
#endif
        mSettingsButton->shouldHightlght(false);
        if (page == EPage::lightsPage) {
            mLightsButton->shouldHightlght(true);
        } else if (page == EPage::colorPage) {
            mSingleColorButton->shouldHightlght(true);
        } else if (page == EPage::palettePage) {
            mMultiColorButton->shouldHightlght(true);
        } else if (page == EPage::moodPage) {
            mMoodButton->shouldHightlght(true);
        }
#ifdef USE_EXPERIMENTAL_FEATURES
        else if (page == EPage::timeoutPage) {
            mTimeoutButton->shouldHightlght(true);
        }
#endif
        else if (page == EPage::settingsPage) {
            mSettingsButton->shouldHightlght(true);
        } else {
            qDebug() << "Do not recognize key " << pageToString(page);
        }
    }

    emit pressedButton(page);
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
    } else {
        qDebug() << " group not found " << ID;
    }
}

void LeftHandMenu::renderUI() {
    if (mIsIn && (mLastRenderTime < mComm->lastReceiveTime())) {
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
#ifdef USE_EXPERIMENTAL_FEATURES
    if (mTimeoutButton->highlighted()) {
        return mTimeoutButton;
    }
#endif
    return nullptr;
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
#ifndef USE_EXPERIMENTAL_FEATURES
    Q_UNUSED(enabled)
    Q_UNUSED(value)
#else
    mTimeoutButton->update(enabled, value);
#endif
}

void LeftHandMenu::enableButtons(bool enable) {
    mButtonsEnabled = enable;

    mSingleColorButton->setEnabled(enable);
    mMultiColorButton->setEnabled(enable);
    mMoodButton->setEnabled(enable);
#ifdef USE_EXPERIMENTAL_FEATURES
    mTimeoutButton->setEnabled(enable);
#endif
}
