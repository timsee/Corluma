/*!
 * \copyright
 * Copyright (C) 2015 - 2018.
 * Released under the GNU General Public License.
 */

#include <QGraphicsEffect>

#include "mainwindow.h"
#include "topmenu.h"
#include "cor/utils.h"
#include "hue/huelight.h"

TopMenu::TopMenu(DataLayer* data, CommLayer* comm, QWidget *parent) : QWidget(parent) {

    mData = data;
    mComm = comm;

    mCurrentPage = EPage::eConnectionPage;

    // --------------
    // Setup Buttons
    // --------------
    mConnectionButton = new QPushButton(this);
    mConnectionButton->setCheckable(true);
    mConnectionButton->setStyleSheet("background-color: rgb(52, 52, 52); ");
    mConnectionButton->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    mConnectionButton->setIcon(QIcon(":images/connectionIcon.png"));
    connect(mConnectionButton, SIGNAL(clicked(bool)), this, SLOT(connectionButtonPressed()));

    mColorPageButton = new QPushButton(this);
    mColorPageButton->setStyleSheet("background-color: rgb(52, 52, 52); ");
    mColorPageButton->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    mColorPageButton->setIcon(QIcon(":images/colorWheel_icon.png"));
    connect(mColorPageButton, SIGNAL(clicked(bool)), this, SLOT(colorButtonPressed()));

    mGroupPageButton = new cor::Button(this);
    mGroupPageButton->setupAsMenuButton((int)EPage::eGroupPage,  mData->colorGroup(EColorGroup::eFire));
    mGroupPageButton->button->setStyleSheet("background-color: rgb(52, 52, 52); ");
    mGroupPageButton->button->setCheckable(true);
    mGroupPageButton->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    connect(mGroupPageButton->button, SIGNAL(clicked(bool)), this, SLOT(groupButtonPressed()));

    mSettingsButton = new QPushButton(this);
    mSettingsButton->setStyleSheet("background-color: rgb(52, 52, 52); ");
    mSettingsButton->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    mSettingsButton->setIcon(QIcon(":images/settingsgear.png"));
    connect(mSettingsButton, SIGNAL(clicked(bool)), this, SLOT(settingsButtonPressed()));

    //---------------
    // Create Spacer
    //---------------

    mSpacer = new QWidget(this);
    mSpacer->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

    // --------------
    // Setup Brightness Slider
    // --------------
    // setup the slider that controls the LED's brightness
    mBrightnessSlider = new cor::Slider(this);
    mBrightnessSlider->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    mBrightnessSlider->slider()->setRange(0,100);
    mBrightnessSlider->slider()->setValue(0);
    mBrightnessSlider->slider()->setTickPosition(QSlider::TicksBelow);
    mBrightnessSlider->slider()->setTickInterval(20);
    mBrightnessSlider->setSliderHeight(0.5f);
    mBrightnessSlider->setSliderColorBackground(QColor(255,255,255));
    connect(mBrightnessSlider, SIGNAL(valueChanged(int)), this, SLOT(brightnessSliderChanged(int)));

    // --------------
    // Setup Preview Button
    // --------------
    mOnOffButton = new QPushButton(this);
    mOnOffButton->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    connect(mOnOffButton, SIGNAL(clicked(bool)), this, SLOT(toggleOnOff()));

    // setup the icons
    mIconData = IconData(124, 124);
    mIconData.setSolidColor(QColor(0,255,0));
    mOnOffButton->setIcon(mIconData.renderAsQPixmap());

    mTopLayout = new QHBoxLayout();
    mTopLayout->setSpacing(5);
    mTopLayout->setContentsMargins(0,5,5,0);
    mTopLayout->addWidget(mOnOffButton, 1);
    mTopLayout->addWidget(mBrightnessSlider, 10);
    mTopLayout->addWidget(mSettingsButton, 1);

    mBottomLayout = new QHBoxLayout();
    mBottomLayout->setSpacing(0);
    mBottomLayout->setContentsMargins(0,0,0,0);
    mBottomLayout->addWidget(mConnectionButton, 1);
    mBottomLayout->addWidget(mColorPageButton,  1);
    mBottomLayout->addWidget(mGroupPageButton,  1);
    mBottomLayout->addWidget(mSpacer,           4);

    mLayout = new QVBoxLayout(this);
    mLayout->setSpacing(5);
    mLayout->setContentsMargins(0,0,0,0);
    mLayout->addLayout(mTopLayout, 2);
    mLayout->addLayout(mBottomLayout, 3);

    setLayout(mLayout);

    // --------------
    // Connection Floating Layout
    // --------------
    mConnectionFloatingLayout = new FloatingLayout(false, this);
    connect(mConnectionFloatingLayout, SIGNAL(buttonPressed(QString)), this, SLOT(floatingLayoutButtonPressed(QString)));
    std::vector<QString> buttons = { QString("Discovery"), QString("New_Collection"), QString("Rooms"), QString("Groups")};
    mConnectionFloatingLayout->setupButtons(buttons);

    // --------------
    // Group Floating Layout
    // --------------
    mGroupFloatingLayout = new FloatingLayout(false, this);
    connect(mGroupFloatingLayout, SIGNAL(buttonPressed(QString)), this, SLOT(floatingLayoutButtonPressed(QString)));

    // --------------
    // Color Page Floating Layouts
    // --------------
    mColorFloatingLayout = new FloatingLayout(false, this);
    connect(mColorFloatingLayout, SIGNAL(buttonPressed(QString)), this, SLOT(floatingLayoutButtonPressed(QString)));

    //NOTE: by making the parent the parent of the floating widgets, they don't get cut off if they overextend over top menu
    mColorVerticalFloatingLayout = new FloatingLayout(true, parent);
    connect(mColorVerticalFloatingLayout, SIGNAL(buttonPressed(QString)), this, SLOT(floatingLayoutButtonPressed(QString)));
    mColorVerticalFloatingLayout->setVisible(false);

    mLastColorButtonKey = "RGB";

    deviceCountReachedZero();
    highlightButton(EPage::eConnectionPage);
}

