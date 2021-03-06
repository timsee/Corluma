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
 * Copyright (C) 2015 - 2020.
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
    void updateGroups(BridgeGroupVector groups, BridgeGroupVector rooms);

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

    /// vector of all HueGroupWidgets
    std::vector<hue::HueGroupWidget*> mWidgets;
};

} // namespace hue
#endif // BRIDGEGROUPSWIDGET_H
