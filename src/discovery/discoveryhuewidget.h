#ifndef DISCOVERYHUEWIDGET_H
#define DISCOVERYHUEWIDGET_H

#include <QScrollArea>

#include "discovery/discoverywidget.h"
#include "editablefieldwidget.h"
#include "hue/bridgeinfowidget.h"
#include "hue/lightdiscovery.h"
#include "hue/bridgegroupswidget.h"
#include "hue/bridgescheduleswidget.h"
#include "greyoutoverlay.h"


/*!
 * \copyright
 * Copyright (C) 2015 - 2018.
 * Released under the GNU General Public License.
 *
 *
 * \brief The DiscoveryHueWidget class helps in discovering a Hue Bridge.
 *        It attempts to find a bridge automatically, and if attempts fail,
 *        it prompts the user to enter the IP address. It also prompts the user
 *        through setup steps such as pressing button on the bridge.
 */
class DiscoveryHueWidget: public DiscoveryWidget
{
    Q_OBJECT

public:
    /*!
     * \brief DiscoverySerialWidget constructor
     * \param parent
     */
    explicit DiscoveryHueWidget(CommLayer *comm, QWidget *parent);

    /// See DiscoveryWidget.h
    void handleDiscovery(bool isActive);

    /*!
     * \brief resize size the widget programmatically
     * \param resizeFullWidget true to resize the widget itself, false to just
     *        resize its contents.
     */
    void resize();

private slots:

    /// handles when a discover hue buttons is pressed
    void discoverHuesPressed(QString);

    /// handles when a bridge is pressed on the bridge list
    void bridgePressed(QString);

    /// handles when the close button is pressed
    void hueDiscoveryClosePressed();

    /// closes the group widget
    void groupsClosePressed();

    /// closes the schedule widget
    void schedulesClosePressed();

    /// handles when the schedules button is pressed for a hue::BridgeInfoWidget
    void schedulesPressed(QString);

    /// handles when the groups button is pressed for a hue::BridgeInfoWidget
    void groupsPressed(QString);

    /// handles when a name change is trigged from a hue::BridgeInfoWidget
    void changedName(QString, QString);

    /// handles when greyout is finished
    void greyOutFadeComplete();

    /// handles when a bridge is deleted from a BridgeInfoWidget
    void deleteBridgeFromAppData(hue::Bridge);

protected:

    /// called when the widget resizes
    virtual void resizeEvent(QResizeEvent *);

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

    /// true to grey out, false to hide the greyout
    void greyOut(bool show);

    /// overlay that greys out everythign under it
    GreyOutOverlay *mGreyOut;

    /// widget for discovering hue lights
    hue::LightDiscovery *mHueLightDiscovery;

    /// widget for displaying the groups of a bridge
    hue::BridgeGroupsWidget *mBridgeGroupsWidget;

    /// widget for displaying the schedules of a widget
    hue::BridgeSchedulesWidget *mBridgeSchedulesWidget;

    /// label to prompt the user through the application.
    QLabel *mLabel;

    /// widget used for scroll area.
    QWidget *mScrollAreaWidget;

    /// layout for scroll area
    QVBoxLayout *mScrollLayout;

    /// scroll area for displaying list.
    QScrollArea *mScrollArea;

    /// widgets for hue displayed in scroll area
    std::vector<hue::BridgeInfoWidget *> mBridgeWidgets;

    /// scaling value for size of pngs
    float mScale;

    /*!
     * \brief updateHueStatusIcon update the main image for the hue discovery page which
     *        shows what step you're on.
     * \param iconPath the path to the new icon for the page.
     */
    void updateHueStatusIcon(QString iconPath);

    /*!
     * \brief mHueDiscoveryState stored state of the Hue Discovery methods.
     *        This is udpated internally by the hueDiscoveryUpdate(int) slot.
     */
    EHueDiscoveryState mHueDiscoveryState;

    /// layout for widget
    QVBoxLayout *mLayout;
};

#endif // DISCOVERYHUEWIDGET_H
