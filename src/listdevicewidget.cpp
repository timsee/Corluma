/*!
 * \copyright
 * Copyright (C) 2015 - 2017.
 * Released under the GNU General Public License.
 */

#include <QGraphicsEffect>
#include <QPainter>
#include <QStyleOption>
#include <QGraphicsScene>

#include "listdevicewidget.h"

ListDeviceWidget::ListDeviceWidget(const SLightDevice& device,
                                           const QString& name,
                                           const std::vector<QColor>& colors,
                                           bool setHighlightable,
                                           QSize size,
                                           QWidget *parent)    {
    mShouldHighlight = setHighlightable;
    this->setParent(parent);
    init(device, name);
    this->setMaximumSize(size);
    updateWidget(device, colors);
}


void ListDeviceWidget::init(const SLightDevice& device, const QString& name) {
    mIsChecked = false;

    // setup icon
    mIconData = IconData(32, 32);
    mDeviceIcon = new QLabel(this);
    mDeviceIcon->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Expanding);
    mDeviceIcon->setAlignment(Qt::AlignHCenter | Qt::AlignVCenter);
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
    QString nameText;
    if (device.type == ECommType::eHTTP
                  || device.type == ECommType::eUDP) {
           nameText += name;
           nameText += "_";
           nameText += QString::number(device.index);
    } else if (device.type == ECommType::eHue) {
        nameText = convertUglyHueNameToPrettyName(name);
    } else {
        nameText = name;
    }
    mController->setText(nameText);
    mController->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

    mStatusIcon = new CorlumaStatusIcon(this);
    mStatusIcon->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Expanding);

    mTypeIcon = new QLabel(this);
    mTypeIcon->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Expanding);

    // setup layout
    mLayout = new QGridLayout(this);
    mLayout->addWidget(mDeviceIcon, 0, 0, 2, 2);
    mLayout->addWidget(mStatusIcon, 0, 2, 1, 1);
    mLayout->addWidget(mTypeIcon,   1, 2, 1, 1);
    mLayout->addWidget(mController, 1, 3, 1, 7);

    mLayout->setContentsMargins(2,2,2,2);
    mLayout->setSpacing(0);

    setLayout(mLayout);

    mKey = structToIdentifierString(device);

    mController->setStyleSheet(createStyleSheet(device));
    mDeviceIcon->setStyleSheet(createStyleSheet(device));
    mTypeIcon->setStyleSheet(createStyleSheet(device));

    mStatusIcon->update(device.isReachable, device.isOn, device.brightness);
    prepareTypeLabel(device.type);
    resizeIconPixmap();
}


