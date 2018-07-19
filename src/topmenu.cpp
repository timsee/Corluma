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

TopMenu::TopMenu(QWidget* parent,
                 DeviceList* data,
                 CommLayer *comm,
                 MainWindow *mainWindow,
                 PalettePage *palettePage,
                 ColorPage *colorPage,
                 MoodPage *moodPage,
                 LightPage *lightPage) : QWidget(parent),
                                    mData(data),
                                    mComm(comm),
                                    mMainWindow(mainWindow),
                                    mPalettePage(palettePage),
                                    mColorPage(colorPage),
                                    mMoodPage(moodPage),
                                    mLightPage(lightPage) {


    QSize size = cor::applicationSize();
    mSize = QSize(size.height() * 0.1f, size.height() * 0.1f);

    mRenderTimer = new QTimer(this);
    connect(mRenderTimer, SIGNAL(timeout()), this, SLOT(updateUI()));
    mRenderTimer->start(100);

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
    mBrightnessSlider->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    mBrightnessSlider->slider()->setRange(0,100);
    mBrightnessSlider->slider()->setValue(0);
    mBrightnessSlider->setFixedHeight(mSize.height() / 2);
    mBrightnessSlider->setSliderHeight(0.8f);
    mBrightnessSlider->setSliderColorBackground(QColor(255,255,255));
    connect(mBrightnessSlider, SIGNAL(valueChanged(int)), this, SLOT(brightnessSliderChanged(int)));

    // --------------
    // Setup Main Palette
    // -------------
    mMainPalette = new cor::LightVectorWidget(10, 1, cor::EPaletteWidgetType::standard, this);
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
    downsizeTextHeightToFit(mMainPalette->size().height());

    // --------------
    // Setup on/off switch
    // --------------
    mOnOffSwitch = new cor::Switch(this);
    mOnOffSwitch->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    mOnOffSwitch->setFixedSize(QSize(mSize.width(), mSize.height() / 2));
    connect(mOnOffSwitch, SIGNAL(switchChanged(bool)), this, SLOT(changedSwitchState(bool)));
    mOnOffSwitch->setSwitchState(ESwitchState::disabled);

    mLayout = new QGridLayout(this);
    mLayout->setSpacing(5);
    mLayout->setContentsMargins(0,
                                mLayout->contentsMargins().top(),
                                mLayout->contentsMargins().right(),
                                mLayout->contentsMargins().bottom());

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
    mMainLayout = new FloatingLayout(false, mMainWindow);
    connect(mMainLayout, SIGNAL(buttonPressed(QString)), this, SLOT(floatingLayoutButtonPressed(QString)));
    std::vector<QString> buttons = { QString("Lights_Page"), QString("Colors_Page"), QString("Palette_Page"), QString("Mood_Page")};
    mMainLayout->setupButtons(buttons, EButtonSize::medium);
    mFloatingMenuEnd = mFloatingMenuStart + mMainLayout->size().height();

    // --------------
    // Connection Floating Layout
    // --------------
    mLightsFloatingLayout = new FloatingLayout(false, mMainWindow);
    connect(mLightsFloatingLayout, SIGNAL(buttonPressed(QString)), this, SLOT(floatingLayoutButtonPressed(QString)));
    buttons = { QString("New_Group"), QString("Settings")};
    mLightsFloatingLayout->setupButtons(buttons, EButtonSize::small);

    // --------------
    // Group Floating Layout
    // --------------
    mPaletteFloatinglayout = new FloatingLayout(false, mMainWindow);
    connect(mPaletteFloatinglayout, SIGNAL(buttonPressed(QString)), this, SLOT(floatingLayoutButtonPressed(QString)));
    buttons = {QString("Settings")};
    mPaletteFloatinglayout->setupButtons(buttons, EButtonSize::small);

    // --------------
    // Moods Floating Layout
    // --------------
    mMoodsFloatingLayout = new FloatingLayout(false, mMainWindow);
    connect(mMoodsFloatingLayout, SIGNAL(buttonPressed(QString)), this, SLOT(floatingLayoutButtonPressed(QString)));
    buttons = {QString("New_Group"), QString("Settings")};
    mMoodsFloatingLayout->setupButtons(buttons, EButtonSize::small);

    // --------------
    // Color Page Floating Layouts
    // --------------
    //NOTE: by making the parent the parent of the floating widgets, they don't get cut off if they overextend over top menu
    mColorFloatingLayout = new FloatingLayout(false, mMainWindow);
    connect(mColorFloatingLayout, SIGNAL(buttonPressed(QString)), this, SLOT(floatingLayoutButtonPressed(QString)));

    mColorVerticalFloatingLayout = new FloatingLayout(false, mMainWindow);
    connect(mColorVerticalFloatingLayout, SIGNAL(buttonPressed(QString)), this, SLOT(floatingLayoutButtonPressed(QString)));
    mColorVerticalFloatingLayout->setVisible(false);

    mLastColorButtonKey = "RGB";

    deviceCountReachedZero();

    mCurrentPage = EPage::palettePage;
    pageButtonPressed(EPage::lightPage);
    mMainLayout->highlightButton("Lights_Page");
}

