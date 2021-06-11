/*!
 * \copyright
 * Copyright (C) 2015 - 2020.
 * Released under the GNU General Public License.
 */

#include "mooddetailedwidget.h"

#include <QGraphicsOpacityEffect>
#include <QScrollBar>
#include <QScroller>
#include <QtCore>
#include <QtGui>

#include "utils/qt.h"

MoodDetailedWidget::MoodDetailedWidget(QWidget* parent, AppData* appData, CommLayer* comm)
    : QWidget(parent),
      mComm{comm},
      mMoodWidget{new DisplayMoodWidget(this, mComm, appData)},
      mMoodSyncWidget{new MoodSyncWidget(this, mComm)},
      mOnOffSwitch{new cor::Switch(this)},
      mFloatingMenu{new FloatingLayout(this)},
      mRowHeight{10u} {
    connect(mOnOffSwitch, SIGNAL(switchChanged(bool)), this, SLOT(changedSwitchState(bool)));
    mOnOffSwitch->setSwitchState(ESwitchState::off);
    mOnOffSwitch->setVisible(true);

    connect(mFloatingMenu,
            SIGNAL(buttonPressed(QString)),
            this,
            SLOT(floatingLayoutButtonPressed(QString)));
    std::vector<QString> buttons = {QString("Group_Edit")};
    mFloatingMenu->setupButtons(buttons, EButtonSize::small);
    mFloatingMenu->setVisible(false);
}

void MoodDetailedWidget::update(const cor::Mood& mood) {
    auto moodCopy = mComm->addMetadataToMood(mood);

    mOnOffSwitch->setSwitchState(ESwitchState::off);
    mMoodWidget->updateMood(moodCopy, true);
    // TODO check if the mood is pre-synced
    mMoodSyncWidget->changeState(ESyncState::notSynced, moodCopy);
}

void MoodDetailedWidget::resize() {
    QSize size = parentWidget()->size();
    setFixedSize(int(size.width() * 0.75f), int(size.height() * 0.75f));

    auto yPos = 0u;
    auto rowHeight = height() / 9;

    mOnOffSwitch->setGeometry(0, yPos, int(size.width() * 0.2), rowHeight);
    auto spacer = mOnOffSwitch->width() / 8;
    auto syncWidth = this->width() - mFloatingMenu->width() - mOnOffSwitch->width()
                     - mOnOffSwitch->geometry().x() - spacer;
    mMoodSyncWidget->setGeometry(mOnOffSwitch->width() + mOnOffSwitch->geometry().x(),
                                 yPos,
                                 syncWidth,
                                 mOnOffSwitch->height());
    yPos += mOnOffSwitch->height();

    QPoint topRight(this->x() + this->width(), this->y());
    mFloatingMenu->move(topRight);

    mMoodWidget->setGeometry(0, yPos, width(), rowHeight * 8);
}

void MoodDetailedWidget::paintEvent(QPaintEvent*) {
    QStyleOption opt;
    opt.initFrom(this);
    QPainter painter(this);

    painter.setRenderHint(QPainter::Antialiasing);
    painter.fillRect(rect(), QBrush(QColor(48, 47, 47)));
}

void MoodDetailedWidget::floatingLayoutButtonPressed(const QString& key) {
    if (key == "Group_Edit") {
        emit pressedClose();
        emit editMood(mMoodWidget->mood().uniqueID());
    }
    mFloatingMenu->highlightButton("");
}

void MoodDetailedWidget::changedSwitchState(bool isOn) {
    if (isOn) {
        emit enableMood(mMoodWidget->mood().uniqueID());
        updateSyncStatus(ESyncState::notSynced);
    }
}

void MoodDetailedWidget::updateSyncStatus(ESyncState state) {
    mMoodSyncWidget->changeState(state, mMoodWidget->mood());
}

void MoodDetailedWidget::resizeEvent(QResizeEvent*) {
    resize();
}

void MoodDetailedWidget::pushIn() {
    isOpen(true);

    moveWidget(
        this,
        QPoint(int(parentWidget()->width() * 0.125f), int(-1 * parentWidget()->height())),
        QPoint(int(parentWidget()->width() * 0.125f), int(parentWidget()->height() * 0.125f)));

    auto widthPoint = int(parentWidget()->width() * 0.875f - topMenu()->width());
    QPoint finishPoint(widthPoint, int(parentWidget()->height() * 0.125f));
    cor::moveWidget(topMenu(), QPoint(widthPoint, int(-1 * parentWidget()->height())), finishPoint);

    mFloatingMenu->setVisible(true);
    raise();
    setVisible(true);
    mFloatingMenu->raise();
    resize();
}

void MoodDetailedWidget::pushOut() {
    isOpen(false);

    moveWidget(
        this,
        QPoint(int(parentWidget()->width() * 0.125f), int(parentWidget()->height() * 0.125f)),
        QPoint(int(parentWidget()->width() * 0.125f), int(-1 * parentWidget()->height())));

    auto widthPoint = int(parentWidget()->width() * 0.875f - topMenu()->size().width());
    QPoint startPoint(widthPoint, int(parentWidget()->height() * 0.125f));
    cor::moveWidget(topMenu(), startPoint, QPoint(widthPoint, int(-1 * parentWidget()->height())));
}
