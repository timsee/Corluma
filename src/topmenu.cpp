/*!
 * \copyright
 * Copyright (C) 2015 - 2018.
 * Released under the GNU General Public License.
 */

#include <QGraphicsEffect>
#include <QApplication>
#include <QScreen>
#include "mainwindow.h"
#include "topmenu.h"
#include "cor/utils.h"
#include "hue/huelight.h"

TopMenu::TopMenu(DataLayer* data, CommLayer* comm, QWidget *parent) : QWidget(parent) {

    mData = data;
    mComm = comm;

    QScreen *screen = QApplication::screens().at(0);
    mSize = QSize(screen->size().height() * 0.05f, screen->size().height() * 0.05f);
#ifdef MOBILE_BUILD
    mSize = QSize(mSize.width() * 2.5, mSize.height() * 2.5);
#endif

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
    mBrightnessSlider->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    mBrightnessSlider->setFixedHeight(mSize.height() / 2);
    mBrightnessSlider->slider()->setRange(0,100);
    mBrightnessSlider->slider()->setValue(0);
    mBrightnessSlider->setSliderHeight(0.8f);
    mBrightnessSlider->setSliderColorBackground(QColor(255,255,255));
    connect(mBrightnessSlider, SIGNAL(valueChanged(int)), this, SLOT(brightnessSliderChanged(int)));

    // --------------
    // Setup Main Palette
    // -------------
    mMainPalette = new cor::PaletteWidget(10, 1, mData->colors(), cor::EPaletteWidgetType::eStandard, this);
    mMainPalette->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    mMainPalette->setFixedHeight(mSize.height() / 2);

    // --------------
    // Setup Label for Devices
    // -------------
    mSelectedDevicesLabel = new QLabel(this);
    mSelectedDevicesLabel->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    mSelectedDevicesLabel->setFixedHeight(mSize.height() / 2);
    mSelectedDevicesLabel->setAlignment(Qt::AlignRight);
    mSelectedDevicesLabel->setWordWrap(true);
    //mSelectedDevicesLabel->setStyleSheet("font-size:10pt;");

    // --------------
    // Setup on/off switch
    // --------------
    mOnOffSwitch = new cor::Switch(this);
    mOnOffSwitch->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    mOnOffSwitch->setFixedSize(QSize(mSize.width(), mSize.height() / 2));
    connect(mOnOffSwitch, SIGNAL(switchChanged(bool)), this, SLOT(changedSwitchState(bool)));
    mOnOffSwitch->setSwitchState(ESwitchState::eDisabled);

    mLayout = new QGridLayout(this);
    mLayout->setSpacing(5);
    mLayout->setContentsMargins(0, mLayout->contentsMargins().top(),
                                0, mLayout->contentsMargins().bottom());

    mLayout->addWidget(mOnOffSwitch,      0, 0,  1, 1);
    mLayout->addWidget(mBrightnessSlider, 0, 1,  1, 9);

    mLayout->addWidget(mMainPalette,               1, 0,  1, 7);
    mLayout->addWidget(mSelectedDevicesLabel,      1, 7,  1, 3);

    mLayout->addWidget(mSpacer,           2, 0,  3, 10);
    setLayout(mLayout);
    mFloatingMenuStart = mOnOffSwitch->height() + mMainPalette->height() + mOnOffSwitch->geometry().y() + mLayout->spacing() * 2 + mLayout->contentsMargins().top();

    // --------------
    // Main Layout
    // --------------
    mMainLayout = new FloatingLayout(false, parent);
    connect(mMainLayout, SIGNAL(buttonPressed(QString)), this, SLOT(floatingLayoutButtonPressed(QString)));
    std::vector<QString> buttons = { QString("Connection_Page"), QString("Colors_Page"), QString("Multi_Colors_Page"), QString("Select_Moods")};
    mMainLayout->setupButtons(buttons, EButtonSize::eMedium);
    mMainLayout->addMultiRoutineIcon(mData->palette(EPalette::eFire));
    mMainLayout->updateGroupPageButtons(mData->colors());

    // --------------
    // Connection Floating Layout
    // --------------
    mConnectionFloatingLayout = new FloatingLayout(false, parent);
    connect(mConnectionFloatingLayout, SIGNAL(buttonPressed(QString)), this, SLOT(floatingLayoutButtonPressed(QString)));
    buttons = { QString("New_Group"), QString("Settings")};
    mConnectionFloatingLayout->setupButtons(buttons);

    // --------------
    // Group Floating Layout
    // --------------
    mGroupFloatingLayout = new FloatingLayout(false, parent);
    connect(mGroupFloatingLayout, SIGNAL(buttonPressed(QString)), this, SLOT(floatingLayoutButtonPressed(QString)));
    buttons = {QString("Settings")};
    mGroupFloatingLayout->setupButtons(buttons);

    // --------------
    // Moods Floating Layout
    // --------------
    mMoodsFloatingLayout = new FloatingLayout(false, parent);
    connect(mMoodsFloatingLayout, SIGNAL(buttonPressed(QString)), this, SLOT(floatingLayoutButtonPressed(QString)));
    buttons = {QString("New_Group"), QString("Settings")};
    mMoodsFloatingLayout->setupButtons(buttons);

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

    connect(mComm, SIGNAL(packetReceived(EProtocolType)), this, SLOT(receivedPacket(EProtocolType)));

    mCurrentPage = EPage::eGroupPage;
    pageButtonPressed(EPage::eConnectionPage);
    mMainLayout->highlightButton("Connection_Page");
}

