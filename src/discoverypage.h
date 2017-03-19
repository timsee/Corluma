#ifndef DISCOVERYPAGE_H
#define DISCOVERYPAGE_H

#include <QWidget>
#include <QPushButton>
#include <QListWidget>

#include "lightingpage.h"
#include "commlayer.h"

namespace Ui {
class DiscoveryPage;
}

/*!
 * \copyright
 * Copyright (C) 2015 - 2017.
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
     * Deconstructor
     */
    ~DiscoveryPage();

    /*!
     * \brief connectCommLayer connect to commlayer. In a future update the commLayer pointer on
     *        every page will be totally removed in favor of DataSync, but for now theres some
     *        edge cases that require certain pages to have a commlayer pointer.
     * \param layer commlayer
     */
    void connectCommLayer(CommLayer *layer) {
        mComm = layer;
        connect(mComm, SIGNAL(hueDiscoveryStateChange(int)), this, SLOT(hueDiscoveryUpdate(int)));
    }

    /// debug function
    void openStartForDebug() { mForceStartOpen = true; }

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

public slots:

    /*!
     * \brief hueDiscoveryUpdate provides an int representation of the EHueDiscoveryState
     *        of Hue's discovery object. Used by the connectionList to display the current
     *        state.
     */
    void hueDiscoveryUpdate(int);

private slots:
    /*!
     * \brief startClicked handle when the start button is clicked.
     */
    void startClicked() { emit startButtonClicked(); }

    /*!
     * \brief plusButtonClicked called whenever the plus button is clicked
     */
    void plusButtonClicked();

    /*!
     * \brief settingsButtonClicked called whenever the settings button is clicked.
     */
    void settingsButtonClicked(bool) { emit settingsButtonClicked(); }

    /*!
     * \brief minusButtonClicked called whenever the minus button is clicked
     */
    void minusButtonClicked();

    /*!
     * \brief connectedListClicked The connected list was clicked on a discovery page.
     *        This allows the user to select one of the connections, but its internal logic
     *        is handled differently between different CommTypes.
     */
    void connectedListClicked(QListWidgetItem *);

    /*!
     * \brief discoveringListClicked The discovering list was clicked on a discovery page.
     *        This allows the user to select one of the connections, but its internal logic
     *        is handled differently between different CommTypes.
     */
    void discoveringListClicked(QListWidgetItem *);

    /*!
     * \brief commTypeSelected called when the comm type updates and changes
     */
    void commTypeSelected(int);

    /*!
     * \brief renderUI renders expensive assets if and only if the assets have had any
     *        change of state.
     */
    void renderUI();

protected:
    /*!
     * \brief showEvent called before the this page is shown. Used to sync up
     *        any changes that may have happened on other pages.
     */
    void showEvent(QShowEvent *);

    /*!
     * \brief hideEvent called as the page is hidden. This happens when a new page
     *        is displayed.
     */
    void hideEvent(QHideEvent *);

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
     * \brief resizeTopMenu resize buttons at top that switch between Hue, Serial, etc.
     */
    void resizeTopMenu();

    /*!
     * \brief yunLineEditHelper helper function for setting the yun line edit.
     */
    void yunLineEditHelper();

    /*!
     * \brief updateHueStatusIcon update the main image for the hue discovery page which
     *        shows what step you're on.
     * \param iconPath the path to the new icon for the page.
     */
    void updateHueStatusIcon(QString iconPath);

    /*!
     * \brief ui pointer to Qt UI form.
     */
    Ui::DiscoveryPage *ui;

    /*!
     * \brief handleSerialDiscovery called by render update thread to display and update
     *        the serial discovered list.
     */
    void handleSerialDiscovery(bool isCurrentCommType);

    /*!
     * \brief handleYunDiscovery called by render update thread to display and update
     *        the yun discovered list and undiscovered list.
     */
    void handleYunDiscovery(bool isCurrentCommType);

    /*!
     * \brief fillList take the connections list and convert it to elements in the QListWidget provided.
     * \param list a QListWidget
     * \param connections a list of connections to fill into the QListWidget.
     */
    void fillList(QListWidget *list, std::list<QString>& connections);

    /*!
     * \brief showAvailableCommTypeButtons helper for adjusting the top menu to show
     *        only available CommTypes. If only one commtype is available, don't show
     *        any of the buttons.
     */
    void showAvailableCommTypeButtons();

    /*!
     * \brief changeCommTypeConnectionState change the connection state and the associated UI
     *        elements based on the parameters.
     * \param type the commtype to change the connection state on
     * \param newState the new state for the commtype.
     */
    void changeCommTypeConnectionState(ECommType type, EConnectionState newState);

    /*!
     * \brief buttonByType helper for getting a QPushButton pointer based off
     *        of a commtype.
     * \param type the commtype to get a button for.
     * \return a pointer to a button used for selecting a commtype.
     */
    QPushButton *buttonByType(ECommType type);


    /*!
     * \brief doesYunControllerExistAlready checks both UDP and HTTP yun device lists to see if it has any knowledge
     *        of the given controller.
     * \param controller name of controller
     * \return true if yun controller exists on discovered or undiscovered list, false otherwise
     */
    bool doesYunControllerExistAlready(QString controller);

    /*!
     * \brief mLastIP stores last IP address used for Yuns
     */
    QString mLastIP;

    /*!
     * \brief mComm pointer to CommLayer.
     */
    CommLayer *mComm;

    /*!
     * \brief mType current CommType displaying its discovery page.
     */
    ECommType mType;

    /*!
     * \brief mHueDiscoveryState stored state of the Hue Discovery methods.
     *        This is udpated internally by the hueDiscoveryUpdate(int) slot.
     */
    EHueDiscoveryState mHueDiscoveryState;

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
