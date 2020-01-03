#ifndef BRIDGEGROUPSWIDGET_H
#define BRIDGEGROUPSWIDGET_H

#include <QScrollArea>
#include <QWidget>

#include "comm/hue/bridge.h"
#include "comm/hue/huegroupwidget.h"
#include "cor/objects/group.h"
#include "cor/objects/page.h"
#include "cor/widgets/topwidget.h"

namespace hue {
/*!
 * \copyright
 * Copyright (C) 2015 - 2019.
 * Released under the GNU General Public License.
 */

/*!
 * \brief The BridgeGroupsWidget class is a simple widget made for displaying metadata
 *        about a group in a list.
 */
class BridgeGroupsWidget : public QWidget, public cor::Page {
    Q_OBJECT
public:
    /// constructor
    explicit BridgeGroupsWidget(QWidget* parent);

    /// update the groups in the widget
    void updateGroups(BridgeGroupVector groups, BridgeRoomVector rooms);

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
    void paintEvent(QPaintEvent*);

private slots:
    /*!
     * \brief closePressed close button pressed from top widget.
     */
    void pressedClose(bool);

private:
    /// title and close button at top of widget.
    cor::TopWidget* mTopWidget;

    /// layout for widget
    QVBoxLayout* mMainLayout;

    /// layout for scroll area
    QVBoxLayout* mScrollLayout;

    /// scroll area for displaying list.
    QScrollArea* mScrollArea;

    /// widget used for scroll area.
    QWidget* mScrollAreaWidget;

    /// vector of all HueGroupWidgets
    std::vector<hue::HueGroupWidget*> mWidgets;
};

} // namespace hue
#endif // BRIDGEGROUPSWIDGET_H
