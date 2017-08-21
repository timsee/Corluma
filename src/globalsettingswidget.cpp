/*!
 * \copyright
 * Copyright (C) 2015 - 2017.
 * Released under the GNU General Public License.
 */

#include <QtCore>
#include <QtGui>
#include <QStyleOption>

#include "corlumautils.h"
#include "globalsettingswidget.h"

GlobalSettingsWidget::GlobalSettingsWidget(QWidget *parent) : QWidget(parent) {

    this->setContentsMargins(10, 10, 10, 10);

    //-----------
    // Labels
    //-----------
    mEnabledConnectionsLabel = new QLabel("Enabled Connections", this);

    const QString labelStyleSheet = "font:bold; font-size:20pt; background-color: rgba(0,0,0,0);";
    const QString transparentStyleSheet = "background-color: rgba(0,0,0,0)";

    mEnabledConnectionsLabel->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    mEnabledConnectionsLabel->setStyleSheet(labelStyleSheet);

    mSpacerPixels = 5;

    //-----------
    // CheckBoxes
    //-----------

    mTimeoutCheckBox = new CorlumaCheckBox("Use Timeout", this);
    connect(mTimeoutCheckBox, SIGNAL(boxChecked(bool)), this, SLOT(timeoutButtonPressed(bool)));

    mAdvanceModeCheckBox = new CorlumaCheckBox("Advance Mode", this);
    connect(mAdvanceModeCheckBox, SIGNAL(boxChecked(bool)), this, SLOT(advanceModeButtonPressed(bool)));
    mAdvanceModeCheckBox->setGeometry(mTimeoutCheckBox->geometry().width() + mSpacerPixels,
                                      mTimeoutCheckBox->geometry().y(),
                                      mTimeoutCheckBox->geometry().width(),
                                      mTimeoutCheckBox->geometry().height());

    mMinHeight = mTimeoutCheckBox->height();

    //-----------
    // Sliders
    //-----------

    mSliderSpeedValue = 425;
    mSpeedSlider = new CorlumaSlider(this);
    mSpeedSlider->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    mSpeedSlider->slider->setRange(1, 1000);
    mSpeedSlider->slider->setValue(mSliderSpeedValue);
    mSpeedSlider->setSliderHeight(0.5f);
    mSpeedSlider->slider->setTickPosition(QSlider::TicksBelow);
    mSpeedSlider->slider->setTickInterval(100);

    mTimeoutSlider = new CorlumaSlider(this);
    mTimeoutSlider->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    mTimeoutSlider->slider->setRange(0,240);
    mTimeoutSlider->slider->setValue(120);
    mTimeoutSlider->setSliderHeight(0.5f);
    mTimeoutSlider->slider->setTickPosition(QSlider::TicksBelow);
    mTimeoutSlider->slider->setTickInterval(40);

    //-----------
    // Slider Labels
    //-----------

    mSpeedLabel = new QLabel("Speed", this);
    mSpeedLabel->setStyleSheet(transparentStyleSheet);
    mSpeedLabel->setMinimumHeight(mSpeedLabel->height() * 2);
    mSpeedLabel->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);

    mTimeoutLabel = new QLabel("Timeout", this);
    mTimeoutLabel->setStyleSheet(transparentStyleSheet);
    mTimeoutLabel->setMinimumHeight(mTimeoutLabel->height() * 2);
    mTimeoutLabel->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);

    //-----------
    // Enabled Connections
    //-----------

    mYunButton    = new QPushButton(this);
    mYunButton->setCheckable(true);
    mYunButton->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);

    mSerialButton = new QPushButton(this);
    mSerialButton->setCheckable(true);
    mSerialButton->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);

    mHueButton    = new QPushButton(this);
    mHueButton->setCheckable(true);
    mHueButton->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);

    mConnectionButtons = { mYunButton,
#ifndef MOBILE_BUILD
                    mSerialButton,
#endif //MOBILE_BUILD
                    mHueButton};

    connect(mSpeedSlider, SIGNAL(valueChanged(int)), this, SLOT(speedChanged(int)));
    connect(mTimeoutSlider, SIGNAL(valueChanged(int)), this, SLOT(timeoutChanged(int)));

    connect(mHueButton, SIGNAL(clicked(bool)), this, SLOT(hueCheckboxClicked(bool)));
    mHueButton->setText("Hue");

    connect(mYunButton, SIGNAL(clicked(bool)), this, SLOT(yunCheckboxClicked(bool)));
    mYunButton->setText("Yun");

#ifndef MOBILE_BUILD
    connect(mSerialButton, SIGNAL(clicked(bool)), this, SLOT(serialCheckboxClicked(bool)));
    mSerialButton->setText("Serial");
#else
    mSerialButton->setHidden(true);
