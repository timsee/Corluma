/*!
 * \copyright
 * Copyright (C) 2015 - 2017.
 * Released under the GNU General Public License.
 */

#include "settingspage.h"
#include "ui_settingspage.h"
#include "commhue.h"
#include "listdevicewidget.h"

#include <QFileDialog>
#include <QDebug>
#include <QSignalMapper>
#include <QScroller>
#include <QMessageBox>
#include <QStandardPaths>
#include <QStyleOption>
#include <QGraphicsOpacityEffect>
#include <QPainter>

#include <algorithm>

SettingsPage::SettingsPage(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::SettingsPage) {
    ui->setupUi(this);

    // setup sliders
    mSliderSpeedValue = 425;
    ui->speedSlider->slider->setRange(1, 1000);
    ui->speedSlider->slider->setValue(mSliderSpeedValue);
    ui->speedSlider->setSliderHeight(0.5f);
    ui->speedSlider->slider->setTickPosition(QSlider::TicksBelow);
    ui->speedSlider->slider->setTickInterval(100);

    ui->timeoutSlider->slider->setRange(0,240);
    ui->timeoutSlider->slider->setValue(120);
    ui->timeoutSlider->setSliderHeight(0.5f);
    ui->timeoutSlider->slider->setTickPosition(QSlider::TicksBelow);
    ui->timeoutSlider->slider->setTickInterval(40);

    mConnectionButtons = { ui->yunButton,
#ifndef MOBILE_BUILD
                    ui->serialButton,
#endif //MOBILE_BUILD
                    ui->hueButton };

    connect(ui->speedSlider, SIGNAL(valueChanged(int)), this, SLOT(speedChanged(int)));
    connect(ui->timeoutSlider, SIGNAL(valueChanged(int)), this, SLOT(timeoutChanged(int)));

    connect(ui->debugButton, SIGNAL(clicked(bool)), this, SLOT(debugButtonClicked(bool)));
    connect(ui->loadButton, SIGNAL(clicked(bool)), this, SLOT(loadButtonClicked(bool)));
    connect(ui->mergeButton, SIGNAL(clicked(bool)), this, SLOT(mergeButtonClicked(bool)));
    connect(ui->saveButton, SIGNAL(clicked(bool)), this, SLOT(saveDataButtonClicked(bool)));

    connect(ui->hueButton, SIGNAL(clicked(bool)), this, SLOT(hueCheckboxClicked(bool)));
    ui->hueButton->setText("Hue");

    connect(ui->yunButton, SIGNAL(clicked(bool)), this, SLOT(yunCheckboxClicked(bool)));
    ui->yunButton->setText("Yun");


    connect(ui->closeButton, SIGNAL(clicked(bool)), this, SLOT(closeButtonPressed(bool)));

#ifndef MOBILE_BUILD
    connect(ui->serialButton, SIGNAL(clicked(bool)), this, SLOT(serialCheckboxClicked(bool)));
    ui->serialButton->setText("Serial");
#else
    ui->serialButton->setHidden(true);
#endif //MOBILE_BUILD

    ui->yunButton->setCheckable(true);
    ui->hueButton->setCheckable(true);
#ifndef MOBILE_BUILD
    ui->serialButton->setCheckable(true);
#endif //MOBILE_BUILD

    QFont font = ui->closeButton->font();
    font.setPointSize(36);
    ui->closeButton->setFont(font);

    QScroller::grabGesture(ui->scrollArea->viewport(), QScroller::LeftMouseButtonGesture);
}

SettingsPage::~SettingsPage() {
    delete ui;
}

void SettingsPage::setupUI() {
    connect(mData, SIGNAL(devicesEmpty()), this, SLOT(deviceCountReachedZero()));
}