TopMenu::~TopMenu() {

}

void TopMenu::changedSwitchState(bool switchState) {
    mData->turnOn(switchState);
    emit buttonPressed("OnOff");
}


void TopMenu::brightnessSliderChanged(int newBrightness) {
    emit brightnessChanged(newBrightness);
}


void TopMenu::updateMenuBar() {
    //-----------------
    // On/Off Data
    //-----------------
    if (mData->currentDevices().size() == 0) {
        mOnOffSwitch->setSwitchState(ESwitchState::eDisabled);
    } else if (mData->isOn()) {
        mOnOffSwitch->setSwitchState(ESwitchState::eOn);
    } else {
        mOnOffSwitch->setSwitchState(ESwitchState::eOff);
    }

    //-----------------
    // Brightness Slider Update
    //-----------------

    updateBrightnessSlider();
}

void TopMenu::updateBrightnessSlider() {
    mBrightnessSlider->setSliderColorBackground(mData->mainColor());

    if (mData->brightness() != mBrightnessSlider->slider()->value()) {
        mBrightnessSlider->blockSignals(true);
        mBrightnessSlider->slider()->setValue(mData->brightness());
        mBrightnessSlider->blockSignals(false);
    }
}



void TopMenu::updateSingleColor(QColor color) {
    mBrightnessSlider->setSliderColorBackground(color);
    updateColorVerticalRoutineButton();
}

void TopMenu::updatePresetPalette(EPalette palette) {
    mBrightnessSlider->setSliderColorBackground(mData->colorsAverage(palette));
}

void TopMenu::deviceCountReachedZero() {
    mBrightnessSlider->enable(false);
    mMainLayout->enableButton("Colors_Page", false);
    mMainLayout->enableButton("Multi_Colors_Page", false);

    mBrightnessSlider->slider()->setValue(0);
    mOnOffSwitch->setSwitchState(ESwitchState::eDisabled);

    mShouldGreyOutIcons = true;
}

void TopMenu::deviceCountChanged() {
    if (mShouldGreyOutIcons
            && (mData->currentDevices().size() > 0)) {
        mBrightnessSlider->enable(true);
        mMainLayout->enableButton("Colors_Page", true);
        mMainLayout->enableButton("Multi_Colors_Page", true);

        if (mData->isOn()) {
            mOnOffSwitch->setSwitchState(ESwitchState::eOn);
        } else {
            mOnOffSwitch->setSwitchState(ESwitchState::eOff);
        }

        mShouldGreyOutIcons = false;
    }

    if (mData->currentDevices().size() > 0) {
        QString devicesText = mData->findCurrentCollection(mComm->collectionList(), true);
        mSelectedDevicesLabel->setText(devicesText);
    } else {
        mSelectedDevicesLabel->setText("");
    }

    // Find the current collection and update according
    QString currentCollection = mData->findCurrentCollection(mComm->collectionList(), false);
    if (currentCollection.compare("") == 0) {
        mConnectionFloatingLayout->updateCollectionButton(":/images/plusIcon.png");
    } else {
        mConnectionFloatingLayout->updateCollectionButton(":/images/editIcon.png");
    }

    if ((!mShouldGreyOutIcons
         && (mData->currentDevices().size() == 0))) {
        deviceCountReachedZero();
    }
    updatePaletteButton();
}

