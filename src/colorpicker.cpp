/*!
 * \copyright
 * Copyright (C) 2015 - 2017.
 * Released under the GNU General Public License.
 */

#include "colorpicker.h"

#include <QDebug>
#include <QSignalMapper>
#include <QGraphicsOpacityEffect>


ColorPicker::ColorPicker(QWidget *parent) :
    QWidget(parent) {

    mWheelIsEnabled = true;

    mAmbientTemperature = 300;
    mAmbientBrightness = 80;
    mMultiArraySize = 10;
    mMultiUsed = 2;
    mWheelOpacity = 1.0f;

    // --------------
    // Setup Thrrole Timer
    // --------------

    mThrottleTimer = new QTimer(this);
    connect(mThrottleTimer, SIGNAL(timeout()), this, SLOT(resetThrottleFlag()));

    // --------------
    // Setup ColorWheel
    // --------------

    mColorWheel = new QLabel(this);
    mColorWheel->setPixmap(QPixmap(":/images/color_wheel.png"));
    mColorWheel->setAlignment(Qt::AlignCenter);
    mColorWheel->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);


    mTempWheel = new QLabel(this);
    mTempWheel->setPixmap(QPixmap(":/images/color_wheel.png"));
    mTempWheel->setAlignment(Qt::AlignCenter);
    mTempWheel->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    mTempWheel->setVisible(false);

    // --------------
    // Setup Sliders
    // --------------

    mTopSlider = new LightsSlider(this);
    mTopSlider->setSliderColorBackground(QColor(255, 0, 0));
    mTopSlider->slider->setRange(0, 255);
    mTopSlider->setSnapToNearestTick(true);
    mTopSlider->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    mTopSlider->setSliderHeight(0.8f);
    mTopSlider->hide();

    mMidSlider = new LightsSlider(this);
    mMidSlider->setSliderColorBackground(QColor(0, 255, 0));
    mMidSlider->slider->setRange(0, 255);
    mTopSlider->setSnapToNearestTick(true);
    mMidSlider->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    mMidSlider->setSliderHeight(0.8f);
    mMidSlider->hide();
    QSizePolicy retainSize = mMidSlider->sizePolicy();
    retainSize.setRetainSizeWhenHidden(true);
    mMidSlider->setSizePolicy(retainSize);

    mBottomSlider = new LightsSlider(this);
    mBottomSlider->slider->setRange(0, 255);
    mBottomSlider->setSliderColorBackground(QColor(0, 0, 255));
    mTopSlider->setSnapToNearestTick(true);
    mBottomSlider->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    mBottomSlider->setSliderHeight(0.8f);
    mBottomSlider->hide();
    retainSize = mBottomSlider->sizePolicy();
    retainSize.setRetainSizeWhenHidden(true);
    mBottomSlider->setSizePolicy(retainSize);


    connect(mTopSlider, SIGNAL(valueChanged(int)), this, SLOT(topSliderChanged(int)));
    connect(mTopSlider->slider, SIGNAL(sliderReleased()), this, SLOT(releasedSlider()));

    connect(mMidSlider, SIGNAL(valueChanged(int)), this, SLOT(midSliderChanged(int)));
    connect(mMidSlider->slider, SIGNAL(sliderReleased()), this, SLOT(releasedSlider()));

    connect(mBottomSlider, SIGNAL(valueChanged(int)), this, SLOT(bottomSliderChanged(int)));
    connect(mBottomSlider->slider, SIGNAL(sliderReleased()), this, SLOT(releasedSlider()));

    // --------------
    // Setup RGB Labels
    // --------------

    mTopLabel = new QLabel(this);
    mTopLabel->setText("R");
    mTopLabel->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Expanding);
    mTopLabel->hide();
    retainSize = mTopLabel->sizePolicy();
    retainSize.setRetainSizeWhenHidden(true);
    mTopLabel->setSizePolicy(retainSize);

    mMidLabel = new QLabel(this);
    mMidLabel->setText("G");
    mMidLabel->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Expanding);
    mMidLabel->hide();
    retainSize = mMidLabel->sizePolicy();
    retainSize.setRetainSizeWhenHidden(true);
    mMidLabel->setSizePolicy(retainSize);


    mBottomLabel = new QLabel(this);
    mBottomLabel->setText("B");
    mBottomLabel->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Expanding);
    mBottomLabel->hide();
    retainSize = mBottomLabel->sizePolicy();
    retainSize.setRetainSizeWhenHidden(true);
    mBottomLabel->setSizePolicy(retainSize);

    // --------------
    // Setup Array Color Buttons
    // --------------

    mArrayColorsButtons = std::vector<QPushButton*>(mMultiArraySize, nullptr);
    QSignalMapper *arrayButtonsMapper = new QSignalMapper(this);
    for (uint32_t i = 0; i < mMultiArraySize; ++i) {
        mArrayColorsButtons[i] = new QPushButton;
        mArrayColorsButtons[i]->setStyleSheet("border: none;");
        mArrayColorsButtons[i]->setCheckable(true);
        mArrayColorsButtons[i]->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
        connect(mArrayColorsButtons[i], SIGNAL(clicked(bool)), arrayButtonsMapper, SLOT(map()));
        arrayButtonsMapper->setMapping(mArrayColorsButtons[i], i);
    }
    connect(arrayButtonsMapper, SIGNAL(mapped(int)), this, SLOT(selectArrayColor(int)));


    // --------------
    // Setup Slider/Label Layout
    // --------------

    mBottomLayout = new QGridLayout;
    mFullLayout = new QVBoxLayout;
    mFullLayout->addWidget(mColorWheel, Qt::AlignCenter);
    mFullLayout->addLayout(mBottomLayout);
    setLayout(mFullLayout);

    // default to standard layout
    changeLayout(ELayoutColorPicker::eStandardLayout);
}

