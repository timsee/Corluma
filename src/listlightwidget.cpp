/*!
 * \copyright
 * Copyright (C) 2015 - 2019.
 * Released under the GNU General Public License.
 */

#include "listlightwidget.h"

#include <QGraphicsEffect>
#include <QGraphicsScene>
#include <QMouseEvent>
#include <QPainter>
#include <QStyleOption>
#include <algorithm>

#include "utils/qt.h"


ListLightWidget::ListLightWidget(const cor::Light& device,
                                 bool setHighlightable,
                                 cor::EWidgetType type,
                                 EOnOffSwitchState switchState,
                                 QWidget* parent)
    : cor::ListItemWidget(device.uniqueID(), parent),
      mNoConnectionPixmap(":/images/questionMark.png"),
      mType{type},
      mSwitchState{switchState},
      mFontPtSize(16) {
    mShouldHighlight = setHighlightable;
    init(device);
    mBlockStateUpdates = false;
    mHideSwitch = false;
    mIsChecked = false;

    mCooldownTimer = new QTimer(this);
    mCooldownTimer->setSingleShot(true);
    connect(mCooldownTimer, SIGNAL(timeout()), this, SLOT(coolDownClick()));

    updateWidget(device);
    handleSwitch();
}


void ListLightWidget::init(const cor::Light& device) {
    // setup icon
    mIconData = IconData(4, 4);

    // setup controller label
    mController = new QLabel(this);

    mTypeIcon = new QLabel(this);
    mTypeIcon->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);

    mOnOffSwitch = new cor::Switch(this);
    mOnOffSwitch->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Expanding);
    connect(mOnOffSwitch, SIGNAL(switchChanged(bool)), this, SLOT(changedSwitchState(bool)));

    mTypePixmap = lightHardwareTypeToPixmap(device.hardwareType());

    QString nameText = createName(device);
    mController->setText(nameText);
    mController->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Expanding);
    mController->setAlignment(Qt::AlignVCenter);

    // setup layout
    mLayout = new QGridLayout(this);
    if (mType == cor::EWidgetType::full) {
        mLayout->addWidget(mTypeIcon, 0, 0, 2, 2);
        mLayout->addWidget(mOnOffSwitch, 0, 2, 2, 4);
        mLayout->addWidget(mController, 2, 0, 1, 18);
        mLayout->setContentsMargins(0, 0, 0, 0);
    } else {
        mLayout->addWidget(mTypeIcon, 0, 0, 2, 2);
        mLayout->addWidget(mOnOffSwitch, 0, 2, 2, 3);
        mLayout->addWidget(mController, 0, 5, 1, 14);
        mLayout->setContentsMargins(10, 0, 0, 0);
    }
    mLayout->setSpacing(2);

    setLayout(mLayout);

    mKey = device.uniqueID();

    mOnOffSwitch->setSwitchState(ESwitchState::disabled);
}


void ListLightWidget::updateWidget(const cor::Light& light) {
    bool shouldRender = false;
    if (!(mDevice == light) && isVisible()) {
        shouldRender = true;
    }
    if (!mHasRendered) {
        shouldRender = true;
        mHasRendered = false;
    }

    if (mDevice.hardwareType() != light.hardwareType() || mLastRenderedSize != size()) {
        mTypePixmap = lightHardwareTypeToPixmap(light.hardwareType());
        resizeIcons();
        mLastRenderedSize = size();
    }

    if (mDevice.name() != light.name()) {
        mController->setText(createName(light));
    }

    mDevice = light;
    mKey = light.uniqueID();

    handleSwitch();

    if (shouldRender) {
        mIconData.setRoutine(light.stateConst());
        mIconPixmap = mIconData.renderAsQPixmap();
        update();
    }
}

void ListLightWidget::handleSwitch() {
    if (mSwitchState == EOnOffSwitchState::locked) {
        mOnOffSwitch->setSwitchState(ESwitchState::disabled);
    } else if (mSwitchState == EOnOffSwitchState::hidden) {
        mOnOffSwitch->setVisible(false);
    } else {
        auto state = mDevice.state();
        if (!mDevice.isReachable()) {
            mOnOffSwitch->setSwitchState(ESwitchState::disabled);
        } else if (state.isOn() && !mBlockStateUpdates) {
            mOnOffSwitch->setSwitchState(ESwitchState::on);
        } else if (!mBlockStateUpdates) {
            mOnOffSwitch->setSwitchState(ESwitchState::off);
        }
    }
}

QString ListLightWidget::convertUglyHueNameToPrettyName(QString name) {
    if (name.contains("color lamp")) {
        name.replace("color lamp", "Color Lamp");
    } else if (name.contains("lightstrip plus")) {
        name.replace("lightstrip plus", "Lightstrip Plus");
    } else if (name.contains("ambiance lamp")) {
        name.replace("ambiance lamp", "Ambiance Lamp");
    } else if (name.contains("bloom")) {
        name.replace("bloom", "Bloom");
    } else if (name.contains("white lamp")) {
        name.replace("white lamp", "White Lamp");
    }
    QString hueString = QString("Hue ");
    name.replace(name.indexOf(hueString), hueString.size(), QString(""));
    return name;
}

