#ifndef DISCOVERYPAGE_H
#define DISCOVERYPAGE_H

#include <QWidget>
#include <QPushButton>
#include <QListWidget>
#include <QLayout>

#include "cor/page.h"
#include "comm/commlayer.h"
#include "cor/devicelist.h"
#include "floatinglayout.h"

class DiscoveryArduCorWidget;
class DiscoveryHueWidget;
class DiscoveryNanoLeafWidget;

/*!
 * \copyright
 * Copyright (C) 2015 - 2019.
 * Released under the GNU General Public License.
 *
 *
 * \brief The DiscoveryPage class provides the user an interface to discover new connections with lights.
 *        Each enabled communication type gets its own dedicated page. When the user opens the app the first time,
 *        they are required to connect to a light before they are able to leave this page.
 *
 */
class DiscoveryPage : public QWidget, public cor::Page
{
    Q_OBJECT

public:
    /*!
     * Constructor
     */
    explicit DiscoveryPage(QWidget *parent, cor::DeviceList *data, CommLayer *layer, AppSettings *appSettings);

    /// debug function
    void openStartForDebug() { mForceStartOpen = true; }

    /*!
     * \brief updateTopMenu helper for adjusting the top menu to show
     *        only available CommTypes. If only one commtype is available, don't show
     *        any of the buttons.
     */
    void updateTopMenu();

    /// called when the widget is shown
    void show();

    /// called when the widget is hidden
    void hide();

signals:
    /*!
     * \brief startButtonClicked sent whenver the start button is clicked so that the discovery
     *        page can be hidden.
     */
    void startButtonClicked();

    /*!
     * \brief settingsButtonClicked sent whenever the settings button is clicked.
     */
    void settingsButtonClicked();

    /*!
     * \brief closeWithoutTransition emits to a parent to close this page without transition.
     */
    void closeWithoutTransition();

private slots:
    /*!
     * \brief startClicked handle when the start button is clicked.
     */
    void startClicked();

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
     * \brief widgetConnectionStateChanged handles whenever a connection status changes for any commtype
     * \param type the commtype where the connection status changes
     * \param connectionState int representation of a EConnectionState that gives the current connection
     *        status.
     */
    void widgetConnectionStateChanged(EProtocolType type, EConnectionState connectionState);

    /*!
     * \brief floatingLayoutButtonPressed handles whenever a floating layout button is presed
     * \param button key for floating layout button
     */
    void floatingLayoutButtonPressed(QString button);

protected:

    /*!
     * \brief resizeEvent called every time the main window is resized.
     */
    void resizeEvent(QResizeEvent *);

    /*!
     * \brief paintEvent paint event for rendering. used to overwrite the background
     *        color of the discovery page so that it hides everything behind it.
     */
    void paintEvent(QPaintEvent *);

private:

    /*!
     * \brief data layer that maintains and tracks the states of the lights
     *        and the saved data of the GUI
     */
    cor::DeviceList *mData;

    /// moves floating layouts to top right position of screen.
    void moveFloatingLayouts();

    /// true if protocol has been discovered, false if its still waiting on its first device.
    bool checkIfDiscovered(EProtocolType type);

    /// floating layout for commtype button
    FloatingLayout *mHorizontalFloatingLayout;

    /// floating layout for settings button
    FloatingLayout *mVerticalFloatingLayout;

    /// used for certain discovery pages as an additional button
    FloatingLayout *mOptionalFloatingLayout;

    /// discovery widget for ArduCor
    DiscoveryArduCorWidget *mArduCorWidget;

    /// discovery widget for hue products
    DiscoveryHueWidget *mHueWidget;

    /// discovery widget for nanoleaf products
    DiscoveryNanoLeafWidget *mNanoLeafWidget;

    /// spacer for floating layouts
    QWidget *mSpacer;

    /// placeholder for main widget
    QWidget *mPlaceholder;

    /// start button of widget
    QPushButton *mStartButton;

    /// layout of widget
    QVBoxLayout *mLayout;

    /*!
     * \brief resizeTopMenu resize buttons at top that switch between Hue, Serial, etc.
     */
    void resizeTopMenu();

    /*!
     * \brief changeCommTypeConnectionState change the connection state and the associated UI
     *        elements based on the parameters.
     * \param type the commtype to change the connection state on
     * \param newState the new state for the commtype.
     */
    void changeCommTypeConnectionState(EProtocolType type, EConnectionState newState);

    /*!
     * \brief mComm pointer to CommLayer.
     */
    CommLayer *mComm;

    /*!
     * \brief mType current ProtocolSettings displaying its discovery page.
     */
    EProtocolType mType;

    /*!
     * \brief mButtonIcons reference to a QPixmap for each of the comm buttons.
     */
    std::vector<QPixmap> mButtonIcons;

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

    /*!
     * \brief mStartTime start time for the application. If discovery happens quickly,
     *        the app automatically hides this page. This timer tracks that.
     */
    QTime mStartTime;

    /// resizes the icons on the discovery page if the floating layout size changes (this typically happens only once)
    void resizeButtonIcons();

    /// the last floating layout height, used to test if resize is needed.
    int mLastFloatingHeight;

    /// pointer to the app states that determine if a protocol (such as arducor or nanoleaf) is currently enabled
    AppSettings *mAppSettings;
};

#endif // DISCOVERYPAGE_H
