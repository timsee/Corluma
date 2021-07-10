/*!
 * \copyright
 * Copyright (C) 2015 - 2020.
 * Released under the GNU General Public License.
 */

#include "rgbsliders.h"
#include "cor/stylesheets.h"

#include <QDebug>
#include <QIntValidator>

RGBSliders::RGBSliders(QWidget* parent)
    : QWidget(parent),
      mRLineEdit{new QLineEdit(this)},
      mGLineEdit{new QLineEdit(this)},
      mBLineEdit{new QLineEdit(this)} {
    const QString lineEditStylesheet =
        "QLineEdit:!enabled { color: #fffefe;  background-color: #201F1F; }";
    mRLineEdit->setValidator(new QIntValidator(0, 255, this));
    mGLineEdit->setValidator(new QIntValidator(0, 255, this));
    mBLineEdit->setValidator(new QIntValidator(0, 255, this));
    mRLineEdit->setStyleSheet(cor::kDarkerGreyBackground);
    mGLineEdit->setStyleSheet(cor::kDarkerGreyBackground);
    mBLineEdit->setStyleSheet(cor::kDarkerGreyBackground);
    connect(mRLineEdit, SIGNAL(textEdited(QString)), this, SLOT(lineRedEditChanged(QString)));
    connect(mGLineEdit, SIGNAL(textEdited(QString)), this, SLOT(lineGreenEditChanged(QString)));
    connect(mBLineEdit, SIGNAL(textEdited(QString)), this, SLOT(lineBlueEditChanged(QString)));

    mRLineEdit->setEnabled(false);
    mGLineEdit->setEnabled(false);
    mBLineEdit->setEnabled(false);

    mRLineEdit->setAlignment(Qt::AlignHCenter);
    mGLineEdit->setAlignment(Qt::AlignHCenter);
    mBLineEdit->setAlignment(Qt::AlignHCenter);

    mRLineEdit->setStyleSheet(lineEditStylesheet);
    mGLineEdit->setStyleSheet(lineEditStylesheet);
    mBLineEdit->setStyleSheet(lineEditStylesheet);

    // --------------
    // Setup Sliders
    // --------------
    mRedSlider = new cor::Slider(this);
    mRedSlider->setColor(QColor(255, 0, 0));
    mRedSlider->setRange(0, 255);
    mRedSlider->setSnapToNearestTick(true);
    mRedSlider->setHeightPercentage(0.8f);
    mRedSlider->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    connect(mRedSlider, SIGNAL(valueChanged(int)), this, SLOT(redSliderChanged(int)));
    connect(mRedSlider, SIGNAL(sliderReleased()), this, SLOT(releasedSlider()));

    mGreenSlider = new cor::Slider(this);
    mGreenSlider->setColor(QColor(0, 255, 0));
    mGreenSlider->setRange(0, 255);
    mGreenSlider->setSnapToNearestTick(true);
    mGreenSlider->setHeightPercentage(0.8f);
    mGreenSlider->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    connect(mGreenSlider, SIGNAL(valueChanged(int)), this, SLOT(greenSliderChanged(int)));
    connect(mGreenSlider, SIGNAL(sliderReleased()), this, SLOT(releasedSlider()));

    mBlueSlider = new cor::Slider(this);
    mBlueSlider->setRange(0, 255);
    mBlueSlider->setColor(QColor(0, 0, 255));
    mBlueSlider->setSnapToNearestTick(true);
    mBlueSlider->setHeightPercentage(0.8f);
    mBlueSlider->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    connect(mBlueSlider, SIGNAL(valueChanged(int)), this, SLOT(blueSliderChanged(int)));
    connect(mBlueSlider, SIGNAL(sliderReleased()), this, SLOT(releasedSlider()));

    // --------------
    // Setup RGB Labels
    // --------------

    mRLabel = new QLabel(this);
    mRLabel->setText("R");
    mRLabel->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);

    mGLabel = new QLabel(this);
    mGLabel->setText("G");
    mGLabel->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);

    mBLabel = new QLabel(this);
    mBLabel->setText("B");
    mBLabel->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
}

