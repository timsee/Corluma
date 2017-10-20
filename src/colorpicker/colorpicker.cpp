/*!
 * \copyright
 * Copyright (C) 2015 - 2017.
 * Released under the GNU General Public License.
 */

#include "colorpicker.h"
#include "corlumautils.h"

#include <QDebug>
#include <QSignalMapper>
#include <QGraphicsOpacityEffect>


ColorPicker::ColorPicker(QWidget *parent) :
    QWidget(parent) {

    mWheelIsEnabled = true;

    mWheelOpacity = 1.0f;

    mPlaceholder = new QWidget(this);
    mPlaceholder->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

    // --------------
    // Setup Thrrole Timer
    // --------------

    mThrottleTimer = new QTimer(this);
    connect(mThrottleTimer, SIGNAL(timeout()), this, SLOT(resetThrottleFlag()));

    // --------------
    // Setup ColorWheel
    // --------------
    QPixmap pixmap(":/images/color_wheel.png");

    mColorWheel = new QLabel(this);
    mColorWheel->setPixmap(pixmap);
    mColorWheel->setAlignment(Qt::AlignCenter);
    mColorWheel->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

    // keep scaled downpixmap rendered as QImage for color lookup
    pixmap = pixmap.scaled(255, 255);
    mRenderedColorWheel = pixmap.toImage();

    // --------------
    // Setup Temp Color Wheel (used for transitions)
    // --------------

    mTempWheel = new QLabel(this);
    mTempWheel->setPixmap(QPixmap(":/images/color_wheel.png"));
    mTempWheel->setAlignment(Qt::AlignCenter);
    mTempWheel->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    mTempWheel->setVisible(false);

    // --------------
    // Setup Widgets
    // --------------

    mRGBSliders = new RGBSliders(this);
    mRGBSliders->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    connect(mRGBSliders, SIGNAL(colorChanged(QColor)), this, SLOT(RGBSlidersColorChanged(QColor)));
    mRGBSliders->setVisible(false);

    mBrightnessSlider = new BrightnessSlider(this);
    mBrightnessSlider->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    connect(mBrightnessSlider, SIGNAL(brightnessChanged(int)), this, SLOT(brightnessSliderChanged(int)));
    mBrightnessSlider->setVisible(false);

    mTempBrightSliders = new TempBrightSliders(this);
    mTempBrightSliders->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    connect(mTempBrightSliders, SIGNAL(temperatureAndBrightnessChanged(int, int)), this, SLOT(tempBrightSlidersChanged(int, int)));
    mTempBrightSliders->setVisible(false);

    mColorGrid = new ColorGrid(this);
    mColorGrid->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    connect(mColorGrid, SIGNAL(multiColorCountChanged(int)), this, SLOT(multiColorCountChanged(int)));
    connect(mColorGrid, SIGNAL(selectedCountChanged(int)), this, SLOT(selectedCountChanged(int)));
    mColorGrid->setVisible(false);

    mColorSchemeGrid = new ColorSchemeGrid(this);
    mColorSchemeGrid->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    mColorSchemeGrid->setVisible(false);

    mColorSchemeCircles = new ColorSchemeCircles(mRenderedColorWheel, this);
    mColorSchemeCircles->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    mColorSchemeCircles->setVisible(false);

    // --------------
    // Setup Slider/Label Layout
    // --------------

    mFullLayout = new QVBoxLayout;
    mFullLayout->addWidget(mColorWheel, 6, Qt::AlignCenter);
    mFullLayout->addWidget(mPlaceholder, 4);
    mFullLayout->setSpacing(0);
    mFullLayout->setContentsMargins(0,0,0,0);
    setLayout(mFullLayout);

    // default to standard layout
    changeLayout(ELayoutColorPicker::eStandardLayout);
}

ColorPicker::~ColorPicker() {
}


void ColorPicker::RGBSlidersColorChanged(QColor color) {
    chooseColor(color);
}

void ColorPicker::brightnessSliderChanged(int brightness) {
    chooseBrightness(brightness);
}

void ColorPicker::tempBrightSlidersChanged(int temperature, int brightness) {
    chooseAmbient(temperature, brightness);
}

void ColorPicker::multiColorChanged(QColor color, int index) {
    emit multiColorUpdate(color, index);
}

void ColorPicker::multiColorCountChanged(int count) {
    emit multiColorCountUpdate(count);
}

void ColorPicker::selectedCountChanged(int count) {
    enableWheel((bool)count);
}

