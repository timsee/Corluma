/*!
 * \copyright
 * Copyright (C) 2015 - 2019.
 * Released under the GNU General Public License.
 */

#include "colorpicker.h"
#include "utils/color.h"

#include <QDebug>
#include <QSignalMapper>
#include <QGraphicsOpacityEffect>


ColorPicker::ColorPicker(QWidget *parent) :
    QWidget(parent), mCircleIndex{0} {

    mWheelIsEnabled = true;
    mCurrentLayoutColorPicker = ELayoutColorPicker::standardLayout;
    mWheelOpacity = 1.0;

    mPlaceholder = new QWidget(this);
    mPlaceholder->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

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
    connect(mBrightnessSlider, SIGNAL(brightnessChanged(uint32_t)), this, SLOT(brightnessSliderChanged(uint32_t)));
    mBrightnessSlider->setVisible(false);

    mTempBrightSliders = new TempBrightSliders(this);
    mTempBrightSliders->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    connect(mTempBrightSliders, SIGNAL(temperatureAndBrightnessChanged(int,uint32_t)), this, SLOT(tempBrightSlidersChanged(int,uint32_t)));
    mTempBrightSliders->setVisible(false);

    mCustomColorPicker = new  CustomColorPicker(this);
    mCustomColorPicker->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    connect(mCustomColorPicker, SIGNAL(multiColorCountChanged(int)), this, SLOT(multiColorCountChanged(int)));
    connect(mCustomColorPicker->palette(), SIGNAL(selectedCountChanged(int)), this, SLOT(selectedCountChanged(int)));
    mCustomColorPicker->setVisible(false);

    mColorSchemeGrid = new SwatchVectorWidget(5, 1, this);
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
    changeLayout(ELayoutColorPicker::standardLayout);
}

void ColorPicker::RGBSlidersColorChanged(const QColor& color) {
    chooseColor(color);
}

void ColorPicker::brightnessSliderChanged(uint32_t brightness) {
    chooseBrightness(brightness);
}

void ColorPicker::tempBrightSlidersChanged(int temperature, uint32_t brightness) {
    chooseAmbient(temperature, brightness, true);
}

void ColorPicker::multiColorChanged(QColor color, int index) {
    Q_UNUSED(color);
    Q_UNUSED(index);
    emit multiColorUpdate();
}

void ColorPicker::multiColorCountChanged(int count) {
    emit multiColorCountUpdate(count);
}

void ColorPicker::selectedCountChanged(int count) {
    enable(bool(count));
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
    mCustomColorPicker->setVisible(false);
    mColorSchemeGrid->setVisible(false);
    mColorSchemeCircles->setVisible(false);
    if (mCurrentLayoutColorPicker == ELayoutColorPicker::standardLayout) {
        mRGBSliders->setVisible(true);
    } else if (mCurrentLayoutColorPicker == ELayoutColorPicker::ambientLayout) {
        mTempBrightSliders->setVisible(true);
    } else if (mCurrentLayoutColorPicker == ELayoutColorPicker::brightnessLayout) {
        mBrightnessSlider->setVisible(true);
    } else if (mCurrentLayoutColorPicker == ELayoutColorPicker::multiColorLayout) {
        mCustomColorPicker->setVisible(true);
    } else if (mCurrentLayoutColorPicker == ELayoutColorPicker::colorSchemeLayout) {
        mColorSchemeGrid->setVisible(true);
        mColorSchemeCircles->setVisible(true);
    }

    changeColorWheel(oldLayout, layout, skipAnimation);
    resize();
}

void ColorPicker::updateColorStates(const QColor& mainColor,
                                    uint32_t brightness,
                                    const std::vector<QColor>& colorSchemes,
                                    const std::vector<QColor>& customColors) {
    mRGBSliders->changeColor(mainColor);
    mBrightnessSlider->changeBrightness(brightness);

    mColorSchemeCircles->updateColorScheme(colorSchemes);
    mColorSchemeCircles->updateColorCount(uint32_t(colorSchemes.size()));

    mCustomColorPicker->updateMultiColor(customColors);
}

