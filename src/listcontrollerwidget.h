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
 * \brief The ListControllerWidget class This widget is used on the SettingsPage as a replacement
 *        for a QListWidget on the connectionList. It shows more information than a standard
 *        QListWidget, by giving an icon that represents the lights states, the name of of the
 *        controller, and the index of the light.
 */
class ListControllerWidget : public QWidget
{
    Q_OBJECT
public:
    /*!
     * \brief LightsListWidget constructor
     */
    explicit ListControllerWidget(QWidget *parent = 0);

    /*!
     * \brief setup setsup an icon using a SLightDevice to generate the data.
     * \param device the device to pull all of the light data from.
     */
    void setup(const SLightDevice& device);

    /*!
     * \brief setup sets up an icon using a SLightDevice to generate the data.
     *        This setup also contains a color group for multi color routines.
     * \param device device to use for the presets of the controller.
     * \param colors colors to use for a multi color routine.
     */
    void setup(const SLightDevice& device, const std::vector<QColor>& colors);

    /*!
     * \brief updateSingleRoutineIcon update icon data to single color routine.
     * \param routine routine to update icon data to.
     * \param color color to use for single color routine.
     */
    void updateSingleRoutineIcon(ELightingRoutine routine, QColor color);

    /*!
     * \brief updateMultiRoutineIcon update icon data to a multi color routine.
     * \param routine routine to update the icon data to
     * \param group color group for multi color routine
     * \param colors colors for color group.
     */
    void updateMultiRoutineIcon(ELightingRoutine routine, EColorGroup group, const std::vector<QColor>& colors);

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
    IconData mIconData;

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