void ColorPicker::changeLayout(ELayoutColorPicker layout,  bool skipAnimation) {
    ELayoutColorPicker oldLayout = mCurrentLayoutColorPicker;
    // reset all flags
    mCurrentLayoutColorPicker = layout;

    //---------------------
    // Add new bottom Layout
    //---------------------

    // update the bottom layout
    mRGBSliders->setVisible(false);
    mBrightnessSlider->setVisible(false);
    mTempBrightSliders->setVisible(false);
    mColorGrid->setVisible(false);
    mColorSchemeGrid->setVisible(false);
    mColorSchemeCircles->setVisible(false);
    if (mCurrentLayoutColorPicker == ELayoutColorPicker::eStandardLayout) {
        mRGBSliders->setVisible(true);
        mRGBSliders->setGeometry(mPlaceholder->geometry());
    } else if (mCurrentLayoutColorPicker == ELayoutColorPicker::eAmbientLayout) {
        mTempBrightSliders->setVisible(true);
        mTempBrightSliders->setGeometry(mPlaceholder->geometry());
    } else if (mCurrentLayoutColorPicker == ELayoutColorPicker::eBrightnessLayout) {
        mBrightnessSlider->setVisible(true);
        mBrightnessSlider->setGeometry(mPlaceholder->geometry());
    } else if (mCurrentLayoutColorPicker == ELayoutColorPicker::eMultiColorLayout) {
        mColorGrid->setVisible(true);
        mColorGrid->setGeometry(mPlaceholder->geometry());
    } else if (mCurrentLayoutColorPicker == ELayoutColorPicker::eColorSchemeLayout) {
        mColorSchemeGrid->setVisible(true);
        mColorSchemeCircles->setVisible(true);
        mColorSchemeCircles->setGeometry(0, 0, this->geometry().width(), this->geometry().height() - mPlaceholder->geometry().height());
        mColorSchemeGrid->setGeometry(mPlaceholder->geometry());
    }

    changeColorWheel(oldLayout, layout, skipAnimation);
    resize();
}

void ColorPicker::updateColorStates(QColor mainColor,
                                    int brightness,
                                    const std::vector<QColor> colorArray,
                                    const std::vector<QColor> colorSchemes,
                                    int colorArrayCount) {
    mRGBSliders->changeColor(mainColor);
    mBrightnessSlider->changeBrightness(brightness);
    mColorGrid->updateMultiColor(colorArray, colorArrayCount);

    mColorSchemeCircles->updateColorScheme(colorSchemes);
    mColorSchemeCircles->updateColorCount(colorSchemes.size()),

    mColorSchemeGrid->updateColorCount(colorSchemes.size()),
    mColorSchemeGrid->updateColorScheme(colorSchemes);
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
    if (shouldSignal) {
        emit colorUpdate(color);
    }
}


void ColorPicker::chooseAmbient(int temperature, int brightness, bool shouldSignal) {
    if (brightness >= 0
            && brightness <= 100
            && temperature >= 153
            && temperature <= 500) {
        if (shouldSignal) {
            emit ambientUpdate(temperature, brightness);
        }
    }
}


void ColorPicker::chooseBrightness(int brightness, bool shouldSignal) {
    if (brightness >= 0
            && brightness <= 100) {
        if (shouldSignal) {
            emit brightnessUpdate(brightness);
        }
    }
}

// ----------------------------
// Slots
// ----------------------------

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
    mCircleIndex = mColorSchemeCircles->positionIsUnderCircle(event->pos());
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
//        if (mCurrentLayoutColorPicker == ELayoutColorPicker::eStandardLayout) {
//            chooseColor(QColor(mTopSlider->slider->value(),
//                               mMidSlider->slider->value(),
//                               mBottomSlider->slider->value()));
//        } else if (mCurrentLayoutColorPicker == ELayoutColorPicker::eAmbientLayout) {
//            chooseAmbient(mTopSlider->slider->value(),
//                          mMidSlider->slider->value());
//        } else if (mCurrentLayoutColorPicker == ELayoutColorPicker::eMultiColorLayout) {
//            // Do nothing...
//        }
    }
}

void ColorPicker::resizeEvent(QResizeEvent *event) {
    Q_UNUSED(event);
    resize();
}


//------------------------------
// Mouse Events
//------------------------------