TopMenu::~TopMenu() {

}

void TopMenu::toggleOnOff() {
    if (mData->currentRoutine() <= ELightingRoutine::eSingleSawtoothFadeOut) {
        mIconData.setSingleLightingRoutine(mData->currentRoutine(), mData->mainColor());
    } else if (mData->currentColorGroup() > EColorGroup::eCustom) {
        mIconData.setMultiLightingRoutine(mData->currentRoutine(), mData->currentColorGroup(), mData->currentGroup());
    } else {
        mIconData.setMultiFade(EColorGroup::eCustom, mData->colorGroup(EColorGroup::eCustom), true);
    }
    mOnOffButton->setIcon(mIconData.renderAsQPixmap());
    if (mData->isOn()) {
        QGraphicsOpacityEffect *effect = new QGraphicsOpacityEffect(mOnOffButton);
        effect->setOpacity(0.5f);
        mOnOffButton->setGraphicsEffect(effect);
        mData->turnOn(false);
    } else {
        mOnOffButton->setIcon(mIconData.renderAsQPixmap());
        QGraphicsOpacityEffect *effect = new QGraphicsOpacityEffect(mOnOffButton);
        effect->setOpacity(1.0f);
        mOnOffButton->setGraphicsEffect(effect);
        mData->turnOn(true);
    }
    emit buttonPressed("OnOff");
}


void TopMenu::brightnessSliderChanged(int newBrightness) {
    emit brightnessChanged(newBrightness);
}


