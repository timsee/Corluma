#ifndef TIMEOUTPAGE_H
#define TIMEOUTPAGE_H

#include <QWidget>
#include "cor/objects/page.h"
#include "cor/widgets/switch.h"
#include "menu/kitchentimerwidget.h"

/*!
 * \copyright
 * Copyright (C) 2015 - 2020.
 * Released under the GNU General Public License.
 *
 *
 * \brief The TimeoutPage class provides a page for the user to view and manage light timeouts.
 */
class TimeoutPage : public QWidget, public cor::Page {
    Q_OBJECT
public:
    explicit TimeoutPage(QWidget* parent);

    /// update whether or not timeouts are enabled, and the value of the number of minutes for a
    /// timeout
    void update(bool timeoutEnabled, int timeoutValue);

signals:

    /// emits when the timeout changes
    void timeoutUpdated(bool, std::uint32_t);

protected:
    /*!
     * \brief resizeEvent called every time the main window is resized.
     */
    void resizeEvent(QResizeEvent*);

private slots:

    /// handles when the timeout switch is changed
    void changedSwitchState(bool);

    /// handles when the timeout value is changed
    void timerChanged(int);

private:
    /// switch for turning all selected lights on and off.
    cor::Switch* mOnOffSwitch;

    /// state label
    QLabel* mStateLabel;

    /// kitchen timer for choosing the timeout values
    KitchenTimerWidget* mTimeChooserWidget;
};

#endif // TIMEOUTPAGE_H
