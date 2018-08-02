#ifndef LISTEDITWIDGET_H
#define LISTEDITWIDGET_H

#include <QObject>
#include <QWidget>
#include <QGridLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>

#include "listcollectionwidget.h"
#include "listdevicewidget.h"
#include "devicelist.h"
#include "comm/commlayer.h"


/*!
 * \copyright
 * Copyright (C) 2015 - 2018.
 * Released under the GNU General Public License.
 *
 * \brief The ListEditWidget class is a ListCollectionWidget that is used on the edit
 *        page to list devices.
 */
class ListEditWidget :  public ListCollectionWidget
{
    Q_OBJECT
public:

    /*!
     * \brief ListEditWidget constructor
     * \param group group of lights
     * \param key unique key for collection
     * \param comm pointer to commlayer
     * \param data pointer to datalayer
     */
    ListEditWidget(QWidget *parent, CommLayer* comm, DeviceList* data);

    /// preferred size for widget
    QSize preferredSize();

    /*!
     * \brief setShowButtons shows and hides all buttons on the widget
     * \param show true to show, false otherwise.
     */
    void setShowButtons(bool show);

    /*!
     * \brief setCheckedDevices takes a list of devices and compares it against all devices in the widget.
     *        If the device exists in both the widgets and the devices, it is set as checked. Otherwise, the widget
     *        is set to unchecked.
     * \param devices list of devices to compare to the widget.
     */
    void setCheckedDevices(std::list<cor::Light> devices);

    /*!
     * \brief updateDevices update the state and number of devices displayed in the widget.
     * \param devices the current state of all desired devices for the widget.
     * \param removeIfNotFound if a widget exists already in the listwidget but doesn't exist in the list provided,
     *        this widget gets removed and all other widgets get shifted.
     */
    void updateDevices(std::list<cor::Light> devices, bool removeIfNotFound = false);

    /// getter for widget contents
    EWidgetContents widgetContents() { return EWidgetContents::devices; }

    /*!
     * \brief devices getter for all devices being displayed by the widget.
     * \return all devices being displayed by the widget.
     */
    const std::list<cor::Light>& devices() { return mDevices; }

    /// getter for checked devices
    std::list<cor::Light> checkedDevices();

signals:

    /*!
     * \brief deviceClicked emitted whenever a device is clicked on the widget. Emits both the widget key
     *        and the device key.
     */
    void deviceClicked(QString, QString);

    /// emitted when a device switch is toggled. Emits the key and the state of the switch
    void deviceSwitchToggled(QString, bool);

protected:

    /*!
     * \brief mouseReleaseEvent picks up when a click (or a tap on mobile) is released.
     */
    virtual void mouseReleaseEvent(QMouseEvent *);

private slots:

    /*!
     * \brief handleClicked hangles a ListDeviceWidget getting clicked, emits a the collection's key and the device's key.
     * \param key the key of the ListDeviceWidget
     */
    void handleClicked(QString key);

private:
    ///TODO: can i skip this buffer?
    std::list<cor::Light> mDevices;

    /*!
     * \brief communication pointer to communication object
     *        for sending comannds to the lights
     */
    CommLayer *mComm;

    /*!
     * \brief data layer that maintains and tracks the states of the lights
     *        and the saved data of the GUI
     */
    DeviceList *mData;
};

#endif // LISTEDITWIDGET_H
