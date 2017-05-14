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
 * Copyright (C) 2015 - 2017.
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
     * \param name name of collection
     * \param devices list of devices contained within the ListDeviceGroupWidget
     * \param key unique key for collection
     * \param listHeight maximum height for collection
     * \param comm pointer to commlayer
     * \param data pointer to datalayer
     * \param hideEdit true for groups that can't be edited such as available and not reachable,
     *        false otherwise
     */
    ListDevicesGroupWidget(const QString& name,
                           std::list<SLightDevice> devices,
                           QString key,
                           int listHeight,
                           CommLayer *comm,
                           DataLayer *data,
                           bool hideEdit = false);


    /*!
     * \brief setCheckedDevices takes a list of devices and compares it against all devices in the widget.
     *        If the device exists in both the widgets and the devices, it is set as checked. Otherwise, the widget
     *        is set to unchecked.
     * \param devices list of devices to compare to the widget.
     */
    void setCheckedDevices(std::list<SLightDevice> devices);

    /*!
     * \brief updateDevices update the state and number of devices displayed in the widget.
     * \param devices the current state of all desired devices for the widget.
     */
    void updateDevices(std::list<SLightDevice> devices);

    /*!
     * \brief devices getter for all devices being displayed by the widget.
     * \return all devices being displayed by the widget.
     */
    const std::list<SLightDevice>& devices() { return mDevices; }

    /*!
     * \brief preferredSize all collection widgets must implement a preferred size. this is the size
     *        the widget wants to be. It may not end up this size but its a baseline if theres no other
     *        widgets pushing against it.
     * \return a QSize representing its ideal size.
     */
    QSize preferredSize();

    /*!
     * \brief setShowButtons shows and hides all buttons on the widget
     * \param show true to show, false otherwise.
     */
    void setShowButtons(bool show);

    /*!
     * \brief isMoodWidget true if ListMoodGroupWidget, false if ListDevicesGroupWidget. Always false in thise case.
     * \return true if ListMoodGroupWidget, false if ListDevicesGroupWidget.
     */
    bool isMoodWidget() { return false; }

    /*!
     * \brief updateRightHandButtons update the edit, select all, select none buttons by resizing them.
     */
    void updateRightHandButtons();

signals:

    /*!
     * \brief deviceClicked emitted whenever a device is clicked on the widget. Emits both the widget key
     *        and the device key.
     */
    void deviceClicked(QString, QString);

    /*!
     * \brief clearAllClicked emitted when clear all is clicked. Should remove all devices from data layer.
     */
    void clearAllClicked(QString);

    /*!
     * \brief selectAllClicked emitted when select all is clicked. Should add all devices to data layer.
     */
    void selectAllClicked(QString);

protected:

    /*!
     * \brief enterEvent picks up when the mouse pointer (or finger on mobile) enters the area of the widget.
     */
    virtual void enterEvent(QEvent *);

    /*!
     * \brief leaveEvent picks up when the mouse pointer (or finger on mobile) leaves the area of the widget.
     */
    virtual void leaveEvent(QEvent *);

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
     * \brief clearButtonClicked clear button is clicked.
     */
    void clearButtonClicked(bool);

    /*!
     * \brief selectAllButtonClicked select all button is clicked.
     */
    void selectAllButtonClicked(bool);

private:

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

    /*!
     * \brief mDevices list of current devices displayed by widgets
     */
    std::list<SLightDevice> mDevices;

    /*!
     * \brief mSelectAllButton button that selects all devices when pushed and adds them to the data layer.
     */
    QPushButton *mSelectAllButton;

    /*!
     * \brief mClearAllButton button that clears all devices when pushed and removes them from the data layer.
     */
    QPushButton *mClearAllButton;


    /*!
     * \brief mCheckedDevices number of checked devices.
     */
    int mCheckedDevices;
};

#endif // LISTDEVICESGROUPWIDGET_H
