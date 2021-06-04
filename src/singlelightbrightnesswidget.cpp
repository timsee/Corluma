/*!
 * \copyright
 * Copyright (C) 2015 - 2020.
 * Released under the GNU General Public License.
 */

#include "singlelightbrightnesswidget.h"

#include <QGraphicsEffect>
#include <QGraphicsScene>
#include <QMouseEvent>
#include <QPainter>
#include <QStyleOption>

#include "utils/qt.h"


SingleLightBrightnessWidget::SingleLightBrightnessWidget(const QSize& size, QWidget* parent)
    : QWidget(parent),
      mIsIn{false},
      mSize{size} {
    // --------------
    // Setup Brightness Slider
    // --------------
    // setup the slider that controls the LED's brightness
    mBrightnessSlider = new cor::Slider(this);
    mBrightnessSlider->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    mBrightnessSlider->setRange(2, 100);
    mBrightnessSlider->setValue(2);
    mBrightnessSlider->setHeightPercentage(0.8f);
    mBrightnessSlider->setColor(QColor(255, 255, 255));
    mBrightnessSlider->enable(true);
    connect(mBrightnessSlider, SIGNAL(valueChanged(int)), this, SLOT(brightnessSliderChanged(int)));
}

void SingleLightBrightnessWidget::updateColor(const QColor& color) {
    bool rerender = false;
    if (mBrightnessSlider->color() != color) {
        rerender = true;
        mBrightnessSlider->setColor(color);
        auto brightness = color.valueF() * 100.0;
        if (int(brightness) != mBrightnessSlider->value()) {
            mBrightnessSlider->blockSignals(true);
            mBrightnessSlider->setValue(int(brightness));
            mBrightnessSlider->blockSignals(false);
        }
    }
    if (rerender) {
        update();
    }
}

void SingleLightBrightnessWidget::updateBrightness(std::uint32_t brightness) {
    auto color = mBrightnessSlider->color();
    color.setHsvF(color.hueF(), color.saturationF(), brightness / 100.0);
    updateColor(color);
}


void SingleLightBrightnessWidget::pushIn(const QPoint& point) {
    if (!mIsIn) {
        cor::moveWidget(this, geometry().topLeft(), point);
        raise();
        setVisible(true);
    }
    mIsIn = true;
}

void SingleLightBrightnessWidget::pushOut(const QPoint& point) {
    if (mIsIn) {
        cor::moveWidget(this, geometry().topLeft(), point);
    }
    mIsIn = false;
}

void SingleLightBrightnessWidget::resizeEvent(QResizeEvent*) {
    mBrightnessSlider->setGeometry(mSize.width(), 0, width() - mSize.width(), height());
}

void SingleLightBrightnessWidget::brightnessSliderChanged(int newBrightness) {
    auto color = mBrightnessSlider->color();
    color.setHsvF(color.hueF(), color.saturationF(), newBrightness / 100.0);
    updateColor(color);
    emit brightnessChanged(std::uint32_t(newBrightness));
}

void SingleLightBrightnessWidget::paintEvent(QPaintEvent*) {
    QStyleOption opt;
    opt.initFrom(this);
    QPainter painter(this);
    QBrush brush(mBrightnessSlider->color());
    painter.setBrush(brush);
    painter.setRenderHint(QPainter::Antialiasing, true);

    auto side = std::min(mSize.width(), height()) * 0.8;
    auto xSpacer = (mSize.width() - side) / 2;
    auto ySpacer = (height() - side) / 2;
    QRect rect(xSpacer, ySpacer, side, side);
    painter.drawEllipse(rect);
}
