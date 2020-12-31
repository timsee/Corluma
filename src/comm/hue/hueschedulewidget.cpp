/*!
 * \copyright
 * Copyright (C) 2015 - 2020.
 * Released under the GNU General Public License.
 */

#include "hueschedulewidget.h"

#include <QGraphicsOpacityEffect>
#include <QStyleOption>
#include <QtCore>
#include <QtGui>

namespace {
const static QString kTimeoutString = "Corluma_timeout_";
}

namespace hue {

HueScheduleWidget::HueScheduleWidget(QWidget* parent, hue::Schedule schedule)
    : QWidget(parent),
      mSchedule(schedule),
      mNameLabel{new QLabel(this)},
      mTimeLabel{new QLabel(this)},
      mStatusLabel{new QLabel(this)},
      mIndexLabel{new QLabel(this)},
      mAutoDeleteLabel{new QLabel(this)},
      mIsTimeout{schedule.name().contains(kTimeoutString)} {
    const QString styleSheet = "background-color: rgba(0,0,0,0);";
    setStyleSheet(styleSheet);

    mNameLabel->setAttribute(Qt::WA_TransparentForMouseEvents, true);
    mTimeLabel->setAttribute(Qt::WA_TransparentForMouseEvents, true);
    mStatusLabel->setAttribute(Qt::WA_TransparentForMouseEvents, true);
    mIndexLabel->setAttribute(Qt::WA_TransparentForMouseEvents, true);
    mAutoDeleteLabel->setAttribute(Qt::WA_TransparentForMouseEvents, true);

    updateWidget(schedule);
}

void HueScheduleWidget::updateWidget(const hue::Schedule& schedule) {
    if (mIsTimeout) {
        mTimeLabel->setVisible(true);
        mStatusLabel->setVisible(false);
        mIndexLabel->setVisible(false);
        mAutoDeleteLabel->setVisible(false);
        auto lightIndex = schedule.name().mid(kTimeoutString.size());
        mNameLabel->setText("<b>Timeout for Light Index: </b>" + lightIndex);
        mTimeLabel->setText("<b>Minutes Until Timeout: </b> "
                            + QString::number(schedule.secondsUntilTimeout() / 60));

    } else {
        mNameLabel->setText("<b>Name:</b> " + schedule.name());

        mTimeLabel->setVisible(true);
        mStatusLabel->setVisible(true);
        mIndexLabel->setVisible(true);
        mAutoDeleteLabel->setVisible(true);

        mTimeLabel->setText("<b>Time:</b> " + schedule.time());
        mStatusLabel->setText(schedule.status() ? "<b>Status:</b> enabled"
                                                : "<b>Status:</b> disabled");
        mIndexLabel->setText("<b>Index:</b> " + QString::number(schedule.index()));
        mAutoDeleteLabel->setText(schedule.autodelete() ? "<b>Autodelete:</b> On"
                                                        : "<b>Autodelete:</b> Off");
    }
}


void HueScheduleWidget::resize() {
    auto yPosColumn1 = 0u;
    auto yPosColumn2 = 0u;
    auto rowHeight = height() / 5;
    auto columnWidth = width() / 2;

    mNameLabel->setGeometry(0, yPosColumn1, width(), rowHeight);
    yPosColumn1 += mNameLabel->height();
    yPosColumn2 += mNameLabel->height();

    mTimeLabel->setGeometry(0, yPosColumn1, columnWidth, rowHeight * 2);
    yPosColumn1 += mTimeLabel->height();
    mStatusLabel->setGeometry(0, yPosColumn1, columnWidth, rowHeight * 2);
    yPosColumn1 += mStatusLabel->height();

    mIndexLabel->setGeometry(columnWidth, yPosColumn2, columnWidth, rowHeight * 2);
    yPosColumn2 += mIndexLabel->height();
    mAutoDeleteLabel->setGeometry(columnWidth, yPosColumn2, columnWidth, rowHeight * 2);
    yPosColumn2 += mAutoDeleteLabel->height();
}

void HueScheduleWidget::resizeEvent(QResizeEvent*) {
    resize();
}

void HueScheduleWidget::paintEvent(QPaintEvent*) {
    QStyleOption opt;
    opt.initFrom(this);
    QPainter painter(this);

    painter.setRenderHint(QPainter::Antialiasing);
    painter.fillRect(rect(), QBrush(QColor(32, 31, 31)));

    // draw line at bottom of widget
    auto lineOffset = 3;
    QRect area(x(), y(), width(), height());
    QPainter linePainter(this);
    linePainter.setRenderHint(QPainter::Antialiasing);
    linePainter.setBrush(QBrush(QColor(255, 255, 255)));
    QLine spacerLine(QPoint(area.x(), area.height() - lineOffset),
                     QPoint(area.width(), area.height() - lineOffset));
    linePainter.drawLine(spacerLine);
}


} // namespace hue