void TopMenu::updateMenuBar() {
    //-----------------
    // On/Off Data
    //-----------------
    if (mData->currentColorGroup() == EColorGroup::eCustom
            && mData->currentRoutine() > cor::ELightingRoutineSingleColorEnd) {
        mIconData.setMultiLightingRoutine(mData->currentRoutine(),
                                          mData->currentColorGroup(),
                                          mData->currentGroup(),
                                          mData->customColorsUsed());
    } else if (mData->currentRoutine() <= cor::ELightingRoutineSingleColorEnd) {
        mIconData.setSingleLightingRoutine(mData->currentRoutine(), mData->mainColor());
    } else {
        mIconData.setMultiLightingRoutine(mData->currentRoutine(), mData->currentColorGroup(), mData->currentGroup());
    }

    mOnOffButton->setIcon(mIconData.renderAsQPixmap());

    //-----------------
    // Brightness Slider Update
    //-----------------

    updateBrightnessSlider();
}

void TopMenu::updateBrightnessSlider() {
    if ((int)mData->currentRoutine() <= (int)ELightingRoutine::eSingleSawtoothFadeOut) {
        mBrightnessSlider->setSliderColorBackground(mData->mainColor());
    } else {
        mBrightnessSlider->setSliderColorBackground(mData->colorsAverage(mData->currentColorGroup()));
    }

    if (mData->brightness() != mBrightnessSlider->slider()->value()) {
        mBrightnessSlider->blockSignals(true);
        mBrightnessSlider->slider()->setValue(mData->brightness());
        mBrightnessSlider->blockSignals(false);
    }
}



void TopMenu::updateSingleColor(QColor color) {
    mIconData.setSolidColor(color);
    mIconData.setSingleLightingRoutine(ELightingRoutine::eSingleGlimmer, color);
    mBrightnessSlider->setSliderColorBackground(color);
    mOnOffButton->setIcon(mIconData.renderAsQPixmap());
}

void TopMenu::updatePresetColorGroup(int colorGroup) {
    mIconData.setMultiFade((EColorGroup)colorGroup, mData->colorGroup((EColorGroup)colorGroup));
    mBrightnessSlider->setSliderColorBackground(mData->colorsAverage((EColorGroup)colorGroup));
    mOnOffButton->setIcon(mIconData.renderAsQPixmap());
}


void TopMenu::resizeMenuIcon(QPushButton *button, QString iconPath, float scale) {
    QPixmap pixmap(iconPath);
    int size = std::min(this->width() / 7 * 0.7f, (float)button->height()) * scale;
    button->setIcon(QIcon(pixmap.scaled(size,
                                        size,
                                        Qt::KeepAspectRatio,
                                        Qt::SmoothTransformation)));
    button->setIconSize(QSize(size, size));
}

void TopMenu::deviceCountReachedZero() {
    mColorPageButton->setEnabled(false);
    mBrightnessSlider->enable(false);

    mBrightnessSlider->slider()->setValue(0);
    QGraphicsOpacityEffect *effect = new QGraphicsOpacityEffect(mOnOffButton);
    effect->setOpacity(0.5f);
    mOnOffButton->setGraphicsEffect(effect);
    mOnOffButton->setEnabled(false);

    mShouldGreyOutIcons = true;
}

void TopMenu::deviceCountChangedOnConnectionPage() {
    if (mShouldGreyOutIcons
            && (mData->currentDevices().size() > 0)) {
        mColorPageButton->setEnabled(true);
        mBrightnessSlider->enable(true);

        QGraphicsOpacityEffect *effect = new QGraphicsOpacityEffect(mOnOffButton);
        effect->setOpacity(1.0);
        mOnOffButton->setGraphicsEffect(effect);
        mOnOffButton->setEnabled(true);

        mShouldGreyOutIcons = false;
    }
    if ((!mShouldGreyOutIcons
         && (mData->currentDevices().size() == 0))) {
        deviceCountReachedZero();
    }
    updateColorGroupButton();
}

void TopMenu::highlightButton(EPage button) {
    if (button == EPage::eColorPage) {
        colorButtonPressed();
    } else if (button == EPage::eConnectionPage) {
        connectionButtonPressed();
    } else if (button == EPage::eGroupPage) {
        groupButtonPressed();
    } else if (button == EPage::eSettingsPage) {
        settingsButtonPressed();
    }
}


void TopMenu::highlightGroupsButton() {
    mConnectionFloatingLayout->highlightButton("Groups");
}

