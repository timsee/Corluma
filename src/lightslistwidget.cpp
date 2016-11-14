#include "lightslistwidget.h"
/*!
 * \copyright
 * Copyright (C) 2015 - 2016.
 * Released under the GNU General Public License.
 */

LightsListWidget::LightsListWidget(QWidget *parent) : QWidget(parent)
{
}

void LightsListWidget::setup(const SLightDevice& device, DataLayer *data) {

    // setup icon
    mIconData = new IconData(256, 256, data);
    mStatusIcon = new QLabel(this);
    mStatusIcon->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    QString reachableStlyeSheet = "background:rgba(0, 0, 0, 0%); font: bold; color: #333;";
    QString backgroundStyleSheet = "background:rgba(0, 0, 0, 0%); font: bold;";

#ifndef MOBILE_BUILD
    mStatusIcon->setMaximumWidth(mStatusIcon->height());
#endif
    mStatusIcon->setStyleSheet(backgroundStyleSheet);

    // setup main label
    mController = new QLabel(this);
    if (device.type == ECommType::eHue) {
        mController->setText("Hue Color Lamp");
    } else {
        mController->setText(device.name);
    }
    mController->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    if(!device.isReachable) {
        mController->setStyleSheet(reachableStlyeSheet);
    } else {
        mController->setStyleSheet(backgroundStyleSheet);
    }

    // setup index label
    if (device.index > 0) {
        mIndex = new QLabel(this);
        mIndex->setText(QString::number(device.index));
        mIndex->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
        if(!device.isReachable) {
            mIndex->setStyleSheet(reachableStlyeSheet);
        } else {
            mIndex->setStyleSheet(backgroundStyleSheet);
        }
    }

    // setup layout
    mLayout = new QHBoxLayout(this);
    mLayout->addWidget(mStatusIcon);
    mLayout->addWidget(mController);
    if (device.index > 0) {
        mLayout->addWidget(mIndex);
    }
    mLayout->setContentsMargins(5,5,5,5);
    setLayout(mLayout);
    this->setStyleSheet(backgroundStyleSheet);

    mLayout->setStretch(0, 1);
    mLayout->setStretch(1, 10);
    mLayout->setStretch(2, 1);

    updateIcon(device);
}

void LightsListWidget::updateColor(QColor color) {
    mIconData->setSolidColor(color);
    mStatusIcon->setPixmap(mIconData->renderAsQPixmap());
}

void LightsListWidget::updateIcon(const SLightDevice& device) {
    if (device.lightingRoutine <= ELightingRoutine::eOff
            || !device.isOn
            || !device.isReachable
            || !device.isValid) {
        mIconData->setSolidColor(QColor(0,0,0));
    } else if (device.lightingRoutine <= ELightingRoutine::eSingleSineFade) {
        mIconData->setSolidColor(device.color);
    } else {
        mIconData->setLightingRoutine(device.lightingRoutine, device.colorGroup);
    }
    QPixmap iconRendered = mIconData->renderAsQPixmap();
    mStatusIcon->setPixmap(iconRendered.scaled(mStatusIcon->width(),
                                                mStatusIcon->width(),
                                                Qt::IgnoreAspectRatio,
                                                Qt::FastTransformation)
);
}
