#ifndef DISPLAYNANOLEAFSCHEDULESWIDGET_H
#define DISPLAYNANOLEAFSCHEDULESWIDGET_H

#include <QScrollArea>
#include <QScroller>
#include <QWidget>
#include "comm/nanoleaf/leafschedule.h"
#include "comm/nanoleaf/leafschedulewidget.h"
#include "cor/widgets/listwidget.h"
#include "utils/qt.h"

/*!
 * \copyright
 * Copyright (C) 2015 - 2020.
 * Released under the GNU General Public License.
 */

/*!
 * \brief The BridgeSchedulesWidget class is a simple widget made for displaying metadata
 *        about the existing schedules in a list. The user can also delete schedules.
 */
class DisplayNanoleafSchedulesWidget : public QWidget {
    Q_OBJECT
public:
    /// constructor
    explicit DisplayNanoleafSchedulesWidget(QWidget* parent)
        : QWidget(parent),
          mListWidget{new cor::ListWidget(this, cor::EListType::oneColumn)} {}

    /// update the nanoleaf schedules in the widget
    void updateSchedules(std::vector<nano::LeafSchedule> schedules, bool isOn) {
        // add new widgets
        for (const auto& schedule : schedules) {
            bool foundSchedule = false;
            for (auto widget : mWidgets) {
                auto scheduleWidget = dynamic_cast<nano::LeafScheduleWidget*>(widget);
                if (schedule == scheduleWidget->schedule()) {
                    scheduleWidget->updateSchedule(schedule, isOn);
                    foundSchedule = true;
                }
            }
            if (!foundSchedule) {
                nano::LeafScheduleWidget* widget =
                    new nano::LeafScheduleWidget(mListWidget->mainWidget(), schedule, isOn);
                mWidgets.push_back(widget);
                mListWidget->insertWidget(widget);
            }
        }
        resize();
    }

    /*!
     * \brief resize size the widget programmatically
     */
    void resize() {
        mListWidget->setGeometry(0, 0, this->width(), this->height());
        // call resize function of each widget
        auto yHeight = 0u;
        QSize widgetSize(mListWidget->width(), int(mListWidget->height() * 0.33));
        for (auto widget : mListWidget->widgets()) {
            auto nanoleafWidget = dynamic_cast<nano::LeafScheduleWidget*>(widget);
            nanoleafWidget->setGeometry(0, yHeight, widgetSize.width(), widgetSize.height());
            yHeight += nanoleafWidget->height();
        }
        mListWidget->mainWidget()->setFixedHeight(yHeight);
        mListWidget->mainWidget()->setFixedWidth(width());
    }

protected:
    void resizeEvent(QResizeEvent*) { resize(); }

private:
    /// widget for displaying a scrollable list of other widgets
    cor::ListWidget* mListWidget;

    /// vector of all HueScheduleWidget
    std::vector<nano::LeafScheduleWidget*> mWidgets;
};

#endif // DISPLAYNANOLEAFSCHEDULESWIDGET_H
