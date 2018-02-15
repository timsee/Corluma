/*!
 * \copyright
 * Copyright (C) 2015 - 2018.
 * Released under the GNU General Public License.
 */

#include <QGraphicsEffect>
#include <QPainter>
#include <QStyleOption>
#include <QGraphicsScene>
#include <QDebug>

#include "cor/statusicon.h"

namespace cor
{

StatusIcon::StatusIcon(QWidget *parent) : QWidget(parent) {
    mIcon = new QLabel(this);
    mIcon->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);

    mBlackIcon = new QLabel(this);
    mIcon->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);

    this->setStyleSheet("background-color:rgba(0,0,0,0);");
}


void StatusIcon::update(bool isReachable, bool isOn, float brightness) {
    mIcon->setFixedSize(this->width(), this->height());
    QPixmap buttonIcon;
    if (!isReachable) {
        buttonIcon = QPixmap(":/images/redButton.png");
    } else if (!isOn) {
        buttonIcon = QPixmap(":/images/blackButton.png");
    } else {
        buttonIcon = QPixmap(":/images/whiteButton.png");

        QPixmap blackPixmap(":/images/blackButton.png");
        blackPixmap = blackPixmap.scaled(mIcon->height() * 0.5f,
                                       mIcon->height() * 0.5f,
                                       Qt::KeepAspectRatio,
                                       Qt::SmoothTransformation);
        mBlackIcon->setPixmap(blackPixmap);

        mBlackIcon->setGeometry(mIcon->geometry());

        // make black icon inverse of brightness
        float blackBrightness = 1.0f - (brightness / 100.0f);
        QGraphicsOpacityEffect *effect = new QGraphicsOpacityEffect(mBlackIcon);
        effect->setOpacity(blackBrightness);
        mBlackIcon->setGraphicsEffect(effect);
    }
    buttonIcon = buttonIcon.scaled(mIcon->height() * 0.5f,
                                   mIcon->height() * 0.5f,
                                   Qt::KeepAspectRatio,
                                   Qt::SmoothTransformation);
    mIcon->setPixmap(buttonIcon);
}

}
