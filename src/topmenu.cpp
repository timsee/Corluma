/*!
 * \copyright
 * Copyright (C) 2015 - 2019.
 * Released under the GNU General Public License.
 */

#include <QGraphicsEffect>
#include "mainwindow.h"
#include "topmenu.h"
#include "utils/qt.h"
#include "cor/exception.h"
#include "hue/huelight.h"


QString pageToString(EPage page) {
    switch (page) {
        case EPage::colorPage:
            return "Color";
        case EPage::lightPage:
            return "Lights";
        case EPage::moodPage:
            return "Moods";
        case EPage::palettePage:
            return "Palette";
        case EPage::discoveryPage:
            return "Discovery";
        case EPage::settingsPage:
            return "Settings";
    }
}

EPage stringToPage(QString string) {
    if (string == "Color") {
        return EPage::colorPage;
    } else if (string == "Lights") {
        return EPage::lightPage;
    } else if (string == "Moods") {
        return EPage::moodPage;
    } else if (string == "Palette") {
        return EPage::palettePage;
    } else if (string == "Settings") {
        return EPage::settingsPage;
    } else if (string == "Discovery") {
        return EPage::discoveryPage;
    }
    THROW_EXCEPTION("String not recognized as a page");
    return {};
}

TopMenu::TopMenu(QWidget* parent,
                 cor::DeviceList *data,
                 CommLayer *comm,
                 GroupData *groups,
                 MainWindow *mainWindow,
                 PalettePage *palettePage,
                 ColorPage *colorPage) : QWidget(parent),
                                    mData(data),
                                    mComm(comm),
                                    mGroups(groups),
                                    mMainWindow(mainWindow),
                                    mPalettePage(palettePage),
                                    mColorPage(colorPage) {


    QSize size = cor::applicationSize();
    mSize = QSize(int(size.height() * 0.1f), int(size.height() * 0.1f));

    mRenderTimer = new QTimer(this);
    connect(mRenderTimer, SIGNAL(timeout()), this, SLOT(updateUI()));
    mRenderTimer->start(100);

    //---------------
    // Create Spacer
    //---------------

    mSpacer = new QWidget(this);
    mSpacer->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    mSpacer->setFixedHeight(mSize.height());

    // --------------
    // Setup menu button
    // --------------
    mMenuButton = new QPushButton(this);
    mMenuButton->setFixedHeight(mSize.height() * 0.8f);
    cor::resizeIcon(mMenuButton, ":/images/hamburger_icon.png", 0.8f);
    connect(mMenuButton, SIGNAL(clicked(bool)), this, SLOT(menuButtonPressed()));

    // --------------
    // Setup Brightness Slider
    // --------------
    // setup the slider that controls the LED's brightness
    mBrightnessSlider = new cor::Slider(this);
    mBrightnessSlider->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    mBrightnessSlider->slider()->setRange(2,100);
    mBrightnessSlider->slider()->setValue(2);
    mBrightnessSlider->setFixedHeight(mSize.height() / 2);
    mBrightnessSlider->setSliderHeight(0.8f);
    mBrightnessSlider->setSliderColorBackground(QColor(255,255,255));
    connect(mBrightnessSlider, SIGNAL(valueChanged(int)), this, SLOT(brightnessSliderChanged(int)));

    // --------------
    // Setup Main Palette
    // -------------
    mMainPalette = new cor::LightVectorWidget(12, 1, false, this);
    mMainPalette->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    mMainPalette->setFixedHeight(mSize.height() / 2);

    // --------------
    // Setup on/off switch
    // --------------
    mOnOffSwitch = new cor::Switch(this);
    mOnOffSwitch->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    mOnOffSwitch->setFixedSize(QSize(mSize.width(), mSize.height() / 2));
    connect(mOnOffSwitch, SIGNAL(switchChanged(bool)), this, SLOT(changedSwitchState(bool)));
    mOnOffSwitch->setSwitchState(ESwitchState::disabled);

    std::vector<QString> buttons;
    // --------------
    // Connection Floating Layout
    // --------------
    mLightsFloatingLayout = new FloatingLayout(false, mMainWindow);
    connect(mLightsFloatingLayout, SIGNAL(buttonPressed(QString)), this, SLOT(floatingLayoutButtonPressed(QString)));
    buttons = { QString("New_Group")};
    mLightsFloatingLayout->setupButtons(buttons, EButtonSize::small);
    mLightsFloatingLayout->setVisible(false);

    // --------------
    // Group Floating Layout
    // --------------
    mPaletteFloatinglayout = new FloatingLayout(false, mMainWindow);
    connect(mPaletteFloatinglayout, SIGNAL(buttonPressed(QString)), this, SLOT(floatingLayoutButtonPressed(QString)));
    buttons = {};
    mPaletteFloatinglayout->setupButtons(buttons, EButtonSize::small);
    mPaletteFloatinglayout->setVisible(false);

    // --------------
    // Select Lights Button
    // --------------
    mSelectLightsButton = new SelectLightsButton(this);
    mSelectLightsButton->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    connect(mSelectLightsButton, SIGNAL(pressed()), this, SLOT(menuButtonPressed()));
    mSelectLightsButton->setFixedSize(mSize.width() * 3, mSize.height() * 0.5);

    // --------------
    // Moods Floating Layout
    // --------------
    mMoodsFloatingLayout = new FloatingLayout(false, mMainWindow);
    connect(mMoodsFloatingLayout, SIGNAL(buttonPressed(QString)), this, SLOT(floatingLayoutButtonPressed(QString)));
    buttons = {QString("New_Group")};
    mMoodsFloatingLayout->setupButtons(buttons, EButtonSize::small);
    mMoodsFloatingLayout->setVisible(false);

    // --------------
    // Color Page Floating Layouts
    // --------------
    //NOTE: by making the parent the parent of the floating widgets, they don't get cut off if they overextend over top menu
    mColorFloatingLayout = new FloatingLayout(false, mMainWindow);
    connect(mColorFloatingLayout, SIGNAL(buttonPressed(QString)), this, SLOT(floatingLayoutButtonPressed(QString)));
    mColorFloatingLayout->setVisible(false);

    mLastColorButtonKey = "RGB";

    deviceCountChanged();

    mCurrentPage = EPage::colorPage;
    showFloatingLayout(mCurrentPage);

    resize(0);
    mSelectLightsButton->pushIn(mStartSelectLightsButton, this->size());
}


