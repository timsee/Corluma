/*!
 * \copyright
 * Copyright (C) 2015 - 2018.
 * Released under the GNU General Public License.
 */

#include <QtCore>
#include <QtGui>
#include <QStyleOption>
#include <QScroller>
#include <QGraphicsOpacityEffect>

#include "bridgeinfowidget.h"

namespace hue
{

BridgeInfoWidget::BridgeInfoWidget(const hue::Bridge& bridge, QWidget *parent) : QWidget(parent), mBridge(bridge) {
    const QString styleSheet = "background-color: rgba(0,0,0,0);";
    this->setStyleSheet(styleSheet);

    mIPAddress  = new QLabel("<b>IP:</b>  " + bridge.IP, this);
    mIPAddress->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    mIPAddress->setAttribute(Qt::WA_TransparentForMouseEvents, true);
    setTitleFontPointSize(14);

    mID = new QLabel("<b>ID:</b>  " + bridge.id, this);
    mID->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    mID->setAttribute(Qt::WA_TransparentForMouseEvents, true);

    mAPI = new QLabel("<b>API:</b>  " + bridge.api, this);
    mAPI->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    mAPI->setAttribute(Qt::WA_TransparentForMouseEvents, true);

    mDiscoverHueButtons = new QPushButton("Discover New Hues", this);
    mDiscoverHueButtons->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    connect(mDiscoverHueButtons, SIGNAL(clicked()), this, SLOT(pressedDiscoverHues()));

    mLayout = new QVBoxLayout(this);

    mLayout->addWidget(mIPAddress, 1);
    mLayout->addWidget(mID, 1);
    mLayout->addWidget(mAPI, 1);
    mLayout->addWidget(mDiscoverHueButtons, 1);

    mIsChecked = false;
}


void BridgeInfoWidget::setChecked(bool checked) {
    mIsChecked = checked;
    repaint();
}

void BridgeInfoWidget::setHeight(int height) {
    int finalHeight = std::max(height, this->height());
    mHeight = finalHeight;
    this->setFixedHeight(height);
}

void BridgeInfoWidget::paintEvent(QPaintEvent *event) {
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

void BridgeInfoWidget::mouseReleaseEvent(QMouseEvent *event) {
    Q_UNUSED(event);
    emit clicked(mBridge.id);
}


void BridgeInfoWidget::pressedDiscoverHues() {
    emit discoverHuesPressed(mBridge.id);
}

void BridgeInfoWidget::setTitleFontPointSize(int pt) {
    if (pt <= 0) pt = 1;
    QString stylesheet = "font-size:" + QString::number(pt)+ "pt;";
    mIPAddress->setStyleSheet(stylesheet);

    QFont font(mIPAddress->font().toString(), pt);
    QFontMetrics fm(font);
    mIPAddress->setMinimumHeight(fm.height());
}

}