ColorPicker::~ColorPicker() {
    delete mThrottleTimer;
}


void ColorPicker::changeLayout(ELayoutColorPicker layout,  bool skipAnimation) {

    //---------------------
    // Remove last bottom layout
    //---------------------

    if (mCurrentLayoutColorPicker == ELayoutColorPicker::eWheelOnlyLayout) {
        // do nothing
    } else if (mCurrentLayoutColorPicker == ELayoutColorPicker::eStandardLayout
               || mCurrentLayoutColorPicker == ELayoutColorPicker::eAmbientLayout
               || mCurrentLayoutColorPicker == ELayoutColorPicker::eBrightnessLayout) {
        mBottomLayout->removeWidget(mTopLabel);
        mBottomLayout->removeWidget(mMidLabel);
        mBottomLayout->removeWidget(mBottomLabel);
        mBottomLayout->removeWidget(mTopSlider);
        mBottomLayout->removeWidget(mMidSlider);
        mBottomLayout->removeWidget(mBottomSlider);
        mTopSlider->hide();
        mMidSlider->hide();
        mBottomSlider->hide();
        mTopLabel->hide();
        mMidLabel->hide();
        mBottomLabel->hide();
    }  else if (mCurrentLayoutColorPicker == ELayoutColorPicker::eMultiColorLayout) {
        mBottomLayout->removeWidget(mTopSlider);
        for (uint32_t i = 0; i < mMultiArraySize; ++i) {
            mBottomLayout->removeWidget(mArrayColorsButtons[i]);
            mArrayColorsButtons[i]->hide();
        }
    }

    ELayoutColorPicker oldLayout = mCurrentLayoutColorPicker;
    // reset all flags
    mCurrentLayoutColorPicker = layout;

    //---------------------
    // Add new bottom Layout
    //---------------------

    // update the bottom layout
    if (mCurrentLayoutColorPicker == ELayoutColorPicker::eStandardLayout) {
        // add necessary widgets
        mBottomLayout->addWidget(mTopLabel, 1, 0);
        mBottomLayout->addWidget(mTopSlider, 1, 1);
        mBottomLayout->addWidget(mMidLabel, 2, 0);
        mBottomLayout->addWidget(mMidSlider, 2, 1);
        mBottomLayout->addWidget(mBottomLabel, 3, 0);
        mBottomLayout->addWidget(mBottomSlider, 3, 1);
        // show and hide widgets
        mTopSlider->show();
        mMidSlider->show();
        mBottomSlider->show();
        mTopLabel->show();
        mMidLabel->show();
        mBottomLabel->show();
        // set ranges
        mTopSlider->slider->blockSignals(true);
        mTopSlider->slider->setRange(0, 255);
        mTopSlider->slider->setTickInterval(1);
        mTopSlider->slider->setTickPosition(QSlider::NoTicks);
        mTopSlider->setMinimumPossible(false, 20);
        mTopSlider->setSliderHeight(0.8f);
        mTopSlider->slider->blockSignals(false);

        mMidSlider->slider->setRange(0, 255);
        mTopSlider->setSliderColorBackground(QColor(255,0,0));
        mMidSlider->setSliderColorBackground(QColor(0,255,0));
        chooseColor(mColor, false);
    } else if (mCurrentLayoutColorPicker == ELayoutColorPicker::eAmbientLayout) {
        // add necessary widgets
        mBottomLayout->addWidget(mTopLabel, 1, 0);
        mBottomLayout->addWidget(mTopSlider, 1, 1);
        mBottomLayout->addWidget(mMidLabel, 2, 0);
        mBottomLayout->addWidget(mMidSlider, 2, 1);
        mBottomLayout->addWidget(mBottomLabel, 3, 0);
        mBottomLayout->addWidget(mBottomSlider, 3, 1);
        // show and hide widgets
        mTopSlider->show();
        mMidSlider->show();
        mBottomSlider->hide();
        mTopLabel->hide();
        mMidLabel->hide();
        mBottomLabel->hide();

        // set ranges
        mTopSlider->slider->blockSignals(true);
        mMidSlider->slider->blockSignals(true);
        mTopSlider->slider->setRange(153, 500);

        mTopSlider->slider->setTickPosition(QSlider::NoTicks);
        mTopSlider->slider->setValue(mAmbientTemperature);
        mTopSlider->setSliderHeight(0.8f);

        mMidSlider->slider->setRange(0, 100);
        mMidSlider->slider->setValue(mAmbientBrightness);
        mMidSlider->slider->blockSignals(false);
        mTopSlider->slider->blockSignals(false);

        chooseAmbient(mAmbientTemperature, mAmbientBrightness, false);
    } else if (mCurrentLayoutColorPicker == ELayoutColorPicker::eBrightnessLayout) {
        // add necessary widgets
        mBottomLayout->addWidget(mTopLabel, 1, 0);
        mBottomLayout->addWidget(mTopSlider, 1, 1);
        mBottomLayout->addWidget(mMidLabel, 2, 0);
        mBottomLayout->addWidget(mMidSlider, 2, 1);
        mBottomLayout->addWidget(mBottomLabel, 3, 0);
        mBottomLayout->addWidget(mBottomSlider, 3, 1);
        // show and hide widgets
        mTopSlider->show();
        mMidSlider->hide();
        mBottomSlider->hide();
        mTopLabel->hide();
        mMidLabel->hide();
        mBottomLabel->hide();

        // set ranges
        mTopSlider->slider->blockSignals(true);
        mTopSlider->slider->setRange(0, 100);
        mTopSlider->slider->setTickPosition(QSlider::NoTicks);
        mTopSlider->slider->setValue(mAmbientBrightness);
        mTopSlider->setMinimumPossible(false, 20);
        mTopSlider->setSliderHeight(0.8f);
        mTopSlider->slider->blockSignals(false);

        chooseBrightness(mWhiteBrightness, false);
    } else if (mCurrentLayoutColorPicker == ELayoutColorPicker::eMultiColorLayout) {
        int halfway = mMultiArraySize / 2;

        mBottomLayout->addWidget(mTopSlider, 1, 0, 1, halfway + 1);

        // show and hide widgets
        mTopSlider->show();
        mMidSlider->hide();
        mBottomSlider->hide();
        mTopLabel->hide();
        mMidLabel->hide();
        mBottomLabel->hide();

        mTopSlider->slider->blockSignals(true);
        mTopSlider->slider->setRange(0, mMultiArraySize * 10);
        mTopSlider->slider->setValue(20);
        mTopSlider->slider->setTickInterval(10);
        mTopSlider->slider->setTickPosition(QSlider::TicksBelow);
        mTopSlider->setMinimumPossible(true, 20);
        mTopSlider->setSliderHeight(0.6f);
        mTopSlider->slider->blockSignals(false);

        int row = 2;
        int col = 0;
        for (uint32_t i = 0; i < mMultiArraySize; ++i) {
            int size = this->size().height() * 0.1f;
            mArrayColorsButtons[i]->setMinimumSize(QSize(size, size));
            size = size * 0.8f;
            mArrayColorsButtons[i]->setIconSize(QSize(size, size));
            QPixmap icon = createSolidColorIcon(mMultiColors[i]);
            mArrayColorsButtons[i]->setIcon(icon.scaled(size,
                                                        size,
                                                        Qt::KeepAspectRatio,
                                                        Qt::SmoothTransformation));
            mArrayColorsButtons[i]->setIcon(icon);
            mBottomLayout->addWidget(mArrayColorsButtons[i], row, col);
            mArrayColorsButtons[i]->show();
            col++;
            if (((i + 1) % halfway) == 0) {
                row++;
                col = 0;
            }
        }
        manageMultiSelected();
    }
    changeColorWheel(oldLayout, layout, skipAnimation);
    resize();
}

