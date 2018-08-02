/*!
 * \copyright
 * Copyright (C) 2015 - 2018.
 * Released under the GNU General Public License.
 */

#include "huegroupwidget.h"

#include <QtCore>
#include <QtGui>
#include <QStyleOption>
#include <QGraphicsOpacityEffect>

namespace hue
{

HueGroupWidget::HueGroupWidget(QWidget *parent, cor::LightGroup group) : QWidget(parent) {
    const QString styleSheet = "background-color: rgba(0,0,0,0);";
    this->setStyleSheet(styleSheet);

    mName = new QLabel("<b>Name:</b> " + group.name, this);
    mName->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    mName->setAttribute(Qt::WA_TransparentForMouseEvents, true);

    mIndex = new QLabel("<b>Index:</b> " + QString::number(group.index), this);
    mIndex->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    mIndex->setAttribute(Qt::WA_TransparentForMouseEvents, true);

    mDeviceCount = new QLabel("<b>Hue Count:</b> " + QString::number(group.devices.size()), this);
    mDeviceCount->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    mDeviceCount->setAttribute(Qt::WA_TransparentForMouseEvents, true);

    if (group.isRoom) {
        mGroupType = new QLabel("<i>Room</i>", this);
    } else {
        mGroupType = new QLabel("<i>Group</i>", this);
    }
    mGroupType->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    mGroupType->setAttribute(Qt::WA_TransparentForMouseEvents, true);

    mLayout = new QVBoxLayout(this);
    mLayout->addWidget(mName);
    mLayout->addWidget(mIndex);
    mLayout->addWidget(mDeviceCount);
    mLayout->addWidget(mGroupType);
}

void HueGroupWidget::paintEvent(QPaintEvent *) {
    QStyleOption opt;
    opt.init(this);
    QPainter painter(this);

    painter.setRenderHint(QPainter::Antialiasing);
    painter.fillRect(this->rect(), QBrush(QColor(32, 31, 31)));
}

}
