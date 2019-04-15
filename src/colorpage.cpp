/*!
 * \copyright
 * Copyright (C) 2015 - 2019.
 * Released under the GNU General Public License.
 */

#include "colorpage.h"
#include "mainwindow.h"
#include "hue/hueprotocols.h"
#include "utils/color.h"
#include "utils/qt.h"

#include <QColorDialog>
#include <QDebug>
#include <QSignalMapper>

ColorPage::ColorPage(QWidget *parent) :
    QWidget(parent),
    mBrightness{50},
    mColorScheme(5, QColor(0, 255, 0)),
    mPalette(QJsonObject()),
    mPageType{EColorPageType::RGB} {

    mBottomMenuState = EBottomMenuShow::showStandard;
    mBottomMenuIsOpen = false;

    mSpacer = new QWidget(this);

    mColorPicker = new ColorPicker(this);
    mColorPicker->setMultiColorDefaults(cor::defaultCustomColors());
    mCurrentMultiRoutine = ERoutine::multiGlimmer;

    mLayout = new QVBoxLayout(this);
    mLayout->addWidget(mSpacer, 1);
    mLayout->addWidget(mColorPicker, 10);

    connect(mColorPicker, SIGNAL(colorUpdate(QColor)), this, SLOT(colorChanged(QColor)));
    connect(mColorPicker, SIGNAL(ambientUpdate(int,uint32_t)), this, SLOT(ambientUpdateReceived(int,uint32_t)));
    connect(mColorPicker, SIGNAL(multiColorCountUpdate(int)), this, SLOT(customColorCountChanged(int)));
    connect(mColorPicker, SIGNAL(multiColorUpdate()), this, SLOT(multiColorChanged()));
    connect(mColorPicker, SIGNAL(brightnessUpdate(uint32_t)), this, SLOT(brightnessUpdate(uint32_t)));
    connect(mColorPicker, SIGNAL(colorsUpdate(std::vector<QColor>)), this, SLOT(colorsChanged(std::vector<QColor>)));

    mSingleRoutineWidget = new RoutineButtonsWidget(EWidgetGroup::singleRoutines, std::vector<QColor>(), this);
    mSingleRoutineWidget->setMaximumWidth(this->width());
    mSingleRoutineWidget->setMaximumHeight(this->height() / 3);
    mSingleRoutineWidget->setGeometry(0, this->height(), mSingleRoutineWidget->width(), mSingleRoutineWidget->height());
    mSingleRoutineWidget->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    connect(mSingleRoutineWidget, SIGNAL(newRoutineSelected(QJsonObject)), this, SLOT(newRoutineSelected(QJsonObject)));
    mCurrentSingleRoutine = mSingleRoutineWidget->routines()[3].second;
    updateColor(QColor(0, 255, 0));

    /// fill with junk data for this case
    mMultiRoutineWidget = new RoutineButtonsWidget(EWidgetGroup::multiRoutines, cor::defaultCustomColors(), this);
    mMultiRoutineWidget->setMaximumWidth(this->width());
    mMultiRoutineWidget->setMaximumHeight(this->height() / 3);
    mMultiRoutineWidget->setGeometry(0, this->height(), mMultiRoutineWidget->width(), mMultiRoutineWidget->height());
    mMultiRoutineWidget->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    connect(mMultiRoutineWidget, SIGNAL(newRoutineSelected(QJsonObject)), this, SLOT(newRoutineSelected(QJsonObject)));
}


void ColorPage::changePageType(EColorPageType page, bool skipAnimation) {
    mPageType = page;

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
        qDebug() << "INFO: don't recognize this page type.. " << int(page);
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
        cor::moveWidget(mSingleRoutineWidget,
                        mSingleRoutineWidget->size(),
                        mSingleRoutineWidget->pos(),
                        QPoint(0, this->height()));

        mBottomMenuState = EBottomMenuShow::showStandard;
    } else if (mBottomMenuState != EBottomMenuShow::showSingleRoutines && shouldShow) {
        mSingleRoutineWidget->singleRoutineColorChanged(mColor);  // update colors of single color routine
        cor::moveWidget(mSingleRoutineWidget,
                        mSingleRoutineWidget->size(),
                        mSingleRoutineWidget->pos(),
                        QPoint(0, this->height() - mSingleRoutineWidget->height()));

        mBottomMenuState = EBottomMenuShow::showSingleRoutines;
    }
}

