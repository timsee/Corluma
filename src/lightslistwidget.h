#ifndef LIGHTSLISTWIDGET_H
#define LIGHTSLISTWIDGET_H

#include <QObject>
#include <QWidget>
#include <QLabel>
#include <QLayout>

#include "icondata.h"
#include "commtype.h"

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
     * \brief setup setsup an icon with a pointer to the datalyer and
     *        a default setting.
     * \param device the device to pull all of the light data from.
     * \param data pointer to data layer.
     */
    void setup(const SLightDevice& device, DataLayer *data);

    /*!
     * \brief updateIcon update the icon with new light data
     * \param device the device data to use for the update
     */
    void updateIcon(const SLightDevice& device);

    /*!
     * \brief updateColor update just the color of the icon.
     * \param color the new color for the icon.
     */
    void updateColor(QColor color);

    /*!
     * \brief itemIndex getter for the index of the lightslistwidget
     * \return string representation of the index of the lightslistwidget
     */
    QString itemIndex() { return mIndex->text(); }

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