void ColorPicker::enableWheel(bool shouldEnable) {
    if (shouldEnable) {
        mWheelOpacity = 1.0f;
        // un-fade out the wheel
        QGraphicsOpacityEffect *effect = new QGraphicsOpacityEffect(mColorWheel);
        effect->setOpacity(mWheelOpacity);
        mColorWheel->setGraphicsEffect(effect);
    } else if (!shouldEnable) {
        mWheelOpacity = 0.333f;
        // fade out the wheel
        QGraphicsOpacityEffect *effect = new QGraphicsOpacityEffect(mColorWheel);
        effect->setOpacity(mWheelOpacity);
        mColorWheel->setGraphicsEffect(effect);
    }
    mWheelIsEnabled = shouldEnable;
}

//------------------------------
// Layout-Specific API
//------------------------------

void ColorPicker::chooseColor(QColor color, bool shouldSignal) {
    mColor = color;

    bool blocked = mTopSlider->slider->blockSignals(true);
    mTopSlider->slider->setValue(color.red());
    mTopSlider->slider->blockSignals(blocked);

    blocked = mMidSlider->slider->blockSignals(true);
    mMidSlider->slider->setValue(color.green());
    mMidSlider->slider->blockSignals(blocked);

    blocked = mBottomSlider->slider->blockSignals(true);
    mBottomSlider->slider->setValue(color.blue());
    mBottomSlider->slider->blockSignals(blocked);

    if (shouldSignal) {
        emit colorUpdate(color);
    }
}


