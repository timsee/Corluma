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
    mTitle->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Expanding);
    mTitle->setStyleSheet(transparentStyleSheet);
    mTitle->setAlignment(Qt::AlignVCenter);

    mButtonState = EGroupButtonState::clearAll;

    mButton = new QLabel(this);
    mButton->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Expanding);
    resize();

    mLayout = new QHBoxLayout;
    mLayout->addWidget(mTitle);
    mLayout->addWidget(mButton);
    this->setLayout(mLayout);

    this->setMinimumHeight(mTitle->height());

    handleSelectAllButton(0u, 0u);
}


void GroupButton::resize() {
    QSize size = preferredButtonSize();
    mClearAllPixmap = QPixmap(":/images/selectAllIcon.png");
    mClearAllPixmap = mClearAllPixmap.scaled(size.width(),
                                               size.height(),
                                               Qt::IgnoreAspectRatio,
                                               Qt::SmoothTransformation);

    mSelectAllPixmap = QPixmap(":/images/uncheckedBox.png");
    mSelectAllPixmap = mSelectAllPixmap.scaled(size.width(),
                                             size.height(),
                                             Qt::IgnoreAspectRatio,
                                             Qt::SmoothTransformation);

    mDisabledPixmap = QPixmap(":/images/disabledX.png");
    mDisabledPixmap = mDisabledPixmap.scaled(size.width(),
                                             size.height(),
                                             Qt::IgnoreAspectRatio,
                                             Qt::SmoothTransformation);

    mTitle->setFixedWidth(this->width() - preferredButtonSize().width());
    mButton->setFixedWidth(preferredButtonSize().width());

    if (handleSelectAllButton(mCheckedCount, mReachableCount)) {
        update();
    }
}

bool GroupButton::handleSelectAllButton(uint32_t checkedDevicesCount, uint32_t reachableDevicesCount) {
    bool renderFlag = false;
    EGroupButtonState state;
    if (reachableDevicesCount == 0) {
        state = EGroupButtonState::disabled;
    } else if (mCheckedCount > 0) {
        state = EGroupButtonState::clearAll;
    } else {
        state = EGroupButtonState::selectAll;
    }

    if (mButtonState != state) {
        mButtonState = state;
        mButton->setPixmap(currentPixmap());
        renderFlag = true;
    }

   if (mCheckedCount != checkedDevicesCount
           || mReachableCount != reachableDevicesCount) {
        mCheckedCount = checkedDevicesCount;
        mReachableCount = reachableDevicesCount;
        renderFlag = true;
   }
   return renderFlag;
}

void GroupButton::setSelectAll(bool shouldSelect) {
    mIsSelected = shouldSelect;
}

void GroupButton::buttonPressed(bool) {
    if (mButtonState != EGroupButtonState::disabled) {
        if (mCheckedCount > 0) {
            mButtonState = EGroupButtonState::selectAll;
            mCheckedCount = 0;
        } else {
            mCheckedCount = mReachableCount;
            mButtonState = EGroupButtonState::clearAll;
        }

        mButton->setPixmap(currentPixmap());

        emit groupSelectAllToggled(mTitle->text(), EGroupButtonState::clearAll == mButtonState);
        update();
    }
}

QColor GroupButton::computeHighlightColor(uint32_t checkedDeviceCount, uint32_t reachableDeviceCount) {
    QColor pureBlue(61, 142, 201);
    QColor pureBlack(32, 31, 31);
    QColor difference(pureBlue.red() - pureBlack.red(),
                      pureBlue.green() - pureBlack.green(),
                      pureBlue.blue() - pureBlack.blue());


    if (checkedDeviceCount == 0 || reachableDeviceCount == 0) {
        return {32, 31, 31, 255};
    }
    auto amountOfBlue = checkedDeviceCount / float(reachableDeviceCount);
    return {int(amountOfBlue * difference.red() + pureBlack.red()),
            int(amountOfBlue * difference.green() + pureBlack.green()),
            int(amountOfBlue * difference.blue() + pureBlack.blue())};
}

void GroupButton::resizeEvent(QResizeEvent *) {
    resize();
    mButton->setPixmap(currentPixmap());
}

void GroupButton::paintEvent(QPaintEvent *) {
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
    if (cor::isMouseEventTouchUpInside(event, this, true)) {
        if (cor::isMouseEventTouchUpInside(event, mButton, false)) {
            QWidget *parentWidget = this->parentWidget();
            auto groupButtonsWidget = qobject_cast<GroupButtonsWidget*>(parentWidget);
            if (groupButtonsWidget->type() == cor::EWidgetType::condensed) {
                if (cor::leftHandMenuMoving()) {
                    return;
                }
            }
            buttonPressed(true);
        } else {
            emit groupButtonPressed(mTitle->text());
        }
    }
    event->ignore();
}


const QPixmap& GroupButton::currentPixmap() {
    switch (mButtonState) {
    case EGroupButtonState::disabled:
        return mDisabledPixmap;
    case EGroupButtonState::clearAll:
        return mClearAllPixmap;
    case EGroupButtonState::selectAll:
        return mSelectAllPixmap;
    }
    THROW_EXCEPTION("Do not recognize pixmap");
}

QSize GroupButton::preferredButtonSize() {
    return {int(mTitle->height() * 0.9), int(mTitle->height() * 0.9)};
}

}
