/*!
 * \copyright
 * Copyright (C) 2015 - 2017.
 * Released under the GNU General Public License.
 */

#include "colorpage.h"
#include "ui_colorpage.h"
#include "mainwindow.h"
#include "hueprotocols.h"

#include <QColorDialog>
#include <QDebug>
#include <QSignalMapper>
#include <QPropertyAnimation>

ColorPage::ColorPage(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::ColorPage) {
    ui->setupUi(this);
    mBottomMenuState = EBottomMenuShow::eShowStandard;
    mBottomMenuIsOpen = false;

    mSingleRoutineWidget = new RoutineButtonsWidget(EWidgetGroup::eSingleRoutines, std::vector<QColor>(), this);
    mSingleRoutineWidget->setMaximumWidth(this->width());
    mSingleRoutineWidget->setMaximumHeight(this->height() / 3);
    mSingleRoutineWidget->setGeometry(0, this->height(), mSingleRoutineWidget->width(), mSingleRoutineWidget->height());
    mSingleRoutineWidget->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    connect(mSingleRoutineWidget, SIGNAL(newRoutineSelected(int)), this, SLOT(newRoutineSelected(int)));

    connect(ui->colorPicker, SIGNAL(colorUpdate(QColor)), this, SLOT(colorChanged(QColor)));
    connect(ui->colorPicker, SIGNAL(ambientUpdate(int, int)), this, SLOT(ambientUpdateReceived(int, int)));
    connect(ui->colorPicker, SIGNAL(multiColorCountChanged(int)), this, SLOT(customColorCountChanged(int)));
    connect(ui->colorPicker, SIGNAL(multiColorChanged(int, QColor)), this, SLOT(multiColorChanged(int, QColor)));
    connect(ui->colorPicker, SIGNAL(brightnessUpdate(int)), this, SLOT(brightnessUpdate(int)));

    mFloatingHorizontalLayout = new FloatingLayout(false, this);
    connect(mFloatingHorizontalLayout, SIGNAL(buttonPressed(QString)), this, SLOT(floatingLayoutButtonPressed(QString)));

    mFloatingVerticalLayout = new FloatingLayout(true, this);
    connect(mFloatingVerticalLayout, SIGNAL(buttonPressed(QString)), this, SLOT(floatingLayoutButtonPressed(QString)));

    mLastButtonKey = "RGB";
}


ColorPage::~ColorPage() {
    delete ui;
}


void ColorPage::setupButtons() {
    mMultiRoutineWidget = new RoutineButtonsWidget(EWidgetGroup::eMultiRoutines, mData->colorGroup(EColorGroup::eCustom), this);
    mMultiRoutineWidget->setMaximumWidth(this->width());
    mMultiRoutineWidget->setMaximumHeight(this->height() / 3);
    mMultiRoutineWidget->setGeometry(0, this->height(), mMultiRoutineWidget->width(), mMultiRoutineWidget->height());
    mMultiRoutineWidget->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    connect(mMultiRoutineWidget, SIGNAL(newRoutineSelected(int)), this, SLOT(newRoutineSelected(int)));
}

void ColorPage::highlightRoutineButton(ELightingRoutine routine) {
    mSingleRoutineWidget->highlightRoutineButton(routine);
}


