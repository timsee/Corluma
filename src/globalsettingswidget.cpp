/*!
 * \copyright
 * Copyright (C) 2015 - 2020.
 * Released under the GNU General Public License.
 */

#include "globalsettingswidget.h"

#include <QStyleOption>
#include <QtCore>
#include <QtGui>

#include "utils/qt.h"

GlobalSettingsWidget::GlobalSettingsWidget(QWidget* parent, AppSettings* appSettings)
    : QWidget(parent),
      mAppSettings(appSettings) {
    //-----------
    // Labels
    //-----------
    mEnabledConnectionsLabel = new QLabel("Enabled Hardware", this);

    const QString labelStyleSheet = "font:bold; font-size:20pt; background-color: rgba(0,0,0,0);";

    mEnabledConnectionsLabel->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    mEnabledConnectionsLabel->setStyleSheet(labelStyleSheet);

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
    mAppSettings->updateTimeout(newTimeout);
}


void GlobalSettingsWidget::checkBoxClicked(EProtocolType type, bool checked) {
    bool successful = mAppSettings->enable(type, checked);
    if (!successful) {
        mConnectionButtons[std::uint32_t(type)]->setChecked(true);
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
    opt.initFrom(this);
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);

    QPainterPath path;
    auto width = this->width() * 0.01;
    path.addRoundedRect(rect(), width, width);
    painter.fillPath(path, QColor(48, 47, 47));
    painter.setPen(QPen(QColor(200, 200, 200), this->width() * 0.005));
    painter.drawPath(path);
}

void GlobalSettingsWidget::timeoutCheckboxPressed(bool isChecked) {
    mAppSettings->enableTimeout(isChecked);
    resize();
}

void GlobalSettingsWidget::resize() {
    auto spacer = height() * 0.02;
    int currentY = spacer;

    if (mEnabledConnectionsLabel->isVisible()) {
        mEnabledConnectionsLabel->setGeometry(spacer * 3,
                                              currentY,
                                              mEnabledConnectionsLabel->width(),
                                              mEnabledConnectionsLabel->height());
        currentY += mEnabledConnectionsLabel->height() + spacer;
    }

    auto buttonSide = (height() - currentY) * 0.8;
    buttonSide = std::min(buttonSide, width() * 0.25);

    mHueButton->setGeometry(spacer * 3, currentY, buttonSide, buttonSide);

    mNanoLeafButton->setGeometry(mHueButton->geometry().x() + mHueButton->width() + spacer,
                                 currentY,
                                 buttonSide,
                                 buttonSide);

    mArduCorButton->setGeometry(mNanoLeafButton->geometry().x() + mNanoLeafButton->width() + spacer,
                                currentY,
                                buttonSide,
                                buttonSide);
}