void TopMenu::highlightButton(QString key) {
    mGroupFloatingLayout->highlightButton(key);
    mMoodsFloatingLayout->highlightButton(key);
    mConnectionFloatingLayout->highlightButton(key);
    mColorFloatingLayout->highlightButton(key);

}

void TopMenu::pageButtonPressed(EPage pageButtonType) {
    showFloatingLayout(pageButtonType);
    switch (pageButtonType) {
        case EPage::eColorPage:
            emit buttonPressed("Color");
            break;
        case EPage::eConnectionPage:
            emit buttonPressed("Connection");
            break;
        case EPage::eGroupPage:
            emit buttonPressed("Group");
            break;
        case EPage::eMoodsPage:
            emit buttonPressed("Moods");
            break;
        case EPage::eSettingsPage:
            emit buttonPressed("Settings");
            break;
    }

}

void TopMenu::resizeEvent(QResizeEvent *event) {
    Q_UNUSED(event);
    updatePaletteButton();
    moveFloatingLayout();
}

void TopMenu::updatePaletteButton() {
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
            mMainLayout->updateColorPageButton(":images/white_wheel.png");
        } else if (bestHueType == EHueType::eAmbient) {
            mMainLayout->updateColorPageButton(":images/ambient_wheel.png");
        } else if (bestHueType == EHueType::eExtended){
            mMainLayout->updateColorPageButton(":images/colorWheel_icon.png");
        } else {
            throw "did not find any hue lights when expecting hue lights";
        }
    } else {
        if (mData->currentDevices().size() == 0) {
            mMainLayout->updateColorPageButton(":images/white_wheel.png");
        } else {
            mMainLayout->updateColorPageButton(":images/colorWheel_icon.png");
        }
    }
}

//--------------------
// FloatingLayout Setup
//--------------------

void TopMenu::setup(MainWindow *mainWindow,
                    GroupPage *groupPage,
                    ColorPage *colorPage,
                    MoodsPage *moodsPage,
                    ConnectionPage *connectionPage) {
    mMainWindow = mainWindow;
    mGroupPage = groupPage;
    mColorPage = colorPage;
    mMoodsPage = moodsPage;
    mConnectionPage = connectionPage;
}

void TopMenu::floatingLayoutButtonPressed(QString button) {
    if (button.compare("Discovery") == 0) {
        mMainWindow->switchToDiscovery();
    } else if (button.compare("New_Group") == 0) {
        if (mMainWindow->currentPage() == EPage::eConnectionPage) {
            mMainWindow->editButtonClicked("", false);
        } else {
            mMainWindow->editButtonClicked("", true);
        }
    } else if (button.compare("Preset_Groups") == 0) {
        if (mData->hasArduinoDevices()) {
            mGroupPage->setMode(EGroupMode::eArduinoPresets);
        } else {
            mGroupPage->setMode(EGroupMode::eHuePresets);
        }
    } else if (button.compare("Select_Moods") == 0) {
        pageButtonPressed(EPage::eMoodsPage);
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
    } else if (button.compare("Connection_Page") == 0) {
        pageButtonPressed(EPage::eConnectionPage);
    } else if (button.compare("Colors_Page") == 0) {
        pageButtonPressed(EPage::eColorPage);
    } else if (button.compare("Multi_Colors_Page") == 0) {
        pageButtonPressed(EPage::eGroupPage);
    } else if (button.compare("Settings") == 0) {
        mCurrentPage = EPage::eSettingsPage;
        emit buttonPressed("Settings");
    }  else {
        qDebug() << "I don't recognize that button type..." << button;
    }
}

