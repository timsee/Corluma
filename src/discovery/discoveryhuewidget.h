#ifndef DISCOVERYHUEWIDGET_H
#define DISCOVERYHUEWIDGET_H

#include "discovery/discoverywidget.h"

/*!
 * \copyright
 * Copyright (C) 2015 - 2017.
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
    explicit DiscoveryHueWidget(CommLayer *comm, QWidget *parent = 0);

    /*!
     * \brief Destructor
     */
    ~DiscoveryHueWidget();

    /// See DiscoveryWidget.h
    void handleDiscovery(bool isActive);

public slots:
    /*!
     * \brief hueDiscoveryUpdate provides an int representation of the EHueDiscoveryState
     *        of Hue's discovery object. Used by the connectionList to display the current
     *        state.
     */
    void hueDiscoveryUpdate(int);

private:

    /// label to prompt the user through the application.
    QLabel *mLabel;

    /// image for displaying a graphic to help with current step of widget.
    QLabel *mImage;

    /// placeholder to add blank space to the hue widget
    QLabel *mPlaceholder;

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
