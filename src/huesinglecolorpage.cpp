/*!
 * \copyright
 * Copyright (C) 2015 - 2017.
 * Released under the GNU General Public License.
 */

#include "huesinglecolorpage.h"
#include "ui_huesinglecolorpage.h"

HueSingleColorPage::HueSingleColorPage(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::HueSingleColorPage) {
    ui->setupUi(this);

    ui->colorPicker->chooseLayout(ELayoutColorPicker::eFullLayout);
    connect(ui->colorPicker, SIGNAL(colorUpdate(QColor)), this, SLOT(colorChanged(QColor)));
    ui->colorPicker->useHueWheel(true);

    // setup the slider that controls the LED's temperature color
    ui->ambientSlider->slider->setRange(153, 500);
    ui->ambientSlider->slider->setValue(173);
    ui->ambientSlider->setSliderHeight(1.0f);
    ui->ambientSlider->setSliderColorBackground(QColor(255, 255, 255));
    //ui->ambientSlider->setSliderImageBackground(":/images/huerange.png");
    connect(ui->ambientSlider, SIGNAL(valueChanged(int)), this, SLOT(ambientValueChanged(int)));
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
    ui->ambientSlider->setSliderColorBackground(ambientColor);
    emit singleColorChanged(ambientColor);
}

void HueSingleColorPage::showEvent(QShowEvent *) {
    bool hueFound = false;
    int hueCT = 0;
    QColor color;
    bool ctHueFound = false;
    for (auto&& device: mData->currentDevices()) {
        if (device.type == ECommType::eHue
                && device.colorMode == EColorMode::eCT) {
            hueCT = utils::rgbToColorTemperature(device.color);
            color = device.color;
            ctHueFound = true;
            hueFound = true;
        } else {
            color = device.color;
            hueFound = true;
        }
    }

    if (hueFound) {
        bool blocked = ui->ambientSlider->slider->blockSignals(true);
        if (ctHueFound) {
            ui->ambientSlider->slider->setValue(hueCT);
        }
        ui->colorPicker->chooseColor(color, false);
        ui->ambientSlider->slider->blockSignals(blocked);
    }
}

void HueSingleColorPage::changePageType(EHuePageType page) {
    mPageType = page;
    if (mPageType == EHuePageType::eRGB) {
        ui->colorPicker->setVisible(true);
        ui->ambientSlider->setVisible(false);

        ((QHBoxLayout*)this->layout())->setStretch(0, 0);
        ((QHBoxLayout*)this->layout())->setStretch(1, 10);
        ((QHBoxLayout*)this->layout())->setStretch(2, 0);
    } else if (mPageType == EHuePageType::eAmbient) {
        ui->colorPicker->setVisible(false);
        ui->ambientSlider->setVisible(true);

        ((QHBoxLayout*)this->layout())->setStretch(0, 0);
        ((QHBoxLayout*)this->layout())->setStretch(1, 0);
        ((QHBoxLayout*)this->layout())->setStretch(2, 10);
    }
}

void HueSingleColorPage::hideEvent(QHideEvent *) {

}