void ColorPage::showMultiRoutineWidget(bool shouldShow) {
    if (mBottomMenuState == EBottomMenuShow::showMultiRoutines && !shouldShow) {
        cor::moveWidget(mMultiRoutineWidget,
                        mMultiRoutineWidget->size(),
                        mMultiRoutineWidget->pos(),
                        QPoint(0, this->height()));

        mBottomMenuState = EBottomMenuShow::showStandard;
    } else if (mBottomMenuState != EBottomMenuShow::showMultiRoutines && shouldShow) {
        cor::moveWidget(mMultiRoutineWidget,
                        mMultiRoutineWidget->size(),
                        mMultiRoutineWidget->pos(),
                        QPoint(0, this->height() - mMultiRoutineWidget->height()));

        mBottomMenuState = EBottomMenuShow::showMultiRoutines;
    }
}

// ----------------------------
// Slots
// ----------------------------

void ColorPage::newRoutineSelected(QJsonObject routineObject) {
    ERoutine routine = stringToRoutine(routineObject["routine"].toString());
    if (routine <= cor::ERoutineSingleColorEnd) {
        routineObject["hue"] = mColor.hueF();
        routineObject["sat"] = mColor.saturationF();
        routineObject["bri"] = mColor.valueF();
    } else {
        Palette palette(paletteToString(EPalette::custom),
                        mColorPicker->colors(),
                        mBrightness);
        routineObject["palette"] = palette.JSON();
    }
    routineObject["isOn"]  = true;
    if (routine != ERoutine::singleSolid) {
        // no speed settings for single color routines currently...
        routineObject["speed"] = 125;
    }

    // get color
    emit routineUpdate(routineObject);
    if (routine <= cor::ERoutineSingleColorEnd) {
        mCurrentSingleRoutine = routineObject;
    } else {
        mCurrentMultiRoutine = routine;
    }
}

void ColorPage::colorChanged(const QColor& color) {
    updateColor(color);
    mCurrentSingleRoutine["hue"] = mColor.hueF();
    mCurrentSingleRoutine["sat"] = mColor.saturationF();
    mCurrentSingleRoutine["bri"] = mColor.valueF();
    mCurrentSingleRoutine["isOn"]  = true;

    emit routineUpdate(mCurrentSingleRoutine);

    if (mBottomMenuState == EBottomMenuShow::showSingleRoutines) {
        mSingleRoutineWidget->singleRoutineColorChanged(color);
    }
}

void ColorPage::customColorCountChanged(int count) {
    Q_UNUSED(count);
    mColorPicker->updateColorStates(mColor,
                                    mBrightness,
                                    mColorScheme,
                                    mPalette.colors());

    mMultiRoutineWidget->multiRoutineColorsChanged(mColorPicker->colors());
}

void ColorPage::multiColorChanged() {
    QJsonObject routineObject;
    routineObject["routine"] = routineToString(mCurrentMultiRoutine);

    Palette palette(paletteToString(EPalette::custom), mColorPicker->colors(), mBrightness);

    routineObject["palette"] = palette.JSON();
    routineObject["isOn"]  = true;
    // no speed settings for single color routines currently...
    routineObject["speed"] = 125;

    emit routineUpdate(routineObject);
    mMultiRoutineWidget->multiRoutineColorsChanged(mColorPicker->colors());
}

void ColorPage::ambientUpdateReceived(int newAmbientValue, uint32_t newBrightness) {
    QJsonObject routineObject;
    routineObject["routine"] = routineToString(ERoutine::singleSolid);
    QColor color = cor::colorTemperatureToRGB(newAmbientValue);
    updateColor(color);
    routineObject["temperature"]   = newAmbientValue;
    routineObject["isOn"] = true;

    emit routineUpdate(routineObject);
    mBrightness = newBrightness;

    if (mBottomMenuState == EBottomMenuShow::showSingleRoutines) {
        mSingleRoutineWidget->singleRoutineColorChanged(color);
    }
    emit brightnessChanged(newBrightness);
}

void ColorPage::colorsChanged(const std::vector<QColor>& colors) {
    emit schemeUpdate(colors);
}

void ColorPage::updateColor(const QColor& color) {
    mColor = color;
    for (auto&& schemeColor : mColorScheme) {
        schemeColor = color;
    }
}
// ----------------------------
// Protected
// ----------------------------


void ColorPage::show(const QColor& color, uint32_t brightness, const std::vector<QColor>& colorScheme, const Palette& palette, uint32_t lightCount) {
    if (lightCount == 0) {
        mColorPicker->enable(false);
    } else {
        mColor = color;
        mBrightness = brightness;
        mColorScheme = colorScheme;
        mPalette = palette;
        mColorPicker->updateColorStates(mColor,
                                        mBrightness,
                                        mColorScheme,
                                        mPalette.colors());
        mColorPicker->enable(true);
    }
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


