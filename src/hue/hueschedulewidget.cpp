/*!
 * \copyright
 * Copyright (C) 2015 - 2019.
 * Released under the GNU General Public License.
 */

#include "hueschedulewidget.h"
#include <QtCore>
#include <QtGui>
#include <QStyleOption>
#include <QGraphicsOpacityEffect>

namespace hue
{

HueScheduleWidget::HueScheduleWidget(QWidget *parent, SHueSchedule schedule) : QWidget(parent), mSchedule(schedule) {
    const QString styleSheet = "background-color: rgba(0,0,0,0);";
    this->setStyleSheet(styleSheet);

    mNameLabel = new QLabel("<b>Name:</b> " + schedule.name, this);
    mNameLabel->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    mNameLabel->setAttribute(Qt::WA_TransparentForMouseEvents, true);

    //-------------
    // Left of widget
    //-------------
    mTimeLabel = new QLabel("<b>Time:</b> " + schedule.time, this);
    mTimeLabel->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    mTimeLabel->setAttribute(Qt::WA_TransparentForMouseEvents, true);

    if (schedule.status) {
        mStatusLabel = new QLabel("<b>Status:</b> enabled", this);
    } else {
        mStatusLabel = new QLabel("<b>Status:</b> disabled", this);
    }
    mStatusLabel->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    mStatusLabel->setAttribute(Qt::WA_TransparentForMouseEvents, true);

    mLeftLayout = new QVBoxLayout;
    mLeftLayout->addWidget(mTimeLabel);
    mLeftLayout->addWidget(mStatusLabel);

    //-------------
    // Right of widget
    //-------------
    mIndexLabel = new QLabel("<b>Index:</b> " + QString::number(schedule.index), this);
    mIndexLabel->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    mIndexLabel->setAttribute(Qt::WA_TransparentForMouseEvents, true);

    if (schedule.autodelete) {
        mAutoDeleteLabel = new QLabel("<b>Autodelete:</b> On", this);

    } else {
        mAutoDeleteLabel = new QLabel("<b>Autodelete:</b> Off", this);
    }
    mAutoDeleteLabel->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    mAutoDeleteLabel->setAttribute(Qt::WA_TransparentForMouseEvents, true);

    mRightLayout = new QVBoxLayout;
    mRightLayout->addWidget(mIndexLabel);
    mRightLayout->addWidget(mAutoDeleteLabel);

    mBottomLayout = new QHBoxLayout;
    mBottomLayout->addLayout(mLeftLayout);
    mBottomLayout->addLayout(mRightLayout);

    mMainLayout = new QVBoxLayout(this);
    mMainLayout->addWidget(mNameLabel);
    mMainLayout->addLayout(mBottomLayout, 3);
}


void HueScheduleWidget::paintEvent(QPaintEvent *) {
    QStyleOption opt;
    opt.init(this);
    QPainter painter(this);

    painter.setRenderHint(QPainter::Antialiasing);
    painter.fillRect(this->rect(), QBrush(QColor(32, 31, 31)));
}


}
