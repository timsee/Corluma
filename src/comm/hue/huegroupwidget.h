#ifndef HUEGROUPWIDGET_H
#define HUEGROUPWIDGET_H

#include <QLabel>
#include <QLayout>
#include <QWidget>

#include "cor/objects/group.h"

namespace hue {
/*!
 * \copyright
 * Copyright (C) 2015 - 2019.
 * Released under the GNU General Public License.
 */

/*!
 * \brief The HueGroupWidget class is a simple widget made for displaying metadata
 *        about a group. Its stored in a list in the BridgeGRoupsWidget.
 */
class HueGroupWidget : public QWidget {
    Q_OBJECT
public:
    /// constructor
    explicit HueGroupWidget(QWidget* parent, const cor::Group& lightGroup);

protected:
    /// paints the background on the widget
    void paintEvent(QPaintEvent*);

private:
    /// name of the group
    QLabel* mName;

    /// index of the group, used by its controller
    QLabel* mIndex;

    QString generateDescription(const cor::Group& group);

    QLabel* mGroupDescription;

    /// layout for widget
    QVBoxLayout* mLayout;
};

} // namespace hue

#endif // HUEGROUPWIDGET_H
