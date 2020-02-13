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


SingleLightBrightnessWidget::SingleLightBrightnessWidget(const QSize& size,
                                                         bool isLeftAlwaysOpen,
                                                         QWidget* parent)
    : QWidget(parent),
      mIsIn{false},
      mSize{size},
      mIsLeftAlwaysOpen{isLeftAlwaysOpen} {
    // --------------
    // Setup Brightness Slider
    // --------------
    // setup the slider that controls the LED's brightness
    mBrightnessSlider = new cor::Slider(this);
    mBrightnessSlider->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    mBrightnessSlider->slider()->setRange(2, 100);
    mBrightnessSlider->slider()->setValue(2);
    mBrightnessSlider->setFixedHeight(size.height() / 2);
    mBrightnessSlider->setHeightPercentage(0.8f);
    mBrightnessSlider->setColor(QColor(255, 255, 255));
    mBrightnessSlider->enable(true);
    connect(mBrightnessSlider, SIGNAL(valueChanged(int)), this, SLOT(brightnessSliderChanged(int)));

    if (isLeftAlwaysOpen) {
        mPositionX = int(mSize.width() * 0.1);
    } else {
        mPositionX = int(mSize.width());
    }
    mTopSpacer = mSize.height() / 8;
    resize();
}

void SingleLightBrightnessWidget::updateColor(const QColor& color) {
    bool rerender = false;
    if (mBrightnessSlider->color() != color) {
        rerender = true;
        mBrightnessSlider->setColor(color);
        auto brightness = color.valueF() * 100.0;
        if (int(brightness) != mBrightnessSlider->slider()->value()) {
            mBrightnessSlider->blockSignals(true);
            mBrightnessSlider->slider()->setValue(int(brightness));
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


void SingleLightBrightnessWidget::pushIn() {
    if (!mIsIn) {
        cor::moveWidget(this,
                        QPoint(mPositionX, int(-1 * height())),
                        QPoint(mPositionX, mTopSpacer));
        raise();
        setVisible(true);
    }
    mIsIn = true;
}

void SingleLightBrightnessWidget::pushOut() {
    if (mIsIn) {
        cor::moveWidget(this,
                        QPoint(mPositionX, mTopSpacer),
                        QPoint(mPositionX, int(-1 * height())));
    }
    mIsIn = false;
}

void SingleLightBrightnessWidget::resize() {
    //  handle global size
    if (mIsIn) {
        this->setGeometry(mPositionX,
                          mTopSpacer,
                          this->parentWidget()->width() - mSize.width(),
                          mSize.height() - mTopSpacer);
    } else {
        this->setGeometry(mPositionX,
                          int(-1 * height()),
                          this->parentWidget()->width() - mSize.width(),
                          mSize.height() - mTopSpacer);
    }

    // handle individual widget sizes
    if (mIsLeftAlwaysOpen) {
        mBrightnessSlider->setGeometry(mSize.width() + 5,
                                       0,
                                       width() - int(mSize.width() * 2),
                                       height());

    } else {
        mBrightnessSlider->setGeometry(mSize.width() + 5,
                                       0,
                                       width() - int(mSize.width() * 1.2),
                                       height());
    }
}

void SingleLightBrightnessWidget::brightnessSliderChanged(int newBrightness) {
    auto color = mBrightnessSlider->color();
    color.setHsvF(color.hueF(), color.saturationF(), newBrightness / 100.0);
    updateColor(color);
    // qDebug() << " new brightness amde " << newBrightness;
    emit brightnessChanged(std::uint32_t(newBrightness));
}

void SingleLightBrightnessWidget::paintEvent(QPaintEvent*) {
    QStyleOption opt;
    opt.init(this);
    QPainter painter(this);
    QBrush brush(mBrightnessSlider->color());
    painter.setBrush(brush);
    painter.setRenderHint(QPainter::Antialiasing, true);

    QRect rect(mSize.width() - height() / 2, height() * 0.05, height() / 2, height() / 2);
    painter.drawEllipse(rect);
}
