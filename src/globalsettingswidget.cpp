/*!
 * \copyright
 * Copyright (C) 2015 - 2018.
 * Released under the GNU General Public License.
 */

#include <QtCore>
#include <QtGui>
#include <QStyleOption>

#include "cor/utils.h"
#include "globalsettingswidget.h"

GlobalSettingsWidget::GlobalSettingsWidget(QWidget *parent,
                                           ProtocolSettings *protocols) :
                                           QWidget(parent),
                                           mProtocolSettings(protocols) {
    mSpacerPixels = 5;

    // set margins as spacer * 2
    this->setContentsMargins(mSpacerPixels * 2,
                             mSpacerPixels * 2,
                             mSpacerPixels * 2,
                             mSpacerPixels * 2);

    //-----------
    // Labels
    //-----------
    mEnabledConnectionsLabel = new QLabel("Enabled Hardware", this);

    const QString labelStyleSheet = "font:bold; font-size:20pt; background-color: rgba(0,0,0,0);";
    const QString transparentStyleSheet = "background-color: rgba(0,0,0,0)";

    mEnabledConnectionsLabel->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    mEnabledConnectionsLabel->setStyleSheet(labelStyleSheet);

    //-----------
    // CheckBoxes
    //-----------

    mTimeoutCheckBox = new cor::CheckBox(this, "Use Timeout");
    connect(mTimeoutCheckBox, SIGNAL(boxChecked(bool)), this, SLOT(timeoutButtonPressed(bool)));

    mMinHeight = mTimeoutCheckBox->height();

    mSettings = new QSettings();
    //-----------
    // Sliders
    //-----------
    mTimeoutSlider = new cor::Slider(this);
    mTimeoutSlider->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    mTimeoutSlider->slider()->setRange(0, 300);
    mTimeoutSlider->slider()->setValue(mSettings->value(kTimeoutValue).toInt());
    mTimeoutSlider->setSliderHeight(0.5f);
    mTimeoutSlider->slider()->setTickPosition(QSlider::TicksBelow);
    mTimeoutSlider->slider()->setTickInterval(60);
    mTimeoutSlider->slider()->setValue(120);
    mTimeoutSlider->setSliderColorBackground(cor::kDefaultColor);
    mTimeoutSlider->setShouldDrawTickLabels(true);

    //-----------
    // Slider Labels
    //-----------

    mTimeoutLabel = new QLabel("Timeout", this);
    mTimeoutLabel->setStyleSheet(transparentStyleSheet);
    mTimeoutLabel->setMinimumHeight(mTimeoutLabel->height() * 2);
    mTimeoutLabel->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);

    //-----------
    // Enabled Connections
    //-----------

    mArduCorButton    = new QPushButton(this);
    mArduCorButton->setCheckable(true);
    mArduCorButton->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);

    mHueButton        = new QPushButton(this);
    mHueButton->setCheckable(true);
    mHueButton->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);

    mNanoLeafButton    = new QPushButton(this);
    mNanoLeafButton->setCheckable(true);
    mNanoLeafButton->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);

    mConnectionButtons = {mArduCorButton, mHueButton, mNanoLeafButton};

    connect(mTimeoutSlider, SIGNAL(valueChanged(int)), this, SLOT(timeoutChanged(int)));

    connect(mHueButton, SIGNAL(clicked(bool)), this, SLOT(hueCheckboxClicked(bool)));
    mHueButton->setText("Hue");

    connect(mArduCorButton, SIGNAL(clicked(bool)), this, SLOT(arduCorButtonClicked(bool)));
    mArduCorButton->setText("ArduCor");

    connect(mNanoLeafButton, SIGNAL(clicked(bool)), this, SLOT(nanoLeafButtonClicked(bool)));
    mNanoLeafButton->setText("NanoLeaf");

    //-----------
    // Access Persistent Memory
    //-----------

    bool useTimeout = mSettings->value(kUseTimeoutKey).toBool();
    mTimeoutCheckBox->setChecked(useTimeout);
    showTimeout(useTimeout);
}


void GlobalSettingsWidget::hueCheckboxClicked(bool checked) {
    checkBoxClicked(EProtocolType::hue, checked);
}

void GlobalSettingsWidget::arduCorButtonClicked(bool checked) {
    checkBoxClicked(EProtocolType::arduCor, checked);
}

void GlobalSettingsWidget::nanoLeafButtonClicked(bool checked) {
    checkBoxClicked(EProtocolType::nanoleaf, checked);
}

