/*!
 * \copyright
 * Copyright (C) 2015 - 2020.
 * Released under the GNU General Public License.
 */

#include "hardwareconnectionwidget.h"

HardwareConnectionWidget::HardwareConnectionWidget(const QString& hardwareIconPath, QWidget* parent)
    : QWidget(parent) {
    mIncomingArrow = new QLabel(this);

    mOutgoingArrow = new QLabel(this);

    mScale = 0.15f;

    auto size = int(width() * mScale);
    mCorlumaLabel = new QLabel(this);
    mCorlumaLabel->setMaximumSize(QSize(size, size));
    mCorlumaPixmap = QPixmap(":images/icon.icns");
    mCorlumaLabel->setPixmap(
        mCorlumaPixmap.scaled(size, size, Qt::KeepAspectRatio, Qt::SmoothTransformation));

    mHardwareLabel = new QLabel(this);
    mHardwareLabel->setMaximumSize(QSize(size, size));
    mHardwarePixmap = QPixmap(hardwareIconPath);
    mHardwareLabel->setPixmap(
        mHardwarePixmap.scaled(size, size, Qt::KeepAspectRatio, Qt::SmoothTransformation));

    mLayout = new QGridLayout(this);
    mLayout->addWidget(mCorlumaLabel, 0, 0, 2, 2);
    mLayout->addWidget(mOutgoingArrow, 0, 2, 1, 6);
    mLayout->addWidget(mIncomingArrow, 1, 2, 1, 6);
    mLayout->addWidget(mHardwareLabel, 0, 8, 2, 2);

    mState = EHardwareConnectionStates::noOutgoingFound;
    changeState(EHardwareConnectionStates::connected);
}

void HardwareConnectionWidget::resizeEvent(QResizeEvent*) {
    auto size = int(width() * mScale);
    mCorlumaLabel->setMaximumSize(QSize(size, size));
    mCorlumaLabel->setPixmap(
        mCorlumaPixmap.scaled(size, size, Qt::KeepAspectRatio, Qt::SmoothTransformation));

    mHardwareLabel->setMaximumSize(QSize(size, size));
    mHardwareLabel->setPixmap(
        mHardwarePixmap.scaled(size, size, Qt::KeepAspectRatio, Qt::SmoothTransformation));

    int width = size * 4;
    int height = size;
    mOutgoingArrow->setMaximumSize(QSize(width, height));
    mOutgoingArrow->setPixmap(
        mOutgoingArrowPixmap.scaled(width, height, Qt::KeepAspectRatio, Qt::SmoothTransformation));

    mIncomingArrow->setMaximumSize(QSize(width, height));
    mIncomingArrow->setPixmap(
        mIncomingArrowPixmap.scaled(width, height, Qt::KeepAspectRatio, Qt::SmoothTransformation));
}

void HardwareConnectionWidget::changeState(EHardwareConnectionStates newState) {
    if (newState != mState) {
        switch (newState) {
            case EHardwareConnectionStates::attemptingIncoming:
                mOutgoingArrowPixmap = QPixmap(":images/outgoingCompleteArrow.png");
                mIncomingArrowPixmap = QPixmap(":images/incomingBrokenArrow.png");
                break;
            case EHardwareConnectionStates::attemptingOutgoing:
                mOutgoingArrowPixmap = QPixmap(":images/outgoingBrokenArrow.png");
                mIncomingArrowPixmap = QPixmap(":images/incomingBrokenArrow.png");
                break;
            case EHardwareConnectionStates::connected:
                mOutgoingArrowPixmap = QPixmap(":images/outgoingCompleteArrow.png");
                mIncomingArrowPixmap = QPixmap(":images/incomingCompleteArrow.png");
                break;
            case EHardwareConnectionStates::noOutgoingFound:
                mOutgoingArrowPixmap = QPixmap(":images/outgoingBrokenArrow.png");
                mIncomingArrowPixmap = QPixmap(":images/incomingBrokenArrow.png");
                break;
        }
        mState = newState;
    }

    auto width = int(this->width() * mScale * 4);
    auto height = int(this->width() * mScale);
    mOutgoingArrow->setMaximumSize(QSize(width, height));
    mOutgoingArrow->setPixmap(
        mOutgoingArrowPixmap.scaled(width, height, Qt::KeepAspectRatio, Qt::SmoothTransformation));

    mIncomingArrow->setMaximumSize(QSize(width, height));
    mIncomingArrow->setPixmap(
        mIncomingArrowPixmap.scaled(width, height, Qt::KeepAspectRatio, Qt::SmoothTransformation));
}
