#ifndef LISTROOMWIDGET_H
#define LISTROOMWIDGET_H

#include <QLabel>
#include <QObject>
#include <QPushButton>
#include <QWidget>

#include "comm/commlayer.h"
#include "cor/lightlist.h"
#include "cor/listlayout.h"
#include "cor/objects/group.h"
#include "cor/widgets/listitemwidget.h"
#include "dropdowntopwidget.h"
#include "groupbuttonswidget.h"
#include "listlightwidget.h"

/*!
 * \copyright
 * Copyright (C) 2015 - 2020.
 * Released under the GNU General Public License.
 *
 * \brief The ListDevicesGroupWidget class is a subclassed ListCollectionWidget that
 * displays all the devices in a collection. It contains a widget at the top that
 * gives info on the collection and the ability to edit it. It also contains an array
 * of ListDeviceWidgets, which can be shown or hidden by clicking the top widget.
 */
class ListRoomWidget : public cor::ListItemWidget {
    Q_OBJECT
public:
    /*!
     * \brief ListDevicesGroupWidget constructor
     *
     * \param group group of lights
     * \param key unique key for collection
     */
    ListRoomWidget(const cor::Room& room,
                   CommLayer* comm,
                   GroupData* groups,
                   const QString& key,
                   cor::EListType listType,
                   cor::EWidgetType type,
                   QWidget* parent);

    /*!
     * \brief setCheckedLights takes a list of devices and compares it against all devices in the
     * widget. If the device exists in both the widgets and the devices, it is set as checked.
     * Otherwise, the widget is set to unchecked.
     *
     * \param lights list of lights to compare to the widget.
     */
    void setCheckedLights(const std::vector<QString>& lights);

    /// update the room with new information.
    void updateRoom(const cor::Room& room);

    /*!
     * \brief setShowButtons shows and hides all buttons on the widget
     *
     * \param show true to show, false otherwise.
     */
    void setShowButtons(bool show);

    /// hides the group buttons widget (if it has one) and all the listdevicewidgets
    void closeWidget();

    /// true if the widget is currently in an open state (and showing groups or lights) or closed
    /// (showing just its name)
    bool isOpen() const noexcept { return mIsOpen; }

    /// getter for the room data.
    const cor::Room& room() { return mRoom; }

    /// getter for the desired height of the widget
    int widgetHeightSum();

signals:

    /*!
     * \brief deviceClicked emitted whenever a device is clicked on the widget. Emits both the
     * widget key and the device key.
     */
    void deviceClicked(QString, QString);

    /*!
     * \brief selectAllClicked emitted when select all is clicked. Should add all devices to data
     * layer.
     */
    void allButtonPressed(QString, bool);

    /*!
     * \brief buttonsShown emitted when the buttons are shown or hidden. emits the key and a boolean
     * representing whether the buttons are shown.
     */
    void buttonsShown(QString, bool);

    /// signals that the group has changed and the size of the widget has potentially changed.
    void groupChanged(QString);

protected:
    /// resizes interal widgets when a resize event is triggered
    void resizeEvent(QResizeEvent*);

    /*!
     * \brief paintEvent paints the background of the widget
     */
    void paintEvent(QPaintEvent* event);

private slots:

    /// called when the dropdown top widget is pressed
    void dropdownTopWidgetPressed();

    /// a group button was pressed. This switches the shown devices to be just the devices in that
    /// group
    void groupPressed(const QString& key);

    /// select all toggled for a given group. true if selecting all of that group, false if
    /// deselecting all for that group
    void selectAllToggled(const QString& key, bool selectAll);

    /*!
     * \brief handleClicked hangles a ListDeviceWidget getting clicked, emits a the collection's key
     * and the device's key.
     *
     * \param key the key of the ListDeviceWidget
     */
    void handleClicked(const QString& key);

private:
    /// returns the number of widgets shown.
    std::size_t numberOfWidgetsShown();

    /// resize the widget
    void resize();

    /// moves widgets into their proper location on a grid.
    void moveWidgets(QSize size, QPoint offset);

    /// show the devices provided
    void showLights(const std::vector<cor::Light>& lights);

    /// update the top widget
    void updateTopWidget();

    /// true if the room has subgroups, false if it doesn't
    bool hasSubgroups() const noexcept { return !mRoom.subgroups().empty(); }

    /// type of widget
    cor::EWidgetType mType;

    /// comm data
    CommLayer* mComm;

    //// group data
    GroupData* mGroupData;

    /// if theres subgroups, this stores the last subgroup name
    QString mLastSubGroupName;

    /// top widget that holds the groups, if any are available.
    GroupButtonsWidget* mGroupsButtonWidget;

    /// layout of all widgets except the dropdownwidget
    cor::ListLayout mListLayout;

    /// widget for showing/hiding and selecting/deselecting
    DropdownTopWidget* mDropdownTopWidget;

    /// stored data for the group.
    cor::Room mRoom;

    /// checks if a group with no subgroups should show widgets
    bool checkIfShowWidgets();

    /*!
     * \brief updateLightWidgets updates the light widgets with the provided lights
     * \param lights lights that should run updates
     * \param updateOnlyVisible true if only the visible lights should update, false if all lights
     * should update
     */
    void updateLightWidgets(const std::vector<cor::Light>& lights, bool updateOnlyVisible);

    /// updates the group names in the GroupButtonsWidget
    void updateGroupNames(const cor::Room& room);

    /// removes any lights that are not found during an update call.
    void removeLightsIfNotFound(const cor::Room& room);

    /// true if showing lights and groups, false if just showing name.
    bool mIsOpen;

    /// helper to count the checked and reachable lights.
    std::pair<std::uint32_t, std::uint32_t> countCheckedAndReachableLights(const cor::Group& group);
};

#endif // LISTROOMWIDGET_H
