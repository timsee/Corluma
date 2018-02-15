/*!
 * \copyright
 * Copyright (C) 2015 - 2018.
 * Released under the GNU General Public License.
 */

#include "topwidget.h"
#include "cor/utils.h"

namespace cor
{

TopWidget::TopWidget(QString title, QString resource, QWidget *parent) : QWidget(parent)
{
    mLayout = new QHBoxLayout();

    mButton = new QPushButton(this);
    mButton->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Expanding);
    connect(mButton, SIGNAL(clicked(bool)), this, SLOT(buttonPressed(bool)));

    mTitle = new QLabel(title, this);
    mTitle->setStyleSheet("font-size:30pt;");
    mTitle->setAlignment(Qt::AlignBottom);
    mTitle->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

    mResource = resource;

    mLayout->addWidget(mTitle, 10);
    mLayout->addWidget(mButton, 1);
    mLayout->setContentsMargins(0,0,0,0);
    mLayout->setSpacing(6);
    this->setLayout(mLayout);
}

void TopWidget::setFontPoint(int pt) {
    QString stylesheet = "font-size:" + QString::number(pt)+ "pt;";
    mTitle->setStyleSheet(stylesheet);
}

void TopWidget::buttonPressed(bool pressed) {
    emit clicked(pressed);
}

void TopWidget::resizeEvent(QResizeEvent *) {
    mButton->setFixedWidth(mButton->height());
    cor::resizeIcon(mButton, mResource, 0.5f);
}

}
