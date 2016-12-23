#include "listcontrollerwidget.h"
/*!
 * \copyright
 * Copyright (C) 2015 - 2016.
 * Released under the GNU General Public License.
 */

ListControllerWidget::ListControllerWidget(QWidget *parent) : QWidget(parent)
{
}

void ListControllerWidget::setup(const SLightDevice& device) {
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

    if (device.lightingRoutine <= ELightingRoutineSingleColorEnd ) {
        updateSingleRoutineIcon(device.lightingRoutine, device.color);
    } else {
        throw "WTFF FBROKENBEFOIWJFEW";
    }
}

void ListControllerWidget::setup(const SLightDevice& device, const std::vector<QColor>& colors) {
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

    updateMultiRoutineIcon(device.lightingRoutine, device.colorGroup, colors);

}

void ListControllerWidget::updateColor(QColor color) {
    mIconData.setSolidColor(color);
    mStatusIcon->setPixmap(mIconData.renderAsQPixmap());
}

void ListControllerWidget::updateSingleRoutineIcon(ELightingRoutine routine, QColor color) {
    mIconData.setSingleLightingRoutine(routine, color);
    QPixmap iconRendered = mIconData.renderAsQPixmap();
    mStatusIcon->setPixmap(iconRendered.scaled(mStatusIcon->width(),
                                                mStatusIcon->width(),
                                                Qt::IgnoreAspectRatio,
                                                Qt::FastTransformation));
}

void ListControllerWidget::updateMultiRoutineIcon(ELightingRoutine routine, EColorGroup group, const std::vector<QColor>& colors) {
    mIconData.setMultiLightingRoutine(routine, group, colors);
    QPixmap iconRendered = mIconData.renderAsQPixmap();
    mStatusIcon->setPixmap(iconRendered.scaled(mStatusIcon->width(),
                                                mStatusIcon->width(),
                                                Qt::IgnoreAspectRatio,
                                                Qt::FastTransformation));
}

