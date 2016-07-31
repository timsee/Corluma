#ifndef LIGHTSLISTWIDGET_H
#define LIGHTSLISTWIDGET_H

#include <QObject>
#include <QWidget>
#include <QLabel>
#include <QLayout>

#include "icondata.h"

/*!
 * \copyright
 * Copyright (C) 2015 - 2016.
 * Released under the GNU General Public License.
 */

/*!
 * \brief The LightsListWidget class This widget is used on the SettingsPage as a replacement
 *        for a QListWidget on the connectionList. It shows more information than a standard
 *        QListWidget, by giving an icon that represents the lights states, the name of of the
 *        controller, and the index of the light.
 */
class LightsListWidget : public QWidget
{
    Q_OBJECT
public:
    /*!
     * \brief LightsListWidget constructor
     */
    explicit LightsListWidget(QWidget *parent = 0);

    /*!
     * \brief setup called after the constructor, required to set up the widget with the
     *        proper data
     * \param name name of the controller for the device
     * \param isOn true if on, false otherwise
     * \param isReachable true if reachable, false otherwise
     * \param color color to use for the icon
     * \param index index of the device
     * \param data pointer to data layer.
     */
    void setup(QString name, bool isOn, bool isReachable, QColor color, int index, DataLayer *data);

    /*!
     * \brief updateIcon update the icon to display a new lighting routine, color group, and color
     * \param color new color for the icon
     * \param routine new lighting routine for the icon
     * \param group new group for the icon.
     */
    void updateIcon(QColor color, ELightingRoutine routine, EColorGroup group);

    /*!
     * \brief updateColor update just the color of the icon.
     * \param color the new color for the icon.
     */
    void updateColor(QColor color);

private:
    /*!
     * \brief mStatusIcon uses mIconData to display an icon
     */
    QLabel *mStatusIcon;

    /*!
     * \brief mIconData creates a QPixmap that represents the current light states
     *        which gets displayed on the mStatusIcon.
     */
    IconData *mIconData;

    /*!
     * \brief mController name of the controller
     */
    QLabel *mController;

    /*!
     * \brief mIndex the index of the specific device
     */
    QLabel *mIndex;

    /*!
     * \brief mLayout layout of widget
     */
    QHBoxLayout *mLayout;
};

#endif // LIGHTSLISTWIDGET_H
