/*!
 * \copyright
 * Copyright (C) 2015 - 2019.
 * Released under the GNU General Public License.
 */

#include <QtCore>
#include <QtGui>
#include <QStyleOption>
#include <QScroller>
#include <QGraphicsOpacityEffect>
#include <QMessageBox>

#include "hue/hueinfowidget.h"
#include "cor/utils.h"

namespace hue
{

HueInfoWidget::HueInfoWidget(HueLight light, QWidget *parent) : QWidget(parent), mLight(light) {
    const QString styleSheet = "background-color: rgba(0,0,0,0);";
    this->setStyleSheet(styleSheet);

    mName = new EditableFieldWidget(light.name, this, 32, "A hue's name must be at most 32 characters long.");
    mName->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    mName->setFontPointSize(14);
    connect(mName, SIGNAL(updatedField(QString)), this, SLOT(nameChanged(QString)));

    mModelID  = new QLabel("<b>Model:</b>  " + light.modelID, this);
    mModelID->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    mModelID->setAttribute(Qt::WA_TransparentForMouseEvents, true);

    mSoftwareVersion  = new QLabel("<b>Software:</b>  " + light.softwareVersion, this);
    mSoftwareVersion->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    mSoftwareVersion->setAttribute(Qt::WA_TransparentForMouseEvents, true);

    mUniqueID = new QLabel("<b>ID:</b>  " + light.uniqueID(), this);
    mUniqueID->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    mUniqueID->setAttribute(Qt::WA_TransparentForMouseEvents, true);

    mType = new QLabel(cor::hueTypeToString(light.hueType), this);
    mType->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    mType->setAttribute(Qt::WA_TransparentForMouseEvents, true);

    mLayout = new QGridLayout(this);

    mLayout->addWidget(mName, 0, 0, 1, 2);
    mLayout->addWidget(mType, 1, 0);
    mLayout->addWidget(mModelID, 2, 0);
    mLayout->addWidget(mSoftwareVersion, 3, 0);
    mLayout->addWidget(mUniqueID, 4, 0);

    mKey = light.uniqueID();

    mIsChecked = false;
    hideDetails(true);
}

void HueInfoWidget::updateLight(HueLight light) {

    // many fields such as the mac address and the type of light won't update, only check the fields that do
    if (light.name.compare(mLight.name) != 0) {
        mName->setText(light.name);
    }

    if (light.softwareVersion.compare(mLight.softwareVersion) != 0) {
        mSoftwareVersion->setText("<b>Software:</b>  " + light.softwareVersion);
    }

    mLight = light;
}

void HueInfoWidget::mouseReleaseEvent(QMouseEvent *event) {
    Q_UNUSED(event);
    emit clicked(mKey);
}

void HueInfoWidget::setChecked(bool checked) {
    mIsChecked = checked;
    repaint();
}

void HueInfoWidget::setHeight(int height) {
    if (mHideDetails) {
        height = height / 2;
    }
    int finalHeight = std::max(height, this->height());
    mHeight = finalHeight;
    this->setFixedHeight(height);
}

void HueInfoWidget::hideDetails(bool shouldHide) {
    mModelID->setHidden(shouldHide);
    mSoftwareVersion->setHidden(shouldHide);
    mUniqueID->setHidden(shouldHide);
    mHideDetails = shouldHide;
}

void HueInfoWidget::paintEvent(QPaintEvent *event) {
    Q_UNUSED(event);
    QStyleOption opt;
    opt.init(this);
    QPainter painter(this);

    painter.setRenderHint(QPainter::Antialiasing);
    if (mIsChecked) {
        painter.fillRect(this->rect(), QBrush(QColor(61, 142, 201, 255)));
    } else {
        //TODO: could I make this transparent in all cases?
        painter.fillRect(this->rect(), QBrush(QColor(32, 31, 31, 255)));
    }

    // draw line at bottom of widget
    QRect area(this->x(), this->y(), this->width(), this->height());
    QPainter linePainter(this);
    linePainter.setRenderHint(QPainter::Antialiasing);
    linePainter.setBrush(QBrush(QColor(255, 255, 255)));
    QLine spacerLine(QPoint(area.x(), area.height() - 3), QPoint(area.width(), area.height() - 3));
    linePainter.drawLine(spacerLine);
}

}

