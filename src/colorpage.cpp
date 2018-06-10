/*!
 * \copyright
 * Copyright (C) 2015 - 2018.
 * Released under the GNU General Public License.
 */

#include "colorpage.h"
#include "mainwindow.h"
#include "hue/hueprotocols.h"
#include "cor/utils.h"


#include <QColorDialog>
#include <QDebug>
#include <QSignalMapper>
#include <QPropertyAnimation>

ColorPage::ColorPage(QWidget *parent, DataLayer *data) :
    QWidget(parent) {
    mData = data;

    mBottomMenuState = EBottomMenuShow::showStandard;
    mBottomMenuIsOpen = false;

    mSingleRoutineWidget = new RoutineButtonsWidget(EWidgetGroup::singleRoutines, std::vector<QColor>(), this);
    mSingleRoutineWidget->setMaximumWidth(this->width());
    mSingleRoutineWidget->setMaximumHeight(this->height() / 3);
    mSingleRoutineWidget->setGeometry(0, this->height(), mSingleRoutineWidget->width(), mSingleRoutineWidget->height());
    mSingleRoutineWidget->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    connect(mSingleRoutineWidget, SIGNAL(newRoutineSelected(QJsonObject)), this, SLOT(newRoutineSelected(QJsonObject)));

    mCurrentSingleRoutine = mSingleRoutineWidget->routines()[3].second;
    mLastColor = QColor(0, 255, 0);

    mSpacer = new QWidget(this);

    mColorPicker = new ColorPicker(this);
    mColorPicker->setMultiColorDefaults(cor::defaultCustomColors());
    mCurrentMultiRoutine = ERoutine::multiGlimmer;

    mLayout = new QVBoxLayout(this);
    mLayout->addWidget(mSpacer, 1);
    mLayout->addWidget(mColorPicker, 10);

    connect(mColorPicker, SIGNAL(colorUpdate(QColor)), this, SLOT(colorChanged(QColor)));
    connect(mColorPicker, SIGNAL(ambientUpdate(int, int)), this, SLOT(ambientUpdateReceived(int, int)));
    connect(mColorPicker, SIGNAL(multiColorCountUpdate(int)), this, SLOT(customColorCountChanged(int)));
    connect(mColorPicker, SIGNAL(multiColorUpdate(QColor, int)), this, SLOT(multiColorChanged(QColor, int)));
    connect(mColorPicker, SIGNAL(brightnessUpdate(int)), this, SLOT(brightnessUpdate(int)));
    connect(mColorPicker, SIGNAL(colorsUpdate(std::vector<QColor>)), this, SLOT(colorsChanged(std::vector<QColor>)));

    /// fill with junk data for this case
    mMultiRoutineWidget = new RoutineButtonsWidget(EWidgetGroup::multiRoutines, cor::defaultCustomColors(), this);
    mMultiRoutineWidget->setMaximumWidth(this->width());
    mMultiRoutineWidget->setMaximumHeight(this->height() / 3);
    mMultiRoutineWidget->setGeometry(0, this->height(), mMultiRoutineWidget->width(), mMultiRoutineWidget->height());
    mMultiRoutineWidget->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    connect(mMultiRoutineWidget, SIGNAL(newRoutineSelected(QJsonObject)), this, SLOT(newRoutineSelected(QJsonObject)));
    // raise single since its default
    mSingleRoutineWidget->raise();

}


ColorPage::~ColorPage() {
}


