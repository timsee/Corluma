/*!
 * \copyright
 * Copyright (C) 2015 - 2020.
 * Released under the GNU General Public License.
 */

#include "listlightwidget.h"

#include <QGraphicsEffect>
#include <QGraphicsScene>
#include <QMouseEvent>
#include <QPainter>
#include <QStyleOption>

#include "utils/qt.h"


ListLightWidget::ListLightWidget(const cor::Light& light,
                                 bool setHighlightable,
                                 EListLightWidgetType type,
                                 QWidget* parent)
    : cor::ListItemWidget(light.uniqueID(), parent),
      mNoConnectionPixmap(":/images/questionMark.png"),
      mType{type},
      mHardwareType{light.hardwareType()},
      mTypePixmap{lightHardwareTypeToPixmap(mHardwareType)},
      mState{light.state()},
      mShouldHighlight{setHighlightable},
      mIsChecked{false},
      mAllowInteraction{true},
      mDisplayState{true},
      mHasRendered{false},
      mFontPtSize(16),
      mLight{light},
      mName{new QLabel(this)},
      mTypeIcon{new QLabel(this)},
      mCondenseStandardWidget{false} {
    mTypeIcon->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    mTypeIcon->setStyleSheet("background-color:rgba(0,0,0,0);");

    mName->setText(createName(light.name()));
    mName->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    mName->setAlignment(Qt::AlignVCenter);
    mName->setStyleSheet("background-color:rgba(0,0,0,0);");

    mKey = light.uniqueID();
    updateWidget(light);
}


void ListLightWidget::updateWidget(const cor::Light& light) {
    bool shouldRender = false;
    if (!(light.state() == mState) && isVisible()) {
        shouldRender = true;
    }
    if (!mHasRendered) {
        shouldRender = true;
        mHasRendered = false;
    }

    if (mHardwareType != light.hardwareType()) {
        mTypePixmap = lightHardwareTypeToPixmap(light.hardwareType());
        mHardwareType = light.hardwareType();
        resizeIcons();
    }

    if (light.name() != mName->text()) {
        mName->setText(createName(light.name()));
    }

    mState = light.state();
    mLight = light;
    if (shouldRender) {
        mIconData.setRoutine(light.state());
        mIconPixmap = mIconData.renderAsQPixmap();
        update();
    }
}

bool ListLightWidget::setHighlightChecked(bool checked) {
    mIsChecked = checked;
    if (mShouldHighlight) {
        update();
    }
    return mIsChecked;
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


void paintLightBarBackground(QPainter& painter, const cor::LightState& state, const QRect& rect) {
    // draw the back rectangle
    QBrush blackBrush(QColor(0, 0, 0));
    if (!state.isOn()) {
        // if its not on, hide it
        blackBrush.setColor(QColor(0, 0, 0, 5));
    }
    painter.setBrush(blackBrush);
    painter.drawRect(rect);
}

} // namespace

void ListLightWidget::paintEvent(QPaintEvent*) {
    QStyleOption opt;
    opt.init(this);
    QPainter painter(this);

    // handle highlight
    if (mIsChecked && mShouldHighlight) {
        painter.fillRect(rect(), QBrush(QColor(61, 142, 201)));
    } else {
        painter.fillRect(rect(), QBrush(QColor(32, 31, 31)));
    }

    QRect stateIconRect = stateIconRegion();
    if (mLight.isReachable()) {
        if (!mState.isOn()) {
            painter.setOpacity(0.25);
        }

        // if the icon is a "full" version, the state icon is more of a brightness bar; the state
        // icon stretches to fill free space with a black widget. then the state icon is overlaid
        // over a percentage of that black rectangele. IE, if brightness is 50%, the state icon is
        // extended over 50% of the black rectangle
        if (mType == EListLightWidgetType::fullBrightnessBar) {
            // paint the background of the brightness bar
            paintLightBarBackground(painter, mState, stateIconRect);

            // generate a brightness for the state
            double brightness = mState.color().valueF();
            if (mState.routine() > cor::ERoutineSingleColorEnd) {
                brightness = mState.palette().brightness() / 100.0;
            }

            // scale down the width of the rect of the state icon, to show how bright it is
            stateIconRect.setWidth(int(stateIconRect.width() * brightness));
        }

        if (mDisplayState) {
            // draw the pixmap stretched to teh width provided
            paintLightState(painter, stateIconRect, mState, mIconPixmap);
        }
    } else if (mDisplayState) {
        painter.setOpacity(0.5);
        painter.drawPixmap(stateIconRect, mNoConnectionPixmap);
    }
}

