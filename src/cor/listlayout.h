#ifndef LISTLAYOUT_H
#define LISTLAYOUT_H


#include <QString>
#include <QWidget>

#include "listitemwidget.h"
#include "cor/dictionary.h"

namespace cor
{

/// type of list
enum class EListType {
    grid,
    linear,
    linear2X
};

/*!
 * \copyright
 * Copyright (C) 2015 - 2018.
 * Released under the GNU General Public License.
 *
 *
 * \brief The ListLayout class computes the layout of a cor::ListWidget
 */
class ListLayout
{
public:

    /// constructor
    ListLayout(EListType type);

    /*!
     * \brief insertWidget insert cor::ListItemWIdget into the layout.
     * \param widget widget to be inserted, if it doesn't already exist. Will reorganize widgets if needed.
     */
    void insertWidget(cor::ListItemWidget* widget);

    /*!
     * \brief removeWidget removes the widget with the given key.
     * \param key key of widget to remove.
     */
    void removeWidget(QString key);

    /*!
     * \brief removeWidget remove cor::ListItemWIdget from the layout
     *        all widgets to the right of the removed widget get moved back one cell.
     * \param widget widget to be removed, if it exists.
     */
    void removeWidget(cor::ListItemWidget* widget);

    /*!
     * \brief widget get a ListCollectionWidget by its index.
     * \param index index of widget to return
     * \return pointer to ListCollectionWidget
     */
    cor::ListItemWidget *widget(uint32_t index);

    /*!
     * \brief widget get a ListCollectionWidget by its key.
     * \param key key of widget to return
     * \return pointer to ListCollectionWidget with matching key, if one exists
     *         otherwise, it returns a nullptr.
     */
    cor::ListItemWidget *widget(QString key);

    /*!
     * \brief widgetPosition gives the widget position based off of the given widget. Position is not *actual* position,
     *        but where it falls in the group overall. For example, top left widget is (0,0). Next widget is (0,1). First
     *        widget in second row is (1,0)
     * \param widget widget to look for position for.
     * \return Position of widget as a point.
     */
    QPoint widgetPosition(QWidget *widget);

    /// getter for widget position based on index in vector.
    QPoint widgetPosition(int index);

    /// getter for widget size
    QSize widgetSize(QSize parentSize);

    /// sort widgets IFF they are device widgets
    /// TODO: this is totally an antipattern...
    void sortDeviceWidgets();

    /// number of widgets in the scroll area.
    uint32_t count() { return uint32_t(mWidgetDictionary.size()); }

    /// getter for all collection widgets.
    const std::vector<cor::ListItemWidget*>& widgets() { return mWidgets; }

    /// getter for type of list
    EListType type() { return mType; }

    /// gives the overall size needed to display the widget
    QSize overallSize();

private:

    /// maps keys to widgets while allowing quicker lookups
    cor::Dictionary<cor::ListItemWidget*> mWidgetDictionary;

    /// list of all widgets displayed in this widget.
    std::vector<cor::ListItemWidget*> mWidgets;

    /// stores type of list
    EListType mType;

};

}

#endif // LISTLAYOUT_H