void TopMenu::changedSwitchState(bool switchState) {
    mData->turnOn(switchState);
    emit buttonPressed("OnOff");
}


void TopMenu::brightnessSliderChanged(int newBrightness) {
    brightnessUpdate(uint32_t(newBrightness));
}

void TopMenu::brightnessUpdate(uint32_t newValue) {
    mData->updateBrightness(newValue);
    mData->turnOn(true);
    // update the top menu bar
    updateBrightnessSlider();

    if (newValue != 0
            && mOnOffSwitch->switchState() != ESwitchState::on) {
        mOnOffSwitch->setSwitchState(ESwitchState::on);
    }
    emit brightnessChanged(newValue);
}

void TopMenu::updateBrightnessSlider() {
    mBrightnessSlider->setSliderColorBackground(mData->mainColor());

    if (mData->brightness() != mBrightnessSlider->slider()->value()) {
        mBrightnessSlider->blockSignals(true);
        mBrightnessSlider->slider()->setValue(mData->brightness());
        mBrightnessSlider->blockSignals(false);
    }
}

void TopMenu::deviceCountChanged() {
    if (mData->devices().empty()) {
        mSelectLightsButton->pushIn(mStartSelectLightsButton, this->size());
    } else {
        mSelectLightsButton->pushOut(mStartSelectLightsButton, this->size());
    }

    if (mShouldGreyOutIcons
            && (mData->devices().size() > 0)) {
        mBrightnessSlider->enable(true);

        if (mData->isOn()) {
            mOnOffSwitch->setSwitchState(ESwitchState::on);
        } else {
            mOnOffSwitch->setSwitchState(ESwitchState::off);
        }

        updateBrightnessSlider();
        mShouldGreyOutIcons = false;
    }

    if ((!mShouldGreyOutIcons
         && (mData->devices().size() == 0))) {
        mBrightnessSlider->enable(false);
        mBrightnessSlider->slider()->setValue(0);
        mOnOffSwitch->setSwitchState(ESwitchState::disabled);

        mShouldGreyOutIcons = true;
    }

    if (mCurrentPage == EPage::colorPage) {
        setupColorFloatingLayout();
    }
}

