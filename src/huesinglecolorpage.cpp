#include "huesinglecolorpage.h"
#include "ui_huesinglecolorpage.h"

HueSingleColorPage::HueSingleColorPage(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::HueSingleColorPage) {
    ui->setupUi(this);

    ui->colorPicker->chooseLayout(ELayoutColorPicker::eFullLayout);
    connect(ui->colorPicker, SIGNAL(colorUpdate(QColor)), this, SLOT(colorChanged(QColor)));
    ui->colorPicker->useHueWheel(true);

    // setup the slider that controls the LED's brightness
    ui->ambientSlider->slider->setRange(153, 500);
    ui->ambientSlider->slider->setValue(173);
    ui->ambientSlider->setSliderHeight(0.5f);
    ui->ambientSlider->setSliderColorBackground(QColor(255, 255, 255));
    connect(ui->ambientSlider, SIGNAL(valueChanged(int)), this, SLOT(ambientValueChanged(int)));


    ui->temperatureButton->setCheckable(true);
    connect(ui->temperatureButton, SIGNAL(clicked(bool)), this, SLOT(temperatureButtonPressed(bool)));

    ui->rgbButton->setCheckable(true);
    connect(ui->rgbButton, SIGNAL(clicked(bool)), this, SLOT(rgbButtonPressed(bool)));

    ui->rgbButton->setChecked(true);
}

HueSingleColorPage::~HueSingleColorPage() {
    delete ui;
}



void HueSingleColorPage::colorChanged(QColor color) {
    mData->updateColor(color);
    emit singleColorChanged(color);
}


void HueSingleColorPage::ambientValueChanged(int newValue) {
    mData->updateCt(newValue);
    QColor ambientColor = utils::colorTemperatureToRGB(newValue);
    ui->ambientSlider->setSliderColorBackground(ambientColor);
    ui->colorPicker->chooseColor(ambientColor, false);
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

void HueSingleColorPage::hideEvent(QHideEvent *) {

}

void HueSingleColorPage::rgbButtonPressed(bool) {
    ui->rgbButton->setChecked(true);
    ui->temperatureButton->setChecked(false);

}

void HueSingleColorPage::temperatureButtonPressed(bool) {
    ui->rgbButton->setChecked(false);
    ui->temperatureButton->setChecked(true);
}
