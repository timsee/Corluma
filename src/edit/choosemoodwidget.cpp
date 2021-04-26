/*!
 * \copyright
 * Copyright (C) 2015 - 2020.
 * Released under the GNU General Public License.
 */
#include "choosemoodwidget.h"
#include <QMessageBox>
#include <QScrollBar>
#include <QScroller>
#include <QStyleOption>
#include <QtCore>
#include <QtGui>
#include "utils/qt.h"

ChooseMoodWidget::ChooseMoodWidget(QWidget* parent, CommLayer* comm, GroupData* groups)
    : QWidget(parent),
      mGroups{groups},
      mComm{comm},
      mCloseButton{new QPushButton(this)},
      mHeader{new QLabel(this)},
      mMoodScrollArea{new QScrollArea(this)},
      mMoodContainer{new MenuMoodContainer(mMoodScrollArea)},
      mDisplayMood{new DisplayMoodWidget(this, comm, groups)},
      mConfirmationLabel{new QLabel(this)},
      mActionButton{new QPushButton(this)} {
    connect(mCloseButton, SIGNAL(clicked(bool)), this, SLOT(closePressed(bool)));
    connect(mMoodContainer,
            SIGNAL(moodSelected(std::uint64_t)),
            this,
            SLOT(clickedMood(std::uint64_t)));
    connect(mActionButton, SIGNAL(clicked(bool)), this, SLOT(actionPresed(bool)));

    mMoodScrollArea->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    QScroller::grabGesture(mMoodScrollArea->viewport(), QScroller::LeftMouseButtonGesture);
    mMoodScrollArea->setWidget(mMoodContainer);
    mMoodScrollArea->setFrameStyle(QFrame::NoFrame);
    mMoodScrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    mMoodScrollArea->horizontalScrollBar()->setEnabled(false);
    mMoodScrollArea->horizontalScrollBar()->setVisible(false);
}

void ChooseMoodWidget::showMoods(cor::EGroupAction action) {
    mSelectedMood = 0u;
    mDesiredAction = action;
    mDisplayMood->reset();
    if (mDesiredAction == cor::EGroupAction::edit) {
        mHeader->setText("Choose a mood to edit:");
    } else if (mDesiredAction == cor::EGroupAction::remove) {
        mHeader->setText("Choose a mood to remove:");
    }
    handleBottomState();
    mMoodContainer->showMoods(mGroups->moods().items(), mTopHeight);
}

void ChooseMoodWidget::handleBottomState() {
    // only enable the button if a group is selected;
    mActionButton->setEnabled(mSelectedMood != 0u);
    if (mDesiredAction == cor::EGroupAction::edit) {
        mActionButton->setText("Edit");
        mActionButton->setStyleSheet(cor::kEditButtonBackground);
    } else if (mDesiredAction == cor::EGroupAction::remove) {
        mActionButton->setText("Delete");
        mActionButton->setStyleSheet(cor::kDeleteButtonBackground);
    }
}

void ChooseMoodWidget::clickedMood(std::uint64_t key) {
    mSelectedMood = key;
    handleBottomState();
    auto mood = mGroups->moodFromID(key);
    mDisplayMood->updateMood(mood, true);
}

void ChooseMoodWidget::resizeCloseButton() {
#ifdef MOBILE_BUILD
    mTopHeight = cor::applicationSize().height() * 0.075;
#else
    mTopHeight = int(cor::applicationSize().height() * 0.1);
#endif
    QPixmap pixmap(":images/closeX.png");
    int closeSize = int(mTopHeight * 0.8);
    int finalSize = int(mTopHeight * 0.5);
    int spacer = (mTopHeight - finalSize) / 4;
    mCloseButton->setIconSize(QSize(finalSize, finalSize));
    mCloseButton->setIcon(
        QIcon(pixmap.scaled(finalSize, finalSize, Qt::KeepAspectRatio, Qt::SmoothTransformation)));
    mCloseButton->setGeometry(spacer, spacer, closeSize, closeSize);
}

void ChooseMoodWidget::pushIn(const QPoint& startPoint, const QPoint& endPoint) {
    setVisible(true);
    raise();
    isOpen(true);
    moveWidget(this, startPoint, endPoint);
}

void ChooseMoodWidget::pushOut(const QPoint& endPoint) {
    moveWidget(this, pos(), endPoint);
    isOpen(false);
}


void ChooseMoodWidget::resizeEvent(QResizeEvent*) {
#ifdef MOBILE_BUILD
    mTopHeight = cor::applicationSize().height() * 0.075;
#else
    mTopHeight = int(cor::applicationSize().height() * 0.1);
#endif
    int yPos = mTopHeight;
    int xSpacer = this->width() / 20;

    resizeCloseButton();
    mHeader->setGeometry(mCloseButton->width() + xSpacer,
                         0,
                         this->width() - mCloseButton->width() - xSpacer,
                         mTopHeight);
    auto buttonHeight = mTopHeight * 0.7;
    mMoodScrollArea->setGeometry(xSpacer,
                                 yPos,
                                 this->width() - xSpacer * 2,
                                 int(this->height() * 3.5 / 10));
    mMoodContainer->setFixedWidth(int(mMoodScrollArea->width() * 0.8));
    mMoodContainer->resize();
    yPos += int(mMoodScrollArea->height() * 1.02);

    mDisplayMood->changeRowHeight(buttonHeight);
    mDisplayMood->setGeometry(xSpacer,
                              yPos,
                              this->width() - xSpacer * 2,
                              this->height() * 4.5 / 10);
    yPos += mDisplayMood->height();

    mConfirmationLabel->setGeometry(xSpacer, yPos, this->width() * 0.7, this->height() - yPos);
    mActionButton->setGeometry(xSpacer * 2 + mConfirmationLabel->width(),
                               yPos,
                               this->width() * 0.2,
                               this->height() - yPos);
}

void ChooseMoodWidget::paintEvent(QPaintEvent*) {
    QStyleOption opt;
    opt.initFrom(this);
    QPainter painter(this);

    painter.setRenderHint(QPainter::Antialiasing);
    painter.fillRect(rect(), QBrush(QColor(48, 47, 47)));
}

void ChooseMoodWidget::closePressed(bool) {
    emit pressedClose();
}

void ChooseMoodWidget::actionPresed(bool) {
    auto moodName = mGroups->moodFromID(mSelectedMood).name();
    if (mDesiredAction == cor::EGroupAction::edit) {
        emit editMood(mSelectedMood);
        qDebug() << "INFO: editing the mood " << moodName;
    } else if (mDesiredAction == cor::EGroupAction::remove) {
        auto text = "Are you sure you want to delete " + moodName + "?";
        auto reply =
            QMessageBox::question(this, "Delete mood?", text, QMessageBox::Yes | QMessageBox::No);
        if (reply == QMessageBox::Yes) {
            qDebug() << "INFO: deleting the mood " << moodName;
            mGroups->removeGroup(mSelectedMood);
            emit updateMoods();
            emit pressedClose();
        }
    }
}