void ColorPicker::handleMouseEvent(QMouseEvent *event) {
    QRect geometry = mColorWheel->geometry();

    if (eventIsOverWheel(event)) {
        QPixmap pixmap = QWidget::grab(mColorWheel->geometry());
        // some mobile and other environments save pixmaps without pixel density taken into account. For example, breaks on iPhone 6 without this.
        if (pixmap.size() != mColorWheel->geometry().size()) {
            pixmap = pixmap.scaled(mColorWheel->geometry().size().width(),
                                   mColorWheel->geometry().size().height());
        }
        QColor color = pixmap.toImage().pixel(event->pos().x() - geometry.x(),
                                              event->pos().y() - geometry.y());
        if (checkIfColorIsValid(color)){
            if (mCurrentLayoutColorPicker == ELayoutColorPicker::eMultiColorLayout) {
                if (mWheelIsEnabled) {
                    mColorGrid->updateSelected(color);
                    for (auto&& index : mColorGrid->selected()) {
                        emit multiColorChanged(color, index);
                    }
                    //updateMultiColor(mMultiColors, mMultiUsed);
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
                mTempBrightSliders->changeTemperatureAndBrightness(ct, brightness);
            } else if (mCurrentLayoutColorPicker == ELayoutColorPicker::eBrightnessLayout) {
                // use the poorly named "value" of the HSV range to calculate the brightness
                int brightness = color.valueF() * 100.0f;
                chooseBrightness(brightness);
                mBrightnessSlider->changeBrightness(brightness);
            } else if (mCurrentLayoutColorPicker == ELayoutColorPicker::eStandardLayout) {
                chooseColor(color);
                mRGBSliders->changeColor(color);
            } else if (mCurrentLayoutColorPicker == ELayoutColorPicker::eColorSchemeLayout) {
                if (mCircleIndex == -1) {
                    mColorSchemeCircles->moveCenterCircle(event->pos(), true);
                    mCircleIndex = 10;
                } else if (mCircleIndex == 10) {
                    mColorSchemeCircles->moveCenterCircle(event->pos(), false);
                } else {
                    mColorSchemeCircles->moveStandardCircle(mCircleIndex, event->pos());
                }

                std::vector<SPickerSelection> circles = mColorSchemeCircles->circles();
                // turn into vector of colors
                std::vector<QColor> colors;
                for (auto&& circle : circles) {
                    colors.push_back(circle.color);
                }
                mColorSchemeGrid->updateColorScheme(colors);
                emit colorsUpdate(colors);
               // mColorSchemeGrid->setColor(2, color);
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
        // check that the normalized distance is correct for the circle
        float distance = QLineF(event->pos(), QPoint(geometry.x() + geometry.height() / 2, geometry.y() + geometry.height() / 2)).length();
        distance = distance / this->height();
        if (distance <= 0.29f) {
            return true;
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
                    && (mColorGrid->selectedCount() == 0)) {
                mWheelOpacity = 0.333f;
            }
            fadeInAnimation->setEndValue(mWheelOpacity);

            QParallelAnimationGroup *group = new QParallelAnimationGroup;
            group->addAnimation(fadeInAnimation);
            group->addAnimation(fadeOutAnimation);
            group->start(QAbstractAnimation::DeleteWhenStopped);
        } else if (mColorGrid->selectedCount() == 0
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
        if (mColorGrid->selectedCount() == 0
                && (newLayout == ELayoutColorPicker::eMultiColorLayout)) {
            shouldEnable = false;
        }
        if (newLayout == ELayoutColorPicker::eMultiColorLayout) {
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
        case ELayoutColorPicker::eColorSchemeLayout:
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

    int wheelSize = this->size().height() * 0.55f;
    if (wheelSize > this->size().width() * 0.85f) {
        wheelSize = this->size().width() * 0.85f;
    }
    mColorWheel->setPixmap(pixmap.scaled(wheelSize,
                                        wheelSize,
                                        Qt::KeepAspectRatio,
                                        Qt::SmoothTransformation));

    if (mCurrentLayoutColorPicker == ELayoutColorPicker::eStandardLayout) {
        mRGBSliders->setGeometry(mPlaceholder->geometry());
    } else if (mCurrentLayoutColorPicker == ELayoutColorPicker::eBrightnessLayout) {
        mBrightnessSlider->setGeometry(mPlaceholder->geometry());
    } else if (mCurrentLayoutColorPicker == ELayoutColorPicker::eAmbientLayout) {
        mTempBrightSliders->setGeometry(mPlaceholder->geometry());
    } else if (mCurrentLayoutColorPicker == ELayoutColorPicker::eMultiColorLayout) {
        mColorGrid->setGeometry(mPlaceholder->geometry());
    } else if (mCurrentLayoutColorPicker == ELayoutColorPicker::eColorSchemeLayout) {
        mColorSchemeGrid->setGeometry(mPlaceholder->geometry());
        mColorSchemeCircles->setGeometry(0, 0, this->geometry().width(), this->geometry().height() - mPlaceholder->geometry().height());
    }
}

