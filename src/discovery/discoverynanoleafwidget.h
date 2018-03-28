#ifndef DISCOVERYNANOLEAFWIDGET_H
#define DISCOVERYNANOLEAFWIDGET_H

#include "discoverywidget.h"
#include "editablefieldwidget.h"
#include "searchwidget.h"

/*!
 * \copyright
 * Copyright (C) 2015 - 2018.
 * Released under the GNU General Public License.
 *
 *
 * \brief The DiscoveryNanoLeafWidget class helps in discovering a NanoLeaf. It provides
 *        instructions to connect to the NanoLeaf and also provides the ability to manually
 *        enter an IP address, if discovery is failing.
 */
class DiscoveryNanoLeafWidget : public DiscoveryWidget
{
    Q_OBJECT

public:
    /*!
     * \brief DiscoveryNanoLeafWidget constructor
     * \param parent
     */
    explicit DiscoveryNanoLeafWidget(CommLayer *comm, QWidget *parent = 0);

    /*!
     * \brief Destructor
     */
    ~DiscoveryNanoLeafWidget();

    /// See DiscoveryWidget.h
    void handleDiscovery(bool isActive);

private slots:

    /// adds a new IP address to the discovery routines for the NanoLeaf
    void plusButtonClicked();

    /// stop looking for a NanoLeaf at the given IP address.
    void minusButtonClicked();

private:

    /*!
     * \brief doesNanoLeafExist
     * \param controller name of controller
     * \return true if nanoleaf controller exists on discovered or undiscovered list, false otherwise
     */
    bool doesNanoLeafExist(QString controller);

    /// widget that is used for searching for IP addresses and listing the connected ones.
    SearchWidget *mSearchWidget;

    /// buffer for last IP address used
    QString mLastIP;

    /// label to prompt the user through the application.
    QLabel *mLabel;

    /// layout for widget
    QVBoxLayout *mLayout;

};

#endif // DISCOVERYNANOLEAFWIDGET_H