void TopMenu::highlightRoomsButton() {
    mConnectionFloatingLayout->highlightButton("Rooms");
}

void TopMenu::settingsButtonPressed() {
    showFloatingLayout(EPage::eSettingsPage);
    emit buttonPressed("Settings");
}

void TopMenu::connectionButtonPressed() {
    mConnectionButton->setChecked(true);
    mConnectionButton->setStyleSheet("background-color: rgb(80, 80, 80); ");

    mColorPageButton->setChecked(false);
    mColorPageButton->setStyleSheet("background-color: rgb(52, 52, 52); ");

    mGroupPageButton->button->setChecked(false);
    mGroupPageButton->button->setStyleSheet("background-color: rgb(52, 52, 52); ");

    showFloatingLayout(EPage::eConnectionPage);
    emit buttonPressed("Connection");
}

void TopMenu::colorButtonPressed() {
    mConnectionButton->setChecked(false);
    mConnectionButton->setStyleSheet("background-color: rgb(52, 52, 52); ");

    mColorPageButton->setChecked(true);
    mColorPageButton->setStyleSheet("background-color: rgb(80, 80, 80); ");

    mGroupPageButton->button->setChecked(false);
    mGroupPageButton->button->setStyleSheet("background-color: rgb(52, 52, 52); ");

    showFloatingLayout(EPage::eColorPage);
    emit buttonPressed("Color");
}

void TopMenu::groupButtonPressed() {
    mConnectionButton->setChecked(false);
    mConnectionButton->setStyleSheet("background-color: rgb(52, 52, 52); ");

    mColorPageButton->setChecked(false);
    mColorPageButton->setStyleSheet("background-color: rgb(52, 52, 52); ");

    mGroupPageButton->button->setChecked(true);
    mGroupPageButton->button->setStyleSheet("background-color: rgb(80, 80, 80); ");

    showFloatingLayout(EPage::eGroupPage);
    emit buttonPressed("Group");
}

void TopMenu::resizeEvent(QResizeEvent *event) {
    Q_UNUSED(event);
    int onOffSize = this->height() / 3;
    mOnOffButton->setIconSize(QSize(onOffSize * 0.8f, onOffSize * 0.8f));
    mOnOffButton->setMinimumHeight(onOffSize);
    resizeMenuIcon(mSettingsButton, ":images/settingsgear.png");
    resizeMenuIcon(mConnectionButton, ":images/connectionIcon.png");
    updateColorGroupButton();
    moveFloatingLayout();
}

void TopMenu::updateColorGroupButton() {
    bool hasHue = mData->hasHueDevices();
    bool hasArduino = mData->hasArduinoDevices();
    if (hasHue && !hasArduino) {
        std::list<cor::Light> devices = mData->currentDevices();
        std::list<HueLight> hues;
        for (auto& device : devices) {
            hues.push_back(mComm->hue()->hueLightFromLight(device));
        }

        EHueType bestHueType = checkForHueWithMostFeatures(hues);
        // get a vector of all the possible hue types for a check.
        if (bestHueType == EHueType::eWhite) {
            resizeMenuIcon(mColorPageButton, ":images/white_wheel.png", 0.75f);
        } else if (bestHueType == EHueType::eAmbient) {
            resizeMenuIcon(mColorPageButton, ":images/ambient_wheel.png", 0.75f);
        } else if (bestHueType == EHueType::eExtended){
            resizeMenuIcon(mColorPageButton, ":images/colorWheel_icon.png", 0.75f);
        } else {
            throw "did not find any hue lights when expecting hue lights";
        }
    } else {
        if (mData->currentDevices().size() == 0) {
            resizeMenuIcon(mColorPageButton, ":images/white_wheel.png", 0.75f);
        } else {
            resizeMenuIcon(mColorPageButton, ":images/colorWheel_icon.png", 0.75f);
        }
    }
}

//--------------------
// FloatingLayout Setup
//--------------------

