/*!
 * \copyright
 * Copyright (C) 2015 - 2020.
 * Released under the GNU General Public License.
 */

#include "lightinfolistwidget.h"

#include <QGraphicsOpacityEffect>
#include <QMessageBox>
#include <QScrollBar>
#include <QScroller>
#include <QStyleOption>
#include <QtCore>
#include <QtGui>

#include "utils/qt.h"

LightInfoListWidget::LightInfoListWidget(QWidget* parent)
    : QWidget(parent),
      mLightInfoScrollArea(new LightInfoScrollArea(this)) {
    setVisible(false);

    connect(mLightInfoScrollArea,
            SIGNAL(lightClicked(const QString&, bool)),
            this,
            SLOT(lightInfoClicked(const QString&, bool)));
    mDeleteButton = new QPushButton("Delete", this);
    mDeleteButton->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    connect(mDeleteButton, SIGNAL(clicked(bool)), this, SLOT(deleteButtonPressed(bool)));
    mDeleteButton->setEnabled(false);
    mDeleteButton->setStyleSheet("background-color:rgb(45,30,30);");
}


void LightInfoListWidget::resize() {
    auto yPos = 0;

    mLightInfoScrollArea->setGeometry(int(width() * 0.01),
                                      yPos,
                                      int(width() * 0.98),
                                      int(height() * 17 / 20));
    yPos += mLightInfoScrollArea->height();

    mDeleteButton->setGeometry(int(width() * 0.01),
                               yPos,
                               int(width() * 0.98),
                               int(height() * 3 / 20));

    mLightInfoScrollArea->resize();
}


void LightInfoListWidget::deleteButtonPressed(bool) {
    QMessageBox::StandardButton reply;
    auto lightName = scrollArea()->lookupCurrentLight();
    QString text = "Delete " + lightName + "? This will remove it from the Hue Bridge.";
    reply = QMessageBox::question(this, "Delete?", text, QMessageBox::Yes | QMessageBox::No);
    if (reply == QMessageBox::Yes) {
        // signal to remove from app
        emit deleteLight(scrollArea()->key());
    }
}

void LightInfoListWidget::paintEvent(QPaintEvent*) {
    QStyleOption opt;
    opt.initFrom(this);
    QPainter painter(this);

    painter.setRenderHint(QPainter::Antialiasing);
    painter.fillRect(rect(), QBrush(QColor(48, 47, 47)));
}


void LightInfoListWidget::lightInfoClicked(const QString&, bool shouldEnableDelete) {
    if (shouldEnableDelete) {
        mDeleteButton->setEnabled(true);
        mDeleteButton->setStyleSheet("background-color:rgb(110,30,30);");
    } else {
        mDeleteButton->setEnabled(false);
        mDeleteButton->setStyleSheet("background-color:rgb(45,30,30);");
    }

    resize();
}
