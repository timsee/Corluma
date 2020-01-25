/*!
 * \copyright
 * Copyright (C) 2015 - 2020.
 * Released under the GNU General Public License.
 */

#include "comm/hue/hueinfowidget.h"

#include <QGraphicsOpacityEffect>
#include <QScroller>
#include <QStyleOption>
#include <QtCore>
#include <QtGui>

#include "utils/qt.h"

namespace hue {

HueInfoWidget::HueInfoWidget(HueMetadata light, QWidget* parent)
    : QWidget(parent),
      mHideDetails{false},
      mLight(light),
      mTypeIcon(new QLabel(this)) {
    const QString styleSheet = "background-color: rgba(0,0,0,0);";
    setStyleSheet(styleSheet);

    mName = new EditableFieldWidget(light.name(),
                                    this,
                                    32,
                                    "A hue's name must be at most 32 characters long.");
    mName->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    mName->setFontPointSize(14);
    connect(mName, SIGNAL(updatedField(QString)), this, SLOT(nameChanged(QString)));

    mModelID = new QLabel("<b>Model:</b>  " + light.modelID(), this);
    mModelID->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    mModelID->setAttribute(Qt::WA_TransparentForMouseEvents, true);

    mSoftwareVersion = new QLabel("<b>Software:</b>  " + light.softwareVersion(), this);
    mSoftwareVersion->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    mSoftwareVersion->setAttribute(Qt::WA_TransparentForMouseEvents, true);

    mUniqueID = new QLabel("<b>ID:</b>  " + light.uniqueID(), this);
    mUniqueID->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    mUniqueID->setAttribute(Qt::WA_TransparentForMouseEvents, true);

    mType = new QLabel(cor::hueTypeToString(light.hueType()), this);
    mType->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    mType->setAttribute(Qt::WA_TransparentForMouseEvents, true);

    mTopLayout = new QHBoxLayout;
    mTopLayout->addWidget(mTypeIcon, 1);
    mTopLayout->addWidget(mName, 5);

    mMainLayout = new QVBoxLayout(this);
    mMainLayout->addLayout(mTopLayout);
    mMainLayout->addWidget(mType);
    mMainLayout->addWidget(mModelID);
    mMainLayout->addWidget(mSoftwareVersion);
    mMainLayout->addWidget(mUniqueID);

    mKey = light.uniqueID();

    mIsChecked = false;
    hideDetails(true);
    resize();
}

void HueInfoWidget::updateLight(HueMetadata light) {
    // many fields such as the mac address and the type of light won't update, only check the fields
    // that do
    if (light.name() != mLight.name()) {
        mName->setText(light.name());
    }

    if (light.softwareVersion() != mLight.softwareVersion()) {
        mSoftwareVersion->setText("<b>Software:</b>  " + light.softwareVersion());
    }

    mLight = light;
}

void HueInfoWidget::mouseReleaseEvent(QMouseEvent* event) {
    Q_UNUSED(event);
    emit clicked(mKey);
}

void HueInfoWidget::setChecked(bool checked) {
    mIsChecked = checked;
    update();
}

void HueInfoWidget::hideDetails(bool shouldHide) {
    mModelID->setHidden(shouldHide);
    mSoftwareVersion->setHidden(shouldHide);
    mUniqueID->setHidden(shouldHide);
    mHideDetails = shouldHide;
}

void HueInfoWidget::resize() {
    QSize size(mName->height(), mName->height());
    mTypeIcon->setFixedSize(size);
    mTypePixmap = lightHardwareTypeToPixmap(mLight.hardwareType());
    mTypePixmap = mTypePixmap.scaled(size.width(),
                                     size.height(),
                                     Qt::IgnoreAspectRatio,
                                     Qt::SmoothTransformation);
    mTypeIcon->setPixmap(mTypePixmap);
}

void HueInfoWidget::resizeEvent(QResizeEvent*) {
    resize();
}

void HueInfoWidget::paintEvent(QPaintEvent*) {
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

} // namespace hue