void TopMenu::highlightButton(QString key) {
    mPaletteFloatinglayout->highlightButton(key);
    mMoodsFloatingLayout->highlightButton(key);
    mLightsFloatingLayout->highlightButton(key);
    mColorFloatingLayout->highlightButton(key);
}

void TopMenu::showMenu() {
    this->setVisible(true);
    mColorFloatingLayout->setVisible(true);
    mLightsFloatingLayout->setVisible(true);
    mPaletteFloatinglayout->setVisible(true);
    mMoodsFloatingLayout->setVisible(true);    

    this->raise();
    mLightsFloatingLayout->raise();
    mColorFloatingLayout->raise();
    mPaletteFloatinglayout->raise();
    mMoodsFloatingLayout->raise();
}

void TopMenu::resize(int xOffset) {
    auto parentSize = qobject_cast<QWidget*>(this->parent())->size();
    this->setGeometry(xOffset, 0,
                      parentSize.width() - xOffset,
                      fixedHeight());

    moveFloatingLayout();

    int yPos = 0;
    int padding = 5;
    mMenuButton->setGeometry(0,
                             yPos,
                             mSize.width(),
                             mSize.height());
    mSpacer->setGeometry(mSize.width(),
                         yPos,
                         this->width() - mSize.width(),
                         mSize.height());

    mOnOffSwitch->setGeometry(mSize.width() * 1.1,
                              yPos,
                              mSize.width(),
                              mSize.height() / 2);

    mBrightnessSlider->setGeometry(mSize.width() * 2 + 5,
                              yPos,
                              this->width() - (mSize.width() * 2.1),
                              mSize.height() / 2);

    yPos += mMenuButton->height() + padding;

    mStartSelectLightsButton = yPos;

    mMainPalette->setGeometry(this->width() * 0.2f,
                              yPos,
                              this->width() * 0.8f,
                              mSize.height() / 2);
    yPos += mMainPalette->height() + padding;
    mFloatingMenuStart = yPos + 3;

    yPos += mBrightnessSlider->height() + 20;

    mFixedHeight = yPos;

    mSelectLightsButton->resize(mStartSelectLightsButton, this->size());
}


void TopMenu::resizeEvent(QResizeEvent *) {
    resize(this->geometry().x());
}

//--------------------
// FloatingLayout Setup
//--------------------

void TopMenu::floatingLayoutButtonPressed(QString button) {
   if (button.compare("Discovery") == 0) {
        mMainWindow->pushInDiscovery();
    } else if (button.compare("New_Group") == 0) {
        mMainWindow->editButtonClicked(mMainWindow->currentPage() == EPage::lightPage);
    } else if (button.compare("Preset_Groups") == 0) {
        if (mData->hasLightWithProtocol(EProtocolType::arduCor)) {
            mPalettePage->setMode(EGroupMode::arduinoPresets);
        } else {
            mPalettePage->setMode(EGroupMode::huePresets);
        }
    } else if (button.compare("Multi") == 0) {
        mLastColorButtonKey = button;
        mColorPage->changePageType(EColorPageType::multi);
    } else if (button.compare("RGB") == 0) {
        mLastColorButtonKey = button;
        mColorPage->changePageType(EColorPageType::RGB);
    } else if (button.compare("Temperature") == 0) {
        mLastColorButtonKey = button;
        mColorPage->changePageType(EColorPageType::ambient);
    } else if (button.compare("ColorScheme") == 0) {
        mLastColorButtonKey = button;
        mColorPage->changePageType(EColorPageType::colorScheme);
    } else if (button.compare("Routine") == 0) {
        mLastColorButtonKey = button;
        mColorPage->handleRoutineWidget();
    } else if (button.compare("Settings") == 0) {
        emit buttonPressed("Settings");
    }  else {
        qDebug() << "I don't recognize that button type..." << button;
    }
}

