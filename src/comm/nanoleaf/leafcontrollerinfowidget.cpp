/*!
 * \copyright
 * Copyright (C) 2015 - 2019.
 * Released under the GNU General Public License.
 */

#include "comm/nanoleaf/leafcontrollerinfowidget.h"

#include <QGraphicsOpacityEffect>
#include <QMessageBox>
#include <QScroller>
#include <QStyleOption>
#include <QtCore>
#include <QtGui>

#include "utils/qt.h"

namespace nano {

LeafControllerInfoWidget::LeafControllerInfoWidget(nano::LeafController controller, QWidget* parent)
    : QWidget(parent),
      mHideDetails{true},
      mTypeIcon(new QLabel(this)) {
    const QString styleSheet = "background-color: rgba(0,0,0,0);";
    setStyleSheet(styleSheet);

    mController = controller;

    mName = new EditableFieldWidget(controller.name,
                                    this,
                                    28,
                                    "A controller's name must be at most 28 characters long.");
    mName->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    mName->setFontPointSize(14);
    connect(mName, SIGNAL(updatedField(QString)), this, SLOT(nameChanged(QString)));

    mIPAdress = new QLabel("<i> " + controller.IP + "</i>", this);
    mIPAdress->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    mIPAdress->setAttribute(Qt::WA_TransparentForMouseEvents, true);

    mFirmware = new QLabel("<b>Firmware</b> " + controller.firmware, this);
    mFirmware->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    mFirmware->setAttribute(Qt::WA_TransparentForMouseEvents, true);

    mSerialNumber = new QLabel("<b>Serial:</b> " + controller.serialNumber, this);
    mSerialNumber->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    mSerialNumber->setAttribute(Qt::WA_TransparentForMouseEvents, true);

    mModelID = new QLabel("<b>Model:</b>  " + controller.model, this);
    mModelID->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    mModelID->setAttribute(Qt::WA_TransparentForMouseEvents, true);

    mTopLayout = new QHBoxLayout;
    mTopLayout->addWidget(mTypeIcon, 1);
    mTopLayout->addWidget(mName, 5);

    mMainLayout = new QVBoxLayout(this);
    mMainLayout->addLayout(mTopLayout);
    mMainLayout->addWidget(mIPAdress);
    mMainLayout->addWidget(mFirmware);
    mMainLayout->addWidget(mSerialNumber);
    mMainLayout->addWidget(mModelID);

    mKey = controller.serialNumber;

    mIsChecked = false;
    hideDetails(true);
}

void LeafControllerInfoWidget::updateController(nano::LeafController controller) {
    // many fields such as the mac address and the type of light won't update, only check the fields
    // that do
    if (controller.name != mController.name) {
        mName->setText(controller.name);
    }

    if (controller.serialNumber != mController.serialNumber) {
        mSerialNumber->setText("<b>Serial:</b>  " + controller.serialNumber);
    }

    mController = controller;
}

void LeafControllerInfoWidget::mouseReleaseEvent(QMouseEvent*) {
    emit clicked(mKey);
}

void LeafControllerInfoWidget::setChecked(bool checked) {
    mIsChecked = checked;
    update();
}

void LeafControllerInfoWidget::hideDetails(bool shouldHide) {
    mModelID->setHidden(shouldHide);
    mSerialNumber->setHidden(shouldHide);
    mFirmware->setHidden(shouldHide);
    mHideDetails = shouldHide;
}

void LeafControllerInfoWidget::resize() {
    QSize size(mName->height(), mName->height());
    mTypeIcon->setFixedSize(size);
    mTypePixmap = lightHardwareTypeToPixmap(ELightHardwareType::nanoleaf);
    mTypePixmap = mTypePixmap.scaled(size.width(),
                                     size.height(),
                                     Qt::IgnoreAspectRatio,
                                     Qt::SmoothTransformation);
    mTypeIcon->setPixmap(mTypePixmap);
}

void LeafControllerInfoWidget::resizeEvent(QResizeEvent*) {
    resize();
}


void LeafControllerInfoWidget::paintEvent(QPaintEvent* event) {
    Q_UNUSED(event);
    QStyleOption opt;
    opt.init(this);
    QPainter painter(this);

    painter.setRenderHint(QPainter::Antialiasing);
    if (mIsChecked) {
        painter.fillRect(rect(), QBrush(QColor(61, 142, 201, 255)));
    } else {
        // TODO: could I make this transparent in all cases?
        painter.fillRect(rect(), QBrush(QColor(32, 31, 31, 255)));
    }

    // draw line at bottom of widget
    QRect area(x(), y(), width(), height());
    QPainter linePainter(this);
    linePainter.setRenderHint(QPainter::Antialiasing);
    linePainter.setBrush(QBrush(QColor(255, 255, 255)));
    QLine spacerLine(QPoint(area.x(), area.height() - 3), QPoint(area.width(), area.height() - 3));
    linePainter.drawLine(spacerLine);
}

} // namespace nano
