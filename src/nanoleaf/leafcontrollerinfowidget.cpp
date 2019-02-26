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

#include "nanoleaf/leafcontrollerinfowidget.h"
#include "utils/qt.h"

namespace nano
{

LeafControllerInfoWidget::LeafControllerInfoWidget(nano::LeafController controller, QWidget *parent) : QWidget(parent) {
    const QString styleSheet = "background-color: rgba(0,0,0,0);";
    this->setStyleSheet(styleSheet);

    mController = controller;

    mName = new EditableFieldWidget(controller.name, this, 28, "A controller's name must be at most 28 characters long.");
    mName->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    mName->setFontPointSize(14);
    connect(mName, SIGNAL(updatedField(QString)), this, SLOT(nameChanged(QString)));

    mIPAdress = new QLabel("<i> " + controller.IP + "</i>", this);
    mIPAdress->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    mIPAdress->setAttribute(Qt::WA_TransparentForMouseEvents, true);

    mFirmware = new QLabel("<b>Firmware</b> " + controller.firmware, this);
    mFirmware->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    mFirmware->setAttribute(Qt::WA_TransparentForMouseEvents, true);

    mSerialNumber  = new QLabel("<b>Serial:</b> " + controller.serialNumber, this);
    mSerialNumber->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    mSerialNumber->setAttribute(Qt::WA_TransparentForMouseEvents, true);

    mModelID  = new QLabel("<b>Model:</b>  " + controller.model, this);
    mModelID->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    mModelID->setAttribute(Qt::WA_TransparentForMouseEvents, true);

    mLayout = new QGridLayout(this);

    mLayout->addWidget(mName, 0, 0, 1, 2);
    mLayout->addWidget(mIPAdress, 1, 0);
    mLayout->addWidget(mFirmware, 2, 0);
    mLayout->addWidget(mSerialNumber, 3, 0);
    mLayout->addWidget(mModelID, 4, 0);

    mKey = controller.serialNumber;

    mIsChecked = false;
    hideDetails(true);
}

void LeafControllerInfoWidget::updateController(nano::LeafController controller) {

    // many fields such as the mac address and the type of light won't update, only check the fields that do
    if (controller.name.compare(mController.name) != 0) {
        mName->setText(controller.name);
    }

    if (controller.serialNumber.compare(mController.serialNumber) != 0) {
        mSerialNumber->setText("<b>Serial:</b>  " + controller.serialNumber);
    }

    mController = controller;
}

void LeafControllerInfoWidget::mouseReleaseEvent(QMouseEvent *event) {
    Q_UNUSED(event);
    emit clicked(mKey);
}

void LeafControllerInfoWidget::setChecked(bool checked) {
    mIsChecked = checked;
    repaint();
}

void LeafControllerInfoWidget::setHeight(int height) {
    if (mHideDetails) {
        height = height / 2;
    }
    int finalHeight = std::max(height, this->height());
    mHeight = finalHeight;
    this->setFixedHeight(height);
}

void LeafControllerInfoWidget::hideDetails(bool shouldHide) {
    mModelID->setHidden(shouldHide);
    mSerialNumber->setHidden(shouldHide);
    mFirmware->setHidden(shouldHide);
    mHideDetails = shouldHide;
}

void LeafControllerInfoWidget::paintEvent(QPaintEvent *event) {
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

