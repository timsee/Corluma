/*!
 * \copyright
 * Copyright (C) 2015 - 2020.
 * Released under the GNU General Public License.
 */

#include "comm/nanoleaf/leaflightinfowidget.h"

#include <QGraphicsOpacityEffect>
#include <QMessageBox>
#include <QScroller>
#include <QStyleOption>
#include <QtCore>
#include <QtGui>

#include "utils/qt.h"

namespace nano {

LeafLightInfoWidget::LeafLightInfoWidget(const nano::LeafMetadata& light, QWidget* parent)
    : QWidget(parent),
      mHideDetails{true},
      mLight{light},
      mTypeIcon(new QLabel(this)) {
    const QString styleSheet = "background-color: rgba(0,0,0,0);";
    setStyleSheet(styleSheet);

    mName = new EditableFieldWidget(mLight.name(),
                                    this,
                                    28,
                                    "A lights's name must be at most 28 characters long.");
    mName->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    mName->setFontPointSize(14);
    connect(mName, SIGNAL(updatedField(QString)), this, SLOT(nameChanged(QString)));

    mIPAdress = new QLabel("<i> " + mLight.IP() + "</i>", this);
    mIPAdress->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    mIPAdress->setAttribute(Qt::WA_TransparentForMouseEvents, true);

    mFirmware = new QLabel("<b>Firmware</b> " + mLight.firmware(), this);
    mFirmware->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    mFirmware->setAttribute(Qt::WA_TransparentForMouseEvents, true);

    mSerialNumber = new QLabel("<b>Serial:</b> " + mLight.serialNumber(), this);
    mSerialNumber->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    mSerialNumber->setAttribute(Qt::WA_TransparentForMouseEvents, true);

    mModelID = new QLabel("<b>Model:</b>  " + mLight.model(), this);
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

    mKey = mLight.serialNumber();

    mIsChecked = false;
    hideDetails(true);
}

void LeafLightInfoWidget::updateLight(const nano::LeafMetadata& light) {
    // many fields such as the mac address and the type of light won't update, only check the fields
    // that do
    if (light.name() != mLight.name()) {
        mName->setText(light.name());
    }

    if (light.serialNumber() != mLight.serialNumber()) {
        mSerialNumber->setText("<b>Serial:</b>  " + light.serialNumber());
    }

    mLight = light;
}

void LeafLightInfoWidget::mouseReleaseEvent(QMouseEvent*) {
    emit clicked(mKey);
}

void LeafLightInfoWidget::setChecked(bool checked) {
    mIsChecked = checked;
    update();
}

void LeafLightInfoWidget::hideDetails(bool shouldHide) {
    mModelID->setHidden(shouldHide);
    mSerialNumber->setHidden(shouldHide);
    mFirmware->setHidden(shouldHide);
    mHideDetails = shouldHide;
}

void LeafLightInfoWidget::resize() {
    QSize size(mName->height(), mName->height());
    mTypeIcon->setFixedSize(size);
    mTypePixmap = lightHardwareTypeToPixmap(ELightHardwareType::nanoleaf);
    mTypePixmap = mTypePixmap.scaled(size.width(),
                                     size.height(),
                                     Qt::IgnoreAspectRatio,
                                     Qt::SmoothTransformation);
    mTypeIcon->setPixmap(mTypePixmap);
}

void LeafLightInfoWidget::resizeEvent(QResizeEvent*) {
    resize();
}


void LeafLightInfoWidget::paintEvent(QPaintEvent*) {
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
