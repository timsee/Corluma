#ifndef LISTROOMWIDGET_H
#define LISTROOMWIDGET_H

#include <QObject>
#include <QWidget>
#include <QLabel>
#include <QPushButton>

#include "cor/listitemwidget.h"
#include "cor/group.h"
#include "cor/listlayout.h"
#include "groupbuttonswidget.h"
#include "listdevicewidget.h"
#include "dropdowntopwidget.h"
#include "comm/commlayer.h"

/*!
 * \copyright
 * Copyright (C) 2015 - 2019.
 * Released under the GNU General Public License.
 *
 * \brief The ListDevicesGroupWidget class is a subclassed ListCollectionWidget that
 *        displays all the devices in a collection. It contains a widget at the top that
 *        gives info on the collection and the ability to edit it. It also contains an array
 *        of ListDeviceWidgets, which can be shown or hidden by clicking the top widget.
 */
class ListRoomWidget :  public cor::ListItemWidget
{
    Q_OBJECT
public:

    /*!
     * \brief ListDevicesGroupWidget constructor
     * \param group group of lights
     * \param key unique key for collection
     */
    ListRoomWidget(const cor::Group& group,
                   CommLayer* comm,
                   GroupData *groups,
                   QString key,
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
    void updateGroup(const cor::Group& group, bool removeIfNotFound);

    /// the top widget of the group widget
    DropdownTopWidget* dropDownTopWidget() { return mDropdownTopWidget; }

    /*!
     * \brief devices getter for all devices being displayed by the widget.
     * \return all devices being displayed by the widget.
     */
    const std::list<QString>& devices() { return mGroup.lights; }

    /// getter for just the reachable devices in the group.
    std::list<cor::Light> reachableDevices();

    /// getter for checked devices
    std::list<cor::Light> checkedDevices();

    /*!
     * \brief setShowButtons shows and hides all buttons on the widget
     * \param show true to show, false otherwise.
     */
    void setShowButtons(bool show);

    /// hides the group buttons widget (if it has one) and all the listdevicewidgets
    void closeWidget();

    /// getter for the group data.
    const cor::Group& group() { return mGroup; }

    /// getter for the desired height of the widget
    int widgetHeightSum();

    /// moves widgets into their proper location on a grid.
    void moveWidgets(QSize size, QPoint offset);

    /// show the devices provided
    void showDevices(const std::list<cor::Light>& devices);

    /// update the top widget
    void updateTopWidget();

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

    /*!
     * \brief buttonsShown emitted when the buttons are shown or hidden. emits the key and a boolean
     *        representing whether the buttons are shown.
     */
    void buttonsShown(QString, bool);

    /// signals that the group has changed and the size of the widget has potentially changed.
    void groupChanged(QString);

protected:

    /// resizes interal widgets when a resize event is triggered
    void resizeEvent(QResizeEvent *);

    /*!
     * \brief mouseReleaseEvent picks up when a click (or a tap on mobile) is released.
     */
    virtual void mouseReleaseEvent(QMouseEvent *);

    /*!
     * \brief paintEvent paints the background of the widget
     */
    void paintEvent(QPaintEvent *event);

private slots:

    /// a group button was pressed. This switches the shown devices to be just the devices in that group
    void groupPressed(QString key);

    /// select all toggled for a given group. true if selecting all of that group, false if deselecting all for that group
    void selectAllToggled(QString key, bool selectAll);

    /*!
     * \brief handleClicked hangles a ListDeviceWidget getting clicked, emits a the collection's key and the device's key.
     * \param key the key of the ListDeviceWidget
     */
    void handleClicked(QString key);

    /// handles when an on/off switch switch for any given device is toggled
    void handleToggledSwitch(QString key, bool isOn) { emit deviceSwitchToggled(key, isOn); }

private:

    /// comm data
    CommLayer *mComm;

    //// group data
    GroupData *mGroupData;

    /// resize the widget
    void resize();

    /// top widget that holds the groups, if any are available.
    GroupButtonsWidget *mGroupsButtonWidget;

    /// layout of all widgets except the dropdownwidget
    cor::ListLayout mListLayout;

    /// widget used for background of grid
    QWidget *mWidget;

    /*!
     * \brief mLayout layout that displays all of the sub widgets.
     */
    QVBoxLayout *mLayout;

    /// widget for showing/hiding and selecting/deselecting
    DropdownTopWidget *mDropdownTopWidget;

    /*!
     * \brief computeHighlightColor computes the top of the widgets highlight color
     *        based on how many devices are selected. When all devices are selected, it is the
     *        same color as a device's highlight. when only some of the devices are selected,
     *        it is slightly darker.
     * \return the QColor that should be used to higlight the widget.
     */
    QColor computeHighlightColor();

    /// stored data for the group.
    cor::Group mGroup;

};

#endif // LISTROOMWIDGET_H