#ifndef CORLUMALISTWIDGET_H
#define CORLUMALISTWIDGET_H

#include <QWidget>
#include <QScrollArea>

#include "listcollectionwidget.h"
/*!
 * \copyright
 * Copyright (C) 2015 - 2017.
 * Released under the GNU General Public License.
 */

/*!
 * \brief The CorlumaListWidget class is an instance of a QScrollArea designed
 *        to hold multiple ListCollectionWidgets in a scroll area that extends vertically.
 *        This widget also handles cases where the ListCollectionWidgets grow and shrink
 *        dynamically.
 */
class CorlumaListWidget : public QScrollArea
{
    Q_OBJECT
public:
    /// constructor
    CorlumaListWidget(QWidget *parent = 0);

    /*!
     * \brief addWidget add a ListCollectionWidget to the scroll area of this widget.
     * \param widget new widget to add.
     */
    void addWidget(ListCollectionWidget *widget);

    /*!
     * \brief removeWidget removes the widget with the given key.
     * \param key key of widget to remove.
     */
    void removeWidget(QString key);

    /// number of widgets in the scroll area.
    uint32_t count() { return mWidgets.size(); }

    /*!
     * \brief widget get a ListCollectionWidget by its index.
     * \param index index of widget to return
     * \return pointer to ListCollectionWidget
     */
    ListCollectionWidget *widget(uint32_t index);

    /*!
     * \brief widget get a ListCollectionWidget by its key.
     * \param key key of widget to return
     * \return pointer to ListCollectionWidget with matching key, if one exists
     *         otherwise, it returns a nullptr.
     */
    ListCollectionWidget *widget(QString key);

    /*!
     * \brief resizeWidgets resizes all the widgets and moves them accordingly.
     */
    void resizeWidgets();

protected:

    /// handles when the widget resizes
    void resizeEvent(QResizeEvent *);

private:

    /// vector of all widgets displayed in this widget.
    std::vector<ListCollectionWidget*> mWidgets;

    /// widget used for scroll area.
    QWidget *mWidget;

    /// search function. returns the index of the widget with the key, and -1 if none is found
    int searchForWidget(QString key);

private slots:

    /*!
     * \brief widgetHeightChanged catches whenever a widget's height changes so that it knows
     *        to move all the other widgets accordingly.
     */
    void widgetHeightChanged(int);
};

#endif // CORLUMALISTWIDGET_H
