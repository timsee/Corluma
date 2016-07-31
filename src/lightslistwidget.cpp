#include "lightslistwidget.h"
/*!
 * \copyright
 * Copyright (C) 2015 - 2016.
 * Released under the GNU General Public License.
 */

LightsListWidget::LightsListWidget(QWidget *parent) : QWidget(parent)
{

}

void LightsListWidget::setup(QString name, bool isOn, bool isReachable, QColor color, int index, DataLayer *data) {

    mIconData = new IconData(32, 32, data);
    mStatusIcon = new QLabel(this);
    if (!isReachable) {
        mIconData->setSolidColor(QColor(40, 40, 40));
        mStatusIcon->setPixmap(mIconData->renderAsQPixmap());
    } else if (!isOn) {
        mIconData->setSolidColor(QColor(0, 0, 0));
        mStatusIcon->setPixmap(mIconData->renderAsQPixmap());
    } else {
        mIconData->setSolidColor(color);
        mStatusIcon->setPixmap(mIconData->renderAsQPixmap());
    }
    mStatusIcon->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    mStatusIcon->setStyleSheet("background:transparent;");

    mController = new QLabel(this);
    mController->setText(name);
    mController->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    mController->setFont(QFont(mController->font().styleName(), 14, 0));
    if(!isReachable) {
        mController->setStyleSheet("background:transparent; font: bold; color: #333;");
    } else {
        mController->setStyleSheet("background:transparent; font: bold;");
    }

    if (index > 0) {
        mIndex = new QLabel(this);
        mIndex->setText(QString::number(index));
        mIndex->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
        mIndex->setFont(QFont(mIndex->font().styleName(), 14, 0));
        if(!isReachable) {
            mIndex->setStyleSheet("background:transparent; font: bold; color: #333;");
        } else {
            mIndex->setStyleSheet("background:transparent; font: bold;");
        }
    }

    mLayout = new QHBoxLayout(this);
    mLayout->addWidget(mStatusIcon);
    mLayout->addWidget(mController);
    if (index > 0) {
        mLayout->addWidget(mIndex);
    }
    //mLayout->setSpacing(0);
    mLayout->setContentsMargins(0,0,0,0);
    setLayout(mLayout);
}

void LightsListWidget::updateColor(QColor color) {
    mIconData->setSolidColor(color);
    mStatusIcon->setPixmap(mIconData->renderAsQPixmap());
}

void LightsListWidget::updateIcon(QColor color, ELightingRoutine routine, EColorGroup group) {
    if ((int)routine <= (int)ELightingRoutine::eSingleSineFade) {
        mIconData->setSolidColor(color);
    } else {
        mIconData->setLightingRoutine(routine, group);
    }
    mStatusIcon->setPixmap(mIconData->renderAsQPixmap());
}
