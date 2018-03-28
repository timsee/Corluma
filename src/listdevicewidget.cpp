/*!
 * \copyright
 * Copyright (C) 2015 - 2018.
 * Released under the GNU General Public License.
 */

#include <QGraphicsEffect>
#include <QPainter>
#include <QStyleOption>
#include <QGraphicsScene>

#include "listdevicewidget.h"

#include <algorithm>

ListDeviceWidget::ListDeviceWidget(const cor::Light& device,
                                   const std::vector<QColor>& colors,
                                   bool setHighlightable,
                                   QSize size,
                                   QWidget *parent)    {
    mShouldHighlight = setHighlightable;
    this->setParent(parent);
    init(device);
    this->setMaximumSize(size);

    mBlockStateUpdates = false;
    mHideSwitch = false;
    mIsChecked = false;

    mCooldownTimer = new QTimer(this);
    mCooldownTimer->setSingleShot(true);
    connect(mCooldownTimer, SIGNAL(timeout()), this, SLOT(coolDownClick()));

    updateWidget(device, colors);
}


void ListDeviceWidget::init(const cor::Light& device) {

    // setup icon
    mIconData = IconData(32, 32);

    // setup controller label
    mController = new QLabel(this);

    mTypeIcon = new QLabel(this);
    mTypeIcon->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

    updateTypeIcon(device.hardwareType);

    mOnOffSwitch = new cor::Switch(this);
    mOnOffSwitch->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Expanding);
    connect(mOnOffSwitch, SIGNAL(switchChanged(bool)), this, SLOT(changedSwitchState(bool)));

    QString nameText = createName(device);
    mController->setText(nameText);
    mController->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

    // setup layout
    mLayout = new QGridLayout(this);
    mLayout->addWidget(mTypeIcon,    0, 0, 2, 2);
    mLayout->addWidget(mOnOffSwitch, 0, 2, 2, 4);

    mLayout->addWidget(mController,  2, 0, 1, 18);

    mLayout->setContentsMargins(0,0,0,0);
    mLayout->setSpacing(2);

    setLayout(mLayout);

    mKey = structToIdentifierString(device);

    mOnOffSwitch->setSwitchState(ESwitchState::eDisabled);
}


void ListDeviceWidget::updateWidget(const cor::Light& device,
                                     const std::vector<QColor>& colors) {
    mDevice = device;

    if (device.lightingRoutine <= cor::ELightingRoutineSingleColorEnd ) {
        mIconData.setSingleLightingRoutine(device.lightingRoutine, device.color);
    } else {
        mIconData.setMultiLightingRoutine(device.lightingRoutine, device.colorGroup, colors);
    }

    QString nameText = createName(device);
    if (nameText.compare(mController->text()) != 0) {
        mController->setText(nameText);
    }

    mKey = structToIdentifierString(device);
    if (!device.isReachable) {
        mOnOffSwitch->setSwitchState(ESwitchState::eDisabled);
    } else if (device.isOn && !mBlockStateUpdates) {
        mOnOffSwitch->setSwitchState(ESwitchState::eOn);
    } else if (!mBlockStateUpdates) {
        mOnOffSwitch->setSwitchState(ESwitchState::eOff);
    }

    mIconPixmap = mIconData.renderAsQPixmap();
    updateTypeIcon(device.hardwareType);
}

QString ListDeviceWidget::convertUglyHueNameToPrettyName(QString name) {
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
    QString hueString = QString("Hue");
    name.replace(name.indexOf(hueString),
                 hueString.size(), QString(""));
    return name;
}

QString ListDeviceWidget::structToIdentifierString(const cor::Light& device) {
    QString returnString = cor::ECommTypeToString(device.type());
    QString onString;
    if(device.isReachable) {
        onString = "AA";
    } else {
        onString = "CC";
    }
    returnString = onString + "," + returnString + "," + device.controller() + "," + QString::number(device.index());
    return returnString;
}

bool ListDeviceWidget::setHighlightChecked(bool checked) {
    mIsChecked = checked;
    if (mIsChecked) {
        QGraphicsOpacityEffect *effect = new QGraphicsOpacityEffect(mOnOffSwitch);
        effect->setOpacity(1.0);
        mOnOffSwitch->setGraphicsEffect(effect);
        mOnOffSwitch->setEnabled(true);
        mOnOffSwitch->setAttribute(Qt::WA_TransparentForMouseEvents, false);
        this->setStyleSheet("background-color:rgb(61,142,201);");
    } else {
        QGraphicsOpacityEffect *effect = new QGraphicsOpacityEffect(mOnOffSwitch);
        effect->setOpacity(0.15);
        mOnOffSwitch->setGraphicsEffect(effect);
        mOnOffSwitch->setEnabled(false);
        mOnOffSwitch->setAttribute(Qt::WA_TransparentForMouseEvents, true);
        this->setStyleSheet("background-color:rgb(32,31,31);");
    }
    repaint();
    return mIsChecked;
}


