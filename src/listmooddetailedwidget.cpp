/*!
 * \copyright
 * Copyright (C) 2015 - 2019.
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
      mKey{0},
      mComm{comm} {
    mTopLabel = new QLabel("Default", this);
    mTopLabel->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    mTopLabel->setAlignment(Qt::AlignBottom);
    mTopLabel->setStyleSheet("font-size:18pt;");

    mOnOffSwitch = new cor::Switch(this);
    mOnOffSwitch->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Expanding);
    connect(mOnOffSwitch, SIGNAL(switchChanged(bool)), this, SLOT(changedSwitchState(bool)));
    mOnOffSwitch->setSwitchState(ESwitchState::off);
    mOnOffSwitch->setVisible(true);

    mSimpleGroupWidget = new ListSimpleGroupWidget(this, cor::EListType::grid);
    mSimpleGroupWidget->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    connect(mSimpleGroupWidget, SIGNAL(deviceClicked(QString)), this, SLOT(clickedDevice(QString)));
    QScroller::grabGesture(mSimpleGroupWidget->viewport(), QScroller::LeftMouseButtonGesture);
    mSimpleGroupWidget->setStyleSheet("background-color:rgba(33,32,32,255);");

    mFloatingMenu = new FloatingLayout(false, parent);
    connect(mFloatingMenu,
            SIGNAL(buttonPressed(QString)),
            this,
            SLOT(floatingLayoutButtonPressed(QString)));
    std::vector<QString> buttons = {QString("Group_Lights"),
                                    QString("Group_Details"),
                                    QString("Group_Edit")};
    mFloatingMenu->setupButtons(buttons, EButtonSize::small);
    mFloatingMenu->setVisible(false);

    //------------
    // ScrollArea Widget
    //------------

    mScrollArea = new QScrollArea(this);
    mScrollArea->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    mScrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

    mAdditionalDetailsWidget = new MoodDetailsWidget(groups, this);
    mAdditionalDetailsWidget->setObjectName("contentWidget");
    QScroller::grabGesture(mScrollArea->viewport(), QScroller::LeftMouseButtonGesture);
    mAdditionalDetailsWidget->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    mAdditionalDetailsWidget->setContentsMargins(0, 0, 0, 0);
    mAdditionalDetailsWidget->setStyleSheet("QWidget#contentWidget{ background-color: #201F1F; } "
                                            "QLabel { background-color: #201F1F; } ");

    mScrollArea->setWidget(mAdditionalDetailsWidget);

    mEditPage = new EditMoodPage(this, mComm, groups);
    mEditPage->setVisible(false);

    mPlaceholder = new QWidget(this);
    mPlaceholder->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

    mTopLayout = new QHBoxLayout;
    mTopLayout->addWidget(mTopLabel, 8);

    mLayout = new QVBoxLayout(this);
    mLayout->setContentsMargins(4, 4, 4, 4);
    mLayout->setSpacing(2);

    mLayout->addWidget(mOnOffSwitch, 1);
    mLayout->addLayout(mTopLayout, 1);
    mLayout->addWidget(mPlaceholder, 8);
}

void ListMoodDetailedWidget::update(const cor::Mood& mood) {
    mSimpleGroupWidget->removeWidgets();
    mKey = mood.uniqueID();
    mTopLabel->setText(mood.name());
    mSimpleGroupWidget->updateDevices(mood.lights(),
                                      cor::EWidgetType::full,
                                      EOnOffSwitchState::hidden,
                                      false,
                                      true);
    mEditPage->showMood(mood, mComm->allDevices());

    mOnOffSwitch->setSwitchState(ESwitchState::off);
    mAdditionalDetailsWidget->display(mood, mPlaceholder->size());
}

void ListMoodDetailedWidget::clickedDevice(const QString&) {
    // qDebug() << " device clicked " << key << " vs" << deviceName;
}

void ListMoodDetailedWidget::resize() {
    QSize size = parentWidget()->size();
    setGeometry(int(size.width() * 0.125f),
                int(size.height() * 0.125f),
                int(size.width() * 0.75f),
                int(size.height() * 0.75f));
    moveFloatingLayout();
    if (mPageKey == "Group_Details") {
        mScrollArea->setGeometry(mPlaceholder->geometry());
        mAdditionalDetailsWidget->setFixedWidth(mScrollArea->viewport()->width());
        mAdditionalDetailsWidget->resize(mPlaceholder->geometry().size());
    } else if (mPageKey == "Group_Lights") {
        mSimpleGroupWidget->setGeometry(mPlaceholder->geometry().x(),
                                        mPlaceholder->geometry().y(),
                                        width() * 0.98,
                                        height());
        mSimpleGroupWidget->resizeWidgets();
    } else if (mPageKey == "Group_Edit") {
        mEditPage->setGeometry(mPlaceholder->geometry());
        mEditPage->resize();
    }
}

void ListMoodDetailedWidget::paintEvent(QPaintEvent*) {
    QStyleOption opt;
    opt.init(this);
    QPainter painter(this);

    painter.setRenderHint(QPainter::Antialiasing);
    painter.fillRect(rect(), QBrush(QColor(48, 47, 47)));
}

void ListMoodDetailedWidget::floatingLayoutButtonPressed(const QString& key) {
    if (key != mPageKey) {
        mPageKey = key;
        if (key == "Group_Details") {
            mScrollArea->setVisible(true);
            mEditPage->setVisible(false);
            mSimpleGroupWidget->setVisible(false);
            mScrollArea->raise();
            mEditPage->hide();
            mEditPage->isOpen(false);
        } else if (key == "Group_Lights") {
            mScrollArea->setVisible(false);
            mEditPage->setVisible(false);
            mSimpleGroupWidget->setVisible(true);
            mSimpleGroupWidget->raise();
            mEditPage->hide();
            mEditPage->isOpen(false);
        } else if (key == "Group_Edit") {
            mScrollArea->setVisible(false);
            mEditPage->setVisible(true);
            mSimpleGroupWidget->setVisible(false);
            mEditPage->raise();
            mEditPage->show();
            mEditPage->isOpen(true);
        }
        resize();
    }
}

void ListMoodDetailedWidget::moveFloatingLayout() {
    QSize size = parentWidget()->size();
    mOnOffSwitch->setFixedWidth(int(size.width() * 0.2));

    QPoint topRight(int(size.width() * 0.875), int(size.height() * 0.125));
    mFloatingMenu->move(topRight);
}

void ListMoodDetailedWidget::changedSwitchState(bool state) {
    if (state) {
        emit enableGroup(mKey);
    }
}

void ListMoodDetailedWidget::resizeEvent(QResizeEvent*) {
    if (isVisible()) {
        resize();
    }
}

void ListMoodDetailedWidget::pushIn() {
    mFloatingMenu->highlightButton("Group_Lights");
    floatingLayoutButtonPressed("Group_Lights");
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
    mSimpleGroupWidget->resizeWidgets();
}

void ListMoodDetailedWidget::pushOut() {
    mFloatingMenu->setVisible(false);
    isOpen(false);
    mPageKey = "";

    moveWidget(
        this,
        QPoint(int(parentWidget()->width() * 0.125f), int(parentWidget()->height() * 0.125f)),
        QPoint(int(parentWidget()->width() * 0.125f), int(-1 * parentWidget()->height())));

    auto widthPoint = int(parentWidget()->width() * 0.875f - topMenu()->size().width());
    QPoint startPoint(widthPoint, int(parentWidget()->height() * 0.125f));
    cor::moveWidget(topMenu(), startPoint, QPoint(widthPoint, int(-1 * parentWidget()->height())));
}