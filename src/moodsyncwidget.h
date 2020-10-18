#ifndef MOODSYNCWIDGET_H
#define MOODSYNCWIDGET_H
/*!
 * \copyright
 * Copyright (C) 2015 - 2020.
 * Released under the GNU General Public License.
 */
#include <QWidget>

#include "comm/commlayer.h"
#include "cor/objects/mood.h"
#include "cor/widgets/lightvectorwidget.h"
#include "syncwidget.h"

/*!
 * \brief The MoodSyncWidget class displays the syncing status for a mood.
 */
class MoodSyncWidget : public QWidget {
    Q_OBJECT
public:
    explicit MoodSyncWidget(QWidget* parent, CommLayer* comm);

    /// programmatically change the state
    void changeState(ESyncState state, const cor::Mood& desiredMood);

    /// request the current state
    ESyncState state() { return mSyncWidget->state(); }

protected:
    /// called when widget is resized
    void resizeEvent(QResizeEvent*);

private slots:
    /// update the UI
    void updateUI();

private:
    /// mood that is being synced
    cor::Mood mMood;

    /// pointer to comm data
    CommLayer* mComm;

    /// sync widget showing the sync state
    SyncWidget* mSyncWidget;

    /// vector of lights that are being shown
    cor::LightVectorWidget* mLightVector;

    /// renders the widget
    QTimer* mRenderThread;

    /// last time the widget was rendered
    QTime mLastRenderTime;

    /// update the lights
    void updateLights();
};

#endif // MOODSYNCWIDGET_H
