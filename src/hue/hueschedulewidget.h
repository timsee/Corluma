#ifndef HUESCHEDULEWIDGET_H
#define HUESCHEDULEWIDGET_H

#include <QWidget>
#include <QLabel>
#include <QLayout>
#include "hue/hueprotocols.h"

namespace hue
{
/*!
 * \copyright
 * Copyright (C) 2015 - 2018.
 * Released under the GNU General Public License.
 */

/*!
 * \brief The HueScheduleWidget class is a simple widget that displays information
 *        about a Hue schedule. This widget is meant to be in displayed in a list
 *        in a BrightSchedulesWidget
 */
class HueScheduleWidget : public QWidget
{
    Q_OBJECT
public:
    /// constructor
    explicit HueScheduleWidget(QWidget *parent, SHueSchedule schedule);

protected:
    /// paints the background
    void paintEvent(QPaintEvent *);

private:

    /// schedule for the widget
    SHueSchedule mSchedule;

    /// name for the schedule
    QLabel *mNameLabel;

    /// displays the time in Based on ISO8601:2004
    ///TODO: convert this to a human readable format
    QLabel *mTimeLabel;

    /// index of the schedule, used byt he controller
    QLabel *mIndexLabel;

    /// status of the schedule, whether its enabled or not
    QLabel *mStatusLabel;

    /// true if this scheduel runs once, false if repeats
    QLabel *mAutoDeleteLabel;

    /// main layout for the widget
    QVBoxLayout *mMainLayout;

    /// layout of the left of the widget
    QVBoxLayout *mLeftLayout;

    /// layout of the right of the widget
    QVBoxLayout *mRightLayout;

    /// bottom of the layout
    QHBoxLayout *mBottomLayout;

};

}

#endif // HUESCHEDULEWIDGET_H
