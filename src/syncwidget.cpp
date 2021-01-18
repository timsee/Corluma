/*!
 * \copyright
 * Copyright (C) 2015 - 2020.
 * Released under the GNU General Public License.
 */

#include "syncwidget.h"
#include <QMovie>

SyncWidget::SyncWidget(QWidget* parent)
    : QWidget(parent),
      mState{ESyncState::notSynced},
      mLabel{new QLabel(this)},
      mMovieLabel{new QLabel(this)} {
    mLabel->setAlignment(Qt::AlignHCenter | Qt::AlignVCenter);
    changeState(ESyncState::notSynced);

    mMovie = new QMovie(":/images/syncing.gif");
    mMovieLabel->setMovie(mMovie);
}

void SyncWidget::changeState(ESyncState state) {
    if (state != mState) {
        mState = state;
        if (mState == ESyncState::notSynced || mState == ESyncState::synced) {
            QString resourcePath;
            if (mState == ESyncState::notSynced) {
                resourcePath = ":images/closeX.png";
            } else if (mState == ESyncState::synced) {
                resourcePath = ":images/checkmark.png";
            }
            QPixmap syncPixmap(resourcePath);
            syncPixmap = syncPixmap.scaled(int(height() * 0.7f),
                                           int(height() * 0.7f),
                                           Qt::KeepAspectRatio,
                                           Qt::SmoothTransformation);
            mLabel->setVisible(true);
            mMovieLabel->setVisible(false);
            mLabel->setPixmap(syncPixmap);
        } else if (mState == ESyncState::syncing) {
            mLabel->setVisible(false);
            mMovieLabel->setVisible(true);
            mMovie->start();
        }
    }
}


void SyncWidget::resizeEvent(QResizeEvent*) {
    auto height = this->height();
    mLabel->setFixedSize(QSize(width(), height));
    mMovieLabel->setFixedSize(QSize(height, height));
    mMovie->setScaledSize(QSize(height, height));
}
