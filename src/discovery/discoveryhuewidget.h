#ifndef DISCOVERYHUEWIDGET_H
#define DISCOVERYHUEWIDGET_H

#include <QScrollArea>

#include "discovery/discoverywidget.h"
#include "editablefieldwidget.h"
#include "hue/bridgeinfowidget.h"
#include "hue/lightdiscovery.h"
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

public slots:

    /*!
     * \brief IPFieldChanged IP Address field has changed, try to discover the IP
     *        address provided.
     */
    void IPFieldChanged(QString);

protected:
    /*!
     * \brief resizeEvent called every time the main window is resized.
     */
    void resizeEvent(QResizeEvent *);

private slots:

    /// handles when a discover hue buttons is pressed
    void discoverHuesPressed(QString);

    /// handles when a bridge is pressed on the bridge list
    void bridgePressed(QString);

    /// handles when the close button is pressed
    void hueDiscoveryClosePressed();

    /// handles when greyout is finished
    void greyOutFadeComplete();

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

    /// label to prompt the user through the application.
    QLabel *mLabel;

    /// image for displaying a graphic to help with current step of widget.
    QLabel *mImage;

    /// label for displaying before the IP Address editable field.
    QLabel *mIPAddressInfo;

    /// cachced pixmap of hue bridge
    QPixmap mBridgePixmap;

    /// IP address of bridge
    EditableFieldWidget *mIPAddress;

    /// instructions on what to do with IP address
    QLabel *mIPAddressDebug;

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

    /// layout for bridge picture and info
    QHBoxLayout *mBridgeLayout;

    /// layout for IP information
    QHBoxLayout *mIPLayout;
};

#endif // DISCOVERYHUEWIDGET_H