void TopMenu::showFloatingLayout(EPage newPage) {
    if (newPage != mCurrentPage) {
        // move old menu back
        switch(mCurrentPage) {
        case EPage::colorPage:
            pushRightFloatingLayout(mColorFloatingLayout);
            break;
        case EPage::lightPage:
            pushRightFloatingLayout(mLightsFloatingLayout);
            break;
        case EPage::moodPage:
            pushRightFloatingLayout(mMoodsFloatingLayout);
            break;
        case EPage::palettePage:
            pushRightFloatingLayout(mPaletteFloatinglayout);
            break;
        }

        // move new menu forward
        switch(newPage) {
        case EPage::colorPage:
            pullLeftFloatingLayout(mColorFloatingLayout);
            break;
        case EPage::lightPage:
            pullLeftFloatingLayout(mLightsFloatingLayout);
            break;
        case EPage::moodPage:
            pullLeftFloatingLayout(mMoodsFloatingLayout);
            break;
        case EPage::palettePage:
            pullLeftFloatingLayout(mPaletteFloatinglayout);
            break;
        }

        mCurrentPage = newPage;
    }
}

FloatingLayout *TopMenu::currentFloatingLayout() {
    FloatingLayout *layout;
    switch (mCurrentPage) {
    case EPage::lightPage:
        layout = mLightsFloatingLayout;
        break;
    case EPage::moodPage:
        layout = mMoodsFloatingLayout;
        break;
    case EPage::palettePage:
        layout = mPaletteFloatinglayout;
        break;
    case EPage::colorPage:
        layout = mColorFloatingLayout;
        break;
    }
    return layout;
}

void TopMenu::moveFloatingLayout() {
    auto parentSize = qobject_cast<QWidget*>(this->parent())->size();
    QPoint topRight(parentSize.width(), mFloatingMenuStart);

    currentFloatingLayout()->move(topRight);
    moveHiddenLayouts();
}


void TopMenu::setupColorFloatingLayout() {
    bool hasHue      = mData->hasLightWithProtocol(EProtocolType::hue);
    bool hasArduino  = mData->hasLightWithProtocol(EProtocolType::arduCor);
    bool hasNanoLeaf = mData->hasLightWithProtocol(EProtocolType::nanoleaf);

    EColorMenuType type;
    EHueType bestHueType = EHueType::MAX;
    if (hasArduino || hasNanoLeaf) {
        type = EColorMenuType::arduinoMenu;
    } else if (hasHue) {
        // get list of all current devices
        std::list<cor::Light> devices = mData->devices();
        std::list<HueLight> hues;
        for (auto& device : devices) {
            hues.push_back(mComm->hue()->hueLightFromLight(device));
        }
        bestHueType = checkForHueWithMostFeatures(hues);
        // get a vector of all the possible hue types for a check.
        if (bestHueType == EHueType::white) {
            type = EColorMenuType::none;
        } else if (bestHueType == EHueType::ambient) {
            type = EColorMenuType::none;
        } else if (bestHueType == EHueType::extended){
            type = EColorMenuType::hueMenu;
        } else {
            THROW_EXCEPTION("did not find any hue lights when expecting hue lights");
        }
    } else {
        type = EColorMenuType::none;
    }

    std::vector<QString> horizontalButtons;
    if (type != mColorMenuType) {
        if (type == EColorMenuType::arduinoMenu) {
            horizontalButtons = {QString("Multi"), QString("Routine"), QString("Temperature"), QString("RGB")};
            mColorFloatingLayout->highlightButton(mLastColorButtonKey);
            mColorPage->changePageType(EColorPageType::RGB, true);
        } else if (type == EColorMenuType::hueMenu) {
            // get a vector of all the possible hue types for a check.
            if (bestHueType == EHueType::white) {
                horizontalButtons = {};
                mColorPage->changePageType(EColorPageType::brightness, true);
            } else if (bestHueType == EHueType::ambient) {
                horizontalButtons = {};
                mColorPage->changePageType(EColorPageType::ambient, true);
            } else if (bestHueType == EHueType::extended){
                horizontalButtons = {QString("Temperature"), QString("RGB")};
                mColorPage->changePageType(EColorPageType::RGB, true);
            } else {
                THROW_EXCEPTION("did not find any hue lights when expecting hue lights");
            }
        } else {
            horizontalButtons = {};
            mColorPage->changePageType(EColorPageType::RGB, true);
        }
        mColorMenuType = type;

        mColorFloatingLayout->setupButtons(horizontalButtons, EButtonSize::small);
        mColorFloatingLayout->highlightButton("RGB");

        if (mColorFloatingLayout->geometry().width() != (this->width() - mColorFloatingLayout->geometry().x())) {
            pullLeftFloatingLayout(mColorFloatingLayout);
        }
    }
}

