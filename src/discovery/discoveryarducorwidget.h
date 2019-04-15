#ifndef DISCOVERYYUNWIDGET_H
#define DISCOVERYYUNWIDGET_H

#include <QPushButton>
#include <QLineEdit>

#include "discovery/discoverywidget.h"
#include "searchwidget.h"
#include "comm/commarducor.h"

/*!
 * \copyright
 * Copyright (C) 2015 - 2019.
 * Released under the GNU General Public License.
 *
 *
 * \brief The DiscoveryArduCorWidget class is a discovery widget for helping with
 *        both Yun UDP and HTTP communication streams. It allows the user to enter
 *        IP addresses manually, and then shows which IP addresses are currently running
 *        discovery and which have passed a discovery check.
 */
class DiscoveryArduCorWidget: public DiscoveryWidget
{
    Q_OBJECT

public:
    /*!
     * \brief DiscoveryArduCorWidget constructor
     * \param parent
     */
    explicit DiscoveryArduCorWidget(CommLayer *comm, QWidget *parent);

    /// See DiscoveryWidget.h
    void handleDiscovery(bool isActive);

private slots:

    /*!
     * \brief plusButtonClicked called whenever the plus button is clicked
     */
    void plusButtonClicked();

    /*!
     * \brief minusButtonClicked called whenever the minus button is clicked
     */
    void minusButtonClicked();

private:
    /*!
     * \brief doesYunControllerExistAlready checks both UDP and HTTP yun device lists to see if it has any knowledge
     *        of the given controller.
     * \param controller name of controller
     * \return true if yun controller exists on discovered or undiscovered list, false otherwise
     */
    bool doesYunControllerExistAlready(const QString& controller);

    /// widget that is used for searching for IP addresses and listing the connected ones.
    SearchWidget *mSearchWidget;

    /// buffer for last IP address used
    QString mLastIP;

    /// top label that explains the widget
    QLabel *mTopLabel;

    /// layout for widget
    QVBoxLayout *mLayout;
};

#endif // DISCOVERYYUNWIDGET_H