void ListDeviceWidget::paintEvent(QPaintEvent *event) {
    Q_UNUSED(event);
    QStyleOption opt;
    opt.init(this);
    QPainter painter(this);

    if (mIsChecked) {
        painter.fillRect(this->rect(), QBrush(QColor(61, 142, 201, 255)));
    } else {
        //TODO: could I make this transparent in all cases?
        painter.fillRect(this->rect(), QBrush(QColor(32, 31, 31, 255)));
    }

    int x;
    if (mHideSwitch) {
        x = mTypeIcon->x() + mTypeIcon->width() + 5;
    } else {
        x = mOnOffSwitch->x() + mOnOffSwitch->width() + 5;
    }

    QRect rect(x,
               10,
               this->width() / 3,
               this->height() * 0.6f / 2);


    // make brush with icon data in it
    QBrush brush(QColor(0,0,0));

    mIconPixmap = mIconPixmap.scaled(rect.width(),
                                     rect.height(),
                                     Qt::IgnoreAspectRatio,
                                     Qt::SmoothTransformation);
    QBrush brush2(mIconPixmap);

    painter.setBrush(brush);
    painter.drawRect(rect);

    if (mDevice.isReachable) {
        // set brightness width
        rect.setWidth((float)rect.width() * mDevice.brightness / 100.0f);
        painter.setBrush(brush2);
        painter.drawRect(rect);
        if (!mDevice.isOn) {
            QBrush brush3(QColor(0,0,0,200));
            painter.setBrush(brush3);
            painter.drawRect(rect);
        }
    }
}

void ListDeviceWidget::mouseReleaseEvent(QMouseEvent *event) {
    Q_UNUSED(event);
    setHighlightChecked(!mIsChecked);
    emit clicked(mKey);
}


void ListDeviceWidget::changedSwitchState(bool newState) {
    mBlockStateUpdates = true;
    mCooldownTimer->start(4000);
    emit switchToggled(mKey, newState);
}

void ListDeviceWidget::coolDownClick() {
    mBlockStateUpdates = false;
}

QString ListDeviceWidget::createName(const cor::Light& device) {
    QString nameText;
    if (device.type() == ECommType::eHTTP
#ifndef MOBILE_BUILD
            || device.type() == ECommType::eSerial
#endif
            || device.type() == ECommType::eUDP) {
        nameText = device.name;
    } else if (device.type() == ECommType::eHue) {
        nameText = convertUglyHueNameToPrettyName(device.name);
    } else {
        nameText = device.controller();
    }
    return nameText;
}

void ListDeviceWidget::hideOnOffSwitch(bool shouldHide) {
    mHideSwitch = shouldHide;
    if (shouldHide) {
        mOnOffSwitch->setHidden(mHideSwitch);
    }
}

void ListDeviceWidget::updateTypeIcon(ELightHardwareType type) {
    QString typeResource;
    switch (type) {
        case ELightHardwareType::eSingleLED:
            typeResource = QString(":/images/led_icon.png");
            break;
        case ELightHardwareType::eLightbulb:
            typeResource = QString(":/images/hue_bulb.png");
            break;
        case ELightHardwareType::eCube:
            typeResource = QString(":/images/cube_icon.png");
            break;
        case ELightHardwareType::e2DArray:
            typeResource = QString(":/images/array_icon.jpg");
            break;
        case ELightHardwareType::eLightStrip:
            typeResource = QString(":/images/light_strip.png");
            break;
        case ELightHardwareType::eRing:
            typeResource = QString(":/images/ring_icon.png");
            break;
        case ELightHardwareType::eBloom:
            typeResource = QString(":/images/hue_bloom.png");
            break;
        case ELightHardwareType::eNanoLeaf:
            typeResource = QString(":/images/nanoleaf_icon.png");
            break;
        case ELightHardwareType::ELightHardwareType_MAX:
        default:
            typeResource = QString(":/images/led_icon.png");
            break;
    }
    QSize size(this->height() * 0.5f,
               this->height() * 0.5f);
    mTypePixmap = QPixmap(typeResource);
    mTypePixmap = mTypePixmap.scaled(size.width(),
                                     size.height(),
                                     Qt::IgnoreAspectRatio,
                                     Qt::SmoothTransformation);
    mTypeIcon->setMaximumSize(size.width(), size.height());
    mTypeIcon->setPixmap(mTypePixmap);
}
