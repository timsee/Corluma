#ifndef BRIDGESCHEDULESWIDGET_H
#define BRIDGESCHEDULESWIDGET_H

#include <QScrollArea>
#include <QWidget>

#include "comm/hue/bridge.h"
#include "comm/hue/hueschedulewidget.h"
#include "cor/objects/page.h"
#include "cor/widgets/topwidget.h"

namespace hue {
/*!
 * \copyright
 * Copyright (C) 2015 - 2020.
 * Released under the GNU General Public License.
 */

/*!
 * \brief The BridgeSchedulesWidget class is a simple widget made for displaying metadata
 *        about the existing schedules in a list. The user can also delete schedules.
 */
class BridgeSchedulesWidget : public QWidget, public cor::Page {
    Q_OBJECT
public:
    /// constructor
    explicit BridgeSchedulesWidget(QWidget* parent);

    /// update the hue schedules in the widget
    void updateSchedules(std::vector<hue::Schedule> schedules);

    /*!
     * \brief resize size the widget programmatically
     */
    void resize();

protected:
    /*!
     * \brief paintEvent used to draw the background of the widget.
     */
    void paintEvent(QPaintEvent*);

private:
    /// layout for widget
    QVBoxLayout* mMainLayout;

    /// layout for scroll area
    QVBoxLayout* mScrollLayout;

    /// scroll area for displaying list.
    QScrollArea* mScrollArea;

    /// widget used for scroll area.
    QWidget* mScrollAreaWidget;

    /// vector of all HueScheduleWidget
    std::vector<hue::HueScheduleWidget*> mWidgets;
};

} // namespace hue
#endif // BRIDGESCHEDULESWIDGET_H
