#ifndef LIST_SIMPLE_GROUP_WIDGET_H
#define LIST_SIMPLE_GROUP_WIDGET_H

#include <QObject>
#include <QWidget>
#include <QGridLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>

#include "listlightwidget.h"

#include "cor/devicelist.h"
#include "cor/listwidget.h"
#include "cor/dictionary.h"
#include "comm/commlayer.h"


/*!
 * \copyright
 * Copyright (C) 2015 - 2019.
 * Released under the GNU General Public License.
 *
 * \brief The ListSimpleGroupWidget class is a subclassed cor::ListWidget that
 *        displays a list of listdevices
 */
class ListSimpleGroupWidget :  public cor::ListWidget
{
    Q_OBJECT
public:

    /// constructor
    ListSimpleGroupWidget(QWidget *parent, cor::EListType type);

    /*!
     * \brief setCheckedDevices takes a list of devices and compares it against all devices in the widget.
     *        If the device exists in both the widgets and the devices, it is set as checked. Otherwise, the widget
     *        is set to unchecked.
     * \param devices list of devices to compare to the widget.
     */
    void setCheckedDevices(const std::list<cor::Light>& devices);

    /// removes all widget from group.
    void removeWidgets();

    /*!
     * \brief updateDevices update the state and number of devices displayed in the widget.
     * \param devices the current state of all desired devices for the widget.
     * \param removeIfNotFound if a widget exists already in the listwidget but doesn't exist in the list provided,
     *        this widget gets removed and all other widgets get shifted.
     */
    void updateDevices(const std::list<cor::Light>& devices,
                       cor::EWidgetType listWidgetType,
                       EOnOffSwitchState switchState,
                       bool canHighlight,
                       bool skipOff);

    /// display only the devices in this list, hiding all others.
    void displayOnlyDevices(const std::list<cor::Light>& devices);


    /// getter for checked devices
    const std::list<cor::Light> checkedDevices();

signals:

    /*!
     * \brief deviceClicked emitted whenever a device is clicked on the widget. Emits both the widget key
     *        and the device key.
     */
    void deviceClicked(QString);

    /// emits the key and state when on/off switch for a device toggled.
    void deviceSwitchToggled(QString, bool);

    /*!
     * \brief selectAllClicked emitted when select all is clicked. Should add all devices to data layer.
     */
    void allButtonPressed(QString, bool);

private slots:

    /*!
     * \brief handleClicked hangles a ListDeviceWidget getting clicked, emits a the collection's key and the device's key.
     * \param key the key of the ListDeviceWidget
     */
    void handleClicked(QString key) { emit deviceClicked(key); }


    /// handles when an on/off switch switch for any given device is toggled
    void handleToggledSwitch(QString key, bool isOn) { emit deviceSwitchToggled(key, isOn); }
};

#endif // LIST_SIMPLE_GROUP_WIDGET_H
