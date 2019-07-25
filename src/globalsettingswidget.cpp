/*!
 * \copyright
 * Copyright (C) 2015 - 2019.
 * Released under the GNU General Public License.
 */

#include <QStyleOption>
#include <QtCore>
#include <QtGui>

#include "globalsettingswidget.h"
#include "utils/qt.h"

GlobalSettingsWidget::GlobalSettingsWidget(QWidget* parent, AppSettings* appSettings)
    : QWidget(parent), mAppSettings(appSettings) {
    mSpacerPixels = 5;

    // set margins as spacer * 2
    this->setContentsMargins(
        mSpacerPixels * 2, mSpacerPixels * 2, mSpacerPixels * 2, mSpacerPixels * 2);

    //-----------
    // Labels
    //-----------
    mEnabledConnectionsLabel = new QLabel("Enabled Hardware", this);

    const QString labelStyleSheet = "font:bold; font-size:20pt; background-color: rgba(0,0,0,0);";
    const QString transparentStyleSheet = "background-color: rgba(0,0,0,0)";

    mEnabledConnectionsLabel->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    mEnabledConnectionsLabel->setStyleSheet(labelStyleSheet);


    mTimeoutWidget = new TimeoutWidget(
        this, mAppSettings->timeout(), mAppSettings->timeoutEnabled(), mSpacerPixels);

    mMinHeight = this->height() / 8;

    //-----------
    // Enabled Connections
    //-----------

    mArduCorButton = new QPushButton(this);
    mArduCorButton->setCheckable(true);
    mArduCorButton->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    connect(mArduCorButton, SIGNAL(clicked(bool)), this, SLOT(arduCorButtonClicked(bool)));
    mArduCorButton->setText("ArduCor");

    mHueButton = new QPushButton(this);
    mHueButton->setCheckable(true);
    mHueButton->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    connect(mHueButton, SIGNAL(clicked(bool)), this, SLOT(hueCheckboxClicked(bool)));
    mHueButton->setText("Hue");

    mNanoLeafButton = new QPushButton(this);
    mNanoLeafButton->setCheckable(true);
    mNanoLeafButton->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    connect(mNanoLeafButton, SIGNAL(clicked(bool)), this, SLOT(nanoLeafButtonClicked(bool)));
    mNanoLeafButton->setText("NanoLeaf");

    mConnectionButtons = {mArduCorButton, mHueButton, mNanoLeafButton};
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
    if (mAppSettings->enabled(EProtocolType::hue)) {
        mHueButton->setChecked(true);
    } else {
        mHueButton->setChecked(false);
    }

    if (mAppSettings->enabled(EProtocolType::arduCor)) {
        mArduCorButton->setChecked(true);
    } else {
        mArduCorButton->setChecked(false);
    }

    if (mAppSettings->enabled(EProtocolType::nanoleaf)) {
        mNanoLeafButton->setChecked(true);
    } else {
        mNanoLeafButton->setChecked(false);
    }
}


void GlobalSettingsWidget::timeoutChanged(int newTimeout) {
    emit timeoutUpdate(newTimeout);
}



void GlobalSettingsWidget::checkBoxClicked(EProtocolType type, bool checked) {
    bool successful = mAppSettings->enable(type, checked);
    if (!successful) {
        mConnectionButtons[uint32_t(type)]->setChecked(true);
        // mConnectionButtons[mAppSettings->indexOfProtocolSettings(type)]->setStyleSheet("background-color:#4A4949;");
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
    //   mTimeoutSlider->setSliderColorBackground(QColor(255, 127, 0));
}

void GlobalSettingsWidget::show() {
    checkCheckBoxes();
    resize();
}

void GlobalSettingsWidget::paintEvent(QPaintEvent*) {
    QStyleOption opt;
    opt.init(this);
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);

    QPainterPath path;
    path.addRoundedRect(this->rect(), 10, 10);
    // painter.setPen(QPen(Qt::black, 10));
    painter.fillPath(path, QColor(48, 47, 47));
    painter.drawPath(path);
}

void GlobalSettingsWidget::timeoutCheckboxPressed(bool isChecked) {
    emit timeoutEnabled(isChecked);
    mTimeoutWidget->show(isChecked);
    if (isChecked) {
        mTimeoutWidget->changeTimeoutLabel("Timeout Enabled");
    } else {
        mTimeoutWidget->changeTimeoutLabel("Timeout Disabled");
    }
    resize();
}

int GlobalSettingsWidget::timeoutValue() {
    return mTimeoutWidget->timeoutValue();
}

void GlobalSettingsWidget::resize() {
    int currentY = mTimeoutWidget->resize(mEnabledConnectionsLabel->height());

    if (mEnabledConnectionsLabel->isVisible()) {
        mEnabledConnectionsLabel->setGeometry(mSpacerPixels,
                                              currentY,
                                              mEnabledConnectionsLabel->width(),
                                              mEnabledConnectionsLabel->height());
        currentY += mEnabledConnectionsLabel->height() + mSpacerPixels;
    }

    auto buttonHeight = int(this->width() * 0.2f);
    auto buttonWidth = int(this->width() * 0.2f);
    const auto& appSize = cor::applicationSize();
    auto ratio = float(appSize.height()) / appSize.width();
    if (ratio > 1.3f) {
        buttonWidth = int(this->width() * 0.3f);
    }

    mHueButton->setGeometry(mSpacerPixels, currentY, buttonWidth, buttonHeight);

    mNanoLeafButton->setGeometry(mHueButton->geometry().x() + mHueButton->width() + mSpacerPixels,
                                 currentY,
                                 buttonWidth,
                                 buttonHeight);

    mArduCorButton->setGeometry(
        mNanoLeafButton->geometry().x() + mNanoLeafButton->width() + mSpacerPixels,
        currentY,
        buttonWidth,
        buttonHeight);

    currentY += mHueButton->height() + 2 * mSpacerPixels;

    if (mTimeoutWidget->isTimoutEnabled()) {
        currentY += mSpacerPixels;
    }

    this->setFixedHeight(currentY);
    this->parentWidget()->adjustSize();
}
