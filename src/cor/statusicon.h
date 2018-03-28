#ifndef COR_STATUS_ICON_H
#define COR_STATUS_ICON_H

#include <QObject>
#include <QWidget>
#include <QLabel>

namespace cor
{

/*!
 * \copyright
 * Copyright (C) 2015 - 2018.
 * Released under the GNU General Public License.
 *
 * \brief The StatusIcon class is a very simple QWidget that displays
 *        an indicator of the current status of a device. If it is showing red,
 *        the device cannot be reached. If it showing black or white, it is off or on respectively.
 *        It also shows the current brightness as a shade of grey.
 */
class StatusIcon : public QWidget
{
    Q_OBJECT
public:

    /// constructor
    explicit StatusIcon(QWidget *parent = 0);

    /*!
     * \brief update update the icon's state.
     * \param isReachable true if reachable, false otherwise
     * \param isOn true if on, false otherwise.
     * \param brightness brightness between 0 and 100.
     */
    void update(bool isReachable, bool isOn, float brightness);

private:

    /// black icon used for overlaying
    QLabel *mBlackIcon;

    /// main icon
    QLabel *mIcon;

};

}

#endif // COR_STATUS_ICON_H