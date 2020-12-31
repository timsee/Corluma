#ifndef DISCOVERYPAGE_H
#define DISCOVERYPAGE_H

#include <QLayout>
#include <QListWidget>
#include <QPushButton>
#include <QWidget>

#include "comm/commlayer.h"
#include "cor/lightlist.h"
#include "cor/objects/page.h"
#include "floatinglayout.h"

class DiscoveryArduCorWidget;
class DiscoveryHueWidget;
class DiscoveryNanoLeafWidget;
class ControllerWidget;

/*!
 * \copyright
 * Copyright (C) 2015 - 2020.
 * Released under the GNU General Public License.
 *
 *
 * \brief The DiscoveryPage class provides the user an interface to discover new light hardware.
 * Each type of hardware gets its own page with its own dedicated widgets. This page is used to walk
 * the user through set up steps (IE, for a Hue Bridge, it shows the user auto discovered bridges,
 * allows them to enter an IP address manually, and tells them to press the button on the bridge to
 * finish the discovery steps). Then, once a light is fully set up, the user can modify some
 * settings, or delete the connection info. This page can also be used to view light-specific
 * metadata that doesn't have a place in the UI of the rest of the app. For instance, for Hue
 * Bridges you can view the schedules in the bridge, and some metadata such as the API version.
 *
 */
class DiscoveryWidget : public QWidget, public cor::Page {
    Q_OBJECT

public:
    /*!
     * Constructor
     */
    explicit DiscoveryWidget(QWidget* parent,
                             cor::LightList* data,
                             CommLayer* layer,
                             AppSettings* appSettings,
                             ControllerWidget* controllerPage);

    /// getter for hue widget
    DiscoveryHueWidget* hueWidget() { return mHueWidget; }

    /// debug function
    void openStartForDebug() { mForceStartOpen = true; }

    /// getter for current protocol
    EProtocolType currentProtocol() { return mType; }

    /// change the row height of the discovery widgets
    void changeRowHeight(int rowHeight);

    /// switch the protocol of the widget.
    void switchProtocol(EProtocolType type) {
        mType = type;
        protocolTypeSelected(mType);
    }

    /// called when the widget is shown
    void show();

    /// called when the widget is hidden
    void hide();

    /// highlight the lights on the discovery page.
    void highlightLights();

public slots:
    /*!
     * \brief floatingLayoutButtonPressed handles whenever a floating layout button is presed
     * \param button key for floating layout button
     */
    void floatingLayoutButtonPressed(const QString& button);

signals:

    /// show the controller widget.
    void showControllerWidget();

    /// a connection state has changed for a widget.
    void connectionStateChanged(EProtocolType type, EConnectionState newState);

    /// emits when an indiviual light is clicked from a controller's preview
    void lightClicked(QString);

    /// emits when a select all/none button is clicked from a controller's preview.
    void selectAllLightsClicked(QString, EProtocolType, bool);

public slots:
    /// handle when a light is deleted.
    void deleteLight(QString);

private slots:

    /*!
     * \brief protocolTypeSelected called when the comm type updates and changes
     */
    void protocolTypeSelected(EProtocolType);

    /*!
     * \brief renderUI renders expensive assets if and only if the assets have had any
     *        change of state.
     */
    void renderUI();

    /*!
     * \brief widgetConnectionStateChanged handles whenever a connection status changes for any
     * commtype
     *
     * \param type the commtype where the connection status changes
     * \param connectionState int representation of a EConnectionState that gives the current
     * connection status.
     */
    void widgetConnectionStateChanged(EProtocolType type, EConnectionState connectionState);

    /// should show a controller widget.
    void shouldShowControllerWidget();

protected:
    /*!
     * \brief resizeEvent called every time the main window is resized.
     */
    void resizeEvent(QResizeEvent*);

    /*!
     * \brief paintEvent paint event for rendering. used to overwrite the background
     *        color of the discovery page so that it hides everything behind it.
     */
    void paintEvent(QPaintEvent*);

private:
    /*!
     * \brief data layer that maintains and tracks the states of the lights
     *        and the saved data of the GUI
     */
    cor::LightList* mData;

    /// true if protocol has been discovered, false if its still waiting on its first device.
    bool checkIfDiscovered(EProtocolType type);

    /// spacer for floating layouts
    QWidget* mSpacer;

    /// placeholder for main widget
    QWidget* mPlaceholder;

    /// discovery widget for ArduCor
    DiscoveryArduCorWidget* mArduCorWidget;

    /// discovery widget for hue products
    DiscoveryHueWidget* mHueWidget;

    /// discovery widget for nanoleaf products
    DiscoveryNanoLeafWidget* mNanoLeafWidget;

    /*!
     * \brief resize resize buttons at top that switch between Hue, Serial, etc.
     */
    void resize();

    /*!
     * \brief changeCommTypeConnectionState change the connection state and the associated UI
     *        elements based on the parameters.
     *
     * \param type the commtype to change the connection state on
     * \param newState the new state for the commtype.
     */
    void changeCommTypeConnectionState(EProtocolType type, EConnectionState newState);

    /*!
     * \brief mComm pointer to CommLayer.
     */
    CommLayer* mComm;

    /*!
     * \brief mType current ProtocolSettings displaying its discovery page.
     */
    EProtocolType mType;

    /*!
     * \brief mConnectionStates The connection state of any
     *        specific commtype
     */
    std::vector<EConnectionState> mConnectionStates;

    /*!
     * \brief mForceStartOpen set by debugging functions, forces the start button
     *        to stay active.
     */
    bool mForceStartOpen;

    /// pointer to the app states that determine if a protocol (such as arducor or nanoleaf) is
    /// currently enabled
    AppSettings* mAppSettings;

    /*!
     * \brief mRenderThread timer that calls a renderUI() function on each of the pages.
     *        This function is used to render more expensive assets if and only if they
     *        received a change in state since the previous renderUI call.
     */
    QTimer* mRenderThread;
};

#endif // DISCOVERYPAGE_H
