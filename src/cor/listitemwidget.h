#ifndef LISTITEMWIDGET_H
#define LISTITEMWIDGET_H

#include <QWidget>

namespace cor
{
/*!
 * \copyright
 * Copyright (C) 2015 - 2018.
 * Released under the GNU General Public License.
 *
 *
 *
 * \brief The ListItemWidget class is a widget that is meant to be displayed in a ListWidget.
 *        It is identifiable by its key.
 */
class ListItemWidget : public QWidget
{
    Q_OBJECT
public:
    /// constructor
    explicit ListItemWidget(const QString& key, QWidget *parent);

    /// getter for key
    const QString& key() const { return mKey; }

protected:

    /// key for widget.
    QString mKey;
};

}

#endif // LISTITEMWIDGET_H