void TopMenu::showFloatingLayout(EPage newPage) {
    if (newPage != mCurrentPage) {
        // move old menu back
        switch(mCurrentPage) {
        case EPage::eColorPage:
            pushRightFloatingLayout(mColorFloatingLayout);
            mColorVerticalFloatingLayout->setVisible(false);
            break;
        case EPage::eConnectionPage:
            pushRightFloatingLayout(mConnectionFloatingLayout);
            break;
        case EPage::eMoodsPage:
            pushRightFloatingLayout(mMoodsFloatingLayout);
            break;
        case EPage::eGroupPage:
            pushRightFloatingLayout(mGroupFloatingLayout);
            break;
        case EPage::eSettingsPage:
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
            mConnectionFloatingLayout->updateGroupPageButtons(mData->colors());
            break;
        case EPage::eMoodsPage:
            pullLeftFloatingLayout(mMoodsFloatingLayout);
            break;
        case EPage::eGroupPage:
            pullLeftFloatingLayout(mGroupFloatingLayout);
            mGroupFloatingLayout->updateGroupPageButtons(mData->colors());
            break;
        case EPage::eSettingsPage:
            break;
        }

        mCurrentPage = newPage;
    }
}

FloatingLayout *TopMenu::currentFloatingLayout() {
    FloatingLayout *layout;
    switch (mCurrentPage) {
    case EPage::eConnectionPage:
        layout = mConnectionFloatingLayout;
        break;
    case EPage::eMoodsPage:
        layout = mMoodsFloatingLayout;
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
    QPoint topRight(mMainLayout->width(), mFloatingMenuStart);
    mMainLayout->move(topRight);

    topRight = QPoint(this->width(), mFloatingMenuStart);

    currentFloatingLayout()->move(topRight);
    if (mCurrentPage == EPage::eColorPage) {
        mColorVerticalFloatingLayout->move(QPoint(topRight.x(), topRight.y() + currentFloatingLayout()->geometry().height()));
    }
    moveHiddenLayouts();
}


void TopMenu::setupColorFloatingLayout() {

    bool hasHue = mData->hasHueDevices();
    bool hasArduino = mData->hasArduinoDevices();
    bool hasNanoLeaf = mData->hasNanoLeafDevices();
    std::vector<QString> horizontalButtons;
    std::vector<QString> verticalButtons;
    if (hasArduino || hasNanoLeaf){
        horizontalButtons = {QString("Temperature"), QString("RGB"), QString("Settings")};
        verticalButtons = {QString("Routine")};

        mColorFloatingLayout->highlightButton(mLastColorButtonKey);
        updateColorVerticalRoutineButton();
        mColorPage->changePageType(EColorPageType::eRGB, true);
    } else if (hasHue) {
        // get list of all current devices
        std::list<cor::Light> devices = mData->currentDevices();
        std::list<HueLight> hues;
        for (auto& device : devices) {
            hues.push_back(mComm->hue()->hueLightFromLight(device));
        }
        EHueType bestHueType = checkForHueWithMostFeatures(hues);
        // get a vector of all the possible hue types for a check.
        if (bestHueType == EHueType::eWhite) {
            horizontalButtons = {QString("Settings")};
            mColorPage->changePageType(EColorPageType::eBrightness, true);
        } else if (bestHueType == EHueType::eAmbient) {
            horizontalButtons = {QString("Settings")};
            mColorPage->changePageType(EColorPageType::eAmbient, true);
        } else if (bestHueType == EHueType::eExtended){
            horizontalButtons = {QString("Temperature"), QString("RGB"), QString("Settings")};
            mColorPage->changePageType(EColorPageType::eRGB, true);
        } else {
            throw "did not find any hue lights when expecting hue lights";
        }
    } else {
        // shouldn't get here...
        throw "trying to open single color page when no recognized devices are selected";
    }

    mColorVerticalFloatingLayout->setupButtons(verticalButtons);
    mColorFloatingLayout->setupButtons(horizontalButtons);
    mColorFloatingLayout->highlightButton("RGB");
    updatePaletteButton();

    mColorFloatingLayout->updateMultiPageButton(mData->colors());

    updateColorVerticalRoutineButton();
}

void TopMenu::updateColorVerticalRoutineButton() {
    if (mColorPage->pageType() == EColorPageType::eRGB) {
        mColorVerticalFloatingLayout->setVisible(true);
        mColorVerticalFloatingLayout->updateRoutine(mData->currentRoutineObject(), mData->paletteColors());
    } else if (mColorPage->pageType() == EColorPageType::eMulti) {
        mColorVerticalFloatingLayout->setVisible(true);
        mColorVerticalFloatingLayout->updateRoutine(mData->currentRoutineObject(), mData->palette(EPalette::eCustom));
    } else if (mColorPage->pageType() == EColorPageType::eAmbient
               || mColorPage->pageType() == EColorPageType::eBrightness
               || mColorPage->pageType() == EColorPageType::eColorScheme) {
        mColorVerticalFloatingLayout->setVisible(false);
    }
}

void TopMenu::pushRightFloatingLayout(FloatingLayout *layout) {
    QPoint topRight(this->width() - layout->width(),
                    mFloatingMenuStart);
    layout->setGeometry(topRight.x(),
                        topRight.y(),
                        layout->width(), layout->height());
    QPropertyAnimation *animation = new QPropertyAnimation(layout, "pos");
    animation->setDuration(TRANSITION_TIME_MSEC);
    animation->setStartValue(layout->pos());
    animation->setEndValue(QPoint(this->width() + layout->width(),
                                  mFloatingMenuStart));
    animation->start();

    if (layout == mColorFloatingLayout) {
        QPoint startPoint(this->width(),
                          mFloatingMenuStart + currentFloatingLayout()->geometry().height());
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
                    mFloatingMenuStart);
    layout->setGeometry(this->width() + layout->width(),
                        mFloatingMenuStart,
                        layout->width(), layout->height());
    QPropertyAnimation *animation = new QPropertyAnimation(layout, "pos");
    animation->setDuration(TRANSITION_TIME_MSEC);
    animation->setStartValue(layout->pos());
    animation->setEndValue(topRight);
    animation->start();

    if (layout == mColorFloatingLayout) {
        QPoint startPoint(this->width(),
                          mFloatingMenuStart + currentFloatingLayout()->geometry().height());
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
                        mFloatingMenuStart);
        mColorFloatingLayout->setGeometry(topRight.x(),
                                          topRight.y(),
                                          mColorFloatingLayout->width(),
                                          mColorFloatingLayout->height());
        mColorVerticalFloatingLayout->setGeometry(topRight.x(),
                                                  topRight.y() + mColorFloatingLayout->height(),
                                                  mColorFloatingLayout->width(),
                                                  mColorFloatingLayout->height());
    }
    if (layout != mMoodsFloatingLayout) {
        QPoint topRight(this->width(),
                        mFloatingMenuStart);
        mMoodsFloatingLayout->setGeometry(topRight.x(),
                                          topRight.y(),
                                          mMoodsFloatingLayout->width(),
                                          mMoodsFloatingLayout->height());
    }
    if (layout != mGroupFloatingLayout) {
        QPoint topRight(this->width(),
                        mFloatingMenuStart);
        mGroupFloatingLayout->setGeometry(topRight.x(),
                                          topRight.y(),
                                          mGroupFloatingLayout->width(),
                                          mGroupFloatingLayout->height());
    }
    if (layout != mConnectionFloatingLayout) {
        QPoint topRight(this->width(),
                        mFloatingMenuStart);
        mConnectionFloatingLayout->setGeometry(topRight.x(),
                                               topRight.y(),
                                               mConnectionFloatingLayout->width(),
                                               mConnectionFloatingLayout->height());

    }
}

void TopMenu::receivedPacket(EProtocolType) {
    if (mData->currentDevices() != mLastDevices) {
        std::list<cor::Light> currentDevices = mData->currentDevices();
        for (auto device : currentDevices) {
            mComm->fillDevice(device);
        }
        mLastDevices = currentDevices;
        mMainPalette->updateDevices(currentDevices);
    }
}
