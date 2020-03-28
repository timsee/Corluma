#ifndef PARENTGROUPWIDGET_H
#define PARENTGROUPWIDGET_H

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
 *
 * @brief The ParentGroupWidget class is a widget that displays all the subgroups and lights
 * associated with a parent. subgroups only go one level deep, so if a parent is "First Floor
 * Lights" and this parent contains "Living Room" as a subgroup, and "Living Room" contains
 * "Computer Desk" of a subgroup of it, then "Computer Desk" is treated as a subgroup of "First
 * Floor Lights", not "Living Room"
 */
class ParentGroupWidget : public cor::ListItemWidget {
    Q_OBJECT
public:
    /// constructor
    ParentGroupWidget(const cor::Group& room,
                      const std::vector<std::uint64_t>& subgroups,
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

    /// update the widget with new information.
    void updateState(const cor::Group& group, const std::vector<std::uint64_t>& subgroups);

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
    const cor::Group& group() { return mGroup; }

    /// getter for the desired height of the widget
    int widgetHeightSum();

signals:

    /*!
     * \brief deviceClicked emitted whenever a device is clicked on the widget. Emits both the
     * widget key and the device key.
     */
    void deviceClicked(std::uint64_t, QString);

    /*!
     * \brief selectAllClicked emitted when select all is clicked. Should add all devices to data
     * layer.
     */
    void allButtonPressed(std::uint64_t, bool);

    /*!
     * \brief buttonsShown emitted when the buttons are shown or hidden. emits the key and a boolean
     * representing whether the buttons are shown.
     */
    void buttonsShown(std::uint64_t, bool);

    /// signals that the group has changed and the size of the widget has potentially changed.
    void groupChanged(std::uint64_t);

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
    bool hasSubgroups() const noexcept { return !mSubgroups.empty(); }

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
    cor::Group mGroup;

    /// stored data for the subgroups
    std::vector<std::uint64_t> mSubgroups;

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
    void updateGroupNames(const std::vector<std::uint64_t>& subgroups);

    /// removes any lights that are not found during an update call.
    void removeLightsIfNotFound(const cor::Group& group);

    /// true if showing lights and groups, false if just showing name.
    bool mIsOpen;

    /// helper to count the checked and reachable lights.
    std::pair<std::uint32_t, std::uint32_t> countCheckedAndReachableLights(const cor::Group& group);
};

#endif // PARENTGROUPWIDGET_H