void ColorPicker::chooseAmbient(int temperature, int brightness, bool shouldSignal) {
    if (brightness >= 0
            && brightness <= 100
            && temperature >= 153
            && temperature <= 500) {
        mAmbientTemperature = temperature;
        mAmbientBrightness = brightness;

        mTopSlider->setSliderColorBackground(utils::colorTemperatureToRGB(temperature));

        mTopSlider->blockSignals(true);
        mTopSlider->slider->setValue(temperature);
        mTopSlider->blockSignals(false);

        QColor brightColor(2.5f * brightness,
                           2.5f * brightness,
                           2.5f * brightness);
        mMidSlider->setSliderColorBackground(brightColor);
        mMidSlider->blockSignals(true);
        mMidSlider->slider->setValue(brightness);
        mMidSlider->blockSignals(false);

        if (shouldSignal) {
            emit ambientUpdate(temperature, brightness);
        }
    }
}


void ColorPicker::chooseBrightness(int brightness, bool shouldSignal) {
    if (brightness >= 0
            && brightness <= 100) {
        mWhiteBrightness = brightness;

        QColor brightColor(2.5f * brightness,
                           2.5f * brightness,
                           2.5f * brightness);

        mTopSlider->setSliderColorBackground(brightColor);

        mTopSlider->blockSignals(true);
        mTopSlider->slider->setValue(brightness);
        mTopSlider->blockSignals(false);

        if (shouldSignal) {
            emit brightnessUpdate(brightness);
        }
    }
}

void ColorPicker::updateMultiColor(const std::vector<QColor> colors, int count) {
    mMultiUsed = count;
    if (mMultiUsed < 2) {
        mMultiUsed = 2;
    }
    mMultiColors = colors;
    QPixmap greyIcon = createSolidColorIcon(QColor(140,140,140));

    for (uint32_t i = 0; i < mMultiUsed; ++i) {
        mArrayColorsButtons[i]->setEnabled(true);
        QPixmap icon = createSolidColorIcon(mMultiColors[i]);
        mArrayColorsButtons[i]->setIcon(icon);
    }

    for (uint32_t i = mMultiUsed; i < mMultiArraySize; ++i) {
        mArrayColorsButtons[i]->setIcon(greyIcon);
        mArrayColorsButtons[i]->setEnabled(false);
    }

    updateMultiColorSlider();
    manageMultiSelected();
}


void ColorPicker::selectArrayColor(int index) {
    // check if new index is already in list
    auto result = std::find(mMultiSelected.begin(), mMultiSelected.end(), index);
    if (result != mMultiSelected.end()) {
        // if is found, remove it and deselect, but only if it isn't the only selected object
        mMultiSelected.remove(index);
    } else {
        // if it isn't found, add it and select
        mMultiSelected.push_back(index);
    }
    manageMultiSelected();
}

// ----------------------------
// Slots
// ----------------------------

void ColorPicker::topSliderChanged(int newValue) {
    if (!mThrottleFlag) {
        mThrottleFlag = true;
        if (mCurrentLayoutColorPicker == ELayoutColorPicker::eStandardLayout
                || mCurrentLayoutColorPicker == ELayoutColorPicker::eWheelOnlyLayout) {
            chooseColor(QColor(newValue, mColor.green(), mColor.blue()));
        } else if (mCurrentLayoutColorPicker == ELayoutColorPicker::eAmbientLayout) {
            chooseAmbient(newValue, mAmbientBrightness);
        } else if (mCurrentLayoutColorPicker == ELayoutColorPicker::eBrightnessLayout) {
            chooseBrightness(newValue);
        } else if (mCurrentLayoutColorPicker == ELayoutColorPicker::eMultiColorLayout) {
            emit multiColorCountChanged(newValue / 10);
        }
    }
}

