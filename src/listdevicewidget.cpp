/*!
 * \copyright
 * Copyright (C) 2015 - 2017.
 * Released under the GNU General Public License.
 */

#include <QGraphicsEffect>
#include <QPainter>
#include <QStyleOption>
#include "listdevicewidget.h"

ListDeviceWidget::ListDeviceWidget(const SLightDevice& device,
                                           const QString& name,
                                           const std::vector<QColor>& colors,
                                           QWidget *parent) : QWidget(parent) {
    init(device, name);
    updateWidget(device, colors);
}


void ListDeviceWidget::init(const SLightDevice& device, const QString& name) {
    mIsChecked = false;

    // setup icon
    mIconData = IconData(256, 256);
    mStatusIcon = new QLabel(this);
    mStatusIcon->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

#ifndef MOBILE_BUILD
    mStatusIcon->setMaximumWidth(mStatusIcon->height());
#endif

    QString type;
    if (device.type == ECommType::eHue) {
        type = "Hue";
    } else if (device.type == ECommType::eHTTP
               || device.type == ECommType::eUDP) {
        type = "Yun";
    }
#ifndef MOBILE_BUILD
    else if (device.type == ECommType::eSerial) {
        type = "Serial";
    }
#endif

    // setup controller label
    mController = new QLabel(this);
    QString nameText = "  ";
    if (device.type == ECommType::eHTTP
                  || device.type == ECommType::eUDP) {
           nameText += type;
           nameText += " ";
           nameText += name;
           nameText += "_";
           nameText += QString::number(device.index);
    } else if (device.type == ECommType::eHue) {
        nameText = convertUglyHueNameToPrettyName(name);
    } else {
        nameText = type + " " + name;
    }
    mController->setText(nameText);
    mController->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

    // setup layout
    mLayout = new QHBoxLayout(this);
    mLayout->addWidget(mStatusIcon);
    mLayout->addWidget(mController);

    mLayout->setContentsMargins(0,0,0,0);

    mLayout->setSpacing(0);

    setLayout(mLayout);

    mLayout->setStretch(0, 1);
    mLayout->setStretch(1, 10);

    mKey = structToIdentifierString(device);

    mDevice = device;

    QString stylesheet = createStyleSheet(mDevice);
    mController->setStyleSheet(stylesheet);
    mStatusIcon->setStyleSheet(stylesheet);
}


void  ListDeviceWidget::updateWidget(const SLightDevice& device,
                                     const std::vector<QColor>& colors) {
    mDevice = device;

    if (device.lightingRoutine <= utils::ELightingRoutineSingleColorEnd ) {
        mIconData.setSingleLightingRoutine(device.lightingRoutine, device.color);
        QPixmap iconRendered = mIconData.renderAsQPixmap();
        mStatusIcon->setPixmap(iconRendered.scaled(mStatusIcon->width(),
                                                    mStatusIcon->width(),
                                                    Qt::IgnoreAspectRatio,
                                                    Qt::FastTransformation));
    } else {
        mIconData.setMultiLightingRoutine(device.lightingRoutine, device.colorGroup, colors);
        QPixmap iconRendered = mIconData.renderAsQPixmap();
        mStatusIcon->setPixmap(iconRendered.scaled(mStatusIcon->width(),
                                                    mStatusIcon->width(),
                                                    Qt::IgnoreAspectRatio,
                                                    Qt::FastTransformation));
    }

    if (!device.isReachable) {
        QGraphicsOpacityEffect *effect = new QGraphicsOpacityEffect(mStatusIcon);
        effect->setOpacity(0.25f);
        mStatusIcon->setGraphicsEffect(effect);
    } else if (!device.isOn) {
        QGraphicsOpacityEffect *effect = new QGraphicsOpacityEffect(mStatusIcon);
        effect->setOpacity(0.75f);
        mStatusIcon->setGraphicsEffect(effect);
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
    name.prepend("  ");
    return name;
}

QString ListDeviceWidget::structToIdentifierString(const SLightDevice& device) {
    QString returnString = "";
    if (device.type == ECommType::eHTTP
            || device.type == ECommType::eUDP) {
        returnString = "Yun" + utils::ECommTypeToString(device.type);
    } else {
        returnString = utils::ECommTypeToString(device.type);
    }
    QString onString;
    if(device.isOn && device.isReachable) {
        onString = "AA";
    } else if (device.isReachable){
        onString = "BB";
    } else {
        onString = "CC";
    }
    returnString = onString + "," + returnString + "," + device.name + "," + QString::number(device.index);
    return returnString;
}

bool ListDeviceWidget::setChecked(bool checked) {
    mIsChecked = checked;
    QString stylesheet = createStyleSheet(mDevice);
    mController->setStyleSheet(stylesheet);
    mStatusIcon->setStyleSheet(stylesheet);
    repaint();
    return mIsChecked;
}

void ListDeviceWidget::paintEvent(QPaintEvent *event) {
    Q_UNUSED(event);
    QStyleOption opt;
    opt.init(this);
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);
    if (mIsChecked) {
        painter.fillRect(this->rect(), QBrush(QColor(61, 142, 201, 255)));
    } else {
        painter.fillRect(this->rect(), QBrush(QColor(32, 31, 31, 255)));
    }
}


QString ListDeviceWidget::createStyleSheet(const SLightDevice& device) {
    QString styleSheet;
    QString backgroundStyleSheet = "";
    QString offStyleSheet = "color: #555;";
    QString unReachableStyleSheet = " color: #333;";
    QString checkedStyleSheet = "background-color: #3d8ec9;";

    if (mIsChecked) {
        styleSheet = checkedStyleSheet;
    } else if(!device.isReachable) {
       styleSheet = unReachableStyleSheet;
    } else if (!device.isOn) {
        styleSheet = offStyleSheet;
    } else {
        styleSheet = backgroundStyleSheet;
    }
#ifdef MOBILE_BUILD
    styleSheet += "font: 14pt;";
#endif
    return styleSheet;
}

void ListDeviceWidget::mouseReleaseEvent(QMouseEvent *event) {
    Q_UNUSED(event);
    setChecked(!checked());
    emit clicked(mKey);
}