void SettingsPage::updateUI() {

    if (mData->shouldUseHueAssets())  {
        ui->timeoutSlider->setHidden(true);
        ui->timeoutLabel->setHidden(true);
    } else {
        ui->timeoutSlider->setHidden(false);
        ui->timeoutLabel->setHidden(false);
    }

    if (mData->currentRoutine() <= ELightingRoutine::eSingleSawtoothFadeOut) {
        ui->speedSlider->setSliderColorBackground(mData->mainColor());
        ui->timeoutSlider->setSliderColorBackground(mData->mainColor());
    } else {
        ui->speedSlider->setSliderColorBackground(mData->colorsAverage(mData->currentColorGroup()));
        ui->timeoutSlider->setSliderColorBackground(mData->colorsAverage(mData->currentColorGroup()));
    }
}

// ----------------------------
// Slots
// ----------------------------

void SettingsPage::speedChanged(int newSpeed) {
    mSliderSpeedValue = newSpeed;
    int finalSpeed;
    // first half of slider is going linearly between 20 FPS down to 1 FPS
    if (newSpeed < 500) {
        float percent = newSpeed / 500.0f;
        finalSpeed = (int)((1.0f - percent) * 2000.0f);
    } else {
        // second half maps 1FPS to 0.01FPS
        float percent = newSpeed - 500.0f;
        finalSpeed = (500 - percent) / 5.0f;
        if (finalSpeed < 2.0f) {
            finalSpeed = 2.0f;
        }
    }
    mData->updateSpeed(finalSpeed);
}

void SettingsPage::timeoutChanged(int newTimeout) {
   mData->updateTimeout(newTimeout);
}


// ----------------------------
// Protected
// ----------------------------


void SettingsPage::showEvent(QShowEvent *event) {
    Q_UNUSED(event);
    updateUI();
    checkCheckBoxes();

    if (mData->currentDevices().size() == 0) {
        deviceCountReachedZero();

    } else {
        ui->speedSlider->enable(true);
        ui->timeoutSlider->enable(true);

        ui->speedSlider->setSliderColorBackground(mData->mainColor());
        ui->timeoutSlider->setSliderColorBackground(mData->mainColor());

        // default the settings bars to the current colors
        ui->speedSlider->slider->setValue(mSliderSpeedValue);
        ui->timeoutSlider->slider->setValue(mData->timeout());
    }
}

void SettingsPage::hideEvent(QHideEvent *) {

}

void SettingsPage::resizeEvent(QResizeEvent *event) {
    Q_UNUSED(event);
//    int height = static_cast<int>(ui->hueButton->size().height() * 0.8f);
//    int width = static_cast<int>(ui->hueButton->size().width() * 0.6f);
//    QString stylesheet = "QPushButton::indicator { width: ";
//    stylesheet += QString::number(width);
//    stylesheet +=  "px; height: ";
//    stylesheet +=  QString::number(height);
//    stylesheet += "px; }";
//    ui->hueButton->setStyleSheet(stylesheet);
//    ui->yunButton->setStyleSheet(stylesheet);
//#ifndef MOBILE_BUILD
//    ui->serialButton->setStyleSheet(stylesheet);
//#endif //MOBILE_BUILD

    ui->hueButton->setMinimumHeight(ui->hueButton->width());
    ui->hueButton->setMaximumHeight(ui->hueButton->width());

    ui->yunButton->setMinimumHeight(ui->yunButton->width());
    ui->yunButton->setMaximumHeight(ui->yunButton->width());

#ifndef MOBILE_BUILD
    ui->serialButton->setMinimumHeight(ui->serialButton->width());
    ui->serialButton->setMaximumHeight(ui->serialButton->width());
#endif //MOBILE_BUILD


    ui->speedSlider->setMinimumSize(QSize(this->width() * 0.8f, this->height() * 0.1f));
    ui->speedSlider->setMaximumSize(QSize(this->width() * 0.8f, this->height() * 0.1f));

    ui->timeoutSlider->setMinimumSize(QSize(this->width() * 0.8f, this->height() * 0.1f));
    ui->timeoutSlider->setMaximumSize(QSize(this->width() * 0.8f, this->height() * 0.1f));

    ui->scrollArea->widget()->setMaximumWidth(this->width() * 0.9f);

    ui->debugButton->setMinimumHeight(ui->debugButton->width());
    ui->resetButton->setMinimumHeight(ui->debugButton->width());
    ui->loadButton->setMinimumHeight(ui->loadButton->width());
    ui->saveButton->setMinimumHeight(ui->saveButton->width());
    ui->mergeButton->setMinimumHeight(ui->mergeButton->width());

    int min = std::min(ui->closeButton->width(), ui->closeButton->height()) * 0.85f;
    ui->closeButton->setIconSize(QSize(min, min));

}