void ColorPicker::midSliderChanged(int newValue) {
    if (!mThrottleFlag) {
        mThrottleFlag = true;
        if (mCurrentLayoutColorPicker == ELayoutColorPicker::eStandardLayout
                || mCurrentLayoutColorPicker == ELayoutColorPicker::eWheelOnlyLayout) {
            chooseColor(QColor(mColor.red(), newValue, mColor.blue()));
        } else if (mCurrentLayoutColorPicker == ELayoutColorPicker::eAmbientLayout) {
            chooseAmbient(mAmbientTemperature, newValue);
        }
    }
}

void ColorPicker::bottomSliderChanged(int newValue) {
    if (!mThrottleFlag) {
        mThrottleFlag = true;
        if (mCurrentLayoutColorPicker == ELayoutColorPicker::eStandardLayout
                || mCurrentLayoutColorPicker == ELayoutColorPicker::eWheelOnlyLayout) {
            chooseColor(QColor(mColor.red(), mColor.green(), newValue));
        }
    }
}

void ColorPicker::releasedSlider() {
    if (mCurrentLayoutColorPicker == ELayoutColorPicker::eStandardLayout
            || mCurrentLayoutColorPicker == ELayoutColorPicker::eWheelOnlyLayout) {
        chooseColor(QColor(mTopSlider->slider->value(),
                           mMidSlider->slider->value(),
                           mBottomSlider->slider->value()));
    } else if (mCurrentLayoutColorPicker == ELayoutColorPicker::eAmbientLayout) {
        chooseAmbient(mTopSlider->slider->value(),
                      mMidSlider->slider->value());
    } else if (mCurrentLayoutColorPicker == ELayoutColorPicker::eMultiColorLayout) {
        emit multiColorCountChanged(mTopSlider->slider->value() / 10);
    }
}

void ColorPicker::showEvent(QShowEvent *event) {
    Q_UNUSED(event);
    mThrottleTimer->start(25);
}

void ColorPicker::hideEvent(QHideEvent *event) {
    Q_UNUSED(event);
    mThrottleTimer->stop();
    mThrottleFlag = false;
}

void ColorPicker::hideTempWheel() {
    mTempWheel->setVisible(false);
}

// ----------------------------
// Protected
// ----------------------------


void ColorPicker::mousePressEvent(QMouseEvent *event) {
    mThrottleFlag = true;
    mPressTime.restart();
    handleMouseEvent(event);
}

void ColorPicker::mouseMoveEvent(QMouseEvent *event) {
    if (!mThrottleFlag) {
        mThrottleFlag = true;
        handleMouseEvent(event);
    }
}

void ColorPicker::mouseReleaseEvent(QMouseEvent *event) {
    Q_UNUSED(event);
    if (eventIsOverWheel(event)
            && mPressTime.elapsed() > 500) {
        if (mCurrentLayoutColorPicker == ELayoutColorPicker::eStandardLayout
                || mCurrentLayoutColorPicker == ELayoutColorPicker::eWheelOnlyLayout) {
            chooseColor(QColor(mTopSlider->slider->value(),
                               mMidSlider->slider->value(),
                               mBottomSlider->slider->value()));
        } else if (mCurrentLayoutColorPicker == ELayoutColorPicker::eAmbientLayout) {
            chooseAmbient(mTopSlider->slider->value(),
                          mMidSlider->slider->value());
        } else if (mCurrentLayoutColorPicker == ELayoutColorPicker::eMultiColorLayout) {
            // Do nothing...
        }
    }
}

void ColorPicker::resizeEvent(QResizeEvent *event) {
    Q_UNUSED(event);
    resize();
}


//------------------------------
// Multi Layout-Specific
//------------------------------


void ColorPicker::updateMultiColorSlider() {
    int r = 0;
    int g = 0;
    int b = 0;
    for (uint32_t i = 0; i < mMultiUsed; ++i) {
        r = r + mMultiColors[i].red();
        g = g + mMultiColors[i].green();
        b = b + mMultiColors[i].blue();
    }
    QColor average(r / mMultiUsed,
                   g / mMultiUsed,
                   b / mMultiUsed);
    mTopSlider->setSliderColorBackground(average);
}