void ColorPicker::setMultiColorDefaults(const std::vector<QColor>& colors) {
    mCustomColorPicker->updateMultiColor(colors);
}

void ColorPicker::updateBottomMenuState(bool enable) {
    if (mCurrentLayoutColorPicker == ELayoutColorPicker::standardLayout) {
        auto effect = new QGraphicsOpacityEffect(mRGBSliders);
        effect->setOpacity(mWheelOpacity);
        mRGBSliders->setGraphicsEffect(effect);
        mRGBSliders->enable(enable);
    } else if (mCurrentLayoutColorPicker == ELayoutColorPicker::ambientLayout) {
        auto effect = new QGraphicsOpacityEffect(mTempBrightSliders);
        effect->setOpacity(mWheelOpacity);
        mTempBrightSliders->setGraphicsEffect(effect);
        mTempBrightSliders->setEnabled(enable);
    } else if (mCurrentLayoutColorPicker == ELayoutColorPicker::brightnessLayout) {
        auto effect = new QGraphicsOpacityEffect(mBrightnessSlider);
        effect->setOpacity(mWheelOpacity);
        mBrightnessSlider->setGraphicsEffect(effect);
        mBrightnessSlider->setEnabled(enable);
    } else if (mCurrentLayoutColorPicker == ELayoutColorPicker::multiColorLayout) {
        auto effect = new QGraphicsOpacityEffect(mCustomColorPicker);
        effect->setOpacity(mWheelOpacity);
        mCustomColorPicker->setGraphicsEffect(effect);
        mCustomColorPicker->setEnabled(enable);
    } else if (mCurrentLayoutColorPicker == ELayoutColorPicker::colorSchemeLayout) {
        auto effect = new QGraphicsOpacityEffect(mColorSchemeGrid);
        effect->setOpacity(mWheelOpacity);
        mColorSchemeGrid->setGraphicsEffect(effect);
        auto effect2 = new QGraphicsOpacityEffect(mColorSchemeCircles);
        effect2->setOpacity(mWheelOpacity);
        mColorSchemeGrid->setGraphicsEffect(effect2);
        mColorSchemeGrid->setEnabled(enable);
        mColorSchemeCircles->setEnabled(enable);
    }
}

void ColorPicker::enable(bool shouldEnable) {
    if (shouldEnable) {
        mWheelOpacity = 1.0;
        // un-fade out the wheel
        auto effect = new QGraphicsOpacityEffect(mColorWheel);
        effect->setOpacity(mWheelOpacity);
        mColorWheel->setGraphicsEffect(effect);
        this->setEnabled(true);
        updateBottomMenuState(true);
    } else if (!shouldEnable) {
        mWheelOpacity = 0.333;
        // fade out the wheel
        auto effect = new QGraphicsOpacityEffect(mColorWheel);
        effect->setOpacity(mWheelOpacity);
        mColorWheel->setGraphicsEffect(effect);
        this->setEnabled(false);
        updateBottomMenuState(false);
    }
    mWheelIsEnabled = shouldEnable;
}

//------------------------------
// Layout-Specific API
//------------------------------

void ColorPicker::chooseColor(const QColor& color, bool shouldSignal) {
    if (shouldSignal) {
        emit colorUpdate(color);
    }
}


void ColorPicker::chooseAmbient(int temperature, uint32_t brightness, bool shouldSignal) {
    if (brightness <= 100
            && temperature >= 153
            && temperature <= 500) {
        if (shouldSignal) {
            emit ambientUpdate(temperature, brightness);
        }
    }
}


void ColorPicker::chooseBrightness(uint32_t brightness, bool shouldSignal) {
    if (brightness <= 100) {
        if (shouldSignal) {
            emit brightnessUpdate(brightness);
        }
    }
}

// ----------------------------
// Slots
// ----------------------------


void ColorPicker::hideTempWheel() {
    mTempWheel->setVisible(false);
}

// ----------------------------
// Protected
// ----------------------------


void ColorPicker::mousePressEvent(QMouseEvent *event) {
    mCircleIndex = mColorSchemeCircles->positionIsUnderCircle(event->pos());
    mPressTime.restart();
    handleMouseEvent(event);
}