void TopMenu::setup(MainWindow *mainWindow,
                    GroupPage *groupPage,
                    ColorPage *colorPage,
                    ConnectionPage *connectionPage) {
    mMainWindow = mainWindow;
    mGroupPage = groupPage;
    mColorPage = colorPage;
    mConnectionPage = connectionPage;
}

void TopMenu::floatingLayoutButtonPressed(QString button) {
    if (button.compare("Discovery") == 0) {
        mMainWindow->switchToDiscovery();
    } else if (button.compare("New_Collection") == 0) {
        mMainWindow->editButtonClicked("", false);
    } else if (button.compare("Preset_Groups") == 0) {
        if (mData->hasArduinoDevices()) {
            mGroupPage->showPresetArduinoGroups();
        } else {
            mGroupPage->showPresetHueGroups();
        }
    } else if (button.compare("New_Mood") == 0) {
        mMainWindow->editButtonClicked("", true);
        // devicesButtonClicked(true);
    } else if (button.compare("Select_Moods") == 0) {
        mGroupPage->showCustomMoods();
    } else if (button.compare("Multi") == 0) {
        mLastColorButtonKey = button;
        mColorPage->changePageType(EColorPageType::eMulti);
        mColorVerticalFloatingLayout->highlightRoutineButton(false);
        updateColorVerticalRoutineButton();
    } else if (button.compare("RGB") == 0) {
        mLastColorButtonKey = button;
        mColorPage->changePageType(EColorPageType::eRGB);
        mColorVerticalFloatingLayout->highlightRoutineButton(false);
        updateColorVerticalRoutineButton();
    } else if (button.compare("Temperature") == 0) {
        mLastColorButtonKey = button;
        mColorPage->changePageType(EColorPageType::eAmbient);
        updateColorVerticalRoutineButton();
    } else if (button.compare("ColorScheme") == 0) {
        mLastColorButtonKey = button;
        mColorPage->changePageType(EColorPageType::eColorScheme);
        updateColorVerticalRoutineButton();
    } else if (button.compare("Routine") == 0) {
        mLastColorButtonKey = button;
        mColorPage->handleRoutineWidget();
        updateColorVerticalRoutineButton();
    } else if (button.compare("Groups") == 0) {
        mConnectionPage->displayGroups();
    } else if (button.compare("Rooms") == 0) {
        mConnectionPage->displayRooms();
    } else {
        qDebug() << "I don't recognize that button type...";
    }
}

void TopMenu::showFloatingLayout(EPage newPage) {
    if (newPage != mCurrentPage) {
        // move old menu back
        switch(mCurrentPage) {
        case EPage::eColorPage:
            pushRightFloatingLayout(mColorFloatingLayout);
            break;
        case EPage::eConnectionPage:
            if (newPage != EPage::eSettingsPage) {
                pushRightFloatingLayout(mConnectionFloatingLayout);
            }
            break;
        case EPage::eGroupPage:
            pushRightFloatingLayout(mGroupFloatingLayout);
            break;
        case EPage::eSettingsPage:
            // execution won't reach here (trademarked)
            break;
        }

        // move new menu forward
        switch(newPage) {
        case EPage::eColorPage:
            setupColorFloatingLayout();
            mColorVerticalFloatingLayout->setVisible(true);
            pullLeftFloatingLayout(mColorFloatingLayout);
            break;
        case EPage::eConnectionPage:
            pullLeftFloatingLayout(mConnectionFloatingLayout);
            break;
        case EPage::eGroupPage:
        {
            std::vector<QString> buttons;
            buttons = { QString("Select_Moods"), QString("Preset_Groups"), QString("New_Mood")};
            mGroupPage->showCustomMoods();

            mGroupFloatingLayout->setupButtons(buttons);
            pullLeftFloatingLayout(mGroupFloatingLayout);
            mGroupFloatingLayout->updateGroupPageButtons(mData->colors());
            mGroupFloatingLayout->highlightButton(buttons[0]);
            break;
        }
        case EPage::eSettingsPage:
            mColorVerticalFloatingLayout->setVisible(false);
            break;
        }

        // settings handled as special case and is not stored here
        if (newPage != EPage::eSettingsPage) {
            mCurrentPage = newPage;
        }
    }
}