void ColorPicker::manageMultiSelected() {
    // check all selected devices, store all that are higher than multi used count.
    std::list<uint32_t> indicesToRemove;
    for (auto index : mMultiSelected) {
        if (index >= mMultiUsed) {
            // index cant be selected cause its more the max count, remove from selected indices
            indicesToRemove.push_back(index);
        }
    }

    // if indices should be removed, remove them
    if (indicesToRemove.size() > 0) {
        for (auto&& index : indicesToRemove) {
            mMultiSelected.remove(index);
        }
    }

    // enable wheel if any devices are selected, otherwise disable the wheel
    enableWheel((mMultiSelected.size() > 0));

    // now do the GUI work for highlighting...
    // deselect all, we'll be reselecting in here!
    for (uint32_t i = 0; i < mMultiArraySize; ++i) {
        mArrayColorsButtons[i]->setChecked(false);
    }
    for (auto index : mMultiSelected) {
        mArrayColorsButtons[index]->setChecked(true);
    }
}


//------------------------------
// Mouse Events
//------------------------------

void ColorPicker::handleMouseEvent(QMouseEvent *event) {
    QRect geometry = mColorWheel->geometry();

    if (eventIsOverWheel(event)) {
        QPixmap pixmap = QWidget::grab(mColorWheel->geometry());
        QColor color = pixmap.toImage().pixel(event->pos().x() - geometry.x(),
                                              event->pos().y() - geometry.y());
        if (checkIfColorIsValid(color)){
            if (mCurrentLayoutColorPicker == ELayoutColorPicker::eMultiColorLayout) {
                if (mWheelIsEnabled) {
                    for (auto&& index : mMultiSelected) {
                        mMultiColors[index] = color;
                        emit multiColorChanged(index, color);
                    }
                    updateMultiColor(mMultiColors, mMultiUsed);
                }
            } else if (mCurrentLayoutColorPicker == ELayoutColorPicker::eAmbientLayout) {
                // use the poorly named "value" of the HSV range to calculate the brightness
                int brightness = color.valueF() * 100.0f;
                // adjust the color so that it has a maxed out value in the HSV colorspace
                color.setHsv(color.hue(),
                             color.saturation(),
                             255);
                // then calculate then use the resulting QColor to convert to color temperature.
                int ct = utils::rgbToColorTemperature(color);
                chooseAmbient(ct, brightness);
            } else if (mCurrentLayoutColorPicker == ELayoutColorPicker::eBrightnessLayout) {
                // use the poorly named "value" of the HSV range to calculate the brightness
                int brightness = color.valueF() * 100.0f;
                chooseBrightness(brightness);
            } else if (mCurrentLayoutColorPicker == ELayoutColorPicker::eStandardLayout) {
                chooseColor(color);
            }
        }
    }
}

bool ColorPicker::eventIsOverWheel(QMouseEvent *event) {
    // solve for potential deadspace since the qLabel expands horizontally to fit in its
    // container if needed. So this enforces that mouse press events only trigger if they are
    // directly above the circle.
    QRect geometry = mColorWheel->geometry();
    int deadZoneLeft = geometry.x();
    int deadZoneRight = geometry.x() + geometry.width();
    int deadZoneTop = geometry.y();
    int deadZoneBottom = geometry.y() + geometry.height();
    // check if its in the right square region
    if ((event->pos().x() > deadZoneLeft
            && event->pos().x() < deadZoneRight
            && event->pos().y() > deadZoneTop
            && event->pos().y() < deadZoneBottom)) {
        // check that its in the right circular region
        QPoint center(geometry.width() / 2,
                      geometry.height() / 2);
        QLineF line(event->pos(), center);
        float min = (float)std::min(geometry.width(), geometry.height());
        float length = line.length() / min;
        if (length < 0.42f) { // number used as deadzone, 0.5 is theoretical maximum
            return true;
        } else {
            return false;
        }
    }
    return false;
}

//------------------------------
// Helpers
//------------------------------