void ColorPicker::mouseMoveEvent(QMouseEvent *event) {
    handleMouseEvent(event);
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
            if (mCurrentLayoutColorPicker == ELayoutColorPicker::multiColorLayout) {
                if (mWheelIsEnabled) {
                    mCustomColorPicker->updateSelected(color);
                    emit multiColorUpdate();
                }
            } else if (mCurrentLayoutColorPicker == ELayoutColorPicker::ambientLayout) {
                // use the poorly named "value" of the HSV range to calculate the brightness
                auto brightness = uint32_t(color.valueF() * 100.0);
                // adjust the color so that it has a maxed out value in the HSV colorspace
                color.setHsv(color.hue(),
                             color.saturation(),
                             255);
                // then calculate then use the resulting QColor to convert to color temperature.
                int ct = cor::rgbToColorTemperature(color);
                chooseAmbient(ct, brightness, true);
                mTempBrightSliders->changeTemperatureAndBrightness(ct, brightness);
            } else if (mCurrentLayoutColorPicker == ELayoutColorPicker::brightnessLayout) {
                // use the poorly named "value" of the HSV range to calculate the brightness
                auto brightness = uint32_t(color.valueF() * 100.0);
                chooseBrightness(brightness);
                mBrightnessSlider->changeBrightness(brightness);
            } else if (mCurrentLayoutColorPicker == ELayoutColorPicker::standardLayout) {
                chooseColor(color);
                mRGBSliders->changeColor(color);
            } else if (mCurrentLayoutColorPicker == ELayoutColorPicker::colorSchemeLayout) {
                if (mCircleIndex == -1) {
                    mColorSchemeCircles->moveCenterCircle(event->pos(), true);
                    mCircleIndex = 10;
                } else if (mCircleIndex == 10) {
                    mColorSchemeCircles->moveCenterCircle(event->pos(), false);
                } else {
                    mColorSchemeCircles->moveStandardCircle(uint32_t(mCircleIndex), event->pos());
                }

                // turn into vector of colors
                std::vector<QColor> colors;
                for (const auto& circle : mColorSchemeCircles->circles()) {
                    colors.push_back(circle.color);
                }
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
        double distance = QLineF(event->pos(),
                                QPoint(geometry.x() + geometry.height() / 2,
                                       geometry.y() + geometry.height() / 2)).length();
        distance = distance / this->height();
        if (distance <= 0.29) {
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
    int wheelSize = int(this->size().height() * 0.55f);
    if (wheelSize > this->size().width() * 0.85f) {
        wheelSize = int(this->size().width() * 0.85f);
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

            auto fadeOutEffect = new QGraphicsOpacityEffect(mTempWheel);
            mTempWheel->setGraphicsEffect(fadeOutEffect);
            auto fadeOutAnimation = new QPropertyAnimation(fadeOutEffect, "opacity");
            fadeOutAnimation->setDuration(animationMsec);
            fadeOutAnimation->setStartValue(mWheelOpacity);
            fadeOutAnimation->setEndValue(0.0f);
            connect(fadeOutAnimation, SIGNAL(finished()), this, SLOT(hideTempWheel()));

            auto fadeInEffect = new QGraphicsOpacityEffect(mColorWheel);
            mColorWheel->setGraphicsEffect(fadeInEffect);
            auto fadeInAnimation = new QPropertyAnimation(fadeInEffect, "opacity");
            fadeInAnimation->setDuration(animationMsec);
            fadeInAnimation->setStartValue(0.0f);
            mWheelOpacity = 1.0;
            // catch edge case wehre multi color picker is sometimes disabled by default
            if (newLayout == ELayoutColorPicker::multiColorLayout
                    && (mCustomColorPicker->palette()->selectedCount() == 0)) {
                mWheelOpacity = 0.333;
            }
            fadeInAnimation->setEndValue(mWheelOpacity);

            auto group = new QParallelAnimationGroup;
            group->addAnimation(fadeInAnimation);
            group->addAnimation(fadeOutAnimation);
            group->start(QAbstractAnimation::DeleteWhenStopped);
        } else if (mCustomColorPicker->palette()->selectedCount() == 0
                       && (newLayout == ELayoutColorPicker::multiColorLayout
                           || oldLayout == ELayoutColorPicker::multiColorLayout)
                       && (newLayout != oldLayout)) {
            mTempWheel->setVisible(false);

            // If the resources match but its either changing from or changing to a multi layout
            // and no indices are selected so the multi layout should be opaque.
            float startOpacity;
            // don't need two animations, just do one.
            if (newLayout == ELayoutColorPicker::multiColorLayout) {
                startOpacity = 1.0f;
                mWheelOpacity = 0.333;
            } else if (oldLayout == ELayoutColorPicker::multiColorLayout) {
                startOpacity = 0.333f;
                mWheelOpacity = 1.0;
            } else {
                startOpacity = 0.0f;
                mWheelOpacity = 0.0;
                qDebug() << "WARNING: shouldn't get here...";
            }

            if (oldLayout == ELayoutColorPicker::multiColorLayout) {
                // just set opacity here immediately cause of weird bug...
                mWheelOpacity = 1.0;
                // un-fade out the wheel
                auto effect = new QGraphicsOpacityEffect(mColorWheel);
                effect->setOpacity(mWheelOpacity);
                mColorWheel->setGraphicsEffect(effect);
            } else {
                auto fadeEffect = new QGraphicsOpacityEffect(mColorWheel);
                mColorWheel->setGraphicsEffect(fadeEffect);
                auto fadeAnimation = new QPropertyAnimation(fadeEffect, "opacity");
                fadeAnimation->setDuration(animationMsec);
                fadeAnimation->setStartValue(startOpacity);
                fadeAnimation->setEndValue(mWheelOpacity);
                fadeAnimation->start();
            }

        }
    } else {
        mTempWheel->setVisible(false);
    }

}

bool ColorPicker::checkIfColorIsValid(const QColor& color) {
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

QString ColorPicker::getWheelPixmapPath(ELayoutColorPicker layout) {
    QString name;
    switch (layout)
    {
        case ELayoutColorPicker::standardLayout:
        case ELayoutColorPicker::multiColorLayout:
        case ELayoutColorPicker::colorSchemeLayout:
            name = QString(":/images/color_wheel.png");
            break;
        case ELayoutColorPicker::ambientLayout:
            name = QString(":/images/ambient_wheel.png");
            break;
        case ELayoutColorPicker::brightnessLayout:
            name = QString(":/images/white_wheel.png");
            break;
    }
    return name;
}

void ColorPicker::resize() {
    QPixmap pixmap(getWheelPixmapPath(mCurrentLayoutColorPicker));

    int wheelSize = int(this->size().height() * 0.55f);
    if (wheelSize > this->size().width() * 0.85f) {
        wheelSize = int(this->size().width() * 0.85f);
    }
    mColorWheel->setPixmap(pixmap.scaled(wheelSize,
                                        wheelSize,
                                        Qt::KeepAspectRatio,
                                        Qt::SmoothTransformation));

    if (mCurrentLayoutColorPicker == ELayoutColorPicker::standardLayout) {
        mRGBSliders->setGeometry(mPlaceholder->geometry());
    } else if (mCurrentLayoutColorPicker == ELayoutColorPicker::brightnessLayout) {
        mBrightnessSlider->setGeometry(mPlaceholder->geometry());
    } else if (mCurrentLayoutColorPicker == ELayoutColorPicker::ambientLayout) {
        mTempBrightSliders->setGeometry(mPlaceholder->geometry());
    } else if (mCurrentLayoutColorPicker == ELayoutColorPicker::multiColorLayout) {
        mCustomColorPicker->setGeometry(mPlaceholder->geometry());
        mCustomColorPicker->palette()->updateColors(mCustomColorPicker->palette()->colors());
    } else if (mCurrentLayoutColorPicker == ELayoutColorPicker::colorSchemeLayout) {
        mColorSchemeGrid->setGeometry(mPlaceholder->geometry());
        mColorSchemeCircles->setGeometry(0, 0, this->geometry().width(), this->geometry().height() - mPlaceholder->geometry().height());
    }
}

