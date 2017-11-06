/*!
 * \copyright
 * Copyright (C) 2015 - 2017.
 * Released under the GNU General Public License.
 */

#include "colorpage.h"
#include "mainwindow.h"
#include "comm/hueprotocols.h"
#include "corlumautils.h"

#include <QColorDialog>
#include <QDebug>
#include <QSignalMapper>
#include <QPropertyAnimation>

ColorPage::ColorPage(QWidget *parent) :
    QWidget(parent) {
    mBottomMenuState = EBottomMenuShow::eShowStandard;
    mBottomMenuIsOpen = false;

    mSingleRoutineWidget = new RoutineButtonsWidget(EWidgetGroup::eSingleRoutines, std::vector<QColor>(), this);
    mSingleRoutineWidget->setMaximumWidth(this->width());
    mSingleRoutineWidget->setMaximumHeight(this->height() / 3);
    mSingleRoutineWidget->setGeometry(0, this->height(), mSingleRoutineWidget->width(), mSingleRoutineWidget->height());
    mSingleRoutineWidget->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    connect(mSingleRoutineWidget, SIGNAL(newRoutineSelected(int)), this, SLOT(newRoutineSelected(int)));

    mSpacer = new QWidget(this);

    mColorPicker = new ColorPicker(this);

    mLayout = new QVBoxLayout(this);
    mLayout->addWidget(mSpacer, 1);
    mLayout->addWidget(mColorPicker, 10);

    connect(mColorPicker, SIGNAL(colorUpdate(QColor)), this, SLOT(colorChanged(QColor)));
    connect(mColorPicker, SIGNAL(ambientUpdate(int, int)), this, SLOT(ambientUpdateReceived(int, int)));
    connect(mColorPicker, SIGNAL(multiColorCountUpdate(int)), this, SLOT(customColorCountChanged(int)));
    connect(mColorPicker, SIGNAL(multiColorUpdate(QColor, int)), this, SLOT(multiColorChanged(QColor, int)));
    connect(mColorPicker, SIGNAL(brightnessUpdate(int)), this, SLOT(brightnessUpdate(int)));
    connect(mColorPicker, SIGNAL(colorsUpdate(std::vector<QColor>)), this, SLOT(colorsChanged(std::vector<QColor>)));
}


ColorPage::~ColorPage() {
}


void ColorPage::setupButtons() {
    mMultiRoutineWidget = new RoutineButtonsWidget(EWidgetGroup::eMultiRoutines, mData->colorGroup(EColorGroup::eCustom), this);
    mMultiRoutineWidget->setMaximumWidth(this->width());
    mMultiRoutineWidget->setMaximumHeight(this->height() / 3);
    mMultiRoutineWidget->setGeometry(0, this->height(), mMultiRoutineWidget->width(), mMultiRoutineWidget->height());
    mMultiRoutineWidget->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    connect(mMultiRoutineWidget, SIGNAL(newRoutineSelected(int)), this, SLOT(newRoutineSelected(int)));
    // raise single since its default
    mSingleRoutineWidget->raise();
}

void ColorPage::highlightRoutineButton(ELightingRoutine routine) {
    mSingleRoutineWidget->highlightRoutineButton(routine);
}


void ColorPage::changePageType(EColorPageType page, bool skipAnimation) {
    mPageType = page;

    mColorPicker->updateColorStates(mData->mainColor(),
                                       mData->brightness(),
                                       mData->colorGroup(EColorGroup::eCustom),
                                       createColorScheme(mData->currentDevices()),
                                       mData->customColorsUsed());
    if (mPageType == EColorPageType::eRGB) {
        mColorPicker->changeLayout(ELayoutColorPicker::eStandardLayout, skipAnimation);
    } else if (mPageType == EColorPageType::eAmbient) {
        mColorPicker->changeLayout(ELayoutColorPicker::eAmbientLayout, skipAnimation);
    } else if (mPageType == EColorPageType::eBrightness) {
        mColorPicker->changeLayout(ELayoutColorPicker::eBrightnessLayout, skipAnimation);
    } else if (mPageType == EColorPageType::eMulti) {
        mColorPicker->changeLayout(ELayoutColorPicker::eMultiColorLayout, skipAnimation);
    } else if (mPageType == EColorPageType::eColorScheme) {
        mColorPicker->changeLayout(ELayoutColorPicker::eColorSchemeLayout, skipAnimation);
    } else {
        qDebug() << "INFO: don't recognize this page type.. " << (int)page;
    }

    showSingleRoutineWidget(false);
    showMultiRoutineWidget(false);
    mBottomMenuIsOpen = false;
}


