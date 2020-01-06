/*!
 * \copyright
 * Copyright (C) 2015 - 2019.
 * Released under the GNU General Public License.
 */

#include "arducorinfowidget.h"

#include <QGraphicsOpacityEffect>
#include <QScroller>
#include <QStyleOption>
#include <QtCore>
#include <QtGui>

ArduCorInfoWidget::ArduCorInfoWidget(ArduCorMetadata light, QWidget* parent)
    : QWidget(parent),
      mHideDetails{false},
      mMetadata(light),
      mTypeIcon(new QLabel(this)) {
    const QString styleSheet = "background-color: rgba(0,0,0,0);";
    setStyleSheet(styleSheet);

    mName = new QLabel(light.uniqueID(), this);
    mName->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    setTitleFontPointSize(14);

    QString pathString = commTypeToString(light.commType()) + " " + light.controller() + " "
                         + QString::number(light.index());
    mPathLabel = new QLabel(pathString, this);
    mPathLabel->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    mPathLabel->setAttribute(Qt::WA_TransparentForMouseEvents, true);

    mHardwareTypeLabel =
        new QLabel("<b>Hardware:</b>  " + hardwareTypeToString(light.hardwareType()), this);
    mHardwareTypeLabel->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    mHardwareTypeLabel->setAttribute(Qt::WA_TransparentForMouseEvents, true);

    mAPILabel = new QLabel("<b>API:</b>  " + QString::number(mMetadata.majorAPI()) + "."
                               + QString::number(mMetadata.minorAPI()),
                           this);
    mAPILabel->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    mAPILabel->setAttribute(Qt::WA_TransparentForMouseEvents, true);

    mTopLayout = new QHBoxLayout;
    mTopLayout->addWidget(mTypeIcon, 1);
    mTopLayout->addWidget(mName, 5);

    mMainLayout = new QVBoxLayout(this);
    mMainLayout->addLayout(mTopLayout);
    mMainLayout->addWidget(mPathLabel);
    mMainLayout->addWidget(mHardwareTypeLabel);
    mMainLayout->addWidget(mAPILabel);

    mKey = light.uniqueID();

    mIsChecked = false;
    hideDetails(true);
}


void ArduCorInfoWidget::updateLight(ArduCorMetadata light) {
    mMetadata = std::move(light);
}

void ArduCorInfoWidget::mouseReleaseEvent(QMouseEvent*) {
    emit clicked(mKey);
}

void ArduCorInfoWidget::setChecked(bool checked) {
    mIsChecked = checked;
    update();
}

void ArduCorInfoWidget::hideDetails(bool shouldHide) {
    mAPILabel->setHidden(shouldHide);
    mHardwareTypeLabel->setHidden(shouldHide);
    mHideDetails = shouldHide;
}

void ArduCorInfoWidget::resizeEvent(QResizeEvent*) {
    resize();
}

void ArduCorInfoWidget::resize() {
    QSize size(mName->height(), mName->height());
    mTypeIcon->setFixedSize(size);
    mTypePixmap = lightHardwareTypeToPixmap(mMetadata.hardwareType());
    mTypePixmap = mTypePixmap.scaled(size.width(),
                                     size.height(),
                                     Qt::IgnoreAspectRatio,
                                     Qt::SmoothTransformation);
    mTypeIcon->setPixmap(mTypePixmap);
}

void ArduCorInfoWidget::paintEvent(QPaintEvent*) {
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



void ArduCorInfoWidget::setTitleFontPointSize(int pt) {
    if (pt <= 0) {
        pt = 1;
    }
    QString stylesheet = "font-size:" + QString::number(pt) + "pt;";
    mName->setStyleSheet(stylesheet);

    QFont font(mName->font().toString(), pt);
    QFontMetrics fm(font);
    mName->setMinimumHeight(fm.height());
}
