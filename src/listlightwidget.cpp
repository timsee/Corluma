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


namespace {

int findFontSize(QWidget* parent, cor::EWidgetType type) {
    QString text("123456789");
    if (type == cor::EWidgetType::full) {
        text = QString("1234567891234567");
    }
    QLabel label(text, parent);
    // calcuate the text's size
    auto fontWidth = label.fontMetrics().boundingRect(text).width();
    // calculate the button's size
    auto widgetWidth = label.width();
    QFont font(label.font());
    auto fontPtSize = label.font().pointSize();
    if (fontWidth > widgetWidth) {
        for (auto i = fontPtSize - 1; i > 0; --i) {
            font.setPointSize(i);
            label.setFont(font);
            fontPtSize = i;
            fontWidth = label.fontMetrics().boundingRect(text).width();
            if (fontWidth < widgetWidth) {
                // font is small enough to fit
                break;
            }
        }
    }
    return fontPtSize;
}

} // namespace

ListLightWidget::ListLightWidget(const cor::Light& device,
                                 bool setHighlightable,
                                 cor::EWidgetType type,
                                 EOnOffSwitchState switchState,
                                 QWidget* parent)
    : cor::ListItemWidget(device.uniqueID(), parent),
      mType{type},
      mSwitchState{switchState},
      mFontPtSize(findFontSize(parent, type)) {
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

    mTypePixmap = lightHardwareTypeToPixmap(device.hardwareType);

    QString nameText = createName(device);
    mController->setText(nameText);
    auto font = mController->font();
    font.setPointSize(mFontPtSize);
    mController->setFont(font);
    mController->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Expanding);

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


void ListLightWidget::updateWidget(const cor::Light& device) {
    bool shouldRender = false;
    if (!(mDevice == device) && isVisible()) {
        shouldRender = true;
    }
    if (!mHasRendered) {
        shouldRender = true;
        mHasRendered = false;
    }

    if (mDevice.hardwareType != device.hardwareType || mLastRenderedSize != size()) {
        mTypePixmap = lightHardwareTypeToPixmap(device.hardwareType);
        resizeIcons();
        mLastRenderedSize = size();
    }

    if (mDevice.name != device.name) {
        mController->setText(createName(device));
    }

    mDevice = device;
    mKey = device.uniqueID();

    handleSwitch();

    if (shouldRender) {
        QJsonObject object = cor::lightToJson(device);
        mIconData.setRoutine(object);
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
        if (!mDevice.isReachable) {
            mOnOffSwitch->setSwitchState(ESwitchState::disabled);
        } else if (mDevice.isOn && !mBlockStateUpdates) {
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
        // TODO: could I make this transparent in all cases?
        painter.fillRect(rect(), QBrush(QColor(32, 31, 31)));
    }

    if (mDevice.isReachable) {
        QRect rect;
        int x = mTypeIcon->width() + 5 + mTypeIcon->pos().x();
        if (mOnOffSwitch->isVisible()) {
            x += mOnOffSwitch->width() + 5;
        }
        if (mType == cor::EWidgetType::full) {
            rect = QRect(x, 10, width() / 2, int(height() * 0.6f / 2));
        } else {
            rect = QRect(x, int(height() * 0.25f), int(height() * 0.5f), int(height() * 0.5f));
        }

        // make brush with icon data in it
        QBrush brush;
        if (mDevice.isOn) {
            brush = QColor(0, 0, 0);
        } else {
            brush = QColor(0, 0, 0, 5);
        }

        if (mIconPixmap.size() != rect.size()) {
            mIconPixmap = mIconPixmap.scaled(rect.width(),
                                             rect.height(),
                                             Qt::IgnoreAspectRatio,
                                             Qt::FastTransformation);
        }

        QBrush brush2(mIconPixmap);

        if (!mDevice.isOn) {
            painter.setOpacity(0.25);
        }
        painter.setBrush(brush);
        painter.drawRect(rect);

        if (mType == cor::EWidgetType::full) {
            // set brightness width
            double brightness = mDevice.color.valueF();
            if (mDevice.routine > cor::ERoutineSingleColorEnd) {
                brightness = mDevice.palette.brightness() / 100.0;
            }
            rect.setWidth(int(rect.width() * brightness));
        } else {
            rect.setWidth(int(rect.width()));
        }
        painter.setBrush(brush2);
        painter.drawRect(rect);
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
        nameText = device.name;
    } else if (device.protocol() == EProtocolType::hue) {
        nameText = convertUglyHueNameToPrettyName(device.name);
    } else {
        nameText = device.name;
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
    mTypePixmap = lightHardwareTypeToPixmap(mDevice.hardwareType);
    mTypePixmap = mTypePixmap.scaled(size.width(),
                                     size.height(),
                                     Qt::IgnoreAspectRatio,
                                     Qt::SmoothTransformation);
    mTypeIcon->setPixmap(mTypePixmap);

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
