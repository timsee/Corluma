/*!
 * \copyright
 * Copyright (C) 2015 - 2020.
 * Released under the GNU General Public License.
 */

#include "listmooddetailedwidget.h"

#include <QGraphicsOpacityEffect>
#include <QScrollBar>
#include <QScroller>
#include <QStyleOption>
#include <QtCore>
#include <QtGui>

#include "utils/qt.h"

ListMoodDetailedWidget::ListMoodDetailedWidget(QWidget* parent, GroupData* groups, CommLayer* comm)
    : QWidget(parent),
      mComm{comm} {
    mOnOffSwitch = new cor::Switch(this);
    mOnOffSwitch->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Expanding);
    connect(mOnOffSwitch, SIGNAL(switchChanged(bool)), this, SLOT(changedSwitchState(bool)));
    mOnOffSwitch->setSwitchState(ESwitchState::off);
    mOnOffSwitch->setVisible(true);

    mFloatingMenu = new FloatingLayout(parent);
    connect(mFloatingMenu,
            SIGNAL(buttonPressed(QString)),
            this,
            SLOT(floatingLayoutButtonPressed(QString)));
    std::vector<QString> buttons = {QString("Group_Edit")};
    mFloatingMenu->setupButtons(buttons, EButtonSize::small);

    //------------
    // ScrollArea Widget
    //------------
    mMoodWidget = new DisplayMoodWidget(this, mComm, groups);
    mMoodWidget->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

    mLayout = new QVBoxLayout(this);
    mLayout->setContentsMargins(4, 4, 4, 4);
    mLayout->setSpacing(2);

    mLayout->addWidget(mOnOffSwitch, 1);
    mLayout->addWidget(mMoodWidget, 8);

    resize();
}

void ListMoodDetailedWidget::update(const cor::Mood& mood) {
    auto moodCopy = mComm->addMetadataToMood(mood);

    mOnOffSwitch->setSwitchState(ESwitchState::off);
    mMoodWidget->updateMood(moodCopy, true);
}

void ListMoodDetailedWidget::resize() {
    mMoodWidget->changeRowHeight(this->height() / 10);
    QSize size = parentWidget()->size();
    setFixedSize(int(size.width() * 0.75f), int(size.height() * 0.75f));
    mOnOffSwitch->setFixedWidth(int(size.width() * 0.2));
    QPoint topRight(this->x() + this->width(), this->y());
    mFloatingMenu->move(topRight);
}

void ListMoodDetailedWidget::paintEvent(QPaintEvent*) {
    QStyleOption opt;
    opt.init(this);
    QPainter painter(this);

    painter.setRenderHint(QPainter::Antialiasing);
    painter.fillRect(rect(), QBrush(QColor(48, 47, 47)));
}

void ListMoodDetailedWidget::floatingLayoutButtonPressed(const QString& key) {
    if (key == "Group_Edit") {
        emit pressedClose();
        emit editMood(mMoodWidget->mood().uniqueID());
    }
    mFloatingMenu->highlightButton("");
}

void ListMoodDetailedWidget::changedSwitchState(bool state) {
    if (state) {
        emit enableMood(mMoodWidget->mood().uniqueID());
    }
}

void ListMoodDetailedWidget::resizeEvent(QResizeEvent*) {
    resize();
}

void ListMoodDetailedWidget::pushIn() {
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

void ListMoodDetailedWidget::pushOut() {
    isOpen(false);

    moveWidget(
        this,
        QPoint(int(parentWidget()->width() * 0.125f), int(parentWidget()->height() * 0.125f)),
        QPoint(int(parentWidget()->width() * 0.125f), int(-1 * parentWidget()->height())));

    auto widthPoint = int(parentWidget()->width() * 0.875f - topMenu()->size().width());
    QPoint startPoint(widthPoint, int(parentWidget()->height() * 0.125f));
    cor::moveWidget(topMenu(), startPoint, QPoint(widthPoint, int(-1 * parentWidget()->height())));
}