void ColorPicker::changeColorWheel(ELayoutColorPicker oldLayout, ELayoutColorPicker newLayout, bool skipAnimation) {

    //------------------
    // get resources
    //------------------

    // get paths to resources for two color wheels
    QString oldLayoutResource = getWheelPixmapPath(oldLayout);
    QString newLayoutResource = getWheelPixmapPath(newLayout);

    // get pixmaps for two color wheels
    QPixmap oldPixmap(oldLayoutResource);
    QPixmap newPixmap(newLayoutResource);

    //------------------
    // resize and set pixmaps
    //------------------

    if (mCurrentLayoutColorPicker == ELayoutColorPicker::eWheelOnlyLayout) {
        int min = std::min(this->width(), this->height());
        mColorWheel->setFixedHeight(min);
        mTempWheel->setFixedHeight(min);
        int wheelSize = min * 0.9f;
        mTempWheel->setPixmap(oldPixmap.scaled(wheelSize,
                                               wheelSize,
                                               Qt::KeepAspectRatio,
                                               Qt::SmoothTransformation));
        mColorWheel->setPixmap(newPixmap.scaled(wheelSize,
                                                wheelSize,
                                                Qt::KeepAspectRatio,
                                                Qt::SmoothTransformation));
    } else {
        mColorWheel->setFixedHeight(this->size().height() * 0.7f);
        int wheelSize = this->size().height() * 0.55f;
        if (wheelSize > this->size().width() * 0.85f) {
            wheelSize = this->size().width() * 0.85f;
        }
        mTempWheel->setPixmap(oldPixmap.scaled(wheelSize,
                                               wheelSize,
                                               Qt::KeepAspectRatio,
                                               Qt::SmoothTransformation));
        mColorWheel->setPixmap(newPixmap.scaled(wheelSize,
                                                wheelSize,
                                                Qt::KeepAspectRatio,
                                                Qt::SmoothTransformation));
    }

    const int animationMsec = 250;

    //------------------
    // Animate if needed
    //------------------

    if (!skipAnimation) {
        // check if a new color wheel is being added. if the two resources don't match
        // do a fade in/fade out between the color wheels.
        if (oldLayoutResource.compare(newLayoutResource) != 0) {
            // make temp wheel visible and overlapping the color wheel
            mTempWheel->setVisible(true);
            mTempWheel->setGeometry(mColorWheel->geometry());

            QGraphicsOpacityEffect *fadeOutEffect = new QGraphicsOpacityEffect(mTempWheel);
            mTempWheel->setGraphicsEffect(fadeOutEffect);
            QPropertyAnimation *fadeOutAnimation = new QPropertyAnimation(fadeOutEffect, "opacity");
            fadeOutAnimation->setDuration(animationMsec);
            fadeOutAnimation->setStartValue(mWheelOpacity);
            fadeOutAnimation->setEndValue(0.0f);
            connect(fadeOutAnimation, SIGNAL(finished()), this, SLOT(hideTempWheel()));

            QGraphicsOpacityEffect *fadeInEffect = new QGraphicsOpacityEffect(mColorWheel);
            mColorWheel->setGraphicsEffect(fadeInEffect);
            QPropertyAnimation *fadeInAnimation = new QPropertyAnimation(fadeInEffect, "opacity");
            fadeInAnimation->setDuration(animationMsec);
            fadeInAnimation->setStartValue(0.0f);
            mWheelOpacity = 1.0f;
            // catch edge case wehre multi color picker is sometimes disabled by default
            if (newLayout == ELayoutColorPicker::eMultiColorLayout
                    && (mMultiSelected.size() == 0)) {
                mWheelOpacity = 0.333f;
            }
            fadeInAnimation->setEndValue(mWheelOpacity);

            QParallelAnimationGroup *group = new QParallelAnimationGroup;
            group->addAnimation(fadeInAnimation);
            group->addAnimation(fadeOutAnimation);
            group->start(QAbstractAnimation::DeleteWhenStopped);
        } else if (mMultiSelected.size() == 0
                       && (newLayout == ELayoutColorPicker::eMultiColorLayout
                           || oldLayout == ELayoutColorPicker::eMultiColorLayout)
                       && (newLayout != oldLayout)) {

            mTempWheel->setVisible(false);

            // If the resources match but its either changing from or changing to a multi layout
            // and no indices are selected so the multi layout should be opaque.
            float startOpacity;
            // don't need two animations, just do one.
            if (newLayout == ELayoutColorPicker::eMultiColorLayout) {
                startOpacity = 1.0f;
                mWheelOpacity = 0.333f;
            } else if (oldLayout == ELayoutColorPicker::eMultiColorLayout) {
                startOpacity = 0.333f;
                mWheelOpacity = 1.0f;
            } else {
                startOpacity = 0.0f;
                mWheelOpacity = 0.0f;
                qDebug() << "WARNING: shouldn't get here...";
            }

            if (oldLayout == ELayoutColorPicker::eMultiColorLayout) {
                // just set opacity here immediately cause of weird bug...
                mWheelOpacity = 1.0f;
                // un-fade out the wheel
                QGraphicsOpacityEffect *effect = new QGraphicsOpacityEffect(mColorWheel);
                effect->setOpacity(mWheelOpacity);
                mColorWheel->setGraphicsEffect(effect);
            } else {
                QGraphicsOpacityEffect *fadeEffect = new QGraphicsOpacityEffect(mColorWheel);
                mColorWheel->setGraphicsEffect(fadeEffect);
                QPropertyAnimation *fadeAnimation = new QPropertyAnimation(fadeEffect, "opacity");
                fadeAnimation->setDuration(animationMsec);
                fadeAnimation->setStartValue(startOpacity);
                fadeAnimation->setEndValue(mWheelOpacity);
                fadeAnimation->start();
            }

        }
    } else {
        bool shouldEnable = true;
        if (mMultiSelected.size() == 0
                && (newLayout == ELayoutColorPicker::eMultiColorLayout)) {
            shouldEnable = false;
        }
        enableWheel(shouldEnable);
        mTempWheel->setVisible(false);
    }

}

