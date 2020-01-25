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
                                 cor::EWidgetType type,
                                 QWidget* parent)
    : cor::ListItemWidget(light.uniqueID(), parent),
      mNoConnectionPixmap(":/images/questionMark.png"),
      mType{type},
      mHardwareType{light.hardwareType()},
      mState{light.state()},
      mIsReachable{light.isReachable()},
      mShouldHighlight{setHighlightable},
      mIsChecked{false},
      mFontPtSize(16),
      mLight{light} {
    init(light.uniqueID(), light.name(), light.hardwareType());
    updateWidget(light);
}


void ListLightWidget::init(const QString& uniqueID,
                           const QString& name,
                           ELightHardwareType hardwareType) {
    // setup icon
    mIconData = IconData(4, 4);

    // setup controller label
    mController = new QLabel(this);

    mTypeIcon = new QLabel(this);
    mTypeIcon->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);

    mTypePixmap = lightHardwareTypeToPixmap(hardwareType);

    QString nameText = createName(name);
    mController->setText(nameText);
    mController->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    mController->setAlignment(Qt::AlignVCenter);

    mKey = uniqueID;
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

    if (mHardwareType != light.hardwareType() || mLastRenderedSize != size()) {
        mTypePixmap = lightHardwareTypeToPixmap(light.hardwareType());
        resizeIcons();
        mLastRenderedSize = size();
    }

    if (light.name() != mController->text()) {
        mController->setText(createName(light.name()));
    }

    mState = light.state();
    mIsReachable = light.isReachable();
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


void ListLightWidget::paintEvent(QPaintEvent*) {
    QStyleOption opt;
    opt.init(this);
    QPainter painter(this);

    if (mIsChecked && mShouldHighlight) {
        painter.fillRect(rect(), QBrush(QColor(61, 142, 201)));
    } else {
        painter.fillRect(rect(), QBrush(QColor(32, 31, 31)));
    }

    int x = (iconRegion().width() + spacer() * 1.5);
    auto side = iconRegion().width();
    int y;
    if (mType == cor::EWidgetType::full) {
        y = iconRegion().height();
    } else {
        y = height() - iconRegion().height();
    }

    if (mIsReachable) {
        QRect rect;
        if (mType == cor::EWidgetType::full) {
            rect = QRect(x, y, width() / 2, int(height() * 0.6f / 2));
        } else {
            rect = QRect(x, y, side, side);
        }

        if (!mState.isOn()) {
            painter.setOpacity(0.25);
        }

        if (mType == cor::EWidgetType::full) {
            // draw the back rectangle
            QBrush blackBrush(QColor(0, 0, 0));
            if (!mState.isOn()) {
                // if its not on, hide it
                blackBrush.setColor(QColor(0, 0, 0, 5));
            }
            painter.setBrush(blackBrush);
            painter.drawRect(rect);

            // set brightness width
            double brightness = mState.color().valueF();
            if (mState.routine() > cor::ERoutineSingleColorEnd) {
                brightness = mState.palette().brightness() / 100.0;
            }
            rect.setWidth(int(rect.width() * brightness));
        } else {
            rect.setWidth(int(rect.width()));
        }

        // draw the pixmap stretched to teh width provided
        if (mState.isOn()) {
            if (mIconPixmap.size() != rect.size()) {
                mIconPixmap = mIconPixmap.scaled(rect.width(),
                                                 rect.height(),
                                                 Qt::IgnoreAspectRatio,
                                                 Qt::FastTransformation);
            }
            QBrush iconBrush(mIconPixmap);
            painter.setBrush(iconBrush);
            painter.drawRect(rect);
        } else {
            QBrush blackBrush(QColor(0, 0, 0));
            painter.setBrush(blackBrush);
            painter.drawRect(rect);
        }
    } else {
        painter.setOpacity(0.5);
        // draw a quesiton mark in this region
        auto xPos = std::max(x, int(this->width() * 0.18f - x + side));
        QRect rect(xPos, y, side, side);
        painter.drawPixmap(rect, mNoConnectionPixmap);
    }
}

void ListLightWidget::mouseReleaseEvent(QMouseEvent* event) {
    if ((mType == cor::EWidgetType::condensed) && cor::leftHandMenuMoving()) {
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
    if (mType == cor::EWidgetType::full) {
        if (nameText.size() > 20) {
            nameText = nameText.mid(0, 17) + "...";
        }
    } else {
        if (nameText.size() > 17) {
            nameText = nameText.mid(0, 14) + "...";
        }
    }
    return nameText;
}


void ListLightWidget::resizeIcons() {
    mTypePixmap = lightHardwareTypeToPixmap(mHardwareType);
    auto min = std::min(iconRegion().width(), iconRegion().height());
    mTypePixmap = mTypePixmap.scaled(min, min, Qt::IgnoreAspectRatio, Qt::SmoothTransformation);
    mTypeIcon->setPixmap(mTypePixmap);
    mNoConnectionPixmap = QPixmap(":/images/questionMark.png");
    mNoConnectionPixmap =
        mNoConnectionPixmap.scaled(min, min, Qt::KeepAspectRatio, Qt::SmoothTransformation);

    auto topSpacer = (height() - iconRegion().height()) / 2;
    int spaceBesidesName;
    if (mType == cor::EWidgetType::full) {
        spaceBesidesName = spacer() * 2 + iconRegion().width();
    } else {
        spaceBesidesName = (spacer() + iconRegion().width()) * 2;
    }
    mTypeIcon->setGeometry(spacer(), topSpacer, iconRegion().width(), iconRegion().height());
    mController->setGeometry(spaceBesidesName,
                             topSpacer,
                             width() - spaceBesidesName,
                             iconRegion().height());
}


void ListLightWidget::resizeEvent(QResizeEvent*) {
    resizeIcons();
}

int ListLightWidget::spacer() {
    return this->geometry().width() / 20;
}

QSize ListLightWidget::iconRegion() {
    int width = this->geometry().width() / 6 - spacer();
    if (mType == cor::EWidgetType::full) {
        return QSize(width, int(height() / 3 * 2));
    } else {
        return QSize(width, int(height() * 0.8));
    }
}
