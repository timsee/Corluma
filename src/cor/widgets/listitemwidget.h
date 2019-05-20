#ifndef LISTITEMWIDGET_H
#define LISTITEMWIDGET_H

#include <QWidget>

namespace cor {
/*!
 * \copyright
 * Copyright (C) 2015 - 2019.
 * Released under the GNU General Public License.
 *
 *
 * \brief The ListItemWidget class is a widget that is meant to be displayed in a ListWidget.
 *        It is identifiable by its key.
 */
class ListItemWidget : public QWidget {
    Q_OBJECT
public:
    /// constructor
    explicit ListItemWidget(const QString& key, QWidget* parent) : QWidget(parent), mKey(key) {}

    /// getter for key
    const QString& key() const { return mKey; }

protected:
    /// key for widget.
    QString mKey;
};

} // namespace cor

namespace std {
template <>
struct hash<cor::ListItemWidget> {
    size_t operator()(const cor::ListItemWidget& k) const {
        return std::hash<std::string>{}(k.key().toStdString());
    }
};
} // namespace std


#endif // LISTITEMWIDGET_H
