/*!
 * \copyright
 * Copyright (C) 2015 - 2017.
 * Released under the GNU General Public License.
 */

#include "huesinglecolorpage.h"
#include "ui_huesinglecolorpage.h"

#include <QtCore>
#include <QtGui>
#include <QStyleOption>
#include <QGraphicsOpacityEffect>
#include <QDesktopWidget>

HueSingleColorPage::HueSingleColorPage(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::HueSingleColorPage) {
    ui->setupUi(this);

    ui->colorPicker->chooseLayout(ELayoutColorPicker::eFullLayout);
    connect(ui->colorPicker, SIGNAL(colorUpdate(QColor)), this, SLOT(colorChanged(QColor)));
    ui->colorPicker->useHueWheel(true);

    // setup the slider that controls the LED's temperature color
    //ui->ambientPicker->setPixmap(QPixmap(":/images/huerangeFade.png"));
    ui->ambientPicker->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    ui->ambientPicker->setStyleSheet("border: 2px solid #d0d0cf;");

    // --------------
    // Setup Thrrole Timer
    // --------------

    mThrottleTimer = new QTimer(this);
    connect(mThrottleTimer, SIGNAL(timeout()), this, SLOT(resetThrottleFlag()));

    resizeAmbientPicker();
}

HueSingleColorPage::~HueSingleColorPage() {
    delete ui;
}



void HueSingleColorPage::colorChanged(QColor color) {
    mData->updateColor(color);
    emit singleColorChanged(color);
    emit updateMainIcons();
}


void HueSingleColorPage::ambientValueChanged(int newValue) {
    mData->updateCt(newValue);
    QColor ambientColor = utils::colorTemperatureToRGB(newValue);
    ui->colorPicker->chooseColor(ambientColor, false);
    //ui->ambientSlider->setSliderColorBackground(ambientColor);
    emit singleColorChanged(ambientColor);
}

void HueSingleColorPage::showEvent(QShowEvent *) {
    bool hueFound = false;
    //int hueCT = 0;
    //QColor color;
    for (auto&& device: mData->currentDevices()) {
        if (device.type == ECommType::eHue
                && device.colorMode == EColorMode::eCT) {
            //hueCT = utils::rgbToColorTemperature(device.color);
           // color = device.color;
            hueFound = true;
        } else {
            //color = device.color;
            hueFound = true;
        }
    }

    if (hueFound) {
        mThrottleTimer->start(100);
    }
}

void HueSingleColorPage::resizeAmbientPicker() {
    QPixmap pixmap(":/images/huerangeFade.png");
    ui->ambientPicker->setPixmap(pixmap.scaled(this->width(),
                                        this->height(),
                                        Qt::IgnoreAspectRatio,
                                        Qt::SmoothTransformation));
}

void HueSingleColorPage::changePageType(EHuePageType page) {
    mPageType = page;
    if (mPageType == EHuePageType::eRGB) {
        ui->colorPicker->setVisible(true);
        ui->ambientPicker->setVisible(false);

        ((QHBoxLayout*)this->layout())->setStretch(0, 0);
        ((QHBoxLayout*)this->layout())->setStretch(1, 10);
        ((QHBoxLayout*)this->layout())->setStretch(2, 0);

        resizeAmbientPicker();
    } else if (mPageType == EHuePageType::eAmbient) {
        ui->colorPicker->setVisible(false);
        ui->ambientPicker->setVisible(true);

        ((QHBoxLayout*)this->layout())->setStretch(0, 0);
        ((QHBoxLayout*)this->layout())->setStretch(1, 0);
        ((QHBoxLayout*)this->layout())->setStretch(2, 10);
    }
}

void HueSingleColorPage::resizeEvent(QResizeEvent *) {
    resizeAmbientPicker();

}

void HueSingleColorPage::hideEvent(QHideEvent *) {
    mThrottleTimer->stop();
    mThrottleFlag = false;
}


void HueSingleColorPage::paintEvent(QPaintEvent *) {
    QPixmap pixmap(":/images/huerangeFade.png");

    ui->ambientPicker->setPixmap(pixmap.scaled(this->width() - 6,
                                        this->height() - 6,
                                        Qt::IgnoreAspectRatio,
                                        Qt::SmoothTransformation));
    QImage tmp(ui->ambientPicker->pixmap()->toImage());
    QPainter painter(&tmp);

    painter.setRenderHint(QPainter::Antialiasing, true);
    QPen pen(QColor(208, 208, 207), 2);
    painter.setPen(pen);
    QColor ambientColor = utils::colorTemperatureToRGB(mHueCTValue);
    ambientColor.setHsv(ambientColor.hue(), ambientColor.saturation(), mBrightness * 2.55f);
    QBrush brush(ambientColor);
    painter.setBrush(brush);
    int circleRadius = this->width() / 20.0f;
    painter.drawEllipse(QPointF(mNormalizedX * this->width(),
                                mNormalizedY * this->height() - circleRadius * 2),
                        circleRadius, circleRadius);
    ui->ambientPicker->setPixmap(QPixmap::fromImage(tmp));
}


void HueSingleColorPage::mousePressEvent(QMouseEvent *event) {
    mThrottleFlag = true;
    mPressTime.restart();
    handleMouseEvent(event);
}

void HueSingleColorPage::mouseMoveEvent(QMouseEvent *event) {
    if (!mThrottleFlag) {
        mThrottleFlag = true;
        handleMouseEvent(event);
    }
}

void HueSingleColorPage::mouseReleaseEvent(QMouseEvent *event) {
    Q_UNUSED(event);
    if (mPressTime.elapsed() > 500) {
        handleMouseEvent(event);
    }
}


void HueSingleColorPage::handleMouseEvent(QMouseEvent *event) {
    int deadZoneLeft = 0;
    int deadZoneRight = this->width();
    int deadZoneTop = 0;
    int deadZoneBottom = this->height();
    if (event->pos().x() > deadZoneLeft
            && event->pos().x() < deadZoneRight
            && event->pos().y() > deadZoneTop
            && event->pos().y() < deadZoneBottom) {

        mNormalizedX= utils::clamp(event->pos().x() / (float)this->width(), 0.01f, 1.0f);
        float yNormalized = utils::map(event->pos().y(), 0, (float)this->height(), 0, (float)this->height()) / (float)this->height();
        mNormalizedX = utils::clamp(mNormalizedX, 0.01f, 1.0f);
        mNormalizedY = utils::clamp(yNormalized, 0.01f, 1.0f);
        mHueCTValue = (int)utils::map(mNormalizedX, 0.0f, 1.0f, 153.0f, 500.0f);
        mBrightness = (int)(yNormalized * 100.0f);
        ambientValueChanged(mHueCTValue);
        emit brightnessChanged(mBrightness);
    }
}


void HueSingleColorPage::resetThrottleFlag() {
    mThrottleFlag = false;
}
