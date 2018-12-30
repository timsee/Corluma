/*!
 * \copyright
 * Copyright (C) 2015 - 2019.
 * Released under the GNU General Public License.
 */

#include <QtCore>
#include <QtGui>
#include <QStyleOption>
#include <QGraphicsOpacityEffect>

#include "nowifiwidget.h"

NoWifiWidget::NoWifiWidget(QWidget *parent) : QWidget(parent) {
    mText = new QLabel("<b>No Wifi Detected :(</b>", this);
    mText->setStyleSheet("font-size:20pt;");
    mText->setAlignment(Qt::AlignHCenter | Qt::AlignVCenter);
    mText->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

    mImage =new QLabel("", this);
    mImage->setAlignment(Qt::AlignHCenter);
    mImage->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

    mLayout = new QVBoxLayout;
    mLayout->addWidget(mText, 1);
    mLayout->addWidget(mImage, 5);
    this->setLayout(mLayout);
}


void NoWifiWidget::paintEvent(QPaintEvent *) {
    QStyleOption opt;
    opt.init(this);
    QPainter painter(this);

    painter.setRenderHint(QPainter::Antialiasing);
    painter.fillRect(this->rect(), QBrush(QColor(48, 47, 47)));
}

void NoWifiWidget::resizeEvent(QResizeEvent *) {
    mPixmap = QPixmap(":images/no_wifi.png");
    int width = int(this->width() * 0.5f);
    mImage->setPixmap(mPixmap.scaled(width,
                                     width,
                                     Qt::KeepAspectRatio,
                                     Qt::SmoothTransformation));
}
