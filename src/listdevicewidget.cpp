/*!
 * \copyright
 * Copyright (C) 2015 - 2019.
 * Released under the GNU General Public License.
 */

#include <QGraphicsEffect>
#include <QPainter>
#include <QStyleOption>
#include <QGraphicsScene>

#include "listdevicewidget.h"

#include <algorithm>

ListDeviceWidget::ListDeviceWidget(const cor::Light& device,
                                   bool setHighlightable,
                                   QSize size,
                                   QWidget *parent) : cor::ListItemWidget(device.uniqueID(),
                                                                          parent)   {

    this->setFixedSize(size);

    mShouldHighlight = setHighlightable;
    init(device);

    mBlockStateUpdates = false;
    mHideSwitch = false;
    mIsChecked = false;

    mCooldownTimer = new QTimer(this);
    mCooldownTimer->setSingleShot(true);
    connect(mCooldownTimer, SIGNAL(timeout()), this, SLOT(coolDownClick()));

    updateWidget(device);
}


void ListDeviceWidget::init(const cor::Light& device) {

    // setup icon
    mIconData = IconData(4, 4);

    // setup controller label
    mController = new QLabel(this);

    mTypeIcon = new QLabel(this);
    mTypeIcon->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Expanding);

    mOnOffSwitch = new cor::Switch(this);
    mOnOffSwitch->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Expanding);
    connect(mOnOffSwitch, SIGNAL(switchChanged(bool)), this, SLOT(changedSwitchState(bool)));

    updateTypeIcon(device.hardwareType);

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

    mOnOffSwitch->setSwitchState(ESwitchState::disabled);
}


void ListDeviceWidget::updateWidget(const cor::Light& device) {
    bool shouldRender = false;
    if (!(mDevice == device)) {
        shouldRender = true;
    }

    updateTypeIcon(device.hardwareType);

    mDevice = device;
    mKey = structToIdentifierString(device);

    QString nameText = createName(device);
    if (nameText.compare(mController->text()) != 0) {
        mController->setText(nameText);
    }

    if (!device.isReachable) {
        mOnOffSwitch->setSwitchState(ESwitchState::disabled);
    } else if (device.isOn && !mBlockStateUpdates) {
        mOnOffSwitch->setSwitchState(ESwitchState::on);
    } else if (!mBlockStateUpdates) {
        mOnOffSwitch->setSwitchState(ESwitchState::off);
    }

    if (shouldRender) {
        QJsonObject object = cor::lightToJson(device);
        mIconData.setRoutine(object);
        mIconPixmap = mIconData.renderAsQPixmap();
        repaint();
    }
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
    return commTypeToString(device.commType()) + "_" + device.uniqueID() + "_" + device.controller();
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
               this->width() / 2,
               int(this->height() * 0.6f / 2));


    // make brush with icon data in it
    QBrush brush(QColor(0,0,0));

    mIconPixmap = mIconPixmap.scaled(rect.width(),
                                     rect.height(),
                                     Qt::IgnoreAspectRatio,
                                     Qt::SmoothTransformation);
    QBrush brush2(mIconPixmap);

    painter.setBrush(brush);
    painter.drawRect(rect);

    // set brightness width
    double brightness = mDevice.color.valueF();
    if (mDevice.routine > cor::ERoutineSingleColorEnd) {
        brightness = mDevice.palette.brightness() / 100.0;
    }
    rect.setWidth(int(rect.width() * brightness));
    painter.setBrush(brush2);
    painter.drawRect(rect);
    if (!mDevice.isOn) {
        QBrush brush3(QColor(0,0,0,200));
        painter.setBrush(brush3);
        painter.drawRect(rect);
    }
}

void ListDeviceWidget::mouseReleaseEvent(QMouseEvent *) {
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
    if (device.protocol() == EProtocolType::arduCor) {
        nameText = device.name;
    } else if (device.protocol() == EProtocolType::hue) {
        nameText = convertUglyHueNameToPrettyName(device.name);
    } else {
        nameText = device.controller();
    }
    if (nameText.size() > 20) {
        nameText = nameText.mid(0, 17) + "...";
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
        case ELightHardwareType::singleLED:
            typeResource = QString(":/images/led_icon.png");
            break;
        case ELightHardwareType::hueBulb:
            typeResource = QString(":/images/hue_bulb.png");
            break;
        case ELightHardwareType::cube:
            typeResource = QString(":/images/cube_icon.png");
            break;
        case ELightHardwareType::hueGo:
            typeResource = QString(":/images/hue_go.png");
            break;
        case ELightHardwareType::hueBulbRound:
            typeResource = QString(":/images/hue_bulb_round.png");
            break;
        case ELightHardwareType::hueIris:
            typeResource = QString(":/images/hue_iris.png");
            break;
        case ELightHardwareType::hueSpot:
            typeResource = QString(":/images/hue_spot.png");
            break;
        case ELightHardwareType::hueAura:
            typeResource = QString(":/images/hue_aura.png");
            break;
        case ELightHardwareType::hueCandle:
            typeResource = QString(":/images/hue_candle.png");
            break;
        case ELightHardwareType::hueDownlight:
            typeResource = QString(":/images/hue_downlight.png");
            break;
        case ELightHardwareType::hueLamp:
            typeResource = QString(":/images/hue_lamp.png");
            break;
        case ELightHardwareType::hueStorylight:
            typeResource = QString(":/images/hue_storylight.png");
            break;
        case ELightHardwareType::rectangle:
            typeResource = QString(":/images/array_icon.jpg");
            break;
        case ELightHardwareType::lightStrip:
            typeResource = QString(":/images/light_strip.png");
            break;
        case ELightHardwareType::ring:
            typeResource = QString(":/images/ring_icon.png");
            break;
        case ELightHardwareType::bloom:
            typeResource = QString(":/images/hue_bloom.png");
            break;
        case ELightHardwareType::nanoleaf:
            typeResource = QString(":/images/nanoleaf_icon.png");
            break;
        case ELightHardwareType::MAX:
            typeResource = QString(":/images/led_icon.png");
            break;
    }
    QSize size(int(this->height() * 0.5f),
               int(this->height() * 0.5f));

    mTypePixmap = QPixmap(typeResource);
    mTypeIcon->setFixedSize(size);
    mTypePixmap = mTypePixmap.scaled(size.width(),
                                     size.height(),
                                     Qt::IgnoreAspectRatio,
                                     Qt::SmoothTransformation);

    QSize onOffSize(size.width() *  2, size.height() * 2);
    mOnOffSwitch->setFixedSize(size);
    mTypeIcon->setPixmap(mTypePixmap);
}