void  ListDeviceWidget::updateWidget(const SLightDevice& device,
                                     const std::vector<QColor>& colors) {
    mDevice = device;

    int widgetSize = std::min(this->width(), this->height());
    if (device.lightingRoutine <= utils::ELightingRoutineSingleColorEnd ) {
        mIconData.setSingleLightingRoutine(device.lightingRoutine, device.color);
        mDeviceIcon->setFixedSize(widgetSize, widgetSize);
    } else {
        mIconData.setMultiLightingRoutine(device.lightingRoutine, device.colorGroup, colors);
    }
    mDeviceIcon->setFixedSize(widgetSize * 0.75f, widgetSize);
    mIconPixmap = mIconData.renderAsQPixmap();
    resizeIconPixmap();

    if (!device.isReachable) {
        mController->setStyleSheet(createStyleSheet(mDevice));
        mDeviceIcon->setStyleSheet(createStyleSheet(mDevice));
    } else if (!device.isOn) {
        QLinearGradient alphaGradient(mDeviceIcon->rect().topRight(), mDeviceIcon->rect().bottomLeft());
        alphaGradient.setColorAt(1.0, Qt::gray);
        alphaGradient.setColorAt(0.7, Qt::gray);
        alphaGradient.setColorAt(0.5, Qt::transparent);
        alphaGradient.setColorAt(0.3, Qt::gray);
        alphaGradient.setColorAt(0.1, Qt::gray);
        QLinearGradient alphaGradient2(mDeviceIcon->rect().topLeft(), mDeviceIcon->rect().bottomRight());
        alphaGradient2.setColorAt(1.0, Qt::gray);
        alphaGradient2.setColorAt(0.5, Qt::transparent);
        alphaGradient2.setColorAt(0.0, Qt::gray);
        QGraphicsOpacityEffect *effect = new QGraphicsOpacityEffect(mDeviceIcon);
        effect->setOpacityMask(alphaGradient);
       // effect->setOpacityMask(alphaGradient2);

        mDeviceIcon->setGraphicsEffect(effect);

//        QGraphicsOpacityEffect *effect2 = new QGraphicsOpacityEffect(mDeviceIcon);
//        effect2->setOpacityMask(alphaGradient2);
//        mDeviceIcon->setGraphicsEffect(effect2);
    } else {
        QGraphicsOpacityEffect *effect = new QGraphicsOpacityEffect(mDeviceIcon);
        mDeviceIcon->setGraphicsEffect(effect);
    }

    mStatusIcon->update(device.isReachable, device.isOn, device.brightness);
    prepareTypeLabel(device.type);
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

QString ListDeviceWidget::structToIdentifierString(const SLightDevice& device) {
    QString returnString = "";
    if (device.type == ECommType::eHTTP
            || device.type == ECommType::eUDP) {
        returnString = "Yun" + utils::ECommTypeToString(device.type);
    } else {
        returnString = utils::ECommTypeToString(device.type);
    }
    QString onString;
    if(device.isReachable) {
        onString = "AA";
    } else {
        onString = "CC";
    }
    returnString = onString + "," + returnString + "," + device.name + "," + QString::number(device.index);
    return returnString;
}

bool ListDeviceWidget::setHighlightChecked(bool checked) {
    mIsChecked = checked;
    repaint();
    return mIsChecked;
}


void ListDeviceWidget::paintEvent(QPaintEvent *event) {
    Q_UNUSED(event);
    QStyleOption opt;
    opt.init(this);
    QPainter painter(this);

    painter.setRenderHint(QPainter::Antialiasing);
    if (mShouldHighlight) {
        if (mIsChecked) {
            painter.fillRect(this->rect(), QBrush(QColor(61, 142, 201, 255)));
        } else {
            //TODO: could I make this transparent in all cases?
            painter.fillRect(this->rect(), QBrush(QColor(32, 31, 31, 255)));
        }
    }

    QRect rect(mDeviceIcon->geometry().x(),
               mDeviceIcon->geometry().y() + mDeviceIcon->height() / 4,
               mDeviceIcon->height() * 0.666f,
               mDeviceIcon->height() * 0.666f);

    // make brush with icon data in it
    QBrush brush(mIconPixmap);
    painter.setBrush(brush);

    painter.drawEllipse(rect);
}


void ListDeviceWidget::resizeIconPixmap() {
    QRect rect(mDeviceIcon->geometry().x(),
               mDeviceIcon->geometry().y() + mDeviceIcon->height() / 4,
               mDeviceIcon->height() * 0.666f,
               mDeviceIcon->height() * 0.666f);

    if ((mIconPixmap.size().width() != rect.width())
            || (mIconPixmap.size().height() != rect.height())) {
        mIconPixmap = mIconPixmap.scaled(rect.width(),
                                         rect.height(),
                                         Qt::KeepAspectRatio,
                                         Qt::SmoothTransformation);
    }
}

QString ListDeviceWidget::createStyleSheet(const SLightDevice& device) {
    QString styleSheet;

#ifdef MOBILE_BUILD
    styleSheet += " font: 10pt;";
#endif
    styleSheet += "background-color: rgba(0,0,0,0);";

    return styleSheet;
}

void ListDeviceWidget::mouseReleaseEvent(QMouseEvent *event) {
    Q_UNUSED(event);
    //setChecked(!checked());
    emit clicked(mKey);
}


void ListDeviceWidget::prepareTypeLabel(ECommType type) {
    if (type == ECommType::eHTTP
#ifndef MOBILE_BUILD
            || type == ECommType::eSerial
#endif
            || type == ECommType::eUDP) {
        QPixmap logoIcon(QPixmap(":/images/arduino-logo.png"));
        logoIcon = logoIcon.scaled(mStatusIcon->height() * 0.6f,
                                       mStatusIcon->height() * 0.6f,
                                       Qt::KeepAspectRatio,
                                       Qt::SmoothTransformation);
        mTypeIcon->setPixmap(logoIcon);
    } else if (type == ECommType::eHue) {
        mTypeIcon->setText("Hue");
    }

}