void GlobalSettingsWidget::checkCheckBoxes() {
    if (mProtocolSettings->enabled(EProtocolType::hue)) {
        mHueButton->setChecked(true);
    } else {
        mHueButton->setChecked(false);
    }

    if (mProtocolSettings->enabled(EProtocolType::arduCor)) {
        mArduCorButton->setChecked(true);
    } else {
        mArduCorButton->setChecked(false);
    }

    if (mProtocolSettings->enabled(EProtocolType::nanoleaf)) {
        mNanoLeafButton->setChecked(true);
    } else {
        mNanoLeafButton->setChecked(false);
    }
}


void GlobalSettingsWidget::timeoutChanged(int newTimeout) {
   emit timeoutUpdate(newTimeout);
   mSettings->setValue(kTimeoutValue, QString::number(newTimeout));
}



void GlobalSettingsWidget::checkBoxClicked(EProtocolType type, bool checked) {
    bool successful = mProtocolSettings->enable(type, checked);
    if (!successful) {
        mConnectionButtons[(uint32_t)type]->setChecked(true);
       // mConnectionButtons[mProtocolSettings->indexOfProtocolSettings(type)]->setStyleSheet("background-color:#4A4949;");
    }

    if (!checked) {
        if (type == EProtocolType::arduCor) {
            mArduCorButton->setChecked(false);
        } else if (type == EProtocolType::hue) {
            mHueButton->setChecked(false);
        } else if (type == EProtocolType::nanoleaf) {
            mNanoLeafButton->setChecked(false);
        }
    }

    emit protocolSettingsUpdate(type, checked);
}

void GlobalSettingsWidget::updateUI() {
 //   mTimeoutSlider->setSliderColorBackground(cor::kDefaultColor);
}

void GlobalSettingsWidget::show() {
    checkCheckBoxes();

    mTimeoutSlider->enable(true);

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

void GlobalSettingsWidget::timeoutButtonPressed(bool isChecked) {
    mSettings->setValue(kUseTimeoutKey, QString::number((int)isChecked));
    emit timeoutEnabled(isChecked);
    showTimeout(isChecked);
}

int GlobalSettingsWidget::timeoutValue() {
    return mTimeoutSlider->slider()->value();
}


void GlobalSettingsWidget::showTimeout(bool showTimeout) {
    mTimeoutLabel->setVisible(showTimeout);
    mTimeoutSlider->setVisible(showTimeout);
    resize();
}

void GlobalSettingsWidget::resize() {
    
    // resize the checkboxes widths, if needed
    mTimeoutCheckBox->downsizeTextWidthToFit(this->width() * 0.45f);

    uint32_t currentY = 0;
    mSliderMinWidth = this->width() * 0.66f;
    int sliderHeight =  mEnabledConnectionsLabel->height() * 2.75f;

    mTimeoutCheckBox->setGeometry(mSpacerPixels,
                                  mSpacerPixels,
                                  mTimeoutCheckBox->geometry().width(),
                                  mTimeoutCheckBox->geometry().height());

    currentY += mTimeoutCheckBox->height() + mSpacerPixels;

    if (mTimeoutSlider->isVisible()) {
        mTimeoutLabel->setGeometry(mSpacerPixels,
                                 currentY,
                                 mTimeoutLabel->width(),
                                 sliderHeight);
        mTimeoutSlider->setGeometry(mTimeoutLabel->geometry().width() + 2 * mSpacerPixels,
                                  currentY,
                                  mSliderMinWidth,
                                  sliderHeight);
        currentY += mTimeoutSlider->height() + mSpacerPixels;
    }

    if (mEnabledConnectionsLabel->isVisible()) {
        mEnabledConnectionsLabel->setGeometry(mSpacerPixels,
                                              currentY,
                                              mEnabledConnectionsLabel->width(),
                                              mEnabledConnectionsLabel->height());
        currentY += mEnabledConnectionsLabel->height() + mSpacerPixels;
    }

    int buttonSize = this->width() * 0.2f;
    if (mHueButton->isVisible()) {
        mHueButton->setGeometry(mSpacerPixels,
                                currentY,
                                buttonSize,
                                buttonSize);

        mNanoLeafButton->setGeometry(mHueButton->geometry().x() + mHueButton->width() + mSpacerPixels,
                                currentY,
                                buttonSize,
                                buttonSize);

        mArduCorButton->setGeometry(mNanoLeafButton->geometry().x() + mNanoLeafButton->width() + mSpacerPixels,
                                currentY,
                                buttonSize,
                                buttonSize);

        currentY += mHueButton->height() + 2 * mSpacerPixels;
    }

    if (!mTimeoutSlider->isVisible())
    {
        currentY += mSpacerPixels;
    }

    this->setMinimumHeight(currentY);
    this->setMaximumHeight(currentY);
    this->parentWidget()->adjustSize();
}
