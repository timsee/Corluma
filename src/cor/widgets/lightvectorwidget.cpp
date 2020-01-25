/*!
 * \copyright
 * Copyright (C) 2015 - 2020.
 * Released under the GNU General Public License.
 */

#include "lightvectorwidget.h"

#include "utils/qt.h"

namespace cor {

LightVectorWidget::LightVectorWidget(std::uint32_t width,
                                     std::uint32_t height,
                                     bool fillFromLeft,
                                     QWidget* parent)
    : QWidget(parent),
      mFillFromLeft{fillFromLeft},
      mHideOffLights{false},
      mMaximumSize{width * height},
      mWidth{width},
      mHeight{height} {
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
    std::uint32_t i = 0;
    for (std::uint32_t h = 0; h < mHeight; ++h) {
        for (std::uint32_t w = 0; w < mWidth; ++w) {
            QString iString(i);
            cor::LightState state;
            state.routine(ERoutine::singleSolid);
            state.color(QColor(0, 0, 0));
            mArrayColorsButtons[i] = new cor::Button(this, state);
            mArrayColorsButtons[i]->setLabelMode(true);
            mArrayColorsButtons[i]->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
            connect(mArrayColorsButtons[std::size_t(i)],
                    SIGNAL(clicked()),
                    this,
                    SLOT(toggleArrayColor()));

            QSizePolicy sizePolicy = mArrayColorsButtons[std::size_t(i)]->sizePolicy();
            sizePolicy.setRetainSizeWhenHidden(true);
            mArrayColorsButtons[i]->setSizePolicy(sizePolicy);
            mArrayColorsButtons[i]->setVisible(false);
            mLayout->addWidget(mArrayColorsButtons[i], int(h), int(w));
            ++i;
        }
    }
    setLayout(mLayout);
}

void LightVectorWidget::updateLights(const std::vector<cor::Light>& lights) {
    if (mFillFromLeft) {
        std::uint32_t i = 0;
        for (const auto& light : lights) {
            auto state = light.state();
            if (i < mMaximumSize) {
                if (state.isOn() || !mHideOffLights) {
                    mArrayColorsButtons[i]->updateRoutine(state);
                    mArrayColorsButtons[i]->setVisible(true);
                    ++i;
                }
            }
        }
        for (; i < mMaximumSize; ++i) {
            mArrayColorsButtons[i]->setVisible(false);
        }
    } else {
        std::uint32_t i = mMaximumSize - 1;
        for (const auto& light : lights) {
            auto state = light.state();
            if (i > 0) {
                if (state.isOn() || !mHideOffLights) {
                    mArrayColorsButtons[i]->updateRoutine(state);
                    mArrayColorsButtons[i]->setVisible(true);
                    --i;
                }
            }
        }
        for (; i > 0; --i) {
            mArrayColorsButtons[i]->setVisible(false);
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
    for (const auto& button : mArrayColorsButtons) {
        button->setAttribute(Qt::WA_TransparentForMouseEvents, !enable);
    }
}

void LightVectorWidget::resizeEvent(QResizeEvent*) {
    const QSize widgetSize(height() / int(mHeight), height() / int(mHeight));
    for (auto widget : mArrayColorsButtons) {
        widget->setFixedSize(widgetSize);
    }
}

} // namespace cor
