/*!
 * \copyright
 * Copyright (C) 2015 - 2019.
 * Released under the GNU General Public License.
 */

#include "lightvectorwidget.h"

#include "utils/qt.h"

namespace cor {

LightVectorWidget::LightVectorWidget(int width, int height, bool fillFromLeft, QWidget* parent)
    : QWidget(parent) {
    mWidth = width;
    mHeight = height;
    mMaximumSize = width * height;
    mFillFromLeft = fillFromLeft;

    // --------------
    // Setup Layout
    // --------------

    mLayout = new QGridLayout;
    mLayout->setContentsMargins(0, 0, 0, 0);
    mLayout->setHorizontalSpacing(0);

    // --------------
    // Setup Array Color Buttons
    // --------------

    mArrayColorsButtons = std::vector<cor::Button*>(std::uint32_t(mMaximumSize), nullptr);
    mArrayLabels = std::vector<QLabel*>(std::uint32_t(mMaximumSize), nullptr);
    int i = 0;
    for (int h = 0; h < mHeight; ++h) {
        for (int w = 0; w < mWidth; ++w) {
            QString iString(i);
            cor::Light light(iString, iString, ECommType::MAX);
            light.routine = ERoutine::singleSolid;
            light.color = QColor(0, 0, 0);
            QJsonObject routineObject = lightToJson(light);
            mArrayColorsButtons[std::size_t(i)] = new cor::Button(this, routineObject);
            mArrayColorsButtons[std::size_t(i)]->setLabelMode(true);
            mArrayColorsButtons[std::size_t(i)]->setSizePolicy(QSizePolicy::Fixed,
                                                               QSizePolicy::Fixed);
            connect(mArrayColorsButtons[std::size_t(i)],
                    SIGNAL(clicked()),
                    this,
                    SLOT(toggleArrayColor()));

            QSizePolicy sizePolicy = mArrayColorsButtons[std::size_t(i)]->sizePolicy();
            sizePolicy.setRetainSizeWhenHidden(true);
            mArrayColorsButtons[std::size_t(i)]->setSizePolicy(sizePolicy);
            mArrayColorsButtons[std::size_t(i)]->setVisible(false);
            mLayout->addWidget(mArrayColorsButtons[std::size_t(i)], h, w);
            ++i;
        }
    }
    setLayout(mLayout);
}

void LightVectorWidget::updateDevices(const std::vector<cor::Light>& devices) {
    if (mFillFromLeft) {
        int i = 0;
        for (const auto& device : devices) {
            bool skip = mHideOffDevices && !device.isOn;
            if (i < mMaximumSize && !skip) {
                QJsonObject routineObject = lightToJson(device);
                mArrayColorsButtons[std::uint32_t(i)]->updateRoutine(routineObject);
                mArrayColorsButtons[std::uint32_t(i)]->setVisible(true);
            }
            ++i;
        }
        for (; i < mMaximumSize; ++i) {
            mArrayColorsButtons[std::uint32_t(i)]->setVisible(false);
        }
    } else {
        int i = mMaximumSize - 1;
        for (const auto& device : devices) {
            bool skip = mHideOffDevices && !device.isOn;
            if (i > 0 && !skip) {
                QJsonObject routineObject = lightToJson(device);
                mArrayColorsButtons[std::uint32_t(i)]->updateRoutine(routineObject);
                mArrayColorsButtons[std::uint32_t(i)]->setVisible(true);
            }
            --i;
        }
        for (; i > 0; --i) {
            mArrayColorsButtons[std::uint32_t(i)]->setVisible(false);
        }
    }
}

uint32_t LightVectorWidget::selectedCount() {
    uint32_t i = 0;
    for (auto widget : mArrayColorsButtons) {
        if (widget->isChecked()) {
            ++i;
        }
    }
    return i;
}

void LightVectorWidget::toggleArrayColor() {
    emit selectedCountChanged(int(selectedCount()));
}

void LightVectorWidget::enableButtonInteraction(bool enable) {
    for (auto&& button : mArrayColorsButtons) {
        button->setAttribute(Qt::WA_TransparentForMouseEvents, !enable);
    }
}

void LightVectorWidget::resizeEvent(QResizeEvent*) {
    const QSize widgetSize(height() / mHeight, height() / mHeight);
    for (auto widget : mArrayColorsButtons) {
        widget->setFixedSize(widgetSize);
    }
}

} // namespace cor
