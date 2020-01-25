#ifndef DISCOVERYWIDGET_H
#define DISCOVERYWIDGET_H

#include <QLabel>
#include <QLayout>
#include <QListWidget>
#include <QWidget>

#include "comm/commlayer.h"


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
class DiscoveryWidget : public QWidget {
    Q_OBJECT

public:
    /// constructor
    explicit DiscoveryWidget(QWidget* parent);

    /*!
     * \brief ~DiscoveryWidget Deconstructor
     */
    virtual ~DiscoveryWidget() {}

    /*!
     * \brief handleDiscovery run the discovery roiutines and associated GUI updates
     *        for this widget. If its active, more GUI updates are required since its
     *        currentl displayed
     *
     * \param isActive true if displaying this widget, false if widget is in background.
     */
    virtual void handleDiscovery(bool isActive) = 0;

signals:
    /*!
     * \brief connectionStatusChanged emitted whenever the connection status changes
     *        for a communication type
     *
     * \param type the CommType, such as Hue or Serial
     * \param status the new status for the CommType. Maps to a EConnectionState value.
     */
    void connectionStatusChanged(EProtocolType type, EConnectionState status);

protected:
    /// pointer to commlayer, used for discovery
    CommLayer* mComm;

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
