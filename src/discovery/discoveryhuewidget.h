#ifndef DISCOVERYHUEWIDGET_H
#define DISCOVERYHUEWIDGET_H

#include <QScrollArea>

#include "comm/hue/bridgegroupswidget.h"
#include "comm/hue/bridgescheduleswidget.h"
#include "comm/hue/lightdiscovery.h"
#include "cor/widgets/listwidget.h"
#include "cor/widgets/textinputwidget.h"
#include "discovery/discoverytypewidget.h"
#include "display/displaypreviewbridgewidget.h"
#include "editablefieldwidget.h"
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

    /// handles when a discover hue buttons is pressed
    void discoverHuesPressed(const QString&);

    /// handles when a bridge is pressed on the bridge list
    void bridgePressed(const QString&);

    /// handles an IP from the IP widget, giving a warning if necessary
    void textInputAddedIP(const QString& IP);

    /// handles when the close button is pressed
    void hueDiscoveryClosePressed();

    /// handles when a name change is trigged from a hue::BridgeInfoWidget
    void changedName(const QString&, const QString&);

    /// handles when a bridge is deleted from a BridgeInfoWidget
    void deleteBridgeFromAppData(hue::Bridge);

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

    /// updates the GUI of the bridge
    void updateBridgeGUI();

    /// true if bridge is discovered, false if not.
    bool mBridgeDiscovered;

    /// list of selected lights.
    cor::LightList* mSelectedLights;

    /// widget for discovering hue lights
    hue::LightDiscovery* mHueLightDiscovery;

    /// label to prompt the user through the application.
    QLabel* mLabel;

    /// widget for displaying a scrollable list of other widgets
    cor::ListWidget* mListWidget;

    /// scaling value for size of pngs
    float mScale;

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
