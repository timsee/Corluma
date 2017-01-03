#include <QGraphicsEffect>

#include "listdevicewidget.h"

/*!
 * \copyright
 * Copyright (C) 2015 - 2016.
 * Released under the GNU General Public License.
 */


ListDeviceWidget::ListDeviceWidget(const SLightDevice& device,
                                           const QString& name,
                                           QWidget *parent) : QWidget(parent) {
    init(device, name);
    if (device.lightingRoutine <= utils::ELightingRoutineSingleColorEnd ) {
        mIconData.setSingleLightingRoutine(device.lightingRoutine, device.color);
        QPixmap iconRendered = mIconData.renderAsQPixmap();
        mStatusIcon->setPixmap(iconRendered.scaled(mStatusIcon->width(),
                                                    mStatusIcon->width(),
                                                    Qt::IgnoreAspectRatio,
                                                    Qt::FastTransformation));
    } else {
        throw "Wrong Parameters for ListDeviceWidget";
    }

    if (!device.isOn) {
        QGraphicsOpacityEffect *effect = new QGraphicsOpacityEffect(mStatusIcon);
        effect->setOpacity(0.25f);
        mStatusIcon->setGraphicsEffect(effect);
    }
}


ListDeviceWidget::ListDeviceWidget(const SLightDevice& device,
                                           const QString& name,
                                           const std::vector<QColor>& colors,
                                           QWidget *parent) : QWidget(parent) {
    init(device, name);
    mIconData.setMultiLightingRoutine(device.lightingRoutine, device.colorGroup, colors);
    QPixmap iconRendered = mIconData.renderAsQPixmap();
    mStatusIcon->setPixmap(iconRendered.scaled(mStatusIcon->width(),
                                                mStatusIcon->width(),
                                                Qt::IgnoreAspectRatio,
                                                Qt::FastTransformation));
    if (!device.isOn) {
        QGraphicsOpacityEffect *effect = new QGraphicsOpacityEffect(mStatusIcon);
        effect->setOpacity(0.25f);
        mStatusIcon->setGraphicsEffect(effect);
    }
}


void ListDeviceWidget::init(const SLightDevice& device, const QString& name) {
    // setup icon
    mIconData = IconData(256, 256);
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
    mController->setText(name);

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
}