void ColorPage::handleRoutineWidget() {
    bool showSingle = false;
    bool showMulti = false;
    if (mPageType == EColorPageType::eRGB
            && !mBottomMenuIsOpen) {
        showSingle = true;
    } else if (mPageType == EColorPageType::eMulti
               && !mBottomMenuIsOpen) {
        showMulti = true;
    }
    showSingleRoutineWidget(showSingle);
    showMultiRoutineWidget(showMulti);
    mBottomMenuIsOpen = !mBottomMenuIsOpen;
}

EColorPageType ColorPage::pageType() {
    return mPageType;
}


void ColorPage::showSingleRoutineWidget(bool shouldShow) {
    if (mBottomMenuState == EBottomMenuShow::eShowSingleRoutines && !shouldShow) {
        QPropertyAnimation *animation = new QPropertyAnimation(mSingleRoutineWidget, "pos");
        animation->setDuration(TRANSITION_TIME_MSEC);
        animation->setStartValue(mSingleRoutineWidget->pos());
        animation->setEndValue(QPoint(0, this->height()));
        animation->start();
        mBottomMenuState = EBottomMenuShow::eShowStandard;
    } else if (mBottomMenuState != EBottomMenuShow::eShowSingleRoutines && shouldShow) {
        mSingleRoutineWidget->raise();
        mSingleRoutineWidget->singleRoutineColorChanged(mData->mainColor());  // update colors of single color routine
        QPropertyAnimation *animation = new QPropertyAnimation(mSingleRoutineWidget, "pos");
        animation->setDuration(TRANSITION_TIME_MSEC);
        animation->setStartValue(mSingleRoutineWidget->pos());
        animation->setEndValue(QPoint(0, this->height() - mSingleRoutineWidget->height()));
        animation->start();
        mBottomMenuState = EBottomMenuShow::eShowSingleRoutines;
    }
}

void ColorPage::showMultiRoutineWidget(bool shouldShow) {
    if (mBottomMenuState == EBottomMenuShow::eShowMultiRoutines && !shouldShow) {
        QPropertyAnimation *animation = new QPropertyAnimation(mMultiRoutineWidget, "pos");
        animation->setDuration(TRANSITION_TIME_MSEC);
        animation->setStartValue(mMultiRoutineWidget->pos());
        animation->setEndValue(QPoint(0, this->height()));
        animation->start();
        mBottomMenuState = EBottomMenuShow::eShowStandard;
    } else if (mBottomMenuState != EBottomMenuShow::eShowMultiRoutines && shouldShow) {
        mMultiRoutineWidget->raise();
        QPropertyAnimation *animation = new QPropertyAnimation(mMultiRoutineWidget, "pos");
        animation->setDuration(TRANSITION_TIME_MSEC);
        animation->setStartValue(mMultiRoutineWidget->pos());
        animation->setEndValue(QPoint(0, this->height() - mMultiRoutineWidget->height()));
        animation->start();
        mMultiRoutineWidget->multiRoutineColorsChanged(mData->colorGroup(EColorGroup::eCustom),
                                                       mData->customColorsUsed());
        mBottomMenuState = EBottomMenuShow::eShowMultiRoutines;
    }
}

std::vector<QColor> ColorPage::createColorScheme(std::list<SLightDevice> devices) {
    std::vector<QColor> colorScheme;
    int count = 0;
    int max = 5;
    for (auto&& device : devices) {
        if (count >= max) {
            break;
        } else {
           colorScheme.push_back(device.color);
        }
        count++;
    }
    return colorScheme;
}

// ----------------------------
// Slots
// ----------------------------