FloatingLayout *TopMenu::currentFloatingLayout() {
    FloatingLayout *layout;
    switch (mCurrentPage) {
    case EPage::eConnectionPage:
        layout = mConnectionFloatingLayout;
        break;
    case EPage::eGroupPage:
        layout = mGroupFloatingLayout;
        break;
    case EPage::eColorPage:
        layout = mColorFloatingLayout;
        break;
    default:
        layout = mConnectionFloatingLayout;
        break;
    }
    return layout;
}

void TopMenu::moveFloatingLayout() {
    QPoint topRight(this->width(), mBottomLayout->geometry().y());
    currentFloatingLayout()->move(topRight);
    currentFloatingLayout()->raise();
    if (mCurrentPage == EPage::eColorPage) {
        mColorVerticalFloatingLayout->move(QPoint(topRight.x(), topRight.y() + currentFloatingLayout()->geometry().height()));
        mColorVerticalFloatingLayout->raise();
    }
    moveHiddenLayouts();
}


void TopMenu::setupColorFloatingLayout() {
    bool hasHue = mData->hasHueDevices();
    bool hasArduino = mData->hasArduinoDevices();
    std::vector<QString> horizontalButtons;
    std::vector<QString> verticalButtons;
    if (hasHue && !hasArduino) {
        // get list of all current devices
        std::list<cor::Light> devices = mData->currentDevices();
        std::list<HueLight> hues;
        for (auto& device : devices) {
            hues.push_back(mComm->hue()->hueLightFromLight(device));
        }
        EHueType bestHueType = checkForHueWithMostFeatures(hues);
        // get a vector of all the possible hue types for a check.
        if (bestHueType == EHueType::eWhite) {
            mColorPage->changePageType(EColorPageType::eBrightness, true);
        } else if (bestHueType == EHueType::eAmbient) {
            mColorPage->changePageType(EColorPageType::eAmbient, true);
        } else if (bestHueType == EHueType::eExtended){
            horizontalButtons = {QString("Temperature"), QString("RGB")};
            if (mData->currentDevices().size() > 1) {
                horizontalButtons.push_back(QString("ColorScheme"));
            }
            mColorPage->changePageType(EColorPageType::eRGB, true);
        } else {
            throw "did not find any hue lights when expecting hue lights";
        }
    } else if (hasArduino){
        horizontalButtons = {QString("Temperature"), QString("RGB"), QString("Multi"), QString("ColorScheme")};
        verticalButtons = {QString("Routine")};

        mColorFloatingLayout->highlightButton(mLastColorButtonKey);
        mColorFloatingLayout->addMultiRoutineIcon(mData->colorGroup(EColorGroup::eRGB));
        updateColorVerticalRoutineButton();
        mColorPage->changePageType(EColorPageType::eRGB, true);
    } else if (!hasHue && !hasArduino) {
        // shouldn't get here...
        throw "trying to open single color page when no recognized devices are selected";
    }
    updateColorGroupButton();

    mColorVerticalFloatingLayout->setupButtons(verticalButtons);
    mColorFloatingLayout->setupButtons(horizontalButtons);
    mColorFloatingLayout->highlightButton("RGB");

    updateColorVerticalRoutineButton();
    mColorFloatingLayout->addMultiRoutineIcon(mData->colorGroup(EColorGroup::eRGB));
}

void TopMenu::updateColorVerticalRoutineButton() {
    if (mColorPage->pageType() == EColorPageType::eRGB) {
        mColorVerticalFloatingLayout->setVisible(true);
        mColorVerticalFloatingLayout->updateRoutineSingleColor(mData->currentRoutine(),
                                                               mData->mainColor());
    } else if (mColorPage->pageType() == EColorPageType::eMulti) {
        mColorVerticalFloatingLayout->setVisible(true);
        mColorVerticalFloatingLayout->updateRoutineMultiColor(mData->currentRoutine(),
                                                              mData->colorGroup(EColorGroup::eCustom),
                                                              mData->customColorsUsed());
    } else if (mColorPage->pageType() == EColorPageType::eAmbient
               || mColorPage->pageType() == EColorPageType::eBrightness
               || mColorPage->pageType() == EColorPageType::eColorScheme) {
        mColorVerticalFloatingLayout->setVisible(false);
    }
}

