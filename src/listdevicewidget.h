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
 * \brief The ListDeviceWidget class This widget is used on the SettingsPage as a replacement
 *        for a QListWidget on the connectionList. It shows more information than a standard
 *        QListWidget, by giving an icon that represents the lights states, the name of of the
 *        controller, and the index of the light.
 */
class ListDeviceWidget : public QWidget
{
    Q_OBJECT
public:
    /*!
     * \brief ListDeviceWidget Constructor for single color routines.
     * \param device device for the widget
     * \param name name for device on list
     * \param parent parent widget
     */
    explicit ListDeviceWidget(const SLightDevice& device,
                              const QString& name,
                              QWidget *parent = 0);

    /*!
     * \brief ListDeviceWidget Constructor for multi color routines.
     * \param device device for the widget
     * \param name name for device on list
     * \param parent parent widget
     */
    explicit ListDeviceWidget(const SLightDevice& device,
                              const QString& name,
                              const std::vector<QColor>& colors,
                              QWidget *parent = 0);

    /*!
     * \brief itemIndex getter for the index of the lightslistwidget
     * \return string representation of the index of the lightslistwidget
     */
    QString itemIndex() { return mIndex->text(); }

private:

    /// Called by constructors
    void init(const SLightDevice& device, const QString& name);

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
