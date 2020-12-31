#ifndef HUESCHEDULEWIDGET_H
#define HUESCHEDULEWIDGET_H

#include <QLabel>
#include <QLayout>
#include <QWidget>

#include "comm/hue/bridge.h"
#include "comm/hue/hueprotocols.h"

namespace hue {
/*!
 * \copyright
 * Copyright (C) 2015 - 2020.
 * Released under the GNU General Public License.
 */

/*!
 * \brief The HueScheduleWidget class is a simple widget that displays information
 *        about a Hue schedule. This widget is meant to be in displayed in a list
 *        in a BrightSchedulesWidget
 */
class HueScheduleWidget : public QWidget {
    Q_OBJECT
public:
    /// constructor
    explicit HueScheduleWidget(QWidget* parent, hue::Schedule schedule);

    /// update the widget with a new schedule
    void updateWidget(const hue::Schedule& schedule);

protected:
    /// paints the background
    void paintEvent(QPaintEvent*);

    /// resize the widget
    void resizeEvent(QResizeEvent*);

private:
    /// resize programmatically
    void resize();

    /// schedule for the widget
    hue::Schedule mSchedule;

    /// name for the schedule
    QLabel* mNameLabel;

    /// displays the time in Based on ISO8601:2004
    /// TODO: convert this to a human readable format
    QLabel* mTimeLabel;

    /// status of the schedule, whether its enabled or not
    QLabel* mStatusLabel;

    /// index of the schedule, used byt he controller
    QLabel* mIndexLabel;

    /// true if this scheduel runs once, false if repeats
    QLabel* mAutoDeleteLabel;

    /// true if is a timeout
    bool mIsTimeout;
};

} // namespace hue

#endif // HUESCHEDULEWIDGET_H
