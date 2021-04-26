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

    mNewLightButton = new QPushButton("Find New Lights", this);
    mNewLightButton->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    connect(mNewLightButton, SIGNAL(clicked(bool)), this, SLOT(findNewLightButtonPressed(bool)));
    mNewLightButton->setEnabled(true);
    mNewLightButton->setStyleSheet(cor::kEditButtonBackground);
}


void LightInfoListWidget::resize() {
    auto yPos = height() * 0.02;

    mLightInfoScrollArea->setGeometry(int(width() * 0.02),
                                      yPos,
                                      int(width() * 0.96),
                                      int(height() * 0.96 * 17 / 20));
    yPos += mLightInfoScrollArea->height();

    mNewLightButton->setGeometry(int(width() * 0.02),
                                 yPos,
                                 int(width() * 0.96),
                                 int(height() * 0.96 * 3 / 20));

    mLightInfoScrollArea->resize();
}

void LightInfoListWidget::updateLightName(QString uniqueID, QString newName) {
    for (auto widget : mLightInfoScrollArea->hueWidgets()) {
        if (widget->key() == uniqueID) {
            auto metadata = widget->metadata();
            metadata.name(newName);
            widget->updateLight(metadata);
        }
    }
}

void LightInfoListWidget::deleteLightFromDisplay(QString uniqueID) {
    mLightInfoScrollArea->deleteLight(uniqueID);
}

void LightInfoListWidget::changeRowHeight(int rowHeight) {
    for (auto widget : mLightInfoScrollArea->hueWidgets()) {
        widget->changeRowHeight(rowHeight);
    }
}

void LightInfoListWidget::changeNamePressed(QString key, QString name) {
    emit changeLightName(key, name);
}

void LightInfoListWidget::deleteButtonPressed(QString key, QString) {
    emit deleteLight(key);
}

void LightInfoListWidget::findNewLightButtonPressed(bool) {
    emit findNewLightClicked();
}

void LightInfoListWidget::paintEvent(QPaintEvent*) {
    QStyleOption opt;
    opt.initFrom(this);
    QPainter painter(this);

    painter.setRenderHint(QPainter::Antialiasing);
    painter.fillRect(rect(), QBrush(QColor(48, 47, 47)));
}


void LightInfoListWidget::lightInfoClicked(const QString&, bool) {
    resize();
}
