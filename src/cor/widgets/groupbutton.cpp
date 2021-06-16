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
#include <QPainterPath>
#include <QStyleOption>

#include "menu/menusubgroupcontainer.h"
#include "utils/qt.h"

namespace cor {

const int kGroupButtonBorderWidth = 5;

GroupButton::GroupButton(const QString& key, const QString& text, QWidget* parent)
    : QWidget(parent),
      mKey{key},
      mIsSelected{false},
      mShowSelectAll{true},
      mHighlightByCountOfLights{true},
      mShowStates{false},
      mArrowState{EArrowState::disabled},
      mReachableCount{0},
      mCheckedCount{0},
      mButtonHeight{0},
      mCheckBox{new cor::CheckBox(this)},
      mArrowIcon{new QLabel(this)},
      mPaletteWidget{new cor::PaletteWidget(this)},
      mTitle{new QLabel(text, this)},
      mRectOptions{EPaintRectOptions::onlyBottom} {
    mPaletteWidget->skipOffLightStates(true);
    mPaletteWidget->showInSingleLine(true);
    mPaletteWidget->shouldPreferPalettesOverRoutines(true);
    mPaletteWidget->setBrightnessMode(cor::EBrightnessMode::average);

    mTitle->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    mTitle->setStyleSheet(cor::kTransparentAndBoldStylesheet);
    mTitle->setAlignment(Qt::AlignVCenter);
    mTitle->setAttribute(Qt::WA_TransparentForMouseEvents);

    mArrowIcon->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    mArrowIcon->setAlignment(Qt::AlignCenter);
    mArrowIcon->setStyleSheet(cor::kTransparentAndBoldStylesheet);

    connect(mCheckBox,
            SIGNAL(clicked(ECheckboxState)),
            this,
            SLOT(checkBoxClicked(ECheckboxState)));

    handleSelectAllCheckbox(0u, 0u);
}


void GroupButton::resize() {
    auto originalIconSide = mButtonHeight;
    const auto& size = iconSize();
    mButtonHeight = size.height();

    auto xPos = width();

    if (mArrowState != EArrowState::disabled) {
        if (mButtonHeight != originalIconSide) {
            QPixmap pixmap;
            if (mArrowState == EArrowState::closed) {
                pixmap = QPixmap(":/images/closedArrow.png");
            } else if (mArrowState == EArrowState::open) {
                pixmap = QPixmap(":/images/openedArrow.png");
            }
            pixmap = pixmap.scaled(mButtonHeight,
                                   mButtonHeight,
                                   Qt::KeepAspectRatio,
                                   Qt::SmoothTransformation);
            mArrowIcon->setPixmap(pixmap);
        }
        mArrowIcon->setGeometry(xPos - mButtonHeight, 0u, mButtonHeight, height());
        xPos -= mArrowIcon->width();
    }

    if (mShowSelectAll) {
        mCheckBox->setGeometry(xPos - size.width(), 0, size.width(), height());
        xPos -= mCheckBox->width();
    }
    auto previewHeight = height() / 4;
    mPaletteWidget->setGeometry(kGroupButtonBorderWidth,
                                kGroupButtonBorderWidth,
                                xPos,
                                previewHeight);

    auto spaceWidth = (width() / 20);
    mTitle->setGeometry(spaceWidth, previewHeight, xPos - spaceWidth, height() - previewHeight);

    if (handleSelectAllCheckbox(mCheckedCount, mReachableCount)) {
        update();
    }
}

bool GroupButton::handleSelectAllCheckbox(std::uint32_t checkedDevicesCount,
                                          uint32_t reachableDevicesCount) {
    bool renderFlag = false;
    ECheckboxState state;
    if (reachableDevicesCount == 0) {
        state = ECheckboxState::disabled;
    } else if (checkedDevicesCount > 0) {
        state = ECheckboxState::checked;
    } else {
        state = ECheckboxState::unchecked;
    }

    if (mShowSelectAll) {
        mCheckBox->setVisible(true);
        if (mCheckBox->checkboxState() != state) {
            mCheckBox->checkboxState(state);
            renderFlag = true;
        }
    } else {
        renderFlag = true;
        mCheckBox->setVisible(false);
    }

    if (mCheckedCount != checkedDevicesCount || mReachableCount != reachableDevicesCount) {
        mCheckedCount = checkedDevicesCount;
        mReachableCount = reachableDevicesCount;
        renderFlag = true;
    }
    if (renderFlag) {
        update();
    }
    return renderFlag;
}

void GroupButton::setSelectAll(bool shouldSelect) {
    mIsSelected = shouldSelect;
}

void GroupButton::checkBoxClicked(ECheckboxState state) {
    if (state != ECheckboxState::disabled) {
        if (mCheckedCount > 0) {
            mCheckBox->checkboxState(ECheckboxState::unchecked);
            mCheckedCount = 0;
        } else {
            mCheckedCount = mReachableCount;
            mCheckBox->checkboxState(ECheckboxState::checked);
        }

        emit groupSelectAllToggled(mTitle->text(),
                                   ECheckboxState::checked == mCheckBox->checkboxState());
        update();
    }
}

void GroupButton::changeArrowState(EArrowState arrowState) {
    mArrowState = arrowState;
    if (mArrowState == EArrowState::open) {
        mArrowIcon->setVisible(true);
    } else if (mArrowState == EArrowState::closed) {
        mArrowIcon->setVisible(true);
    } else if (mArrowState == EArrowState::disabled) {
        mArrowIcon->setVisible(false);
    }
    resize();
}


void GroupButton::resizeEvent(QResizeEvent*) {
    resize();
}

void GroupButton::paintEvent(QPaintEvent*) {
    QStyleOption opt;
    opt.initFrom(this);
    QPainter painter(this);
    QPen pen(Qt::white, kGroupButtonBorderWidth);
    painter.setPen(pen);

    QPainterPath path;
    path.addRect(rect());

    if (mHighlightByCountOfLights) {
        painter.fillPath(path, QBrush(computeHighlightColor(mCheckedCount, mReachableCount)));
    } else {
        painter.fillPath(path, QBrush(cor::kBackgroundColor));
    }

    paintRect(painter, rect(), mRectOptions);
}


void GroupButton::mouseReleaseEvent(QMouseEvent* event) {
    if (cor::isMouseEventTouchUpInside(event, this, true)) {
        if (!(mShowSelectAll && cor::isMouseEventTouchUpInside(event, mCheckBox, false))) {
            emit groupButtonPressed(mTitle->text());
        }
    }
    event->ignore();
}

QSize GroupButton::iconSize() {
    return {int(height()), int(height())};
}

} // namespace cor
