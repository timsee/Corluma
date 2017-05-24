#include <QGraphicsEffect>


#include "topmenu.h"

TopMenu::TopMenu(DataLayer* data, QWidget *parent) : QWidget(parent) {

    mData = data;

    // --------------
    // Setup Buttons
    // --------------
    mConnectionButton = new QPushButton(this);
    mConnectionButton->setCheckable(true);
    mConnectionButton->setStyleSheet("background-color: rgb(52, 52, 52); ");
    mConnectionButton->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    mConnectionButton->setIcon(QIcon(":images/connectionIcon.png"));
    connect(mConnectionButton, SIGNAL(clicked(bool)), this, SLOT(connectionButtonPressed()));

    mColorPageButton = new CorlumaButton(this);
    mColorPageButton->setupAsMenuButton((int)EPage::eColorPage);
    mColorPageButton->button->setStyleSheet("background-color: rgb(52, 52, 52); ");
    mColorPageButton->button->setCheckable(true);
    mColorPageButton->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    connect(mColorPageButton->button, SIGNAL(clicked(bool)), this, SLOT(colorButtonPressed()));


    mGroupPageButton = new CorlumaButton(this);
    mGroupPageButton->setupAsMenuButton((int)EPage::eGroupPage,  mData->colorGroup(EColorGroup::eSevenColor));
    mGroupPageButton->button->setStyleSheet("background-color: rgb(52, 52, 52); ");
    mGroupPageButton->button->setCheckable(true);
    mGroupPageButton->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    connect(mGroupPageButton->button, SIGNAL(clicked(bool)), this, SLOT(groupButtonPressed()));

    mSettingsButton = new QPushButton(this);
    mSettingsButton->setStyleSheet("background-color: rgb(52, 52, 52); ");
    mSettingsButton->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    mSettingsButton->setIcon(QIcon(":images/settingsgear.png"));
    connect(mSettingsButton, SIGNAL(clicked(bool)), this, SLOT(settingsButtonPressed()));

    // --------------
    // Setup Brightness Slider
    // --------------
    // setup the slider that controls the LED's brightness
    mBrightnessSlider = new CorlumaSlider(this);
    mBrightnessSlider->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    mBrightnessSlider->slider->setRange(0,100);
    mBrightnessSlider->slider->setValue(0);
    mBrightnessSlider->slider->setTickPosition(QSlider::TicksBelow);
    mBrightnessSlider->slider->setTickInterval(20);
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
    mTopLayout->addWidget(mBrightnessSlider, 5);

    mBottomLayout = new QHBoxLayout();
    mBottomLayout->setSpacing(0);
    mBottomLayout->setContentsMargins(0,0,0,0);
    mBottomLayout->addWidget(mConnectionButton, 1);
    mBottomLayout->addWidget(mColorPageButton,  1);
    mBottomLayout->addWidget(mGroupPageButton,  1);
    mBottomLayout->addWidget(mSettingsButton,   1);

    mLayout = new QVBoxLayout(this);
    mLayout->setSpacing(5);
    mLayout->setContentsMargins(0,0,0,0);
    mLayout->addLayout(mTopLayout, 2);
    mLayout->addLayout(mBottomLayout, 4);

    setLayout(mLayout);

    deviceCountReachedZero();
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
    // Multi Color Button Update
    //-----------------
    if (mData->currentRoutine() <= utils::ELightingRoutineSingleColorEnd) {
        EColorGroup closestGroup = mData->closestColorGroupToColor(mData->mainColor());
        mGroupPageButton->updateIconPresetColorRoutine(ELightingRoutine::eMultiBarsMoving,
                                                            closestGroup,
                                                            mData->colorGroup(closestGroup));
    } else {
        if (mData->currentColorGroup() == EColorGroup::eCustom) {
            // do nothing
        } else {
            mGroupPageButton->updateIconPresetColorRoutine(mData->currentRoutine(),
                                                                mData->currentColorGroup(),
                                                                mData->currentGroup());
        }
    }

    //-----------------
    // On/Off Data
    //-----------------
    if (mData->currentColorGroup() == EColorGroup::eCustom
            && mData->currentRoutine() > utils::ELightingRoutineSingleColorEnd) {
        mIconData.setMultiLightingRoutine(mData->currentRoutine(),
                                          mData->currentColorGroup(),
                                          mData->currentGroup(),
                                          mData->customColorsUsed());
    } else if (mData->currentRoutine() <= utils::ELightingRoutineSingleColorEnd) {
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

    if (mData->brightness() != mBrightnessSlider->slider->value()) {
        mBrightnessSlider->blockSignals(true);
        mBrightnessSlider->slider->setValue(mData->brightness());
        mBrightnessSlider->blockSignals(false);
    }
}



void TopMenu::updateSingleColor(QColor color) {
    mIconData.setSolidColor(color);
    mIconData.setSingleLightingRoutine(mData->currentRoutine(), color);
    mBrightnessSlider->setSliderColorBackground(color);
    mOnOffButton->setIcon(mIconData.renderAsQPixmap());
    mColorPageButton->updateIconSingleColorRoutine(ELightingRoutine::eSingleSolid, color);
}

void TopMenu::updatePresetColorGroup(int lightingRoutine, int colorGroup) {
    mIconData.setMultiFade((EColorGroup)colorGroup, mData->colorGroup((EColorGroup)colorGroup));
    mGroupPageButton->updateIconPresetColorRoutine((ELightingRoutine)lightingRoutine,
                                                        (EColorGroup)colorGroup,
                                                        mData->colorGroup((EColorGroup)colorGroup));
    mBrightnessSlider->setSliderColorBackground(mData->colorsAverage((EColorGroup)colorGroup));
    mOnOffButton->setIcon(mIconData.renderAsQPixmap());
}


void TopMenu::resizeMenuIcon(QPushButton *button, QString iconPath) {
    QPixmap pixmap(iconPath);
    int size = std::min(this->width() / 4 * 0.7f, (float)button->height());
    button->setIcon(QIcon(pixmap.scaled(size,
                                        size,
                                        Qt::KeepAspectRatio,
                                        Qt::SmoothTransformation)));
    button->setIconSize(QSize(size, size));
}

void TopMenu::deviceCountReachedZero() {
    mColorPageButton->enable(false);
    mGroupPageButton->enable(false);
    mBrightnessSlider->enable(false);

    QGraphicsOpacityEffect *effect = new QGraphicsOpacityEffect(mOnOffButton);
    effect->setOpacity(0.5f);
    mOnOffButton->setGraphicsEffect(effect);
    mOnOffButton->setEnabled(false);

    mShouldGreyOutIcons = true;
}

void TopMenu::deviceCountChangedOnConnectionPage() {
    bool anyDevicesReachable = mData->anyDevicesReachable();
    if (mShouldGreyOutIcons
            && (mData->currentDevices().size() > 0)
            &&  anyDevicesReachable) {
        mColorPageButton->enable(true);
        mGroupPageButton->enable(true);
        mBrightnessSlider->enable(true);

        QGraphicsOpacityEffect *effect = new QGraphicsOpacityEffect(mOnOffButton);
        effect->setOpacity(1.0);
        mOnOffButton->setGraphicsEffect(effect);
        mOnOffButton->setEnabled(true);

        mShouldGreyOutIcons = false;
    }
    if ((!mShouldGreyOutIcons
         && (mData->currentDevices().size() == 0))
            || !anyDevicesReachable) {
        deviceCountReachedZero();
    }
    mColorPageButton->updateIconSingleColorRoutine(ELightingRoutine::eSingleSolid, mData->mainColor());
}

void TopMenu::settingsButtonPressed() {
    emit buttonPressed("Settings");
}

void TopMenu::connectionButtonPressed() {
    mConnectionButton->setChecked(true);
    mConnectionButton->setStyleSheet("background-color: rgb(80, 80, 80); ");

    mColorPageButton->button->setChecked(false);
    mColorPageButton->button->setStyleSheet("background-color: rgb(52, 52, 52); ");

    mGroupPageButton->button->setChecked(false);
    mGroupPageButton->button->setStyleSheet("background-color: rgb(52, 52, 52); ");

    emit buttonPressed("Connection");
}

void TopMenu::colorButtonPressed() {
    mConnectionButton->setChecked(false);
    mConnectionButton->setStyleSheet("background-color: rgb(52, 52, 52); ");

    mColorPageButton->button->setChecked(true);
    mColorPageButton->button->setStyleSheet("background-color: rgb(80, 80, 80); ");

    mGroupPageButton->button->setChecked(false);
    mGroupPageButton->button->setStyleSheet("background-color: rgb(52, 52, 52); ");

    emit buttonPressed("Color");
}

void TopMenu::groupButtonPressed() {
    mConnectionButton->setChecked(false);
    mConnectionButton->setStyleSheet("background-color: rgb(52, 52, 52); ");

    mColorPageButton->button->setChecked(false);
    mColorPageButton->button->setStyleSheet("background-color: rgb(52, 52, 52); ");

    mGroupPageButton->button->setChecked(true);
    mGroupPageButton->button->setStyleSheet("background-color: rgb(80, 80, 80); ");

    emit buttonPressed("Group");
}

void TopMenu::hueWhiteLightsFound() {
    mColorPageButton->enable(true);
    mGroupPageButton->enable(false);
    mBrightnessSlider->enable(true);
    mColorPageButton->updateIconSingleColorRoutine(ELightingRoutine::eSingleSolid, QColor(255,255,255));
}

void TopMenu::resizeEvent(QResizeEvent *event) {
    Q_UNUSED(event);
    int onOffSize = this->height() / 3;
    mOnOffButton->setIconSize(QSize(onOffSize * 0.8f, onOffSize * 0.8f));
    mOnOffButton->setMinimumHeight(onOffSize);
    resizeMenuIcon(mSettingsButton, ":images/settingsgear.png");
    resizeMenuIcon(mConnectionButton, ":images/connectionIcon.png");
}
