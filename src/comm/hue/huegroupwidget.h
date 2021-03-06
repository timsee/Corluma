#ifndef HUEGROUPWIDGET_H
#define HUEGROUPWIDGET_H

#include <QLabel>
#include <QLayout>
#include <QWidget>

#include "cor/objects/group.h"

namespace hue {
/*!
 * \copyright
 * Copyright (C) 2015 - 2020.
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
    explicit HueGroupWidget(QWidget* parent, std::uint32_t index, const cor::Group& lightGroup);

protected:
    /// paints the background on the widget
    void paintEvent(QPaintEvent*);

    void resizeEvent(QResizeEvent*) { resize(); }

private:
    /// resizes programmatically
    void resize();

    /// renders the icon for the widget.
    void renderIcon();

    /// name of the group
    QLabel* mName;

    /// icon of group type
    QLabel* mGroupIcon;

    /// index of the group, used by its controller
    QLabel* mIndex;

    /// description of the group in plain english
    QLabel* mGroupDescription;

    /// helper that generates a plain-english description of the group
    QString generateDescription(std::size_t lightCount, bool isRoom);

    /// last size of the icon
    QSize mLastSize;

    /// the group displayed by the widget.
    cor::Group mGroup;
};

} // namespace hue

#endif // HUEGROUPWIDGET_H
