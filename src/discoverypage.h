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


signals:
    /*!
     * \brief startButtonClicked sent whenver the start button is clicked so that the discovery
     *        page can be hidden.
     */
    void startButtonClicked();

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
     * \brief ui pointer to Qt UI form.
     */
    Ui::DiscoveryPage *ui;

    /*!
     * \brief handleSerialDiscovery called by render update thread to display and update
     *        the serial discovered list.
     */
    void handleSerialDiscovery();

    /*!
     * \brief handleYunDiscovery called by render update thread to display and update
     *        the yun discovered list and undiscovered list.
     */
    void handleYunDiscovery();

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
};

#endif // DISCOVERYPAGE_H
