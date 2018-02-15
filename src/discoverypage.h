#ifndef DISCOVERYPAGE_H
#define DISCOVERYPAGE_H

#include <QWidget>
#include <QPushButton>
#include <QListWidget>

#include "lightingpage.h"
#include "comm/commlayer.h"
#include "floatinglayout.h"


#ifndef MOBILE_BUILD
class DiscoverySerialWidget;
#endif
class DiscoveryYunWidget;
class DiscoveryHueWidget;

namespace Ui {
class DiscoveryPage;
}

/*!
 * \copyright
 * Copyright (C) 2015 - 2018.
 * Released under the GNU General Public License.
 *
 *
 * \brief The DiscoveryPage class provides the user an interface to discover new connections with lights.
 *        Each enabled communication type gets its own dedicated page. When the user opens the app the first time,
 *        they are required to connect to a light before they are able to leave this page.
 *
 */
class DiscoveryPage : public QWidget, public LightingPage
{
    Q_OBJECT

public:
    /*!
     * Constructor
     */
    explicit DiscoveryPage(QWidget *parent = 0);

    /*!
     * Destructor
     */
    ~DiscoveryPage();

    /*!
     * \brief connectCommLayer connect to commlayer. In a future update the commLayer pointer on
     *        every page will be totally removed in favor of DataSync, but for now theres some
     *        edge cases that require certain pages to have a commlayer pointer.
     * \param layer commlayer
     */
    void connectCommLayer(CommLayer *layer);

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
    void startClicked() { emit startButtonClicked(); }

    /*!
     * \brief commTypeSelected called when the comm type updates and changes
     */
    void commTypeSelected(int);

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
    void widgetConnectionStateChanged(int type, int connectionState);

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

    /// moves floating layouts to top right position of screen.
    void moveFloatingLayouts();

    /// floating layout for commtype button
    FloatingLayout *mHorizontalFloatingLayout;

    /// floating layout for settings button
    FloatingLayout *mVerticalFloatingLayout;

#ifndef MOBILE_BUILD
    /// discovery widget for serial devices
    DiscoverySerialWidget *mSerialWidget;
#endif

    /// discovery widget for Arduino Yuns
    DiscoveryYunWidget *mYunWidget;

    /// discovery widget for hue products
    DiscoveryHueWidget *mHueWidget;

    /*!
     * \brief resizeTopMenu resize buttons at top that switch between Hue, Serial, etc.
     */
    void resizeTopMenu();

    /*!
     * \brief ui pointer to Qt UI form.
     */
    Ui::DiscoveryPage *ui;

    /*!
     * \brief changeCommTypeConnectionState change the connection state and the associated UI
     *        elements based on the parameters.
     * \param type the commtype to change the connection state on
     * \param newState the new state for the commtype.
     */
    void changeCommTypeConnectionState(ECommType type, EConnectionState newState);

    /*!
     * \brief mComm pointer to CommLayer.
     */
    CommLayer *mComm;

    /*!
     * \brief mType current CommType displaying its discovery page.
     */
    ECommType mType;

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
};

#endif // DISCOVERYPAGE_H
