/*!
 * \copyright
 * Copyright (C) 2015 - 2020.
 * Released under the GNU General Public License.
 */

#include "groupstatewidget.h"

#include <QGraphicsEffect>
#include <QGraphicsScene>
#include <QMouseEvent>
#include <QPainter>
#include <QStyleOption>

#include "utils/qt.h"


GroupStateWidget::GroupStateWidget(const cor::GroupState& groupState, QWidget* parent)
    : cor::ListItemWidget(groupState.stringUniqueID(), parent),
      mGroupState{groupState},
      mIsChecked{false},
      mAllowInteraction{true},
      mHasRendered{false},
      mDisplayState{true},
      mFontPtSize(16),
      mName{new QLabel(this)} {
    mName->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    mName->setAlignment(Qt::AlignVCenter);
    mName->setStyleSheet("background-color:rgba(0,0,0,0);");

    mKey = mGroupState.stringUniqueID();
    updateState(mGroupState);
}


void GroupStateWidget::updateState(const cor::GroupState& groupState) {
    bool shouldRender = false;
    if (!(groupState.state() == mGroupState.state()) && isVisible()) {
        shouldRender = true;
    }
    if (!mHasRendered) {
        shouldRender = true;
        mHasRendered = false;
    }

    if (groupState.name() != mName->text()) {
        mName->setText(createName(groupState.name()));
    }

    mGroupState = groupState;
    if (shouldRender) {
        mIconData.setRoutine(mGroupState.state());
        mIconPixmap = mIconData.renderAsQPixmap();
        update();
    }
}

void GroupStateWidget::setChecked(bool checked) {
    mIsChecked = checked;
    update();
}


namespace {

void paintLightState(QPainter& painter,
                     const QRect& rect,
                     const cor::LightState& state,
                     QPixmap& statePixmap) {
    // draw the pixmap stretched to teh width provided
    if (state.isOn()) {
        if (statePixmap.size() != rect.size()) {
            statePixmap = statePixmap.scaled(rect.width(),
                                             rect.height(),
                                             Qt::IgnoreAspectRatio,
                                             Qt::FastTransformation);
        }
        QBrush iconBrush(statePixmap);
        painter.setBrush(iconBrush);
        painter.drawRect(rect);
    } else {
        QBrush blackBrush(QColor(0, 0, 0));
        painter.setBrush(blackBrush);
        painter.drawRect(rect);
    }
}

} // namespace

void GroupStateWidget::paintEvent(QPaintEvent*) {
    QStyleOption opt;
    opt.initFrom(this);
    QPainter painter(this);

    // handle highlight
    if (mIsChecked) {
        painter.fillRect(rect(), QBrush(QColor(61, 142, 201)));
    } else {
        painter.fillRect(rect(), QBrush(QColor(32, 31, 31)));
    }

    QRect stateIconRect = stateIconRegion();
    if (!mGroupState.state().isOn()) {
        painter.setOpacity(0.25);
    }

    if (mDisplayState) {
        // draw the pixmap stretched to teh width provided
        paintLightState(painter, stateIconRect, mGroupState.state(), mIconPixmap);
    }
}

void GroupStateWidget::mouseReleaseEvent(QMouseEvent* event) {
    if (!mAllowInteraction) {
        event->ignore();
        return;
    }

    if (cor::isMouseEventTouchUpInside(event, this, true)) {
        setChecked(!mIsChecked);
        emit clicked(mKey);
    }
    event->ignore();
}

QString GroupStateWidget::createName(QString nameText) {
    if (nameText.size() > 20) {
        nameText = nameText.mid(0, 17) + "...";
    }
    return nameText;
}

void GroupStateWidget::resize() {
    int nameX = spacer();
    if (mDisplayState) {
        nameX = stateIconRegion().x() + stateIconRegion().width() + spacer();
    }
    mName->setGeometry(nameX, stateIconRegion().y(), width() - nameX, stateIconRegion().height());
}



void GroupStateWidget::resizeEvent(QResizeEvent*) {
    resize();
}

int GroupStateWidget::spacer() {
    return this->geometry().height() / 10;
}

QRect GroupStateWidget::stateIconRegion() {
    auto iconSpacer = this->height() * 0.1;
    int x = spacer();
    int y = iconSpacer;
    int side = int(height() * 0.8);
    return QRect(x, y, side, side);
}
