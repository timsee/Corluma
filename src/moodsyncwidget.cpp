/*!
 * \copyright
 * Copyright (C) 2015 - 2020.
 * Released under the GNU General Public License.
 */

#include "moodsyncwidget.h"

MoodSyncWidget::MoodSyncWidget(QWidget* parent, CommLayer* comm)
    : QWidget(parent),
      mComm{comm},
      mSyncWidget{new SyncWidget(this)},
      mLightVector{new cor::LightVectorWidget(8, 2, true, this)},
      mRenderThread{new QTimer(this)},
      mLastRenderTime{QTime::currentTime()} {
    mLightVector->hideOffLights(false);
    mLightVector->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    mLightVector->enableButtonInteraction(false);

    connect(mRenderThread, SIGNAL(timeout()), this, SLOT(updateUI()));
    mRenderThread->start(100);
}

void MoodSyncWidget::changeState(ESyncState state, const cor::Mood& desiredMood) {
    mSyncWidget->changeState(state);
    mMood = desiredMood;

    auto moodDict = mComm->makeMood(desiredMood);

    // get the current state of the mood lights from the comm layer
    auto currentStates = mComm->lightsByIDs(cor::lightVectorToIDs(moodDict.items()));
    // display the current states
    mLightVector->updateLights(currentStates);
}

void MoodSyncWidget::resizeEvent(QResizeEvent*) {
    int xPos = 0;
    mSyncWidget->setGeometry(xPos, 0, height(), height());
    xPos += mSyncWidget->width();
    mLightVector->setGeometry(xPos, 0, width() - xPos, height());
}

void MoodSyncWidget::updateUI() {
    if (this->isVisible() && mLastRenderTime < mComm->lastUpdateTime()) {
        updateLights();
    }
}

void MoodSyncWidget::updateLights() {
    mLastRenderTime = QTime::currentTime();
    // make a mood dict
    auto moodDict = mComm->makeMood(mMood);
    // get the current state of the mood lights from the comm layer
    auto currentStates = mComm->lightsByIDs(cor::lightVectorToIDs(moodDict.items()));
    // display the current states
    mLightVector->updateLights(currentStates);
}
