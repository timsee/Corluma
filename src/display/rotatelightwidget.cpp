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
      mButtonCancel(new QPushButton("X", this)),
      mInstructionLabel{new QLabel(this)},
      mLeafPanelImage{new nano::LeafPanelImage(this)},
      mLightImage{new QLabel(this)} {
    connect(mButtonOK, SIGNAL(clicked()), this, SLOT(clickedOK()));
    connect(mButtonCancel, SIGNAL(clicked()), this, SLOT(clickedCancel()));
    setVisible(false);

    mInstructionLabel->setWordWrap(true);
    mInstructionLabel->setText("Rotate the Nanoleafs to their proper orientation.");
}

void RotateLightWidget::setNanoleaf(const nano::LeafMetadata& leaf, int rotation) {
    mLeaf = leaf;
    mValue = rotation;
    drawNanoleaf();
}

void RotateLightWidget::drawNanoleaf() {
    // render the image for the panel
    mLeafPanelImage->drawPanels(mLeaf.panels(), mValue);
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
    opt.init(this);
    QPainter painter(this);

    painter.setRenderHint(QPainter::Antialiasing);
    painter.fillRect(rect(), QBrush(QColor(48, 47, 47)));
}

void RotateLightWidget::mousePressEvent(QMouseEvent* event) {
    handleMouseEvent(event);
}

void RotateLightWidget::mouseMoveEvent(QMouseEvent* event) {
    handleMouseEvent(event);
}

void RotateLightWidget::handleMouseEvent(QMouseEvent* event) {
    if ((event->x() > 0) && (event->x() < width()) && (event->y() > 0) && (event->y() < height())) {
        auto centerPoint = QPoint(width() / 2, height() / 2);
        auto line = QLineF(centerPoint, event->pos());
        auto angle = line.angle();
        mValue = angle;
        drawNanoleaf();
    }
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
    auto buttonHeight = height() / 5;
    mButtonCancel->setGeometry(0, yPos, width() / 6, buttonHeight);
    auto instructionLabelWidth = width() - mButtonCancel->width() * 1.05;
    mInstructionLabel->setGeometry(mButtonCancel->width(),
                                   yPos,
                                   instructionLabelWidth,
                                   mButtonCancel->height());
    yPos += mButtonCancel->height();

    mLightImage->setGeometry(0, yPos, width(), buttonHeight * 3);
    yPos += mLightImage->height();

    mButtonOK->setGeometry(0, yPos, width(), buttonHeight);
}
