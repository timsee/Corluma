#ifndef BRIDGEGROUPSWIDGET_H
#define BRIDGEGROUPSWIDGET_H

#include <QWidget>
#include <QScrollArea>

#include "cor/topwidget.h"
#include "cor/lightgroup.h"
#include "cor/page.h"

#include "hue/huegroupwidget.h"

namespace hue
{
/*!
 * \copyright
 * Copyright (C) 2015 - 2018.
 * Released under the GNU General Public License.
 */

/*!
 * \brief The BridgeGroupsWidget class is a simple widget made for displaying metadata
 *        about a group in a list.
 */
class BridgeGroupsWidget : public QWidget, public cor::Page
{
    Q_OBJECT
public:
    /// constructor
    explicit BridgeGroupsWidget(QWidget *parent);

    /// update the groups in the widget
    void updateGroups(std::list<cor::LightGroup> groups);

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

    /// vector of all HueGroupWidgets
    std::vector<hue::HueGroupWidget*> mWidgets;
};

}
#endif // BRIDGEGROUPSWIDGET_H