#ifndef LISTCOLLECTIONSUBWIDGET_H
#define LISTCOLLECTIONSUBWIDGET_H

#include <QString>
#include <QWidget>

/*!
 * \copyright
 * Copyright (C) 2015 - 2017.
 * Released under the GNU General Public License.
 *
 *
 * \brief ListCollectionSubwidget is an object used by all widgets that are used as items
 *        in a ListCollectionWidget.
 */
class ListCollectionSubWidget : public QWidget
{
    Q_OBJECT

public:
    /// virtual destructor
    virtual ~ListCollectionSubWidget(){}

    /*!
     * \brief key getter for key for ListCollectionSubWidget
     * \return key for device
     */
    const QString& key() { return mKey; }

protected:

    /*!
     * \brief mKey stores the unique key used by the ListCollectionSubWidget
     */
    QString mKey;
};


/*!
 * \brief The subWidgetCompare struct comparison operator for ListCollectionSubWidgets.
 *        if the lhs is less than the rhs, it returns true, else, false.
 */
struct subWidgetCompare {
  bool operator() (ListCollectionSubWidget* lhs, ListCollectionSubWidget* rhs) const
  {return (lhs->key().compare(rhs->key()) < 0);}
};

#endif // LISTCOLLECTIONSUBWIDGET_H