bool ColorPicker::checkIfColorIsValid(QColor color) {
    bool colorIsValid = true;
    // check if its a color similar to the background...
    if (color.red() == color.blue() + 1
            && color.blue() == color.green()) {
        colorIsValid = false;
    }

    // miscellaneous edge cases...
    if ((color.red() == 54 && color.green() == 54 && color.blue() == 54)
            || (color.red() == 146 && color.green() == 146 && color.blue() == 146)
            || (color.red() == 137 && color.green() == 137 && color.blue() == 137)
            || (color.red() == 98 && color.green() == 98 && color.blue() == 98)
            || (color.red() == 67 && color.green() == 67 && color.blue() == 67)) {
        colorIsValid = false;
    }
    return colorIsValid;
}

void ColorPicker::resetThrottleFlag() {
    mThrottleFlag = false;
}

QString ColorPicker::getWheelPixmapPath(ELayoutColorPicker layout) {
    QString name;
    switch (layout)
    {
        case ELayoutColorPicker::eStandardLayout:
        case ELayoutColorPicker::eMultiColorLayout:
        case ELayoutColorPicker::eWheelOnlyLayout:
            name = QString(":/images/color_wheel.png");
            break;
        case ELayoutColorPicker::eAmbientLayout:
            name = QString(":/images/ambient_wheel.png");
            break;
        case ELayoutColorPicker::eBrightnessLayout:
            name = QString(":/images/white_wheel.png");
            break;
    default:
        name = QString(":/images/color_wheel.png");
    }
    return name;
}

void ColorPicker::resize() {
    QPixmap pixmap(getWheelPixmapPath(mCurrentLayoutColorPicker));
    bool useStandardWheelSize = true;
    if (mCurrentLayoutColorPicker == ELayoutColorPicker::eStandardLayout
            || mCurrentLayoutColorPicker == ELayoutColorPicker::eAmbientLayout
            || mCurrentLayoutColorPicker == ELayoutColorPicker::eBrightnessLayout) {
        mTopSlider->setFixedHeight(this->size().height() * 0.1f);
        mMidSlider->setFixedHeight(this->size().height() * 0.1f);
        mBottomSlider->setFixedHeight(this->size().height() * 0.1f);
    }  else if (mCurrentLayoutColorPicker == ELayoutColorPicker::eWheelOnlyLayout) {
        useStandardWheelSize = false;
        int min = std::min(this->width(), this->height());
        mColorWheel->setFixedHeight(min);

        int wheelSize = min * 0.9f;
        mColorWheel->setPixmap(pixmap.scaled(wheelSize,
                                            wheelSize,
                                            Qt::KeepAspectRatio,
                                            Qt::SmoothTransformation));
    } else if (mCurrentLayoutColorPicker == ELayoutColorPicker::eMultiColorLayout) {
        mTopSlider->setFixedHeight(this->size().height() * 0.1f);
        for (uint32_t i = 0; i < mMultiArraySize; ++i) {
            int size = this->size().height() * 0.1f;
            mArrayColorsButtons[i]->setMinimumSize(QSize(size, size));
            size = size * 0.8f;
            mArrayColorsButtons[i]->setIconSize(QSize(size, size));
            QPixmap icon = createSolidColorIcon(mMultiColors[i]);
            mArrayColorsButtons[i]->setIcon(icon.scaled(size,
                                                        size,
                                                        Qt::KeepAspectRatio,
                                                        Qt::SmoothTransformation));
        }
    }

    if (useStandardWheelSize) {
        mColorWheel->setFixedHeight(this->size().height() * 0.7f);
        int wheelSize = this->size().height() * 0.55f;
        if (wheelSize > this->size().width() * 0.85f) {
            wheelSize = this->size().width() * 0.85f;
        }
        mColorWheel->setPixmap(pixmap.scaled(wheelSize,
                                            wheelSize,
                                            Qt::KeepAspectRatio,
                                            Qt::SmoothTransformation));
    }
}

QPixmap ColorPicker::createSolidColorIcon(QColor color) {
    QSize size(mTopSlider->geometry().height() * 0.75f,
               mTopSlider->geometry().height() * 0.75f);
    QPixmap pixmap(size);
    pixmap.fill(color);
    return pixmap;
}

