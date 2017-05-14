#ifndef DISCOVERYYUNWIDGET_H
#define DISCOVERYYUNWIDGET_H

#include <QPushButton>
#include <QLineEdit>

#include "discovery/discoverywidget.h"

/*!
 * \copyright
 * Copyright (C) 2015 - 2017.
 * Released under the GNU General Public License.
 *
 *
 * \brief The DiscoveryYunWidget class is a discovery widget for helping with
 *        both Yun UDP and HTTP communication streams. It allows the user to enter
 *        IP addresses manually, and then shows which IP addresses are currently running
 *        discovery and which have passed a discovery check.
 */
class DiscoveryYunWidget: public DiscoveryWidget
{
    Q_OBJECT

public:
    /*!
     * \brief DiscoverySerialWidget constructor
     * \param parent
     */
    explicit DiscoveryYunWidget(CommLayer *comm, QWidget *parent = 0);

    /*!
     * \brief Destructor
     */
    ~DiscoveryYunWidget();

    /// See DiscoveryWidget.h
    void handleDiscovery(bool isActive);

    /*!
     * \brief yunLineEditHelper helper function for setting the yun line edit.
     */
    void yunLineEditHelper();

private slots:

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

private:


    /*!
     * \brief doesYunControllerExistAlready checks both UDP and HTTP yun device lists to see if it has any knowledge
     *        of the given controller.
     * \param controller name of controller
     * \return true if yun controller exists on discovered or undiscovered list, false otherwise
     */
    bool doesYunControllerExistAlready(QString controller);

    /// buffer for last IP address used
    QString mLastIP;

    /// top label that explains the widget
    QLabel *mTopLabel;

    /// layout for the QLineEdit and QPushButtons used for input
    QHBoxLayout *mInputLayout;

    /// plus button in input layout. adds current string to discovery
    QPushButton *mPlusButton;

    /// minus button for input layout. removes current string from discovery
    QPushButton *mMinusButton;

    /// Displays current IP address and allows user to edit it. Can be added or removed from discovery with QPushButtons
    QLineEdit *mLineEdit;

    /// label for connected list
    QLabel *mConnectedLabel;

    /// label for discovering list
    QLabel *mDiscoveringLabel;

    /// widget for displaying connected IP addresses
    QListWidget *mConnectedListWidget;

    /// widget for displaying discovering IP addresses
    QListWidget *mDiscoveringListWidget;

    /// layout for widget
    QVBoxLayout *mLayout;

};

#endif // DISCOVERYYUNWIDGET_H