void ColorPage::changePageType(EColorPageType page, bool skipAnimation) {
    mPageType = page;

    mColorPicker->updateColorStates(mData->mainColor(),
                                       mData->brightness(),
                                       createColorScheme(mData->currentDevices()));
    if (mPageType == EColorPageType::RGB) {
        mColorPicker->changeLayout(ELayoutColorPicker::standardLayout, skipAnimation);
    } else if (mPageType == EColorPageType::ambient) {
        mColorPicker->changeLayout(ELayoutColorPicker::ambientLayout, skipAnimation);
    } else if (mPageType == EColorPageType::brightness) {
        mColorPicker->changeLayout(ELayoutColorPicker::brightnessLayout, skipAnimation);
    } else if (mPageType == EColorPageType::multi) {
        mColorPicker->changeLayout(ELayoutColorPicker::multiColorLayout, skipAnimation);
    } else if (mPageType == EColorPageType::colorScheme) {
        mColorPicker->changeLayout(ELayoutColorPicker::colorSchemeLayout, skipAnimation);
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
    if (mPageType == EColorPageType::RGB
            && !mBottomMenuIsOpen) {
        showSingle = true;
    } else if (mPageType == EColorPageType::multi
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
    if (mBottomMenuState == EBottomMenuShow::showSingleRoutines && !shouldShow) {
        QPropertyAnimation *animation = new QPropertyAnimation(mSingleRoutineWidget, "pos");
        animation->setDuration(TRANSITION_TIME_MSEC);
        animation->setStartValue(mSingleRoutineWidget->pos());
        animation->setEndValue(QPoint(0, this->height()));
        animation->start();
        mBottomMenuState = EBottomMenuShow::showStandard;
    } else if (mBottomMenuState != EBottomMenuShow::showSingleRoutines && shouldShow) {
        mSingleRoutineWidget->raise();
        mSingleRoutineWidget->singleRoutineColorChanged(mData->mainColor());  // update colors of single color routine
        QPropertyAnimation *animation = new QPropertyAnimation(mSingleRoutineWidget, "pos");
        animation->setDuration(TRANSITION_TIME_MSEC);
        animation->setStartValue(mSingleRoutineWidget->pos());
        animation->setEndValue(QPoint(0, this->height() - mSingleRoutineWidget->height()));
        animation->start();
        mBottomMenuState = EBottomMenuShow::showSingleRoutines;
    }
}

void ColorPage::showMultiRoutineWidget(bool shouldShow) {
    if (mBottomMenuState == EBottomMenuShow::showMultiRoutines && !shouldShow) {
        QPropertyAnimation *animation = new QPropertyAnimation(mMultiRoutineWidget, "pos");
        animation->setDuration(TRANSITION_TIME_MSEC);
        animation->setStartValue(mMultiRoutineWidget->pos());
        animation->setEndValue(QPoint(0, this->height()));
        animation->start();
        mBottomMenuState = EBottomMenuShow::showStandard;
    } else if (mBottomMenuState != EBottomMenuShow::showMultiRoutines && shouldShow) {
        mMultiRoutineWidget->raise();
        QPropertyAnimation *animation = new QPropertyAnimation(mMultiRoutineWidget, "pos");
        animation->setDuration(TRANSITION_TIME_MSEC);
        animation->setStartValue(mMultiRoutineWidget->pos());
        animation->setEndValue(QPoint(0, this->height() - mMultiRoutineWidget->height()));
        animation->start();
        mBottomMenuState = EBottomMenuShow::showMultiRoutines;
    }
}

std::vector<QColor> ColorPage::createColorScheme(std::list<cor::Light> devices) {
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

void ColorPage::newRoutineSelected(QJsonObject routineObject) {
    ERoutine routine = stringToRoutine(routineObject["routine"].toString());
    routineObject["red"]   = mLastColor.red();
    routineObject["green"] = mLastColor.green();
    routineObject["blue"]  = mLastColor.blue();
    routineObject["isOn"]  = true;
    // no speed settings for single color routines currently...
    routineObject["speed"] = 125;

    // get color
    mData->updateRoutine(routineObject);
    if (routine <= cor::ERoutineSingleColorEnd) {
        mCurrentSingleRoutine = routineObject;
    } else {
        mCurrentMultiRoutine = routine;
    }
}

void ColorPage::colorChanged(QColor color) {
    mLastColor = color;
    mCurrentSingleRoutine["red"]   = mLastColor.red();
    mCurrentSingleRoutine["green"] = mLastColor.green();
    mCurrentSingleRoutine["blue"]  = mLastColor.blue();
    mCurrentSingleRoutine["isOn"]  = true;

    mData->updateRoutine(mCurrentSingleRoutine);
    // set all the lights to the given brightness
    mData->updateBrightness(mData->brightness());

    if (mBottomMenuState == EBottomMenuShow::showSingleRoutines) {
        mSingleRoutineWidget->singleRoutineColorChanged(color);
    }

    emit updateMainIcons();
}

void ColorPage::customColorCountChanged(int count) {
    Q_UNUSED(count);
    mColorPicker->updateColorStates(mData->mainColor(),
                                    mData->brightness(),
                                    createColorScheme(mData->currentDevices()));

    mMultiRoutineWidget->multiRoutineColorsChanged(mColorPicker->colors());

    emit updateMainIcons();
}

void ColorPage::multiColorChanged(QColor color, int index) {
    Q_UNUSED(color);
    Q_UNUSED(index);
    QJsonObject routineObject;
    routineObject["routine"] = routineToString(mCurrentMultiRoutine);
    Palette palette(paletteToString(EPalette::custom), mColorPicker->colors());
    routineObject["palette"] = palette.JSON();
    routineObject["isOn"]  = true;
    // no speed settings for single color routines currently...
    routineObject["speed"] = 125;

    mData->updateRoutine(routineObject);
    mMultiRoutineWidget->multiRoutineColorsChanged(mColorPicker->colors());

    emit updateMainIcons();
}

void ColorPage::ambientUpdateReceived(int newAmbientValue, int newBrightness) {
    QJsonObject routineObject;
    routineObject["routine"] = routineToString(ERoutine::singleSolid);
    QColor color = cor::colorTemperatureToRGB(newAmbientValue);
    mLastColor = color;
    routineObject["temperature"]   = newAmbientValue;
    routineObject["isOn"] = true;

    mData->updateRoutine(routineObject);
    mData->updateBrightness(newBrightness);

    if (mBottomMenuState == EBottomMenuShow::showSingleRoutines) {
        mSingleRoutineWidget->singleRoutineColorChanged(color);
    }
    emit brightnessChanged(newBrightness);
    emit updateMainIcons();
}

void ColorPage::colorsChanged(std::vector<QColor> colors) {
    mData->updateColorScheme(colors);
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
                                    createColorScheme(mData->currentDevices()));
}

void ColorPage::resizeEvent(QResizeEvent *) {
    mSingleRoutineWidget->resize(QSize(this->width(), this->height()));

    if (mBottomMenuState == EBottomMenuShow::showSingleRoutines) {
        mSingleRoutineWidget->setGeometry(0, this->height() - mSingleRoutineWidget->height(), mSingleRoutineWidget->width(), mSingleRoutineWidget->height());
    } else {
        mSingleRoutineWidget->setGeometry(0, this->height(), mSingleRoutineWidget->width(), mSingleRoutineWidget->height());
    }

    mMultiRoutineWidget->resize(QSize(this->width(), this->height()));
    if (mBottomMenuState == EBottomMenuShow::showMultiRoutines) {
        mMultiRoutineWidget->setGeometry(0, this->height() - mMultiRoutineWidget->height(), mMultiRoutineWidget->width(), mMultiRoutineWidget->height());
    } else {
        mMultiRoutineWidget->setGeometry(0, this->height(), mMultiRoutineWidget->width(), mMultiRoutineWidget->height());
    }
}


