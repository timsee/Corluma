/*!
 * \copyright
 * Copyright (C) 2015 - 2019.
 * Released under the GNU General Public License.
 */

#include "groupbutton.h"
#include "utils/qt.h"

#include <QDebug>
#include <QGraphicsEffect>
#include <QPainter>
#include <QStyleOption>
#include <QGraphicsScene>
#include <QMouseEvent>

namespace cor
{

GroupButton::GroupButton(QWidget *parent, const QString& text) : QWidget(parent),
                                                                         mIsSelected{false},
                                                                         mReachableCount{0},
                                                                         mCheckedCount{0} {
    const QString transparentStyleSheet = "background-color: rgba(0,0,0,0);";

    mTitle = new QLabel(text, this);
    mTitle->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    mTitle->setStyleSheet(transparentStyleSheet);
    mTitle->setAlignment(Qt::AlignBottom);

    mButtonState = EGroupButtonState::clearAll;

    mButton = new QPushButton(this);
    mButton->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    mSelectAllPixmap = QPixmap(":/images/selectAllIcon.png");
    mClearAllPixmap = QPixmap(":/images/uncheckedBox.png");
    mDisabledPixmap = QPixmap(":/images/disabledX.png");

    // make a minimum size for the button
    auto applicationSize = cor::applicationSize();
    int prefferedWidth = std::max(int(applicationSize.height() / 20.0f), int(mButton->size().height() * 0.9f));
    mButton->setIconSize(QSize(prefferedWidth, prefferedWidth));
    mButton->setFixedSize(QSize(prefferedWidth, prefferedWidth));
    mButton->setIcon(QIcon(mClearAllPixmap));
    connect(mButton, SIGNAL(clicked(bool)), this, SLOT(buttonPressed(bool)));

    mLayout = new QHBoxLayout;
    mLayout->addWidget(mTitle);
    mLayout->addWidget(mButton);
    this->setLayout(mLayout);

    this->setMinimumHeight(mTitle->height());
}


void GroupButton::handleSelectAllButton(uint32_t checkedDevicesCount, uint32_t reachableDevicesCount) {
    mCheckedCount = checkedDevicesCount;
    mReachableCount = reachableDevicesCount;

    if (reachableDevicesCount == 0) {
        mButtonState = EGroupButtonState::disabled;
        mButton->setIcon(QIcon(mDisabledPixmap));
    } else if (mCheckedCount > 0) {
        mButtonState = EGroupButtonState::clearAll;
        mButton->setIcon(QIcon(mSelectAllPixmap));
    } else {
        mButtonState = EGroupButtonState::selectAll;
        mButton->setIcon(QIcon(mClearAllPixmap));
    }
    repaint();
}

void GroupButton::setSelectAll(bool shouldSelect) {
    mIsSelected = shouldSelect;
    repaint();
}

void GroupButton::buttonPressed(bool) {
    if (mButtonState != EGroupButtonState::disabled) {
        if (mCheckedCount > 0) {
            mButtonState = EGroupButtonState::selectAll;
            mCheckedCount = 0;
            mButton->setIcon(QIcon(mClearAllPixmap));
        } else {
            mCheckedCount = mReachableCount;
            mButtonState = EGroupButtonState::clearAll;
            mButton->setIcon(QIcon(mSelectAllPixmap));
        }
        emit groupSelectAllToggled(mTitle->text(), EGroupButtonState::clearAll == mButtonState);
        repaint();
    }
}

QColor GroupButton::computeHighlightColor(uint32_t checkedDeviceCount, uint32_t reachableDeviceCount) {
    QColor pureBlue(61, 142, 201);
    QColor pureBlack(32, 31, 31);
    QColor difference(pureBlue.red() - pureBlack.red(),
                      pureBlue.green() - pureBlack.green(),
                      pureBlue.blue() - pureBlack.blue());


    if (checkedDeviceCount == 0 || reachableDeviceCount == 0) {
        return QColor(32, 31, 31, 255);
    } else {
        auto amountOfBlue = checkedDeviceCount / float(reachableDeviceCount);
        return QColor(int(amountOfBlue * difference.red() + pureBlack.red()),
                      int(amountOfBlue * difference.green() + pureBlack.green()),
                      int(amountOfBlue * difference.blue() + pureBlack.blue()));
    }
}

void GroupButton::resizeEvent(QResizeEvent *) {

}

void GroupButton::paintEvent(QPaintEvent *event) {
    Q_UNUSED(event);
    QStyleOption opt;
    opt.init(this);
    QPainter painter(this);
    QPen pen(Qt::white, 5);
    painter.setPen(pen);

    QPainterPath path;
    path.addRect(this->rect());

    if (mButtonState == EGroupButtonState::clearAll) {
        painter.fillPath(path, QBrush(computeHighlightColor(mCheckedCount, mReachableCount)));
    } else {
        painter.fillPath(path, QBrush(QColor(32, 31, 31, 255)));
    }

    if (mIsSelected) {
        painter.drawPath(path);
    }
}


void GroupButton::mouseReleaseEvent(QMouseEvent *event) {
    if (cor::isMouseEventTouchUpInside(event, this)) {
        emit groupButtonPressed(mTitle->text());
    }
    event->ignore();
}

}
