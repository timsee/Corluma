/*!
 * \copyright
 * Copyright (C) 2015 - 2020.
 * Released under the GNU General Public License.
 */

#include "groupbutton.h"

#include <QDebug>
#include <QGraphicsEffect>
#include <QGraphicsScene>
#include <QMouseEvent>
#include <QPainter>
#include <QStyleOption>

#include "groupbuttonswidget.h"
#include "utils/qt.h"

namespace cor {

GroupButton::GroupButton(QWidget* parent, const QString& text)
    : QWidget(parent),
      mButtonState{EGroupButtonState::selectAll},
      mIsSelected{false},
      mReachableCount{0},
      mCheckedCount{0},
      mTitle{new QLabel(text, this)},
      mButton{new QLabel(this)} {
    const QString transparentStyleSheet = "background-color: rgba(0,0,0,0);";

    mTitle->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    mTitle->setStyleSheet(transparentStyleSheet);
    mTitle->setAlignment(Qt::AlignVCenter);

    mButton->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);

    handleSelectAllButton(0u, 0u);
}


void GroupButton::resize() {
    const auto& size = iconSize();
    // spacer is going to be applied twice, but adds 10% of the space overall.
    auto spaceWidth = (width() / 20);
    auto titleWidth = width() - size.width() - spaceWidth * 2;
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

    mTitle->setGeometry(spaceWidth, 0, titleWidth, height());
    mButton->setGeometry(titleWidth + spaceWidth, 0, size.width(), height());
    mButton->setPixmap(currentPixmap());

    if (handleSelectAllButton(mCheckedCount, mReachableCount)) {
        update();
    }
}

bool GroupButton::handleSelectAllButton(std::uint32_t checkedDevicesCount,
                                        uint32_t reachableDevicesCount) {
    bool renderFlag = false;
    EGroupButtonState state;
    if (reachableDevicesCount == 0) {
        state = EGroupButtonState::disabled;
    } else if (checkedDevicesCount > 0) {
        state = EGroupButtonState::clearAll;
    } else {
        state = EGroupButtonState::selectAll;
    }

    if (mButtonState != state) {
        mButtonState = state;
        mButton->setPixmap(currentPixmap());
        renderFlag = true;
    }

    if (mCheckedCount != checkedDevicesCount || mReachableCount != reachableDevicesCount) {
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

QColor GroupButton::computeHighlightColor(std::uint32_t checkedDeviceCount,
                                          uint32_t reachableDeviceCount) {
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

void GroupButton::resizeEvent(QResizeEvent*) {
    resize();
}

void GroupButton::paintEvent(QPaintEvent*) {
    QStyleOption opt;
    opt.init(this);
    QPainter painter(this);
    QPen pen(Qt::white, 5);
    painter.setPen(pen);

    QPainterPath path;
    path.addRect(rect());

    if (mButtonState == EGroupButtonState::clearAll) {
        painter.fillPath(path, QBrush(computeHighlightColor(mCheckedCount, mReachableCount)));
    } else {
        painter.fillPath(path, QBrush(QColor(32, 31, 31, 255)));
    }
}


void GroupButton::mouseReleaseEvent(QMouseEvent* event) {
    if (cor::isMouseEventTouchUpInside(event, this, true)) {
        if (cor::isMouseEventTouchUpInside(event, mButton, false)) {
            auto groupButtonsWidget = qobject_cast<GroupButtonsWidget*>(parentWidget());
            if (groupButtonsWidget != nullptr) {
                if (groupButtonsWidget->type() == cor::EWidgetType::condensed) {
                    if (cor::leftHandMenuMoving()) {
                        return;
                    }
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

QSize GroupButton::iconSize() {
    return {int(height() * 0.75), int(height() * 0.75)};
}

} // namespace cor
