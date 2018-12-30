/*!
 * \copyright
 * Copyright (C) 2015 - 2019.
 * Released under the GNU General Public License.
 */

#include "cor/webview.h"
#include "cor/utils.h"

#include <QFile>
#include <QTextStream>
#include <QDebug>
#include <QtCore>
#include <QtGui>
#include <QStyleOption>
#include <QScroller>

namespace cor
{

WebView::WebView(QString title, QString htmlPath, QWidget *parent) : QWidget(parent) {
    mTextBrowser = new QTextBrowser(this);
    QScroller::grabGesture(mTextBrowser->viewport(), QScroller::LeftMouseButtonGesture);
    mTextBrowser->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    mTextBrowser->setTextInteractionFlags(Qt::LinksAccessibleByMouse);

    mTopWidget = new cor::TopWidget(title, ":images/closeX.png", this);
    connect(mTopWidget, SIGNAL(clicked(bool)), this, SLOT(closeButtonPressed(bool)));

    QFile file(htmlPath);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        qDebug() << "resource not found!";
    }
    QTextStream in(&file);
    mTextBrowser->setHtml(in.readAll());
    mTextBrowser->setOpenExternalLinks(true);

    mGridLayout = new QGridLayout(this);
    mGridLayout->addWidget(mTopWidget, 0, 0, 1, 2);
    mGridLayout->addWidget(mTextBrowser, 1, 0, 1, 2);

    // stretch out the column and rows that need it...
    mGridLayout->setColumnStretch(0, 1);
    mGridLayout->setColumnStretch(1, 5);

    mGridLayout->setRowStretch(0, 1);
    mGridLayout->setRowStretch(1, 10);

    this->setLayout(mGridLayout);
}


void WebView::paintEvent(QPaintEvent *) {
    QStyleOption opt;
    opt.init(this);
    QPainter painter(this);

    painter.setRenderHint(QPainter::Antialiasing);
    painter.fillRect(this->rect(), QBrush(QColor(48, 47, 47)));
}

void WebView::closeButtonPressed(bool ) {
    emit closePressed();
}

}
