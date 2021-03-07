#ifndef DISCOVERYWIDGET_H
#define DISCOVERYWIDGET_H

#include <QLabel>
#include <QLayout>
#include <QListWidget>
#include <QTimer>
#include <QWidget>

#include "comm/commlayer.h"
#include "cor/widgets/textinputwidget.h"
#include "cor/widgets/webview.h"
#include "greyoutoverlay.h"


class ControllerWidget;

/*!
 * \copyright
 * Copyright (C) 2015 - 2020.
 * Released under the GNU General Public License.
 *
 *
 * \brief The DiscoveryWidget class is a widget that gets displayed on the Discovery Page. Each
 *        subclass of this widget maps to a different communication stream. So theres a widget for
 *        Serial arduinos, a widget for Yun arduinos, and a widget for Hues.
 */
class DiscoveryTypeWidget : public QWidget {
    Q_OBJECT

public:
    /// constructor
    explicit DiscoveryTypeWidget(QWidget* parent,
                                 CommLayer* comm,
                                 ControllerWidget* controllerPage);

    /*!
     * \brief ~DiscoveryWidget Deconstructor
     */
    virtual ~DiscoveryTypeWidget() {}

    /*!
     * \brief handleDiscovery run the discovery roiutines and associated GUI updates
     *        for this widget. If its active, more GUI updates are required since its
     *        currentl displayed
     *
     * \param isActive true if displaying this widget, false if widget is in background.
     */
    virtual void handleDiscovery(bool isActive) = 0;

    /// check if an IP exists in the protocol
    virtual void checkIfIPExists(const QString& IP) = 0;

    /// prompt for IP widget
    virtual QString IPWidgetPrompt() = 0;

    /// default value for the IP widget
    virtual QString IPWidgetDefaultValue() = 0;

    /// html to display to help the user debug discovery
    virtual QString discoveryHelpHTML() = 0;

    /// delete a light from the discovery page.
    virtual void deleteLight(const QString& light) = 0;

    /// handles how to higlight lights.
    virtual void highlightLights() = 0;

    /// opens the IP widget
    void openIPWidget();

    /// open a help view to aide in debugging connections
    void openHelpView();

    /// true if the help view is open, false otherwise.
    bool helpViewIsOpen() { return mHelpViewIsOpen; }

public slots:

    /// close the help view.
    void closeHelpView();

signals:

    /// show the controller widget of a specific controller.
    void showControllerWidget();

    /*!
     * \brief connectionStatusChanged emitted whenever the connection status changes
     *        for a communication type
     *
     * \param type the CommType, such as Hue or Serial
     * \param status the new status for the CommType. Maps to a EConnectionState value.
     */
    void connectionStatusChanged(EProtocolType type, EConnectionState status);

    /// handles when a select all button is clicked for a preview widget.
    void selectControllerLights(QString, EProtocolType);

    /// handles when a deselect all button is clicked for a preview widget.
    void deselectControllerLights(QString, EProtocolType);

    /// handles when a light should be selected
    void selectLight(QString);

    /// handles when a light should be deselected
    void deselectLight(QString);

    /// handles when a controller should be deleted.
    void deleteController(QString, EProtocolType);

protected slots:
    /// closes IP widget
    void closeIPWidget();

    /// handles an IP from the IP widget, giving a warning if necessary
    void textInputAddedIP(const QString& IP);

    /// handles when the greyout is clicked
    virtual void greyOutClicked() = 0;

private slots:

    /// rerenders the UI.
    void renderUI();

protected:
    /// resize the help view.
    void resizeHelpView();

    /// pointer to commlayer, used for discovery
    CommLayer* mComm;

    /// pointer to controller page.
    ControllerWidget* mControllerPage;

    /// widget for entering an IP manually
    cor::TextInputWidget* mIPWidget;

    /// help view, used to display help in how to connect to lights
    cor::WebView* mHelpView;

    /// true if help view is open, false otherwise.
    bool mHelpViewIsOpen;

    /// widget for greying out widgets in the background
    GreyOutOverlay* mGreyout;

    /// timer that renders the page every _X_ seconds, only enabled when experimental features are
    /// on.
    QTimer* mRenderTimer;

    /*!
     * \brief fillList helper to fill a list with cor::Controllers
     *
     * \param list list widget to fill
     * \param connections cor::Controllers to use during filling.
     */
    void fillList(QListWidget* list, std::vector<cor::Controller>& connections);

    /*!
     * \brief fillList helper to fill a QListWidget with QStrings
     *
     * \param list QListWidget to fill
     * \param connections names of connections to use during filling.
     */
    void fillList(QListWidget* list, std::vector<QString>& connections);
};
#endif // DISCOVERYWIDGET_H