#endif //MOBILE_BUILD

    //-----------
    // Access Persistent Memory
    //-----------

    // check check boxes if they were checked previously
    mSettings = new QSettings();
    bool useAdvanceMode = mSettings->value(kAdvanceModeKey).toBool();
    mAdvanceModeCheckBox->setChecked(useAdvanceMode);
    showAdvanceMode(useAdvanceMode);

    bool useTimeout = mSettings->value(kUseTimeoutKey).toBool();
    mTimeoutCheckBox->setChecked(useTimeout);
    showTimeout(useTimeout);
}


void GlobalSettingsWidget::hueCheckboxClicked(bool checked) {
    checkBoxClicked(ECommType::eHue, checked);
}

void GlobalSettingsWidget::yunCheckboxClicked(bool checked) {
    checkBoxClicked(ECommType::eUDP, checked);
}

void GlobalSettingsWidget::serialCheckboxClicked(bool checked) {
#ifndef MOBILE_BUILD
    checkBoxClicked(ECommType::eSerial, checked);
#else
    Q_UNUSED(checked);
#endif //MOBILE_BUILD
}

void GlobalSettingsWidget::deviceCountReachedZero() {
    mSpeedSlider->enable(false);
    mTimeoutSlider->enable(false);

    mSpeedSlider->setSliderColorBackground(QColor(150, 150, 150));
    mTimeoutSlider->setSliderColorBackground(QColor(150, 150, 150));
}


void GlobalSettingsWidget::checkCheckBoxes() {
    std::vector<ECommType> types =  mData->commTypeSettings()->commTypes();
    for (uint32_t i = 0; i < types.size(); ++i) {
        QPushButton* checkBox = mConnectionButtons[i];
        ECommType type = types[i];
        if (mData->commTypeSettings()->commTypeEnabled(type)) {
            checkBox->setChecked(true);
           // checkBox->setStyleSheet("background-color:#4A4949;");
        } else {
            checkBox->setChecked(false);
           // checkBox->setStyleSheet("background-color: #000000;");
        }
    }
}

void GlobalSettingsWidget::speedChanged(int newSpeed) {
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

void GlobalSettingsWidget::timeoutChanged(int newTimeout) {
   mData->updateTimeout(newTimeout);
}



void GlobalSettingsWidget::checkBoxClicked(ECommType type, bool checked) {
    bool successful = mData->commTypeSettings()->enableCommType(type, checked);
    if (type == ECommType::eUDP) {
        mData->commTypeSettings()->enableCommType(ECommType::eHTTP, checked);
    }
    if (!successful) {
        mConnectionButtons[mData->commTypeSettings()->indexOfCommTypeSettings(type)]->setChecked(true);
       // mConnectionButtons[mData->commTypeSettings()->indexOfCommTypeSettings(type)]->setStyleSheet("background-color:#4A4949;");
    }

    if (checked) {
        mComm->startup(type);
        if (type == ECommType::eUDP) {
            mComm->startup(ECommType::eHTTP);
        }
    } else {
        mComm->shutdown(type);
        mData->removeDevicesOfType(type);
        if (type == ECommType::eUDP) {
            mComm->shutdown(ECommType::eHTTP);
            mData->removeDevicesOfType(ECommType::eHTTP);
        }

        // update the UI accordingly
        if (type == ECommType::eHue) {
            mHueButton->setChecked(false);
        } else if (type == ECommType::eUDP) {
            mYunButton->setChecked(false);
#ifndef MOBILE_BUILD
        } else if (type == ECommType::eSerial) {
            mSerialButton->setChecked(false);
#endif //MOBILE_BUILD
        }
    }
}

void GlobalSettingsWidget::updateUI() {
    if (mData->currentRoutine() <= ELightingRoutine::eSingleSawtoothFadeOut) {
        mSpeedSlider->setSliderColorBackground(mData->mainColor());
        mTimeoutSlider->setSliderColorBackground(mData->mainColor());
    } else {
        mSpeedSlider->setSliderColorBackground(mData->colorsAverage(mData->currentColorGroup()));
        mTimeoutSlider->setSliderColorBackground(mData->colorsAverage(mData->currentColorGroup()));
    }
}

void GlobalSettingsWidget::show() {
    checkCheckBoxes();

    if (mData->currentDevices().size() == 0) {
        deviceCountReachedZero();

    } else {
        mSpeedSlider->enable(true);
        mTimeoutSlider->enable(true);

        mSpeedSlider->setSliderColorBackground(mData->mainColor());
        mTimeoutSlider->setSliderColorBackground(mData->mainColor());

        // default the settings bars to the current colors
        mSpeedSlider->slider->setValue(mSliderSpeedValue);
        mTimeoutSlider->slider->setValue(mData->timeout());
    }
    resize();
}

void GlobalSettingsWidget::resizeEvent(QResizeEvent *event) {
    Q_UNUSED(event);
}


void GlobalSettingsWidget::paintEvent(QPaintEvent *) {
    QStyleOption opt;
    opt.init(this);
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);

    QPainterPath path;
    path.addRoundedRect(this->rect(), 10, 10);
    //painter.setPen(QPen(Qt::black, 10));
    painter.fillPath(path, QColor(48, 47, 47));
    painter.drawPath(path);
}

