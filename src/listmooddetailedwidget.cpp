/*!
 * \copyright
 * Copyright (C) 2015 - 2019.
 * Released under the GNU General Public License.
 */

#include "listmooddetailedwidget.h"

#include "cor/utils.h"

#include <QtCore>
#include <QtGui>
#include <QStyleOption>
#include <QScroller>
#include <QGraphicsOpacityEffect>

ListMoodDetailedWidget::ListMoodDetailedWidget(QWidget *parent, CommLayer* comm) : QWidget(parent), mComm{comm} {

    mTopLabel = new QLabel("Hollywood", this);
    mTopLabel->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    mTopLabel->setAlignment(Qt::AlignBottom);
    mTopLabel->setStyleSheet("font-size:14pt;");

    mOnOffSwitch = new cor::Switch(this);
    mOnOffSwitch->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Expanding);
    connect(mOnOffSwitch, SIGNAL(switchChanged(bool)), this, SLOT(changedSwitchState(bool)));
    mOnOffSwitch->setSwitchState(ESwitchState::off);
    mOnOffSwitch->setVisible(true);

    mSimpleGroupWidget = new ListSimpleGroupWidget(this, cor::EListType::grid);
    mSimpleGroupWidget->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    connect(mSimpleGroupWidget, SIGNAL(deviceClicked(QString)), this, SLOT(clickedDevice(QString)));
    QScroller::grabGesture(mSimpleGroupWidget->viewport(), QScroller::LeftMouseButtonGesture);
    mSimpleGroupWidget->setStyleSheet("background-color:rgba(33,32,32,255);");

    mFloatingMenu = new FloatingLayout(false, parent);
    connect(mFloatingMenu, SIGNAL(buttonPressed(QString)), this, SLOT(floatingLayoutButtonPressed(QString)));
    std::vector<QString> buttons = { QString("Group_Lights"), QString("Group_Details"), QString("Group_Edit")};
    mFloatingMenu->setupButtons(buttons, EButtonSize::small);
    mFloatingMenu->setVisible(false);

    //------------
    // ScrollArea Widget
    //------------

    mScrollArea = new QScrollArea(this);
    mScrollArea->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    mScrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

    mScrollAreaWidget = new QWidget(this);
    mScrollAreaWidget->setObjectName("contentWidget");
    QScroller::grabGesture(mScrollArea->viewport(), QScroller::LeftMouseButtonGesture);
    mScrollAreaWidget->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    mScrollAreaWidget->setContentsMargins(0,0,0,0);
    mScrollAreaWidget->setStyleSheet("QWidget#contentWidget{ background-color: #201F1F; } QLabel { background-color: #201F1F; } ");

    mScrollLayout = new QVBoxLayout(mScrollAreaWidget);
    mScrollLayout->setSpacing(7);
    mScrollLayout->setContentsMargins(9, 9, 9, 9);
    mScrollAreaWidget->setLayout(mScrollLayout);
    mScrollArea->setVisible(false);
    mScrollArea->setWidget(mScrollAreaWidget);

    mPlaceholder = new QWidget(this);
    mPlaceholder->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

    mTopLayout = new QHBoxLayout;
    mTopLayout->addWidget(mTopLabel, 8);

    mLayout = new QVBoxLayout(this);
    mLayout->setContentsMargins(4,4,4,4);
    mLayout->setSpacing(2);

    mLayout->addWidget(mOnOffSwitch, 1);
    mLayout->addLayout(mTopLayout, 1);
    mLayout->addWidget(mPlaceholder, 8);
}

void ListMoodDetailedWidget::update(const cor::Mood& group) {
    mSimpleGroupWidget->removeWidgets();
    mKey = group.uniqueID();
    mTopLabel->setText(group.name());
    mSimpleGroupWidget->updateDevices(group.lights,
                                      EOnOffSwitchState::hidden,
                                      false, false, true);

    mOnOffSwitch->setSwitchState(ESwitchState::off);
}

void ListMoodDetailedWidget::clickedDevice(QString) {
   // qDebug() << " device clicked " << key << " vs" << deviceName;

}

void ListMoodDetailedWidget::resize() {
    QSize size = qobject_cast<QWidget*>(this->parent())->size();
    this->setGeometry(int(size.width()  * 0.125f),
                      int(size.height() * 0.125f),
                      int(size.width()  * 0.75f),
                      int(size.height() * 0.75f));
    mScrollArea->setGeometry(mSimpleGroupWidget->geometry());
    moveFloatingLayout();
}

void ListMoodDetailedWidget::paintEvent(QPaintEvent *) {
    QStyleOption opt;
    opt.init(this);
    QPainter painter(this);

    painter.setRenderHint(QPainter::Antialiasing);
    painter.fillRect(this->rect(), QBrush(QColor(48, 47, 47)));
}

void ListMoodDetailedWidget::floatingLayoutButtonPressed(QString key) {
    if (key == "Group_Details") {
        mScrollArea->setVisible(true);
        mSimpleGroupWidget->setVisible(false);
        mScrollArea->setGeometry(mPlaceholder->geometry());
        mScrollArea->raise();
    } else if (key == "Group_Lights") {
        mScrollArea->setVisible(false);
        mSimpleGroupWidget->setVisible(true);
        mSimpleGroupWidget->setGeometry(mPlaceholder->geometry());
        mSimpleGroupWidget->raise();
    } else if (key == "Group_Edit") {

    }
}

void ListMoodDetailedWidget::moveFloatingLayout() {
    QSize size = qobject_cast<QWidget*>(this->parent())->size();
    mOnOffSwitch->setFixedWidth(size.width() * 0.2f);

    QPoint topRight(size.width() * 0.875f, size.height() * 0.125f);
    mFloatingMenu->move(topRight);
}


void ListMoodDetailedWidget::show() {
    mFloatingMenu->setVisible(true);
    mFloatingMenu->raise();
    mFloatingMenu->highlightButton("Group_Lights");
    floatingLayoutButtonPressed("Group_Lights");
}

void ListMoodDetailedWidget::hide() {
    mFloatingMenu->setVisible(false);
}

void ListMoodDetailedWidget::changedSwitchState(bool state) {
    if (state) {
        emit enableGroup(mKey);
    } else {

    }
}

void ListMoodDetailedWidget::resizeEvent(QResizeEvent *) {
    mSimpleGroupWidget->resizeWidgets();
    moveFloatingLayout();
    mSimpleGroupWidget->setGeometry(mPlaceholder->geometry());
    mSimpleGroupWidget->raise();
    mScrollArea->setGeometry(mPlaceholder->geometry());
    mScrollArea->raise();
}