void TopMenu::pushRightFloatingLayout(FloatingLayout *layout) {
    QPoint topRight(this->width() - layout->width(),
                    mBottomLayout->geometry().y());
    layout->setGeometry(topRight.x(),
                        topRight.y(),
                        layout->width(), layout->height());
    QPropertyAnimation *animation = new QPropertyAnimation(layout, "pos");
    animation->setDuration(TRANSITION_TIME_MSEC);
    animation->setStartValue(layout->pos());
    animation->setEndValue(QPoint(this->width() + layout->width(),
                                  mBottomLayout->geometry().y()));
    animation->start();

    if (layout == mColorFloatingLayout) {
        QPoint startPoint(this->width(),
                          mBottomLayout->geometry().y() + currentFloatingLayout()->geometry().height());
        QPoint endPoint(startPoint.x() - mColorVerticalFloatingLayout->width(),
                        startPoint.y());
        QPropertyAnimation *animation2 = new QPropertyAnimation(mColorVerticalFloatingLayout, "pos");
        animation2->setDuration(TRANSITION_TIME_MSEC);
        animation2->setStartValue(endPoint);
        animation2->setEndValue(startPoint);
        animation2->start();
    }
}

void TopMenu::pullLeftFloatingLayout(FloatingLayout *layout) {
    QPoint topRight(this->width() - layout->width(),
                    mBottomLayout->geometry().y());
    layout->setGeometry(this->width() + layout->width(),
                        mBottomLayout->geometry().y(),
                        layout->width(), layout->height());
    QPropertyAnimation *animation = new QPropertyAnimation(layout, "pos");
    animation->setDuration(TRANSITION_TIME_MSEC);
    animation->setStartValue(layout->pos());
    animation->setEndValue(topRight);
    animation->start();

    if (layout == mColorFloatingLayout) {
        QPoint startPoint(this->width(),
                          mBottomLayout->geometry().y() + currentFloatingLayout()->geometry().height());
        QPoint endPoint(startPoint.x() - mColorVerticalFloatingLayout->width(),
                        startPoint.y());
        QPropertyAnimation *animation2 = new QPropertyAnimation(mColorVerticalFloatingLayout, "pos");
        animation2->setDuration(TRANSITION_TIME_MSEC);
        animation2->setStartValue(startPoint);
        animation2->setEndValue(endPoint);
        animation2->start();
    }
}

void TopMenu::moveHiddenLayouts() {
    FloatingLayout *layout = currentFloatingLayout();
    if (layout != mColorFloatingLayout) {
        QPoint topRight(this->width(),
                        mBottomLayout->geometry().y());
        mColorFloatingLayout->setGeometry(topRight.x(),
                                          topRight.y(),
                                          mColorFloatingLayout->width(),
                                          mColorFloatingLayout->height());
        mColorVerticalFloatingLayout->setGeometry(topRight.x(),
                                                  topRight.y() + mColorFloatingLayout->height(),
                                                  mColorFloatingLayout->width(),
                                                  mColorFloatingLayout->height());
    }
    if (layout != mGroupFloatingLayout) {
        QPoint topRight(this->width(),
                        mBottomLayout->geometry().y());
        mGroupFloatingLayout->setGeometry(topRight.x(),
                                          topRight.y(),
                                          mGroupFloatingLayout->width(),
                                          mGroupFloatingLayout->height());
    }
    if (layout != mConnectionFloatingLayout) {
        QPoint topRight(this->width(),
                        mBottomLayout->geometry().y());
        mConnectionFloatingLayout->setGeometry(topRight.x(),
                                               topRight.y(),
                                               mConnectionFloatingLayout->width(),
                                               mConnectionFloatingLayout->height());

    }
}

