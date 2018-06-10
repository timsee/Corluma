#ifndef LISTDEVICESGROUPWIDGET_H
#define LISTDEVICESGROUPWIDGET_H

#include <QObject>
#include <QWidget>
#include <QGridLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>

#include "listcollectionwidget.h"
#include "listdevicewidget.h"
#include "datalayer.h"
#include "comm/commlayer.h"


/*!
 * \copyright
 * Copyright (C) 2015 - 2018.
 * Released under the GNU General Public License.
 *
 * \brief The ListDevicesGroupWidget class is a subclassed ListCollectionWidget that
 *        displays all the devices in a collection. It contains a widget at the top that
 *        gives info on the collection and the ability to edit it. It also contains an array
 *        of ListDeviceWidgets, which can be shown or hidden by clicking the top widget.
 */
class ListDevicesGroupWidget :  public ListCollectionWidget
{
    Q_OBJECT
public:

    /*!
     * \brief ListDevicesGroupWidget constructor
     * \param group group of lights
     * \param key unique key for collection
     * \param comm pointer to commlayer
     * \param data pointer to datalayer
     */
    ListDevicesGroupWidget(const cor::LightGroup& group,
                           QString key,
                           CommLayer *comm,
                           DataLayer *data,
                           QWidget *parent);


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

    /*!
     * \brief devices getter for all devices being displayed by the widget.
     * \return all devices being displayed by the widget.
     */
    const std::list<cor::Light>& devices() { return mGroup.devices; }

    /// getter for just the reachable devices in the group.
    std::list<cor::Light> reachableDevices();

    /// getter for checked devices
    const std::list<cor::Light> checkedDevices();

    /*!
     * \brief preferredSize all collection widgets must implement a preferred size. this is the size
     *        the widget wants to be. It may not end up this size but its a baseline if theres no other
     *        widgets pushing against it.
     * \return a QSize representing its ideal size.
     */
    QSize preferredSize();

    /// resize all the children widgets
    void resizeInteralWidgets();

    /*!
     * \brief setShowButtons shows and hides all buttons on the widget
     * \param show true to show, false otherwise.
     */
    void setShowButtons(bool show);

    /// getter for the type of widget
    EWidgetContents widgetContents() { return EWidgetContents::groups; }

    /// getter for the group data.
    const cor::LightGroup& group() { return mGroup; }

signals:

    /*!
     * \brief deviceClicked emitted whenever a device is clicked on the widget. Emits both the widget key
     *        and the device key.
     */
    void deviceClicked(QString, QString);

    /// emits the key and state when on/off switch for a device toggled.
    void deviceSwitchToggled(QString, bool);

    /*!
     * \brief selectAllClicked emitted when select all is clicked. Should add all devices to data layer.
     */
    void allButtonPressed(QString, bool);

protected:

    /*!
     * \brief mouseReleaseEvent picks up when a click (or a tap on mobile) is released.
     */
    virtual void mouseReleaseEvent(QMouseEvent *);

    /*!
     * \brief paintEvent paints the background of the widget
     */
    void paintEvent(QPaintEvent *event);

private slots:

    /*!
     * \brief handleClicked hangles a ListDeviceWidget getting clicked, emits a the collection's key and the device's key.
     * \param key the key of the ListDeviceWidget
     */
    void handleClicked(QString key) { emit deviceClicked(mKey, key); }

    /*!
     * \brief selectAllButtonClicked select all button is clicked.
     */
    void selectAllButtonClicked(bool);

    /// handles when an on/off switch switch for any given device is toggled
    void handleToggledSwitch(QString key, bool isOn) { emit deviceSwitchToggled(key, isOn); }

private:

    /// handle the state of the select all button
    void handleSelectAllButton();

    /*!
     * \brief data layer that maintains and tracks the states of the lights
     *        and the saved data of the GUI
     */
    DataLayer *mData;

    /*!
     * \brief communication pointer to communication object
     *        for sending comannds to the lights
     */
    CommLayer *mComm;

    /*!
     * \brief computeHighlightColor computes the top of the widgets highlight color
     *        based on how many devices are selected. When all devices are selected, it is the
     *        same color as a device's highlight. when only some of the devices are selected,
     *        it is slightly darker.
     * \return the QColor that should be used to higlight the widget.
     */
    QColor computeHighlightColor();

    /// pixmap for the select all button
    QPixmap mSelectAllPixmap;

    /// pixmap for the clear all button
    QPixmap mClearAllPixmap;

    /// stored data for the group.
    cor::LightGroup mGroup;

    /// true if select all is in clear state, false if its in select state
    bool mSelectAllIsClear;

    /*!
     * \brief mSelectAllButton button that selects all devices when pushed and adds them to the data layer.
     */
    QPushButton *mSelectAllButton;
};

#endif // LISTDEVICESGROUPWIDGET_H