bool ListLightWidget::setHighlightChecked(bool checked) {
    mIsChecked = checked;
    if (mShouldHighlight) {
        if (mOnOffSwitch->isVisible()) {
            if (mIsChecked) {
                auto effect = new QGraphicsOpacityEffect(mOnOffSwitch);
                effect->setOpacity(1.0);
                mOnOffSwitch->setGraphicsEffect(effect);
                mOnOffSwitch->setEnabled(true);
                mOnOffSwitch->setAttribute(Qt::WA_TransparentForMouseEvents, false);
            } else {
                auto effect = new QGraphicsOpacityEffect(mOnOffSwitch);
                effect->setOpacity(0.15);
                mOnOffSwitch->setGraphicsEffect(effect);
                mOnOffSwitch->setEnabled(false);
                mOnOffSwitch->setAttribute(Qt::WA_TransparentForMouseEvents, true);
            }
        }
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

    int x = mTypeIcon->width() + 5 + mTypeIcon->pos().x();
    if (mOnOffSwitch->isVisible()) {
        x += mOnOffSwitch->width() + 5;
    }
    auto side = int(height() * 0.45f);
    auto y = int(height() * 0.3f);
    auto state = mDevice.state();
    if (mDevice.isReachable()) {
        QRect rect;
        if (mType == cor::EWidgetType::full) {
            rect = QRect(x, 10, width() / 2, int(height() * 0.6f / 2));
        } else {
            auto xPos = std::max(x, int(this->width() * 0.18f - x + side));
            rect = QRect(xPos, y, side, side);
        }

        if (mIconPixmap.size() != rect.size()) {
            mIconPixmap = mIconPixmap.scaled(rect.width(),
                                             rect.height(),
                                             Qt::IgnoreAspectRatio,
                                             Qt::FastTransformation);
        }

        if (!state.isOn()) {
            painter.setOpacity(0.25);
        }

        if (mType == cor::EWidgetType::full) {
            // draw the back rectangle
            QBrush blackBrush(QColor(0, 0, 0));
            if (!state.isOn()) {
                // if its not on, hide it
                blackBrush.setColor(QColor(0, 0, 0, 5));
            }
            painter.setBrush(blackBrush);
            painter.drawRect(rect);

            // set brightness width
            double brightness = state.color().valueF();
            if (state.routine() > cor::ERoutineSingleColorEnd) {
                brightness = state.palette().brightness() / 100.0;
            }
            rect.setWidth(int(rect.width() * brightness));
        } else {
            rect.setWidth(int(rect.width()));
        }

        // draw the pixmap stretched to teh width provided
        QBrush iconBrush(mIconPixmap);
        painter.setBrush(iconBrush);
        painter.drawRect(rect);
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


void ListLightWidget::changedSwitchState(bool newState) {
    mBlockStateUpdates = true;
    mCooldownTimer->start(4000);
    emit switchToggled(mKey, newState);
}

void ListLightWidget::coolDownClick() {
    mBlockStateUpdates = false;
}

QString ListLightWidget::createName(const cor::Light& device) {
    QString nameText;
    if (device.protocol() == EProtocolType::arduCor
        || device.protocol() == EProtocolType::nanoleaf) {
        nameText = device.name();
    } else if (device.protocol() == EProtocolType::hue) {
        nameText = convertUglyHueNameToPrettyName(device.name());
    } else {
        nameText = device.name();
    }

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

void ListLightWidget::hideOnOffSwitch(bool shouldHide) {
    mHideSwitch = shouldHide;
    if (shouldHide) {
        mOnOffSwitch->setHidden(mHideSwitch);
    }
}

void ListLightWidget::resizeIcons() {
    QSize size(int(height() * 0.5f), int(height() * 0.5f));
    mTypeIcon->setFixedSize(size);
    mTypePixmap = lightHardwareTypeToPixmap(mDevice.hardwareType());
    mTypePixmap = mTypePixmap.scaled(size.width(),
                                     size.height(),
                                     Qt::IgnoreAspectRatio,
                                     Qt::SmoothTransformation);
    mTypeIcon->setPixmap(mTypePixmap);
    QSize size2(int(height() * 0.45f), int(height() * 0.45f));
    mNoConnectionPixmap = QPixmap(":/images/questionMark.png");
    mNoConnectionPixmap = mNoConnectionPixmap.scaled(size2.width(),
                                                     size2.height(),
                                                     Qt::KeepAspectRatio,
                                                     Qt::SmoothTransformation);

    if (mType == cor::EWidgetType::full) {
        mController->setFixedWidth(width());
    } else {
        int offset = size.height() * 3;
        if (mOnOffSwitch->isVisible()) {
            offset += mOnOffSwitch->width();
        }
        mController->setFixedWidth(width() - offset);
    }

    if (mOnOffSwitch->isVisible()) {
        QSize onOffSize(size.width() * 2, size.height() * 2);
        mOnOffSwitch->setFixedSize(size);
    }
}


void ListLightWidget::resizeEvent(QResizeEvent*) {
    resizeIcons();
}