void SettingsPage::checkCheckBoxes() {
    std::vector<ECommType> types =  mData->commTypeSettings()->commTypes();
    for (uint32_t i = 0; i < types.size(); ++i) {
        QPushButton* checkBox = mConnectionButtons[i];
        ECommType type = types[i];
        if (mData->commTypeSettings()->commTypeEnabled(type)) {
            checkBox->setChecked(true);
        } else {
            checkBox->setChecked(false);
        }
    }
}

void SettingsPage::checkBoxClicked(ECommType type, bool checked) {
    bool successful = mData->commTypeSettings()->enableCommType(type, checked);
    if (!successful) {
        mConnectionButtons[mData->commTypeSettings()->indexOfCommTypeSettings(type)]->setChecked(true);
    }

    if (checked) {
        mComm->startup(type);
    } else {
        mComm->shutdown(type);
        mData->removeDevicesOfType(type);
    }
}


void SettingsPage::loadButtonClicked(bool) {
    QFileDialog dialog(this);
    dialog.setFileMode(QFileDialog::AnyFile);
    dialog.setNameFilter(tr("JSON (*.json)"));
    dialog.setViewMode(QFileDialog::Detail);
    dialog.setStyleSheet("color:silver;");
    const QString downloadsFolder = QStandardPaths::writableLocation(QStandardPaths::DownloadLocation);
    dialog.setDirectory(downloadsFolder);
    QStringList fileNames;
    if (dialog.exec()) {
        fileNames = dialog.selectedFiles();
        for (auto& name : fileNames){
            if (!mGroups->loadExternalData(name)) {
                qDebug() << "WARNING: loading external data failed at " << name;
            }
        }
    }
}

void SettingsPage::mergeButtonClicked(bool) {
    qDebug() << "Merge clicekd!";

}

void SettingsPage::debugButtonClicked(bool) {
    std::list<SLightDevice> debugDevices = mGroups->loadDebugData();
    if (debugDevices.size() > 0) {
        mComm->loadDebugData(debugDevices);
        emit debugPressed();
    } else {
        qDebug() << "WARNING: Debug devices not found!";
    }
}

void SettingsPage::saveDataButtonClicked(bool) {
    QString fileName = QFileDialog::getSaveFileName(this,
          tr("Save Group Data"), "CorlumaGroups.json",
          tr("JSON (*.json)"));
    if (fileName.isEmpty()) {
        qDebug() << "WARNING: save file name empty";
        return;
    }
    if (!mGroups->saveFile(fileName)) {
        qDebug() << "WARNING: Save failed!";
    }
}

void SettingsPage::hueCheckboxClicked(bool checked) {
    checkBoxClicked(ECommType::eHue, checked);
}

void SettingsPage::yunCheckboxClicked(bool checked) {
    checkBoxClicked(ECommType::eUDP, checked);
}

void SettingsPage::serialCheckboxClicked(bool checked) {
#ifndef MOBILE_BUILD
    checkBoxClicked(ECommType::eSerial, checked);
#else
    Q_UNUSED(checked);
#endif //MOBILE_BUILD
}

void SettingsPage::deviceCountReachedZero() {
    ui->speedSlider->enable(false);
    ui->timeoutSlider->enable(false);

    ui->speedSlider->setSliderColorBackground(QColor(150, 150, 150));
    ui->timeoutSlider->setSliderColorBackground(QColor(150, 150, 150));
}


void SettingsPage::renderUI() {

}

void SettingsPage::paintEvent(QPaintEvent *) {
    QStyleOption opt;
    opt.init(this);
    QPainter painter(this);

    painter.setRenderHint(QPainter::Antialiasing);
    painter.fillRect(this->rect(), QBrush(QColor(48, 47, 47)));
}