void TopMenu::pushRightFloatingLayout(FloatingLayout *layout) {
    auto parentSize = qobject_cast<QWidget*>(this->parent())->size();
    cor::moveWidget(layout,
                    layout->size(),
                    QPoint(parentSize.width() - layout->width(), mFloatingMenuStart),
                    QPoint(parentSize.width() + layout->width(), mFloatingMenuStart));
}

void TopMenu::pullLeftFloatingLayout(FloatingLayout *layout) {
    auto parentSize = qobject_cast<QWidget*>(this->parent())->size();
    cor::moveWidget(layout,
                    layout->size(),
                    QPoint(parentSize.width() + layout->width(), mFloatingMenuStart),
                    QPoint(parentSize.width() - layout->width(), mFloatingMenuStart));
}

void TopMenu::moveHiddenLayouts() {
    auto parentSize = qobject_cast<QWidget*>(this->parent())->size();
    QPoint topRight(parentSize.width(),
                    mFloatingMenuStart);
    FloatingLayout *layout = currentFloatingLayout();
    if (layout != mColorFloatingLayout) {
        mColorFloatingLayout->setGeometry(topRight.x(),
                                          topRight.y(),
                                          mColorFloatingLayout->width(),
                                          mColorFloatingLayout->height());
    }
    if (layout != mMoodsFloatingLayout) {
        mMoodsFloatingLayout->setGeometry(topRight.x(),
                                          topRight.y(),
                                          mMoodsFloatingLayout->width(),
                                          mMoodsFloatingLayout->height());
    }
    if (layout != mPaletteFloatinglayout) {
        mPaletteFloatinglayout->setGeometry(topRight.x(),
                                            topRight.y(),
                                            mPaletteFloatinglayout->width(),
                                            mPaletteFloatinglayout->height());
    }
    if (layout != mLightsFloatingLayout) {
        mLightsFloatingLayout->setGeometry(topRight.x(),
                                           topRight.y(),
                                           mLightsFloatingLayout->width(),
                                           mLightsFloatingLayout->height());

    }
}

void TopMenu::updateUI() {
    if (mData->devices() != mLastDevices) {
        // get copy of data representation of lights
        auto currentDevices = mData->devices();
        // update devices to be comm representation instead of data representaiton
        for (auto&& device : currentDevices) {
            device = mComm->lightByID(device.uniqueID());
        }
        mLastDevices = currentDevices;

        mMainPalette->updateDevices(currentDevices);
        updateBrightnessSlider();
    }
}


void TopMenu::hideMenuButton(bool shouldHide) {
    mMenuButton->setVisible(!shouldHide);
    mSelectLightsButton->setVisible(!shouldHide);
}