void RGBSliders::changeColor(const QColor& color) {
    bool blocked = mRedSlider->blockSignals(true);
    mRedSlider->setValue(color.red());
    mRLineEdit->setText(QString::number(color.red()));
    mRedSlider->blockSignals(blocked);

    blocked = mGreenSlider->blockSignals(true);
    mGreenSlider->setValue(color.green());
    mGLineEdit->setText(QString::number(color.green()));
    mGreenSlider->blockSignals(blocked);

    blocked = mBlueSlider->blockSignals(true);
    mBlueSlider->setValue(color.blue());
    mBLineEdit->setText(QString::number(color.blue()));
    mBlueSlider->blockSignals(blocked);
}

std::uint32_t RGBSliders::brightness() {
    return std::uint32_t(color().valueF() * 100.0);
}

void RGBSliders::lineRedEditChanged(QString inputString) {
    auto newValue = handleTextInputString(mRLineEdit, inputString);
    auto newColor = QColor(newValue, color().green(), color().blue());
    changeColor(newColor);
    emit colorChanged(newColor);
}

void RGBSliders::lineGreenEditChanged(QString inputString) {
    auto newValue = handleTextInputString(mGLineEdit, inputString);
    auto newColor = QColor(color().red(), newValue, color().blue());
    changeColor(newColor);
    emit colorChanged(newColor);
}

void RGBSliders::lineBlueEditChanged(QString inputString) {
    auto newValue = handleTextInputString(mBLineEdit, inputString);
    auto newColor = QColor(color().red(), color().green(), newValue);
    changeColor(newColor);
    emit colorChanged(newColor);
}


int RGBSliders::handleTextInputString(QLineEdit* lineEdit, QString inputString) {
    int value = inputString.toInt();
    if (inputString.isEmpty() || value == 0) {
        lineEdit->setText("0");
        value = 0;
    }
    if (value > 255) {
        inputString.chop(1);
        lineEdit->setText(inputString);
        value = inputString.toInt();
    }
    if (value <= 255 && value > 0 && inputString.at(0) == '0') {
        inputString = inputString.remove(0, 1);
        lineEdit->setText(inputString);
    }
    return value;
}

void RGBSliders::enable(bool enable) {
    if (!enable) {
        bool blocked = mRedSlider->blockSignals(true);
        mRedSlider->setValue(0);
        mRedSlider->blockSignals(blocked);

        blocked = mGreenSlider->blockSignals(true);
        mGreenSlider->setValue(0);
        mGreenSlider->blockSignals(blocked);

        blocked = mBlueSlider->blockSignals(true);
        mBlueSlider->setValue(0);
        mBlueSlider->blockSignals(blocked);
    }
    setEnabled(enable);
}

void RGBSliders::resizeEvent(QResizeEvent*) {
    auto labelWidth = width() / 20;
    auto spacer = labelWidth / 2;
    auto textEditWidth = labelWidth * 3;
    auto startTextFieldX = labelWidth + spacer;
    auto startSliderX = startTextFieldX + textEditWidth + spacer;
    auto sliderSize = width() - startSliderX;
    auto sliderHeight = height() / 3;
    auto yPos = 0;


    mRLabel->setGeometry(0, yPos, labelWidth, sliderHeight);
    mRLineEdit->setGeometry(startTextFieldX, yPos, textEditWidth, sliderHeight);
    mRedSlider->setGeometry(startSliderX, yPos, sliderSize, sliderHeight);
    yPos += sliderHeight;

    mGLabel->setGeometry(0, yPos, labelWidth, sliderHeight);
    mGLineEdit->setGeometry(startTextFieldX, yPos, textEditWidth, sliderHeight);
    mGreenSlider->setGeometry(startSliderX, yPos, sliderSize, sliderHeight);
    yPos += sliderHeight;

    mBLabel->setGeometry(0, yPos, labelWidth, sliderHeight);
    mBLineEdit->setGeometry(startTextFieldX, yPos, textEditWidth, sliderHeight);
    mBlueSlider->setGeometry(startSliderX, yPos, sliderSize, sliderHeight);
}


void RGBSliders::redSliderChanged(int newValue) {
    emit colorChanged(QColor(newValue, color().green(), color().blue()));
}

void RGBSliders::greenSliderChanged(int newValue) {
    emit colorChanged(QColor(color().red(), newValue, color().blue()));
}

void RGBSliders::blueSliderChanged(int newValue) {
    emit colorChanged(QColor(color().red(), color().green(), newValue));
}

void RGBSliders::releasedSlider() {
    emit colorChanged(color());
}

QColor RGBSliders::color() {
    return QColor(mRedSlider->value(), mGreenSlider->value(), mBlueSlider->value());
}
