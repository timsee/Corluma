#ifndef TIMEOUTPAGE_H
#define TIMEOUTPAGE_H

#include <QWidget>
#include "comm/commlayer.h"
#include "comm/datasynctimeout.h"
#include "cor/lightlist.h"
#include "cor/objects/page.h"
#include "cor/widgets/switch.h"
#include "menu/kitchentimerwidget.h"
#include "menu/lightstimeoutmenu.h"
#include "syncwidget.h"

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
    explicit TimeoutPage(QWidget* parent,
                         CommLayer* comm,
                         cor::LightList* lights,
                         DataSyncTimeout* dataSyncTimeout);

    /// update whether or not timeouts are enabled, and the value of the number of minutes for a
    /// timeout
    void update(bool timeoutEnabled, int timeoutValue);

    /// update the lights.
    void updateLights();

    /// update the row height in lists.
    void changeRowHeight(int rowHeight) { mRowHeight = rowHeight; }

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

    /// renders the timeout menu
    void renderUI();

private:
    /// pointer to the datasync timeout, used to check its status.
    DataSyncTimeout* mDataSyncTimeout;

    /// pointer to selected lights.
    cor::LightList* mData;

    /// switch for turning all selected lights on and off.
    cor::Switch* mOnOffSwitch;

    /// state label
    QLabel* mStateLabel;

    /// kitchen timer for choosing the timeout values
    KitchenTimerWidget* mTimeChooserWidget;

    /// sync widget showing the sync state
    SyncWidget* mSyncWidget;

    /// displays the lights that are part of this group and their current states.
    LightsTimeoutMenu* mLights;

    /// render timer, renders the page
    QTimer* mRenderTimer;

    /// height of a row.
    int mRowHeight;
};

#endif // TIMEOUTPAGE_H
