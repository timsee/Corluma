#ifndef DISCOVERYHUEWIDGET_H
#define DISCOVERYHUEWIDGET_H

#include <QScrollArea>

#include "comm/hue/bridgebutton.h"
#include "comm/hue/bridgegroupswidget.h"
#include "comm/hue/bridgescheduleswidget.h"
#include "cor/widgets/listwidget.h"
#include "cor/widgets/textinputwidget.h"
#include "discovery/discoverytypewidget.h"
#include "display/displaypreviewbridgewidget.h"
#include "greyoutoverlay.h"

/*!
 * \copyright
 * Copyright (C) 2015 - 2020.
 * Released under the GNU General Public License.
 *
 *
 * \brief The DiscoveryHueWidget class helps in discovering a Hue Bridge.
 *        It attempts to find a bridge automatically, and if attempts fail,
 *        it prompts the user to enter the IP address. It also prompts the user
 *        through setup steps such as pressing button on the bridge.
 */
class DiscoveryHueWidget : public DiscoveryTypeWidget {
    Q_OBJECT

public:
    /*!
     * \brief DiscoverySerialWidget constructor
     *
     * \param parent
     */
    explicit DiscoveryHueWidget(QWidget* parent,
                                CommLayer* comm,
                                cor::LightList* selectedLights,
                                ControllerWidget* controllerPage);

    /// change the row height for the discovery hue widget
    void changeRowHeight(int rowHeight) {
        mRowHeight = rowHeight;
        for (auto widget : mBridgeWidgets) {
            if (widget != nullptr) {
                widget->changeRowHeight(rowHeight);
            }
        }
    }

    /// show the widget
    void showWidget();

    /// handle when a bridge updates its name.
    void handleBridgeNameUpdate(const QString& bridgeID, const QString& newName);

    /// handle a light name update
    void updateLightNames();

    /// handle when lights are deleted.
    void handleDeletedLights(const std::vector<QString>&);

    /// See DiscoveryWidget.h
    void handleDiscovery(bool isActive) override;

    /*!
     * \brief resize size the widget programmatically
     *
     * \param resizeFullWidget true to resize the widget itself, false to just
     *        resize its contents.
     */
    void resize();

    /// check if an IP exists
    void checkIfIPExists(const QString& IP) override;

    /// prompt for the IP widget
    QString IPWidgetPrompt() override { return "Add an IP Address for the Bridge:"; }

    /// default value for the IP widget
    QString IPWidgetDefaultValue() override { return "192.168.0.100"; }

    /// delete a light from the discovery page.
    void deleteLight(const QString&) override;

    /// handles how to higlight lights.
    void highlightLights() override;

private slots:
    /// handles when the greyout is clicked
    void greyOutClicked() override;

    /// handles when a bridge is pressed on the bridge list
    void bridgePressed(const QString&);

    /// handles an IP from the IP widget, giving a warning if necessary
    void textInputAddedIP(const QString& IP);

    /// handles when a light is clicked
    void lightSelected(QString key);

    /// handles when a light is deselected.
    void lightDeselected(QString key);

    /// handles when select all is clicked
    void clickedSelectAll(QString key, EProtocolType protocol);

    /// handles when deselect all is clicked
    void clickedDeselectAll(QString key, EProtocolType protocol);

    /// a button for a bridge is pressed, so this bridge should be displayed
    void bridgeButtonPressed(QString);

    /// delete a bridge is pressed, so this bridge should be deleted.
    void deleteBridgePressed(QString, EProtocolType);

protected:
    /// called when the widget resizes
    virtual void resizeEvent(QResizeEvent*) override;

private:
    /*!
     * \brief hueDiscoveryUpdate provides an int representation of the EHueDiscoveryState
     *        of Hue's discovery object. Used by the connectionList to display the current
     *        state.
     */
    void hueDiscoveryUpdate(EHueDiscoveryState);

    /// highlight the bridge buttons, so that the currently selected bridge is highlighted.
    void highlightBridgeButtons();

    /// updates the GUI of the bridge
    void updateBridgeGUI();

    /// true if bridge is discovered, false if not.
    bool mBridgeDiscovered;

    /// list of selected lights.
    cor::LightList* mSelectedLights;

    /// buttons for displaying a bridge
    std::vector<hue::BridgeButton*> mBridgeButtons;

    /// vector of bridge widgets, only one is displayed on the screen at a time
    std::vector<hue::DisplayPreviewBridgeWidget*> mBridgeWidgets;

    /// index of a bridge.
    std::uint32_t mBridgeIndex;

    /// height of a row of a widget
    int mRowHeight;

    /*!
     * \brief updateHueStatusIcon update the main image for the hue discovery page which
     *        shows what step you're on.
     *
     * \param iconPath the path to the new icon for the page.
     */
    void updateHueStatusIcon(QString iconPath);

    /*!
     * \brief mHueDiscoveryState stored state of the Hue Discovery methods.
     *        This is udpated internally by the hueDiscoveryUpdate(int) slot.
     */
    EHueDiscoveryState mHueDiscoveryState;
};

#endif // DISCOVERYHUEWIDGET_H
