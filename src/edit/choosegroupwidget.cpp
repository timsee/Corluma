/*!
 * \copyright
 * Copyright (C) 2015 - 2020.
 * Released under the GNU General Public License.
 */
#include "choosegroupwidget.h"
#include <QMessageBox>
#include <QScrollBar>
#include <QScroller>
#include <QStyleOption>
#include <QtCore>
#include <QtGui>
#include "utils/qt.h"

ChooseGroupWidget::ChooseGroupWidget(QWidget* parent, CommLayer* comm, AppData* appData)
    : QWidget(parent),
      mGroups{appData->groups()},
      mComm{comm},
      mCloseButton{new QPushButton(this)},
      mHeader{new QLabel(this)},
      mGroupScrollArea{new QScrollArea(this)},
      mGroupContainer{new MenuGroupContainer(mGroupScrollArea, appData->groups())},
      mDisplayGroup{new DisplayGroupWidget(this, comm, appData)},
      mConfirmationLabel{new QLabel(this)},
      mActionButton{new QPushButton(this)} {
    connect(mCloseButton, SIGNAL(clicked(bool)), this, SLOT(closePressed(bool)));
    connect(mGroupContainer,
            SIGNAL(groupClicked(std::uint64_t)),
            this,
            SLOT(clickedGroup(std::uint64_t)));
    connect(mActionButton, SIGNAL(clicked(bool)), this, SLOT(actionPresed(bool)));

    mGroupScrollArea->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    QScroller::grabGesture(mGroupScrollArea->viewport(), QScroller::LeftMouseButtonGesture);
    mGroupScrollArea->setWidget(mGroupContainer);
    mGroupScrollArea->setFrameStyle(QFrame::NoFrame);
    mGroupScrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    mGroupScrollArea->horizontalScrollBar()->setEnabled(false);
    mGroupScrollArea->horizontalScrollBar()->setVisible(false);
}

void ChooseGroupWidget::showGroups(const std::vector<std::uint64_t>& groups,
                                   cor::EGroupAction action) {
    mSelectedGroup = 0u;
    mDesiredAction = action;
    mDisplayGroup->reset();
    if (mDesiredAction == cor::EGroupAction::edit) {
        mHeader->setText("Choose a group to edit:");
    } else if (mDesiredAction == cor::EGroupAction::remove) {
        mHeader->setText("Choose a group to remove:");
    }
    handleBottomState();
    mGroupContainer->showGroups(groups, mTopHeight);
}

void ChooseGroupWidget::handleBottomState() {
    // only enable the button if a group is selected;
    mActionButton->setEnabled(mSelectedGroup != 0u);
    if (mDesiredAction == cor::EGroupAction::edit) {
        mActionButton->setText("Edit");
        mActionButton->setStyleSheet(cor::kEditButtonBackground);
    } else if (mDesiredAction == cor::EGroupAction::remove) {
        mActionButton->setText("Delete");
        mActionButton->setStyleSheet(cor::kDeleteButtonBackground);
    }
}

void ChooseGroupWidget::clickedGroup(std::uint64_t key) {
    mSelectedGroup = key;
    handleBottomState();
    auto group = mGroups->groupFromID(key);
    mDisplayGroup->updateGroup(group, true);
}

void ChooseGroupWidget::resizeCloseButton() {
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

void ChooseGroupWidget::pushIn(const QPoint& startPoint, const QPoint& endPoint) {
    setVisible(true);
    raise();
    isOpen(true);
    moveWidget(this, startPoint, endPoint);
}

void ChooseGroupWidget::pushOut(const QPoint& endPoint) {
    moveWidget(this, pos(), endPoint);
    isOpen(false);
}


void ChooseGroupWidget::resizeEvent(QResizeEvent*) {
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
    mGroupScrollArea->setGeometry(xSpacer,
                                  yPos,
                                  this->width() - xSpacer * 2,
                                  int(this->height() * 3.5 / 10));
    mGroupContainer->setFixedWidth(int(mGroupScrollArea->width() * 0.8));
    mGroupContainer->resizeWidgets(int(buttonHeight));
    yPos += int(mGroupScrollArea->height() * 1.02);

    mDisplayGroup->changeRowHeight(buttonHeight);
    mDisplayGroup->setGeometry(xSpacer,
                               yPos,
                               this->width() - xSpacer * 2,
                               this->height() * 4.5 / 10);
    yPos += mDisplayGroup->height();

    mConfirmationLabel->setGeometry(xSpacer, yPos, this->width() * 0.7, this->height() - yPos);
    mActionButton->setGeometry(xSpacer * 2 + mConfirmationLabel->width(),
                               yPos,
                               this->width() * 0.2,
                               this->height() - yPos);
}

void ChooseGroupWidget::paintEvent(QPaintEvent*) {
    QStyleOption opt;
    opt.initFrom(this);
    QPainter painter(this);

    painter.setRenderHint(QPainter::Antialiasing);
    painter.fillRect(rect(), QBrush(QColor(48, 47, 47)));
}

void ChooseGroupWidget::closePressed(bool) {
    emit pressedClose();
}

void ChooseGroupWidget::actionPresed(bool) {
    auto groupName = mGroups->nameFromID(mSelectedGroup);
    if (mDesiredAction == cor::EGroupAction::edit) {
        emit editGroup(mSelectedGroup);
        qDebug() << "INFO: editing the group " << groupName;
    } else if (mDesiredAction == cor::EGroupAction::remove) {
        auto text = "Are you sure you want to delete " + groupName
                    + "? This change will be reflected in other apps.";
        auto reply =
            QMessageBox::question(this, "Delete group?", text, QMessageBox::Yes | QMessageBox::No);
        if (reply == QMessageBox::Yes) {
            qDebug() << "INFO: deleting the group " << groupName;
            mGroups->removeGroup(mSelectedGroup);
            // delete from hue bridge, if applicable.
            mComm->deleteHueGroup(groupName);
            emit updateGroups();
            emit pressedClose();
        }
    }
}