void ColorPage::changePageType(EColorPageType page, bool skipAnimation) {
    mPageType = page;
    if (mPageType == EColorPageType::eRGB) {
        ui->colorPicker->changeLayout(ELayoutColorPicker::eStandardLayout, skipAnimation);
    } else if (mPageType == EColorPageType::eAmbient) {
        ui->colorPicker->changeLayout(ELayoutColorPicker::eAmbientLayout, skipAnimation);
    } else if (mPageType == EColorPageType::eBrightness) {
        ui->colorPicker->changeLayout(ELayoutColorPicker::eBrightnessLayout, skipAnimation);
    } else if (mPageType == EColorPageType::eMulti) {
        ui->colorPicker->updateMultiColor(mData->colorGroup(EColorGroup::eCustom), mData->customColorsUsed());
        ui->colorPicker->changeLayout(ELayoutColorPicker::eMultiColorLayout, skipAnimation);
    } else {
        qDebug() << "INFO: don't recognize this page type.. " << (int)page;
    }

    showSingleRoutineWidget(false);
    showMultiRoutineWidget(false);
    mBottomMenuIsOpen = false;
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

// ----------------------------
// Slots
// ----------------------------

void ColorPage::newRoutineSelected(int newRoutine) {
    mData->updateRoutine((ELightingRoutine)newRoutine);
    updateRoutineButton();
}

void ColorPage::colorChanged(QColor color) {
    std::list<SLightDevice> routineFix;
    for (auto device : mData->currentDevices()) {
        if ((int)device.lightingRoutine > (int)utils::ELightingRoutineSingleColorEnd) {
            routineFix.push_back(device);
        }
    }
    if (routineFix.size() > 0) {
        mData->updateRoutine(ELightingRoutine::eSingleGlimmer);
    }
    mData->updateColor(color);

    if (mBottomMenuState == EBottomMenuShow::eShowSingleRoutines) {
        mSingleRoutineWidget->singleRoutineColorChanged(color);
    }

    mLastColor = color;
    emit singleColorChanged(color);
    emit updateMainIcons();

    mFloatingVerticalLayout->updateRoutineSingleColor(mData->currentRoutine(), mData->mainColor());
}

void ColorPage::customColorCountChanged(int count) {
    mData->updateCustomColorCount(count);
    ui->colorPicker->updateMultiColor(mData->colorGroup(EColorGroup::eCustom), mData->customColorsUsed());

    if (mBottomMenuState == EBottomMenuShow::eShowMultiRoutines) {
        mMultiRoutineWidget->multiRoutineColorsChanged(mData->colorGroup(EColorGroup::eCustom),
                                                       mData->customColorsUsed());
    }

    emit updateMainIcons();
    updateRoutineButton(); // updates the routine button
}

void ColorPage::multiColorChanged(int index, QColor color) {
    mData->updateCustomColorArray(index, color);
    if (mBottomMenuState == EBottomMenuShow::eShowMultiRoutines) {
        mMultiRoutineWidget->multiRoutineColorsChanged(mData->colorGroup(EColorGroup::eCustom),
                                                       mData->customColorsUsed());
    }

    updateRoutineButton(); // updates the colors of the multi routine
    emit updateMainIcons();
}

void ColorPage::ambientUpdateReceived(int newAmbientValue, int newBrightness) {
    mData->updateCt(newAmbientValue);
    emit singleColorChanged(utils::colorTemperatureToRGB(newAmbientValue));
    emit brightnessChanged(newBrightness);
}

// ----------------------------
// Protected
// ----------------------------


void ColorPage::showEvent(QShowEvent *) {
  QColor color = mData->mainColor();
  mLastColor = color;
  ui->colorPicker->chooseColor(color, false);
  mSingleRoutineWidget->singleRoutineColorChanged(color);
  mSingleRoutineWidget->setGeometry(0, this->height(), mSingleRoutineWidget->width(), mSingleRoutineWidget->height());

  highlightRoutineButton(mData->currentRoutine());

  setupFloatingLayout();
  QPoint topRight(this->width(), 0);
  moveFloatingLayout(topRight);
  showFloatingLayout(true);
}

void ColorPage::hideEvent(QHideEvent *event) {
    Q_UNUSED(event);
    showFloatingLayout(false);

}

void ColorPage::resizeEvent(QResizeEvent *) {
    mSingleRoutineWidget->resize(QSize(this->width(), this->height()));
    QPoint topRight(this->width(), 0);
    moveFloatingLayout(topRight);

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


// ----------------------------
// Floating Layout
// ----------------------------

void ColorPage::setupFloatingLayout() {
    bool hasHue = mData->hasHueDevices();
    bool hasArduino = mData->hasArduinoDevices();
    std::vector<QString> horizontalButtons;
    std::vector<QString> verticalButtons;
    if (hasHue && !hasArduino) {
        // get list of all current devices
        std::list<SLightDevice> devices = mData->currentDevices();
        std::list<SHueLight> hues;
        for (auto& device : devices) {
            hues.push_back(mComm->hueLightFromLightDevice(device));
        }
        EHueType bestHueType = utils::checkForHueWithMostFeatures(hues);
        // get a vector of all the possible hue types for a check.
        if (bestHueType == EHueType::eWhite) {
            changePageType(EColorPageType::eBrightness, true);
        } else if (bestHueType == EHueType::eAmbient) {
            changePageType(EColorPageType::eAmbient, true);
        } else if (bestHueType == EHueType::eExtended){
            horizontalButtons = {QString("Temperature"), QString("RGB")};
            changePageType(EColorPageType::eRGB, true);
        } else {
            throw "did not find any hue lights when expecting hue lights";
        }
    } else if (hasArduino){
        horizontalButtons = {QString("Temperature"), QString("RGB"), QString("Multi")};
        verticalButtons = {QString("Routine")};

        mFloatingHorizontalLayout->highlightButton(mLastButtonKey);
        mFloatingHorizontalLayout->addMultiRoutineIcon(mData->colorGroup(EColorGroup::eRGB));
        updateRoutineButton();
        changePageType(EColorPageType::eRGB, true);
    } else if (!hasHue && !hasArduino) {
        // shouldn't get here...
        throw "trying to open single color page when no recognized devices are selected";
    }

    mFloatingVerticalLayout->setupButtons(verticalButtons);
    mFloatingHorizontalLayout->setupButtons(horizontalButtons);
    mFloatingHorizontalLayout->highlightButton("RGB");
    QPoint topRight(this->width(), 0);
    moveFloatingLayout(topRight);

    updateRoutineButton();
    mFloatingHorizontalLayout->addMultiRoutineIcon(mData->colorGroup(EColorGroup::eRGB));
}

void ColorPage::showFloatingLayout(bool show) {
    mFloatingHorizontalLayout->setVisible(show);
    mFloatingVerticalLayout->setVisible(show);
}

void ColorPage::moveFloatingLayout(QPoint point) {
    mFloatingHorizontalLayout->move(point);
    mFloatingVerticalLayout->move(QPoint(point.x(), point.y() + mFloatingHorizontalLayout->geometry().height()));
}

void ColorPage::floatingLayoutButtonPressed(QString buttonType) {
    mLastButtonKey = buttonType;
    if (buttonType.compare("Multi") == 0) {
        changePageType(EColorPageType::eMulti);
        mFloatingVerticalLayout->setVisible(true);
        mFloatingVerticalLayout->highlightRoutineButton(false);
        updateRoutineButton();
    } else if (buttonType.compare("RGB") == 0) {
        changePageType(EColorPageType::eRGB);
        mFloatingVerticalLayout->setVisible(true);
        mFloatingVerticalLayout->highlightRoutineButton(false);
        updateRoutineButton();
    }  else if (buttonType.compare("Temperature") == 0) {
        changePageType(EColorPageType::eAmbient);
        mFloatingVerticalLayout->setVisible(false);
    } else if (buttonType.compare("Routine") == 0) {
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
        updateRoutineButton();
    } else {
        qDebug() << "I don't recognize that button type...";
    }
}


void ColorPage::updateRoutineButton() {
    if (mPageType == EColorPageType::eRGB) {
        mFloatingVerticalLayout->updateRoutineSingleColor(mData->currentRoutine(),
                                                          mData->mainColor());
    } else if (mPageType == EColorPageType::eMulti) {
        mFloatingVerticalLayout->updateRoutineMultiColor(mData->currentRoutine(),
                                                         mData->colorGroup(EColorGroup::eCustom),
                                                         mData->customColorsUsed());
    }
}
