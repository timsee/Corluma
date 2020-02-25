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

LightInfoListWidget::LightInfoListWidget(QWidget* parent, AppSettings* appSettings)
    : QWidget(parent),
      mLightInfoScrollArea(new LightInfoScrollArea(this)),
      mAppSettings(appSettings),
      mProtocolButtons(3, nullptr),
      mCurrentProtocol{EProtocolType::hue} {
    mTopWidget = new cor::TopWidget("View/Edit Lights", ":images/closeX.png", this);
    connect(mTopWidget, SIGNAL(clicked(bool)), this, SLOT(closePressed(bool)));
    mTopWidget->setFontPoint(20);
    setVisible(false);

    for (auto&& button : mProtocolButtons) {
        button = new QPushButton(this);
        button->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
        button->setCheckable(true);
    }
    mProtocolButtons[0]->setText(protocolToString(EProtocolType::hue));
    connect(mProtocolButtons[0], SIGNAL(clicked()), this, SLOT(hueClicked()));
    mProtocolButtons[1]->setText(protocolToString(EProtocolType::nanoleaf));
    connect(mProtocolButtons[1], SIGNAL(clicked()), this, SLOT(nanoleafClicked()));
    mProtocolButtons[2]->setText(protocolToString(EProtocolType::arduCor));
    connect(mProtocolButtons[2], SIGNAL(clicked()), this, SLOT(arducorClicked()));

    connect(mLightInfoScrollArea,
            SIGNAL(lightClicked(const QString&, bool)),
            this,
            SLOT(lightInfoClicked(const QString&, bool)));
    mDeleteButton = new QPushButton("Delete", this);
    mDeleteButton->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    connect(mDeleteButton, SIGNAL(clicked(bool)), this, SLOT(deleteButtonPressed(bool)));
    mDeleteButton->setEnabled(false);
    mDeleteButton->setStyleSheet("background-color:rgb(45,30,30);");

    resize();
}

void LightInfoListWidget::hueClicked() {
    mCurrentProtocol = EProtocolType::hue;
    highlightProtocolType();
    mLightInfoScrollArea->changeProtocol(mCurrentProtocol);
    resize();
}

void LightInfoListWidget::nanoleafClicked() {
    mCurrentProtocol = EProtocolType::nanoleaf;
    highlightProtocolType();
    mLightInfoScrollArea->changeProtocol(mCurrentProtocol);
    resize();
}

void LightInfoListWidget::arducorClicked() {
    mCurrentProtocol = EProtocolType::arduCor;
    highlightProtocolType();
    mLightInfoScrollArea->changeProtocol(mCurrentProtocol);
    resize();
}

bool LightInfoListWidget::handleProtocolType() {
    auto protocolCount = 0u;
    for (auto button : mProtocolButtons) {
        auto protocol = stringToProtocol(button->text());
        auto enabled = mAppSettings->enabled(protocol);
        if (enabled) {
            protocolCount++;
        } else if (mCurrentProtocol == protocol && !enabled) {
            mCurrentProtocol = EProtocolType::MAX;
        }
    }
    // set a new protocol if needed
    if (mCurrentProtocol == EProtocolType::MAX) {
        for (auto button : mProtocolButtons) {
            auto protocol = stringToProtocol(button->text());
            auto enabled = mAppSettings->enabled(protocol);
            if (enabled) {
                mCurrentProtocol = protocol;
                mLightInfoScrollArea->changeProtocol(mCurrentProtocol);
                break;
            }
        }
    }
    return protocolCount > 1;
}
void LightInfoListWidget::resize() {
    QSize size = parentWidget()->size();
    setFixedSize(int(size.width() * 0.75f), int(size.height() * 0.75f));

    auto yPos = 0;
    mTopWidget->setFixedSize(width(), height() * 2 / 20);
    yPos += mTopWidget->height();
    auto xPos = 0;

    bool shouldShowButtons = handleProtocolType();
    auto protocolButtonHeight = 0;
    for (auto button : mProtocolButtons) {
        if (mAppSettings->enabled(stringToProtocol(button->text())) && shouldShowButtons) {
            button->setGeometry(xPos, yPos, width() / 3, height() * 2 / 20);
            xPos += width() / 3;
            button->setVisible(true);
            protocolButtonHeight = button->height();
        } else {
            button->setVisible(false);
        }
    }

    if (shouldShowButtons) {
        highlightProtocolType();
    }
    yPos += protocolButtonHeight;

    mLightInfoScrollArea->setGeometry(int(width() * 0.01),
                                      yPos,
                                      int(width() * 0.98),
                                      int(height() * 13 / 20));
    yPos += mLightInfoScrollArea->height();

    mDeleteButton->setGeometry(int(width() * 0.01),
                               yPos,
                               int(width() * 0.98),
                               int(height() * 3 / 20));

    mLightInfoScrollArea->resize();
}


void LightInfoListWidget::highlightProtocolType() {
    for (auto button : mProtocolButtons) {
        if (mCurrentProtocol == stringToProtocol(button->text())) {
            button->setChecked(true);
        } else {
            button->setChecked(false);
        }
    }
}

void LightInfoListWidget::deleteButtonPressed(bool) {
    QMessageBox::StandardButton reply;
    auto result = scrollArea()->lookupCurrentLight();
    auto type = result.first;
    auto lightName = result.second;

    if (type != EProtocolType::MAX) {
        QString text;
        switch (type) {
            case EProtocolType::arduCor:
                text = "Delete " + lightName
                       + "? This will remove this light and all others using the same arduino or "
                         "raspberry pi from the app memory.";
                break;
            case EProtocolType::hue:
                text = "Delete " + lightName + "? This will remove it from the Hue Bridge.";
                break;
            case EProtocolType::nanoleaf:
                text = "Delete " + lightName + "? This will remove it from the app memory.";
                break;
            case EProtocolType::MAX:
                text = "LIGHT NOT FOUND";
                break;
        }
        reply = QMessageBox::question(this, "Delete?", text, QMessageBox::Yes | QMessageBox::No);
        if (reply == QMessageBox::Yes) {
            // signal to remove from app
            emit deleteLight(scrollArea()->key());
        }
    }
}

void LightInfoListWidget::paintEvent(QPaintEvent*) {
    QStyleOption opt;
    opt.init(this);
    QPainter painter(this);

    painter.setRenderHint(QPainter::Antialiasing);
    painter.fillRect(rect(), QBrush(QColor(48, 47, 47)));
}

void LightInfoListWidget::closePressed(bool) {
    emit pressedClose();
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

void LightInfoListWidget::pushIn() {
    moveWidget(
        this,
        QPoint(int(parentWidget()->width() * 0.125f), int(-1 * parentWidget()->height())),
        QPoint(int(parentWidget()->width() * 0.125f), int(parentWidget()->height() * 0.125f)));

    setVisible(true);
    raise();
    isOpen(true);
}

void LightInfoListWidget::pushOut() {
    moveWidget(
        this,
        QPoint(int(parentWidget()->width() * 0.125f), int(parentWidget()->height() * 0.125f)),
        QPoint(int(parentWidget()->width() * 0.125f), int(-1 * parentWidget()->height())));
    isOpen(false);
}
