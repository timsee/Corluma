#ifndef LEAFSCHEDULEWIDGET_H
#define LEAFSCHEDULEWIDGET_H
#include <QLabel>
#include <QLayout>
#include <QPainter>
#include <QStyleOption>
#include <QWidget>
#include "comm/nanoleaf/leafschedule.h"
#include "cor/widgets/listitemwidget.h"

namespace nano {
/*!
 * \copyright
 * Copyright (C) 2015 - 2020.
 * Released under the GNU General Public License.
 */

/*!
 * \brief The LeafScheduleWidget class is a simple widget that displays information
 *        about a Leaf schedule. This widget is meant to be in displayed in a list
 *        in a DisplayNanoleafSchedulesWidget
 */
class LeafScheduleWidget : public cor::ListItemWidget {
    Q_OBJECT
public:
    /// constructor
    explicit LeafScheduleWidget(QWidget* parent, nano::LeafSchedule schedule, bool isOn)
        : cor::ListItemWidget(QString::number(schedule.ID()), parent),
          mSchedule{schedule},
          mNameLabel{new QLabel(this)},
          mExecutionTime{new QLabel(this)} {
        updateSchedule(schedule, isOn);
    }

    /// update the schedule
    void updateSchedule(const nano::LeafSchedule& schedule, bool isOn) {
        if (schedule.ID() == kTimeoutID) {
            mNameLabel->setText("<b>Timeout Schedule</b>");
            if (isOn) {
                mExecutionTime->setText("<b>Minutes Until:</b> "
                                        + QString::number(schedule.secondsUntilExecution() / 60));
            } else {
                mExecutionTime->setText("Disabled, Light is off.");
            }
        } else {
            mNameLabel->setText(QString("Schedule: " + QString::number(schedule.ID())));
            mExecutionTime->setText("<b>Time:</b> " + schedule.startDate().toString());
        }
    }

    /// getter for schedule displayed by widget.
    const nano::LeafSchedule& schedule() const noexcept { return mSchedule; }

protected:
    /// paints the background
    void paintEvent(QPaintEvent*) {
        QStyleOption opt;
        opt.init(this);
        QPainter painter(this);

        painter.setRenderHint(QPainter::Antialiasing);
        painter.fillRect(rect(), QBrush(QColor(32, 31, 31, 255)));

        // draw line at bottom of widget
        QRect area(x(), y(), width(), height());
        QPainter linePainter(this);
        linePainter.setRenderHint(QPainter::Antialiasing);
        linePainter.setBrush(QBrush(QColor(255, 255, 255)));
        QLine spacerLine(QPoint(area.x(), area.height() - 3),
                         QPoint(area.width(), area.height() - 3));
        linePainter.drawLine(spacerLine);
    }

    /// handles sizing the widgets.
    void resizeEvent(QResizeEvent*) {
        auto yPos = 0u;

        mNameLabel->setGeometry(0, yPos, this->width(), this->height() / 2);
        yPos += mNameLabel->height();
        mExecutionTime->setGeometry(0, yPos, this->width(), this->height() / 2);
    }

private:
    /// schedule for the widget
    nano::LeafSchedule mSchedule;

    /// name for the schedule
    QLabel* mNameLabel;

    /// displays execution time
    QLabel* mExecutionTime;
};

} // namespace nano
#endif // LEAFSCHEDULEWIDGET_H
