/*!
 * \copyright
 * Copyright (C) 2015 - 2018.
 * Released under the GNU General Public License.
 */

#include <QSignalMapper>

#include "lightvectorwidget.h"
#include "cor/utils.h"

namespace cor
{

LightVectorWidget::LightVectorWidget(uint32_t width, uint32_t height,
                             EPaletteWidgetType type,
                             QWidget *parent) : QWidget(parent) {
    mWidth = width;
    mHeight = height;
    mMaximumSize = width * height;
    mType = type;

    // --------------
    // Setup Layout
    // --------------

    mLayout = new QGridLayout;
    mLayout->setContentsMargins(0,0,0,0);
    mLayout->setHorizontalSpacing(0);

    // --------------
    // Setup Array Color Buttons
    // --------------

    mArrayColorsButtons = std::vector<cor::Button*>(mMaximumSize, nullptr);
    mArrayLabels = std::vector<QLabel*>(mMaximumSize, nullptr);
    QSignalMapper *arrayButtonsMapper = new QSignalMapper(this);
    uint32_t i = 0;
    for (uint32_t h = 0; h < mHeight; ++h) {
        for (uint32_t w = 0; w < mWidth; ++w) {
            cor::Light light(QString(i), ECommType::MAX);
            light.routine = ERoutine::singleSolid;
            light.color = QColor(0,0,0);
            QJsonObject routineObject = lightToJson(light);
            mArrayColorsButtons[i] = new cor::Button(this, routineObject);
            mArrayColorsButtons[i]->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
            arrayButtonsMapper->setMapping(mArrayColorsButtons[i], i);
            connect(mArrayColorsButtons[i], SIGNAL(clicked(bool)), arrayButtonsMapper, SLOT(map()));

            QSizePolicy sizePolicy = mArrayColorsButtons[i]->sizePolicy();
            sizePolicy.setRetainSizeWhenHidden(true);
            mArrayColorsButtons[i]->setSizePolicy(sizePolicy);

            if (mType == EPaletteWidgetType::info) {
                mArrayLabels[i]  = new QLabel(this);
                mArrayLabels[i]->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
                mLayout->addWidget(mArrayLabels[i],        h, w * 2);
                mLayout->addWidget(mArrayColorsButtons[i], h, w * 2 + 1);
            } else {
                mLayout->addWidget(mArrayColorsButtons[i], h, w);
            }
            ++i;
        }
    }
    connect(arrayButtonsMapper, SIGNAL(mapped(int)), this, SLOT(toggleArrayColor(int)));
    setLayout(mLayout);

    updateDevices(std::list<cor::Light>());
}

void LightVectorWidget::updateDevices(const std::list<cor::Light>& devices) {
    uint32_t i = 0;
    for (auto&& device : devices) {
        bool skip = mHideOffDevices && !device.isOn;
        if (i < mMaximumSize && !skip) {
            QJsonObject routineObject = lightToJson(device);
            mArrayColorsButtons[i]->updateRoutine(routineObject);
            mArrayColorsButtons[i]->setVisible(true);
            if (mType == EPaletteWidgetType::info) {
                if (device.name.length() > 11) {
                    QString shortenedName = device.name.mid(0, 8) + "...";
                    mArrayLabels[i]->setText(shortenedName);
                } else {
                    mArrayLabels[i]->setText(device.name);
                }
            }
        }
        ++i;
    }
    for (; i < mMaximumSize; ++i) {
        mArrayColorsButtons[i]->setVisible(false);
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

void LightVectorWidget::toggleArrayColor(int index) {
    Q_UNUSED(index);
    emit selectedCountChanged(selectedCount());
}

void LightVectorWidget::enableButtonInteraction(bool enable) {
    for (uint32_t i = 0; i < mArrayColorsButtons.size(); ++i) {
        mArrayColorsButtons[i]->setAttribute(Qt::WA_TransparentForMouseEvents, !enable);
    }
}

}
