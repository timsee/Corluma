/*!
 * \copyright
 * Copyright (C) 2015 - 2019.
 * Released under the GNU General Public License.
 */

#include "syncwidget.h"

#include <QMovie>

SyncWidget::SyncWidget(QWidget* parent) : QWidget(parent), mState{ESyncState::synced} {
    mLabel = new QLabel(this);
    mLabel->setFixedSize(this->size());
    changeState(ESyncState::synced);

    mMovie = new QMovie(":/images/syncing.gif");
    mMovie->setScaledSize(this->size());
}

void SyncWidget::changeState(ESyncState state) {
    if (state == ESyncState::notSynced || state == ESyncState::synced) {
        QString resourcePath;
        if (state == ESyncState::notSynced) {
            resourcePath = ":images/closeX.png";
        } else if (state == ESyncState::synced) {
            resourcePath = ":images/checkmark.png";
        }
        QPixmap syncPixmap(resourcePath);
        syncPixmap = syncPixmap.scaled(int(this->height() * 0.7f),
                                       int(this->height() * 0.7f),
                                       Qt::KeepAspectRatio,
                                       Qt::SmoothTransformation);
        mLabel->setPixmap(syncPixmap);
    } else {
        mLabel->setMovie(mMovie);
        mMovie->start();
    }
}


void SyncWidget::resizeEvent(QResizeEvent*) {
    auto height = this->height();
    mLabel->setFixedSize(QSize(height, height));
    mMovie->setScaledSize(QSize(height, height));
}
