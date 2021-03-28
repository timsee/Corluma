/*!
 * \copyright
 * Copyright (C) 2015 - 2020.
 * Released under the GNU General Public License.
 */

#include "rotatelightwidget.h"
#include <QPainter>
#include <QStyleOption>

RotateLightWidget::RotateLightWidget(QWidget* parent)
    : QWidget(parent),
      mButtonOK(new QPushButton("Save", this)),
      mButtonCancel(new QPushButton("Cancel", this)),
      mInstructionLabel{new QLabel(this)},
      mRotationSlider{new cor::Slider(this)},
      mLeafPanelImage{new nano::LeafPanelImage(this)},
      mLightImage{new QLabel(this)},
      mIsOn{false} {
    isOpen(false);
    connect(mButtonOK, SIGNAL(clicked()), this, SLOT(clickedOK()));
    connect(mButtonCancel, SIGNAL(clicked()), this, SLOT(clickedCancel()));
    setVisible(false);

    mRotationSlider->setRange(-240, 239);
    mRotationSlider->setValue(0);
    mRotationSlider->setColor(QColor(255, 127, 0));
    mRotationSlider->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    connect(mRotationSlider, SIGNAL(valueChanged(int)), this, SLOT(sliderChanged(int)));
    connect(mRotationSlider, SIGNAL(sliderReleased()), this, SLOT(releasedSlider()));

    mLightImage->setAlignment(Qt::AlignHCenter | Qt::AlignVCenter);

    mInstructionLabel->setWordWrap(true);
    mInstructionLabel->setText("Rotate the Nanoleafs to their proper orientation.");
}

void RotateLightWidget::setNanoleaf(const nano::LeafMetadata& leaf,
                                    int rotation,
                                    const cor::Palette& palette,
                                    bool isOn) {
    mLeaf = leaf;
    mValue = rotation;
    mPalette = palette;
    mIsOn = isOn;

    // handle rotation input, which is a slider and a label that shows the value.
    bool blocked = mRotationSlider->blockSignals(true);
    mRotationSlider->setValue(rotation);
    mRotationSlider->blockSignals(blocked);

    drawNanoleaf();
}

void RotateLightWidget::sliderChanged(int value) {
    if (value < 0) {
        value += 360;
    }
    mValue = value;
    // drawNanoleaf();
}

void RotateLightWidget::releasedSlider() {
    drawNanoleaf();
}

void RotateLightWidget::drawNanoleaf() {
    // render the image for the panel
    mLeafPanelImage->drawPanels(mLeaf.panels(), mValue, mPalette, mIsOn);
    // draw the image to the panel label
    mPixmap.convertFromImage(mLeafPanelImage->image());
    if (!mPixmap.isNull()) {
        mPixmap = mPixmap.scaled(mLightImage->width(),
                                 mLightImage->height(),
                                 Qt::KeepAspectRatio,
                                 Qt::SmoothTransformation);
        mLightImage->setPixmap(mPixmap);
    }
}

void RotateLightWidget::resizeEvent(QResizeEvent*) {
    resize();
}

void RotateLightWidget::paintEvent(QPaintEvent*) {
    QStyleOption opt;
    opt.initFrom(this);
    QPainter painter(this);

    painter.setRenderHint(QPainter::Antialiasing);
    painter.fillRect(rect(), QBrush(QColor(48, 47, 47)));
}

void RotateLightWidget::pushIn() {
    auto size = cor::applicationSize();
    float sizeRatio = size.width() / float(size.height());
    if (sizeRatio > 1.0f) {
        cor::moveWidget(
            this,
            QPoint(int(parentWidget()->width() * 0.25f), int(-1 * parentWidget()->height())),
            QPoint(int(parentWidget()->width() * 0.25f), int(parentWidget()->height() * 0.25f)));
    } else {
        cor::moveWidget(
            this,
            QPoint(int(-parentWidget()->width() * 0.1f), int(-1 * parentWidget()->height())),
            QPoint(int(parentWidget()->width() * 0.1f), int(parentWidget()->height() * 0.1f)));
    }

    setVisible(true);
    isOpen(true);
}

void RotateLightWidget::pushOut() {
    auto size = cor::applicationSize();
    float sizeRatio = size.width() / float(size.height());
    if (sizeRatio > 1.0f) {
        moveWidget(
            this,
            QPoint(int(parentWidget()->width() * 0.25f), int(parentWidget()->height() * 0.25f)),
            QPoint(int(parentWidget()->width() * 0.25f), int(-1 * parentWidget()->height())));
    } else {
        moveWidget(
            this,
            QPoint(int(parentWidget()->width() * 0.1f), int(parentWidget()->height() * 0.1f)),
            QPoint(int(parentWidget()->width() * 0.1f), int(-1 * parentWidget()->height())));
    }
    setVisible(false);
    isOpen(false);
}

void RotateLightWidget::resize() {
    QSize size = parentWidget()->size();
    auto appSize = cor::applicationSize();
    float sizeRatio = appSize.width() / float(appSize.height());
    if (sizeRatio > 1.0f) {
        setFixedSize(int(size.width() * 0.5f), int(size.height() * 0.5f));
    } else {
        setFixedSize(int(size.width() * 0.8f), int(size.height() * 0.4f));
    }

    int yPos = 0;
    auto buttonHeight = height() / 6;
    mInstructionLabel->setGeometry(0, yPos, width(), mButtonCancel->height());
    yPos += mInstructionLabel->height();

    mRotationSlider->setGeometry(0, yPos, width(), buttonHeight);
    yPos += mRotationSlider->height();

    mLightImage->setGeometry(0, yPos, width(), buttonHeight * 3);
    yPos += mLightImage->height();

    mButtonCancel->setGeometry(0, yPos, width() / 2, buttonHeight);
    mButtonOK->setGeometry(mButtonCancel->width(), yPos, width() / 2, buttonHeight);
}