void GlobalSettingsWidget::advanceModeButtonPressed(bool isChecked) {
    showAdvanceMode(isChecked);
    mData->commTypeSettings()->commTypes();
    // turn off advanced comm streams
    if (!isChecked) {
        std::vector<ECommType> usedTypes = mData->commTypeSettings()->commTypes();
        for (uint32_t x = 0; x < usedTypes.size(); ++x) {
            if (usedTypes[x] == ECommType::eUDP) {
                if (mData->commTypeSettings()->commTypeEnabled(usedTypes[x])) {
                    checkBoxClicked(ECommType::eUDP, false);
                }
            } else if (usedTypes[x] != ECommType::eHue){
                if (mData->commTypeSettings()->commTypeEnabled(usedTypes[x])) {
                    checkBoxClicked(usedTypes[x], false);
                }
            }

        }
    }
    mSettings->setValue(kAdvanceModeKey, QString::number((int)isChecked));
}

void GlobalSettingsWidget::timeoutButtonPressed(bool isChecked) {
    mSettings->setValue(kUseTimeoutKey, QString::number((int)isChecked));
    showTimeout(isChecked);
}


void GlobalSettingsWidget::showTimeout(bool showTimeout) {
    mTimeoutLabel->setVisible(showTimeout);
    mTimeoutSlider->setVisible(showTimeout);
    resize();
}

void GlobalSettingsWidget::showAdvanceMode(bool showAdvanceMode) {
    mSpeedSlider->setVisible(showAdvanceMode);
    mSpeedLabel->setVisible(showAdvanceMode);

    mEnabledConnectionsLabel->setVisible(showAdvanceMode);

    mHueButton->setVisible(showAdvanceMode);
    mYunButton->setVisible(showAdvanceMode);
#ifndef MOBILE_BUILD
    mSerialButton->setVisible(showAdvanceMode);
#endif //MOBILE_BUILD
    resize();
}

void GlobalSettingsWidget::resize() {
    uint32_t currentY = 0;
    mSliderMinWidth = this->width() * 0.66f;

    mAdvanceModeCheckBox->setGeometry(mTimeoutCheckBox->geometry().width() + mSpacerPixels,
                                      mTimeoutCheckBox->geometry().y(),
                                      mAdvanceModeCheckBox->geometry().width(),
                                      mTimeoutCheckBox->geometry().height());

    currentY += mTimeoutCheckBox->height() + mSpacerPixels;

    if (mTimeoutSlider->isVisible()) {
        mTimeoutLabel->setGeometry(mSpacerPixels,
                                 currentY,
                                 mTimeoutLabel->width(),
                                 mTimeoutLabel->height());
        mTimeoutSlider->setGeometry(mTimeoutLabel->geometry().width() + 2 * mSpacerPixels,
                                  currentY,
                                  mSliderMinWidth,
                                  mTimeoutLabel->height());
        currentY += mTimeoutLabel->height() + mSpacerPixels;
    }

    if (mSpeedSlider->isVisible()) {
        int width = mSpeedLabel->width();
        if (mTimeoutLabel->isVisible()) {
            width = mTimeoutLabel->width();
        }
        mSpeedLabel->setGeometry(mSpacerPixels,
                                 currentY,
                                 width,
                                 mSpeedLabel->height());
        mSpeedSlider->setGeometry(mSpeedLabel->geometry().width() + 2 * mSpacerPixels,
                                  currentY,
                                  mSliderMinWidth,
                                  mSpeedLabel->height());
        currentY += mSpeedLabel->height() + mSpacerPixels;
    }

    if (mEnabledConnectionsLabel->isVisible()) {
        mEnabledConnectionsLabel->setGeometry(mSpacerPixels,
                                              currentY,
                                              mEnabledConnectionsLabel->width(),
                                              mEnabledConnectionsLabel->height());
        currentY += mEnabledConnectionsLabel->height() + mSpacerPixels;
    }

    if (mHueButton->isVisible()) {
        mHueButton->setGeometry(mSpacerPixels,
                                currentY,
                                mSpeedLabel->width(),
                                mSpeedLabel->width());

        mYunButton->setGeometry(mHueButton->geometry().x() + mHueButton->width() + mSpacerPixels,
                                currentY,
                                mSpeedLabel->width(),
                                mSpeedLabel->width());
#ifndef MOBILE_BUILD
        mSerialButton->setGeometry(mYunButton->geometry().x() + mYunButton->width() + mSpacerPixels,
                                currentY,
                                mSpeedLabel->width(),
                                mSpeedLabel->width());
#endif //MOBILE_BUILD

        currentY += mHueButton->height() + 2 * mSpacerPixels;
    }

    this->setMinimumHeight(currentY);
    this->setMaximumHeight(currentY);
    this->parentWidget()->adjustSize();
}