void ColorPage::newRoutineSelected(int newRoutine) {
    mData->updateRoutine((ELightingRoutine)newRoutine);
}

void ColorPage::colorChanged(QColor color) {
    mData->updateColor(color);

    for (auto device : mData->currentDevices()) {
        if (device.lightingRoutine > utils::ELightingRoutineSingleColorEnd) {
            mData->updateRoutine(ELightingRoutine::eSingleGlimmer);
        }
    }

    /// this is not always needed but sometimes it is and its a cheap operation if it isn't.
    /// Some comm streams such as hue combine color and brightness with HSV.
    /// By updating both, the system is forced to check both.
    mData->updateBrightness(mData->brightness());

    if (mBottomMenuState == EBottomMenuShow::eShowSingleRoutines) {
        mSingleRoutineWidget->singleRoutineColorChanged(color);
    }

    mLastColor = color;
    emit singleColorChanged(color);
    emit updateMainIcons();
}

void ColorPage::customColorCountChanged(int count) {
    mData->updateCustomColorCount(count);
    mColorPicker->updateColorStates(mData->mainColor(),
                                    mData->brightness(),
                                    mData->colorGroup(EColorGroup::eCustom),
                                    createColorScheme(mData->currentDevices()),
                                    mData->customColorsUsed());

    if (mBottomMenuState == EBottomMenuShow::eShowMultiRoutines) {
        mMultiRoutineWidget->multiRoutineColorsChanged(mData->colorGroup(EColorGroup::eCustom),
                                                       mData->customColorsUsed());
    }

    emit updateMainIcons();
}

void ColorPage::multiColorChanged(QColor color, int index) {
    mData->updateCustomColorArray(index, color);
    if (mBottomMenuState == EBottomMenuShow::eShowMultiRoutines) {
        mMultiRoutineWidget->multiRoutineColorsChanged(mData->colorGroup(EColorGroup::eCustom),
                                                       mData->customColorsUsed());
    }

    emit updateMainIcons();
}

void ColorPage::ambientUpdateReceived(int newAmbientValue, int newBrightness) {
    mData->updateCt(newAmbientValue);
    emit singleColorChanged(utils::colorTemperatureToRGB(newAmbientValue));
    emit brightnessChanged(newBrightness);
}

void ColorPage::colorsChanged(std::vector<QColor> colors) {
    mData->updateColorScheme(colors);
    /// this is not always needed but sometimes it is and its a cheap operation if it isn't.
    /// Some comm streams such as hue combine color and brightness with HSV.
    /// By updating both, the system is forced to che
    /// ck both.
    mData->updateBrightness(mData->brightness());
    emit updateMainIcons();
}

// ----------------------------
// Protected
// ----------------------------


void ColorPage::show() {
    mLastColor = mData->mainColor();
    mColorPicker->updateColorStates(mData->mainColor(),
                                    mData->brightness(),
                                    mData->colorGroup(EColorGroup::eCustom),
                                    createColorScheme(mData->currentDevices()),
                                    mData->customColorsUsed());
}

void ColorPage::resizeEvent(QResizeEvent *) {
    mSingleRoutineWidget->resize(QSize(this->width(), this->height()));

    if (mBottomMenuState == EBottomMenuShow::eShowSingleRoutines) {
        mSingleRoutineWidget->setGeometry(0, this->height() - mSingleRoutineWidget->height(), mSingleRoutineWidget->width(), mSingleRoutineWidget->height());
    } else {
        mSingleRoutineWidget->setGeometry(0, this->height(), mSingleRoutineWidget->width(), mSingleRoutineWidget->height());
    }

    mMultiRoutineWidget->resize(QSize(this->width(), this->height()));
    if (mBottomMenuState == EBottomMenuShow::eShowMultiRoutines) {
        mMultiRoutineWidget->setGeometry(0, this->height() - mMultiRoutineWidget->height(), mMultiRoutineWidget->width(), mMultiRoutineWidget->height());
    } else {
        mMultiRoutineWidget->setGeometry(0, this->height(), mMultiRoutineWidget->width(), mMultiRoutineWidget->height());
    }
}