TopMenu::~TopMenu() {

}

void TopMenu::changedSwitchState(bool switchState) {
    mData->turnOn(switchState);
    emit buttonPressed("OnOff");
}


void TopMenu::brightnessSliderChanged(int newBrightness) {
    brightnessUpdate(newBrightness);
}

void TopMenu::brightnessUpdate(int newValue) {
    mData->updateBrightness(newValue);
    mData->turnOn(true);
    // update the top menu bar
    updateBrightnessSlider();

    emit brightnessChanged(newValue);
}

void TopMenu::updateMenuBar() {
    //-----------------
    // On/Off Data
    //-----------------
    if (mData->devices().size() == 0) {
        mOnOffSwitch->setSwitchState(ESwitchState::disabled);
    } else if (mData->isOn()) {
        mOnOffSwitch->setSwitchState(ESwitchState::on);
    } else {
        mOnOffSwitch->setSwitchState(ESwitchState::off);
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

void TopMenu::deviceCountReachedZero() {
    mBrightnessSlider->enable(false);
    mMainLayout->enableButton("Colors_Page", false);
    mMainLayout->enableButton("Palette_Page", false);

    mBrightnessSlider->slider()->setValue(0);
    mOnOffSwitch->setSwitchState(ESwitchState::disabled);

    mShouldGreyOutIcons = true;
}

void TopMenu::deviceCountChanged() {
    if (mShouldGreyOutIcons
            && (mData->devices().size() > 0)) {
        mBrightnessSlider->enable(true);
        mMainLayout->enableButton("Colors_Page", true);
        mMainLayout->enableButton("Palette_Page", true);

        if (mData->isOn()) {
            mOnOffSwitch->setSwitchState(ESwitchState::on);
        } else {
            mOnOffSwitch->setSwitchState(ESwitchState::off);
        }

        mShouldGreyOutIcons = false;
    }

    if (mData->devices().size() > 0) {
        QString devicesText = mData->findCurrentCollection(mComm->collectionList(), true);
        mSelectedDevicesLabel->setText(devicesText);
    } else {
        mSelectedDevicesLabel->setText("");
    }

    // Find the current collection and update according
    QString currentCollection = mData->findCurrentCollection(mComm->collectionList(), false);
    if (currentCollection.compare("") == 0) {
        mLightsFloatingLayout->updateCollectionButton(":/images/plusIcon.png");
    } else {
        mLightsFloatingLayout->updateCollectionButton(":/images/editIcon.png");
    }

    if ((!mShouldGreyOutIcons
         && (mData->devices().size() == 0))) {
        deviceCountReachedZero();
    }
    updatePaletteButton();
}

void TopMenu::highlightButton(QString key) {
    mPaletteFloatinglayout->highlightButton(key);
    mMoodsFloatingLayout->highlightButton(key);
    mLightsFloatingLayout->highlightButton(key);
    mColorFloatingLayout->highlightButton(key);
   // mColorVerticalFloatingLayout->highlightButton(key);
}

void TopMenu::pageButtonPressed(EPage pageButtonType) {
    showFloatingLayout(pageButtonType);
    switch (pageButtonType) {
        case EPage::colorPage:
            emit buttonPressed("Color");
            break;
        case EPage::lightPage:
            emit buttonPressed("Lights");
            break;
        case EPage::palettePage:
            emit buttonPressed("Group");
            break;
        case EPage::moodPage:
            emit buttonPressed("Moods");
            break;
        case EPage::settingsPage:
            emit buttonPressed("Settings");
            break;
    }

}

void TopMenu::resizeEvent(QResizeEvent *event) {
    Q_UNUSED(event);
    downsizeTextHeightToFit(mMainPalette->size().height());
    updatePaletteButton();
    moveFloatingLayout();
    int widthSize = this->width() * 0.95f - mSize.width();
    mBrightnessSlider->setFixedWidth(widthSize);
}

void TopMenu::updatePaletteButton() {
    bool hasHue = mData->hasLightWithProtocol(EProtocolType::hue);
    bool hasArduino = mData->hasLightWithProtocol(EProtocolType::arduCor);
    if (hasHue && !hasArduino) {
        std::list<cor::Light> devices = mData->devices();
        std::list<HueLight> hues;
        for (auto& device : devices) {
            hues.push_back(mComm->hue()->hueLightFromLight(device));
            qDebug() << " hue" << mComm->hue()->hueLightFromLight(device);
        }

        EHueType bestHueType = checkForHueWithMostFeatures(hues);
        // get a vector of all the possible hue types for a check.
        if (bestHueType == EHueType::white) {
            mMainLayout->updateColorPageButton(":images/white_wheel.png");
        } else if (bestHueType == EHueType::ambient) {
            mMainLayout->updateColorPageButton(":images/ambient_wheel.png");
        } else if (bestHueType == EHueType::extended){
            mMainLayout->updateColorPageButton(":images/colorWheel_icon.png");
        } else {
            throw "did not find any hue lights when expecting hue lights";
        }
    } else {
        if (mData->devices().size() == 0) {
            mMainLayout->updateColorPageButton(":images/white_wheel.png");
        } else {
            mMainLayout->updateColorPageButton(":images/colorWheel_icon.png");
        }
    }
    updateBrightnessSlider();
}

//--------------------
// FloatingLayout Setup
//--------------------

void TopMenu::floatingLayoutButtonPressed(QString button) {
    if (button.compare("Discovery") == 0) {
        mMainWindow->switchToDiscovery();
    } else if (button.compare("New_Group") == 0) {
        if (mMainWindow->currentPage() == EPage::lightPage) {
            mMainWindow->editButtonClicked(mLightPage->currentGroup(), false);
        } else {
            mMainWindow->editButtonClicked(mMoodPage->currentMood(), true);
        }
    } else if (button.compare("Preset_Groups") == 0) {
        if (mData->hasLightWithProtocol(EProtocolType::arduCor)) {
            mPalettePage->setMode(EGroupMode::arduinoPresets);
        } else {
            mPalettePage->setMode(EGroupMode::huePresets);
        }
    } else if (button.compare("Mood_Page") == 0) {
        pageButtonPressed(EPage::moodPage);
    } else if (button.compare("Multi") == 0) {
        mLastColorButtonKey = button;
        mColorPage->changePageType(EColorPageType::multi);
        updateColorVerticalRoutineButton();
    } else if (button.compare("RGB") == 0) {
        mLastColorButtonKey = button;
        mColorPage->changePageType(EColorPageType::RGB);
        updateColorVerticalRoutineButton();
        mColorVerticalFloatingLayout->highlightButton("");
    } else if (button.compare("Temperature") == 0) {
        mLastColorButtonKey = button;
        mColorPage->changePageType(EColorPageType::ambient);
        updateColorVerticalRoutineButton();
    } else if (button.compare("ColorScheme") == 0) {
        mLastColorButtonKey = button;
        mColorPage->changePageType(EColorPageType::colorScheme);
        updateColorVerticalRoutineButton();
    } else if (button.compare("Routine") == 0) {
        mLastColorButtonKey = button;
        mColorPage->handleRoutineWidget();
        updateColorVerticalRoutineButton();
    } else if (button.compare("Lights_Page") == 0) {
        pageButtonPressed(EPage::lightPage);
    } else if (button.compare("Colors_Page") == 0) {
        pageButtonPressed(EPage::colorPage);
    } else if (button.compare("Palette_Page") == 0) {
        pageButtonPressed(EPage::palettePage);
    } else if (button.compare("Settings") == 0) {
        mCurrentPage = EPage::settingsPage;
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
            mColorVerticalFloatingLayout->setVisible(false);
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
        case EPage::settingsPage:
            break;
        }

        // move new menu forward
        switch(newPage) {
        case EPage::colorPage:
            setupColorFloatingLayout();
            mColorVerticalFloatingLayout->setVisible(true);
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
        case EPage::settingsPage:
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
    case EPage::palettePage:
        layout = mPaletteFloatinglayout;
        break;
    case EPage::colorPage:
        layout = mColorFloatingLayout;
        break;
    default:
        layout = mLightsFloatingLayout;
        break;
    }
    return layout;
}

void TopMenu::moveFloatingLayout() {
    QPoint topRight(mMainLayout->width(), mFloatingMenuStart);
    mMainLayout->move(topRight);

    topRight = QPoint(this->width(), mFloatingMenuStart);

    currentFloatingLayout()->move(topRight);
    if (mCurrentPage == EPage::colorPage) {
        mColorVerticalFloatingLayout->move(QPoint(topRight.x(), topRight.y() + currentFloatingLayout()->geometry().height()));
    }
    moveHiddenLayouts();
}


void TopMenu::setupColorFloatingLayout() {

    bool hasHue      = mData->hasLightWithProtocol(EProtocolType::hue);
    bool hasArduino  = mData->hasLightWithProtocol(EProtocolType::arduCor);
    bool hasNanoLeaf = mData->hasLightWithProtocol(EProtocolType::nanoleaf);
    std::vector<QString> horizontalButtons;
    std::vector<QString> verticalButtons;
    if (hasArduino || hasNanoLeaf){
        horizontalButtons = {QString("Temperature"), QString("RGB"), QString("Settings")};
        verticalButtons = {QString("Multi"), QString("Routine")};

        mColorFloatingLayout->highlightButton(mLastColorButtonKey);
        updateColorVerticalRoutineButton();
        mColorPage->changePageType(EColorPageType::RGB, true);
    } else if (hasHue) {
        // get list of all current devices
        std::list<cor::Light> devices = mData->devices();
        std::list<HueLight> hues;
        for (auto& device : devices) {
            hues.push_back(mComm->hue()->hueLightFromLight(device));
        }
        EHueType bestHueType = checkForHueWithMostFeatures(hues);
        // get a vector of all the possible hue types for a check.
        if (bestHueType == EHueType::white) {
            horizontalButtons = {QString("Settings")};
            mColorPage->changePageType(EColorPageType::brightness, true);
        } else if (bestHueType == EHueType::ambient) {
            horizontalButtons = {QString("Settings")};
            mColorPage->changePageType(EColorPageType::ambient, true);
        } else if (bestHueType == EHueType::extended){
            horizontalButtons = {QString("Temperature"), QString("RGB"), QString("Settings")};
            mColorPage->changePageType(EColorPageType::RGB, true);
        } else {
            throw "did not find any hue lights when expecting hue lights";
        }
    } else {
        // shouldn't get here...
        throw "trying to open single color page when no recognized devices are selected";
    }

    mColorVerticalFloatingLayout->setupButtons(verticalButtons, EButtonSize::small);
    mColorFloatingLayout->setupButtons(horizontalButtons, EButtonSize::small);
    mColorFloatingLayout->highlightButton("RGB");
    updatePaletteButton();

    updateColorVerticalRoutineButton();
}

void TopMenu::updateColorVerticalRoutineButton() {
    if (mColorPage->pageType() == EColorPageType::RGB) {
        mColorVerticalFloatingLayout->setVisible(true);
       // mColorVerticalFloatingLayout->updateRoutine(mData->currentRoutineObject());
    } else if (mColorPage->pageType() == EColorPageType::multi) {
        mColorVerticalFloatingLayout->setVisible(true);
      //  mColorVerticalFloatingLayout->updateRoutine(mData->currentRoutineObject());
    } else if (mColorPage->pageType() == EColorPageType::ambient
               || mColorPage->pageType() == EColorPageType::brightness
               || mColorPage->pageType() == EColorPageType::colorScheme) {
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
    if (layout != mPaletteFloatinglayout) {
        QPoint topRight(this->width(),
                        mFloatingMenuStart);
        mPaletteFloatinglayout->setGeometry(topRight.x(),
                                          topRight.y(),
                                          mPaletteFloatinglayout->width(),
                                          mPaletteFloatinglayout->height());
    }
    if (layout != mLightsFloatingLayout) {
        QPoint topRight(this->width(),
                        mFloatingMenuStart);
        mLightsFloatingLayout->setGeometry(topRight.x(),
                                               topRight.y(),
                                               mLightsFloatingLayout->width(),
                                               mLightsFloatingLayout->height());

    }
}

void TopMenu::downsizeTextHeightToFit(int maxHeight) {
    int computedHeight  = mSelectedDevicesLabel->fontMetrics().height() * 2;
    int fontPtSize = mSelectedDevicesLabel->font().pointSize();
    while (maxHeight < computedHeight && fontPtSize > 3) {
        fontPtSize--;
        QFont font = mSelectedDevicesLabel->font();
        font.setPointSize(fontPtSize);
        mSelectedDevicesLabel->setFont(font);
        computedHeight = mSelectedDevicesLabel->fontMetrics().height() * 2;
    }
}

void TopMenu::updateUI() {
    if (mData->devices() != mLastDevices) {
        std::list<cor::Light> currentDevices = mData->devices();
        for (auto&& device : currentDevices) {
            mComm->fillDevice(device);
        }
        mLastDevices = currentDevices;
        mMainPalette->updateDevices(currentDevices);
        updateBrightnessSlider();
    }
}
