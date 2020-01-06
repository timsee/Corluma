/*!
 * \copyright
 * Copyright (C) 2015 - 2019.
 * Released under the GNU General Public License.
 */

#include "syncwidget.h"
#include <QMovie>

SyncWidget::SyncWidget(QWidget* parent) : QWidget(parent), mState{ESyncState::notSynced} {
    mLabel = new QLabel(this);
    mLabel->setFixedSize(size());
    changeState(ESyncState::notSynced);

    mMovie = new QMovie(":/images/syncing.gif");
    mMovie->setScaledSize(size());
}

void SyncWidget::changeState(ESyncState state) {
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
        mLabel->setPixmap(syncPixmap);
    } else if (mState == ESyncState::syncing) {
        mLabel->setMovie(mMovie);
        mMovie->start();
    }
}


void SyncWidget::resizeEvent(QResizeEvent*) {
    auto height = this->height();
    mLabel->setFixedSize(QSize(height, height));
    mMovie->setScaledSize(QSize(height, height));
}