void ListLightWidget::mouseReleaseEvent(QMouseEvent* event) {
    if ((mType == EListLightWidgetType::standard) && cor::leftHandMenuMoving()) {
        event->ignore();
        return;
    }

    if (!mAllowInteraction) {
        event->ignore();
        return;
    }

    if (cor::isMouseEventTouchUpInside(event, this, true)) {
        setHighlightChecked(!mIsChecked);
        emit clicked(mKey);
    }
    event->ignore();
}

QString ListLightWidget::createName(QString nameText) {
//    if (mType == EListLightWidgetType::fullBrightnessBar) {
//        if (nameText.size() > 20) {
//            nameText = nameText.mid(0, 17) + "...";
//        }
//    } else {
//        if (nameText.size() > 17) {
//            nameText = nameText.mid(0, 14) + "...";
//        }
//    }
    return nameText;
}


namespace {

/// logic to determine if the widget should be condensed or not
bool shouldCondenseWidget(QSize size, bool displayState) {
    auto iconSide = size.height() * 0.8;
    if (displayState) {
       iconSide = iconSide * 2;
    }
    if (iconSide > size.width() * 0.4) {
        return true;
    }
    return false;
}

}
void ListLightWidget::resize() {
    if (mType == EListLightWidgetType::fullBrightnessBar) {
        auto typeIconSide = this->height() * 0.8;
        mTypeIcon->setGeometry(spacer(), this->height() * 0.1, typeIconSide, typeIconSide);
        mName->setGeometry(stateIconRegion().x(),
                           stateIconRegion().y() + stateIconRegion().height(),
                           stateIconRegion().width(),
                           stateIconRegion().height());
    } else if (mType == EListLightWidgetType::standard) {
        mCondenseStandardWidget = shouldCondenseWidget(this->size(), mDisplayState);

        int nameX = 0;
        if (!mCondenseStandardWidget) {
            auto typeIconSide = this->height() * 0.8;
            mTypeIcon->setGeometry(spacer(), this->height() * 0.1, typeIconSide, typeIconSide);
            if (mDisplayState) {
                nameX = stateIconRegion().x() + stateIconRegion().width() + spacer();
            } else {
                nameX = stateIconRegion().x() + spacer();
            }
            mName->setGeometry(nameX,
                               stateIconRegion().y(),
                               width() - nameX,
                               stateIconRegion().height());
        } else {
            auto typeIconSide = this->height() * 0.4;
            mTypeIcon->setGeometry(spacer(), this->height() * 0.1, typeIconSide, typeIconSide);
            mName->setGeometry(nameX,
                               stateIconRegion().y() + stateIconRegion().height(),
                               width() - nameX,
                               stateIconRegion().height());
        }
    }

    // resize icons only if necessary
    if (mTypeIcon->size() != mLastIconSize) {
        mLastIconSize = mTypeIcon->size();
        resizeIcons();
    }
}

void ListLightWidget::resizeIcons() {
    mTypePixmap = lightHardwareTypeToPixmap(mHardwareType);
    auto min = mTypeIcon->height();
    mTypePixmap = mTypePixmap.scaled(min, min, Qt::IgnoreAspectRatio, Qt::SmoothTransformation);
    mTypeIcon->setPixmap(mTypePixmap);
    mNoConnectionPixmap = QPixmap(":/images/questionMark.png");
    mNoConnectionPixmap =
        mNoConnectionPixmap.scaled(min, min, Qt::KeepAspectRatio, Qt::SmoothTransformation);
}


void ListLightWidget::resizeEvent(QResizeEvent*) {
    resize();
}

int ListLightWidget::spacer() {
    return this->geometry().height() / 10;
}

QRect ListLightWidget::stateIconRegion() {
    int x = mTypeIcon->width() + mTypeIcon->pos().x() + spacer();
    int y = mTypeIcon->pos().y();
    if (mType == EListLightWidgetType::fullBrightnessBar) {
        int width = this->width() - spacer() * 3 - mTypeIcon->width();
        int height = int(this->height() / 3);
        return QRect(x, y, width, height);
    } else {
        if (mCondenseStandardWidget) {
            int side = int(height() * 0.4);
            return QRect(x, y, side, side);
        } else {
            int side = int(height() * 0.8);
            return QRect(x, y, side, side);
        }
    }
}
