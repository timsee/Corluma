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
#include "cor/protocols.h"
#include "cor/stylesheets.h"
#include "utils/qt.h"

namespace hue {

HueInfoWidget::HueInfoWidget(HueMetadata light, QWidget* parent)
    : QWidget(parent),
      mKey{light.uniqueID()},
      mIsChecked{false},
      mHideDetails{false},
      mRowHeight{10},
      mBottomSpacer{6},
      mName{new QLabel(light.name(), this)},
      mModelID{new QLabel("<b>Model:</b>  " + light.modelID(), this)},
      mSoftwareVersion{new QLabel("<b>Software:</b>  " + light.softwareVersion(), this)},
      mType{new QLabel(hardwareTypeToString(hue::modelToHardwareType(light.modelID())), this)},
      mUniqueID{new QLabel("<b>ID:</b>  " + light.uniqueID(), this)},
      mChangeNameButton{new QPushButton("Change Name", this)},
      mDeleteButton{new QPushButton("Delete Light", this)},
      mLight(light),
      mTypeIcon(new QLabel(this)) {
    mName->setStyleSheet(cor::kTransparentStylesheet);

    mModelID->setAttribute(Qt::WA_TransparentForMouseEvents, true);
    mModelID->setStyleSheet(cor::kTransparentStylesheet);

    mSoftwareVersion->setAttribute(Qt::WA_TransparentForMouseEvents, true);
    mSoftwareVersion->setStyleSheet(cor::kTransparentStylesheet);

    mUniqueID->setAttribute(Qt::WA_TransparentForMouseEvents, true);
    mUniqueID->setStyleSheet(cor::kTransparentStylesheet);

    mType->setAttribute(Qt::WA_TransparentForMouseEvents, true);
    mType->setStyleSheet(cor::kTransparentStylesheet);

    mType->setAttribute(Qt::WA_TransparentForMouseEvents, true);
    mTypeIcon->setStyleSheet(cor::kTransparentStylesheet);

    connect(mChangeNameButton, SIGNAL(clicked(bool)), this, SLOT(changeNameButtonPressed(bool)));
    connect(mDeleteButton, SIGNAL(clicked(bool)), this, SLOT(deleteButtonPressed(bool)));
    mDeleteButton->setStyleSheet("background-color:rgb(110,30,30);");

    hideDetails(true);
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

void HueInfoWidget::mouseReleaseEvent(QMouseEvent*) {
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
    mChangeNameButton->setHidden(shouldHide);
    mDeleteButton->setHidden(shouldHide);
    mHideDetails = shouldHide;
    resize();
}

void HueInfoWidget::changeNameButtonPressed(bool) {
    emit clickedChangeName(mLight.uniqueID(), mLight.name());
}

void HueInfoWidget::deleteButtonPressed(bool) {
    emit clickedDelete(mLight.uniqueID(), mLight.name());
}

void HueInfoWidget::resize() {
    auto rowHeight = mRowHeight;
    auto topWidth = 0;
    auto yPos = 0;

    auto columnWidth = width() * 0.45;
    auto columnSpacer = width() * 0.05;
    auto columnStart2 = columnSpacer * 2 + columnWidth;

    // top layout
    mTypeIcon->setGeometry(columnSpacer, 0, rowHeight, rowHeight);
    topWidth += mTypeIcon->width();
    topWidth += columnSpacer;
    mName->setGeometry(topWidth, 0, width() - topWidth, rowHeight);
    yPos += mTypeIcon->height();

    if (mHideDetails) {
        // rest of layout
        mType->setGeometry(columnSpacer, yPos, columnWidth, rowHeight);
        yPos += mType->height();
    } else {
        mType->setGeometry(columnSpacer, yPos, columnWidth, rowHeight);
        mModelID->setGeometry(columnStart2, yPos, columnWidth, rowHeight);
        yPos += mType->height();

        mSoftwareVersion->setGeometry(columnSpacer, yPos, columnWidth, rowHeight);
        mUniqueID->setGeometry(columnStart2, yPos, columnWidth, rowHeight);
        yPos += mUniqueID->height();

        mChangeNameButton->setGeometry(columnSpacer, yPos, columnWidth, rowHeight);
        mDeleteButton->setGeometry(columnStart2, yPos, columnWidth, rowHeight);
        yPos += mDeleteButton->height();
    }


    QSize size(mName->height(), mName->height());
    if (mTypePixmap.size() != size) {
        mTypeIcon->setFixedSize(size);
        mTypePixmap = lightHardwareTypeToPixmap(mLight.hardwareType());
        mTypePixmap = mTypePixmap.scaled(size.width(),
                                         size.height(),
                                         Qt::IgnoreAspectRatio,
                                         Qt::SmoothTransformation);
        mTypeIcon->setPixmap(mTypePixmap);
    }

    yPos += mBottomSpacer;
    setFixedHeight(yPos);
}

void HueInfoWidget::resizeEvent(QResizeEvent*) {
    resize();
}

void HueInfoWidget::paintEvent(QPaintEvent*) {
    QStyleOption opt;
    opt.initFrom(this);
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
    QLine spacerLine(QPoint(area.x(), area.height() - mBottomSpacer / 2),
                     QPoint(area.width(), area.height() - mBottomSpacer / 2));
    linePainter.drawLine(spacerLine);
}

} // namespace hue
