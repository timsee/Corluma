#ifndef CORLUMASTATUSICON_H
#define CORLUMASTATUSICON_H

#include <QObject>
#include <QWidget>
#include <QLabel>
/*!
 * \copyright
 * Copyright (C) 2015 - 2017.
 * Released under the GNU General Public License.
 *
 * \brief The CorlumastatusIcon class is a very simple QWidget that displays
 *        an indicator of the current status of a device. If it is showing red,
 *        the device cannot be reached. If it showing black or white, it is off or on respectively.
 *        It also shows the current brightness as a shade of grey.
 */
class CorlumaStatusIcon : public QWidget
{
    Q_OBJECT
public:

    /// constructor
    explicit CorlumaStatusIcon(QWidget *parent = 0);

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

#endif // CORLUMASTATUSICON_H
