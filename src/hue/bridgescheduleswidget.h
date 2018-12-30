#ifndef BRIDGESCHEDULESWIDGET_H
#define BRIDGESCHEDULESWIDGET_H

#include <QWidget>
#include <QScrollArea>

#include "cor/topwidget.h"
#include "cor/page.h"
#include "hue/bridge.h"
#include "hue/hueschedulewidget.h"

namespace hue
{
/*!
 * \copyright
 * Copyright (C) 2015 - 2019.
 * Released under the GNU General Public License.
 */

/*!
 * \brief The BridgeSchedulesWidget class is a simple widget made for displaying metadata
 *        about the existing schedules in a list. The user can also delete schedules.
 */
class BridgeSchedulesWidget : public QWidget, public cor::Page
{
    Q_OBJECT
public:
    /// constructor
    explicit BridgeSchedulesWidget(QWidget *parent);

    /// update the hue schedules in the widget
    void updateSchedules(std::list<SHueSchedule> schedules);

    /*!
     * \brief resize size the widget programmatically
     */
    void resize();

signals:
    /*!
     * \brief pressedClose emited when the close button is pressed.
     */
    void closePressed();

protected:
    /*!
     * \brief paintEvent used to draw the background of the widget.
     */
    void paintEvent(QPaintEvent *);

private slots:
    /*!
     * \brief closePressed close button pressed from top widget.
     */
    void pressedClose(bool);

private:
    /// title and close button at top of widget.
    cor::TopWidget *mTopWidget;

    /// layout for widget
    QVBoxLayout *mMainLayout;

    /// layout for scroll area
    QVBoxLayout *mScrollLayout;

    /// scroll area for displaying list.
    QScrollArea *mScrollArea;

    /// widget used for scroll area.
    QWidget *mScrollAreaWidget;

    /// vector of all HueScheduleWidget
    std::vector<hue::HueScheduleWidget*> mWidgets;
};

}
#endif // BRIDGESCHEDULESWIDGET_H
