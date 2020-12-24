#ifndef COR_LIST_WIDGET_H
#define COR_LIST_WIDGET_H

#include <QScrollArea>
#include <QWidget>

#include "cor/listlayout.h"
#include "cor/widgets/listitemwidget.h"

namespace cor {

/*!
 * \copyright
 * Copyright (C) 2015 - 2020.
 * Released under the GNU General Public License.
 */

/*!
 * \brief The ListWidget class is an instance of a QScrollArea designed
 *        to hold multiple cor::ListItemWidgets in a scroll area that extends vertically.
 *        This widget also handles cases where the cor::ListItemWidgets grow and shrink
 *        dynamically.
 */
class ListWidget : public QScrollArea {
    Q_OBJECT
public:
    /// constructor
    ListWidget(QWidget* parent, EListType type);

    /// getter for main widget
    QWidget* mainWidget() { return mWidget; }

    /*!
     * \brief resizeWidgets resizes all the widgets and moves them accordingly.
     */
    void resizeWidgets();

    /*!
     * \brief insertWidget insert cor::ListItemWIdget into the layout.
     * \param widget widget to be inserted, if it doesn't already exist. Will reorganize widgets if
     * needed.
     */
    void insertWidget(cor::ListItemWidget* widget);

    /*!
     * \brief removeWidget remove cor::ListItemWIdget from the layout
     * all widgets to the right of the removed widget get moved back one cell.
     * \param widget widget to be removed, if it exists.
     */
    void removeWidget(cor::ListItemWidget* widget);

    /*!
     * \brief removeWidget removes the widget with the given key.
     * \param key key of widget to remove.
     */
    void removeWidget(QString key) { mListLayout.removeWidget(key); }

    /// getter for all widgets
    const std::vector<cor::ListItemWidget*>& widgets() { return mListLayout.widgets(); }

    /// call to set up the listwidget when showing it.
    void show();

    /// resets the list widget to its uninitialized state.
    void clearAll();

protected:
    /// handles when the widget resizes
    void resizeEvent(QResizeEvent*);

private:
    /// resizes the widgets
    void resize();

    /// acts as layout for the multiple widgets displayed in the list
    ListLayout mListLayout;

    /// widget used for scroll area.
    QWidget* mWidget;

    /// size of overall background of grid.
    QSize mWidgetSize;
};

} // namespace cor
#endif // COR_LIST_WIDGET_H
