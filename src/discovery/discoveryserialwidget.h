#ifndef DISCOVERYSERIALWIDGET_H
#define DISCOVERYSERIALWIDGET_H
#ifndef MOBILE_BUILD

#include <QWidget>
#include "discovery/discoverywidget.h"

/*!
 * \copyright
 * Copyright (C) 2015 - 2018.
 * Released under the GNU General Public License.
 *
 *
 * \brief The DiscoverySerialWidget class is the discovery widget for serial
 *        connections. No manual search for serial is provided so it only shows
 *        which devices pass the discovery check in the connected list.
 */
class DiscoverySerialWidget: public DiscoveryWidget
{
    Q_OBJECT

public:
    /*!
     * \brief DiscoverySerialWidget constructor
     * \param parent
     */
    explicit DiscoverySerialWidget(CommLayer *comm, QWidget *parent = 0);

    /*!
     * \brief Destructor
     */
    ~DiscoverySerialWidget();

    /// See DiscoveryWidget.h
    void handleDiscovery(bool isActive);

private:

    /// Label at top of widget
    QLabel *mLabel;

    /// list of connected serial devices.
    QListWidget *mListWidget;

    /// layout for widget
    QVBoxLayout *mLayout;
};

#endif //MOBILE_BUILD
#endif // DISCOVERYSERIALWIDGET_H
