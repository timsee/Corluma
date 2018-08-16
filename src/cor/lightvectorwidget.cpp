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

LightVectorWidget::LightVectorWidget(int width, int height,
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

    mArrayColorsButtons = std::vector<cor::Button*>(uint32_t(mMaximumSize), nullptr);
    mArrayLabels = std::vector<QLabel*>(uint32_t(mMaximumSize), nullptr);
    QSignalMapper *arrayButtonsMapper = new QSignalMapper(this);
    int i = 0;
    for (int h = 0; h < mHeight; ++h) {
        for (int w = 0; w < mWidth; ++w) {
            cor::Light light(QString(i), ECommType::MAX);
            light.routine = ERoutine::singleSolid;
            light.color = QColor(0,0,0);
            QJsonObject routineObject = lightToJson(light);
            mArrayColorsButtons[uint32_t(i)] = new cor::Button(this, routineObject);
            mArrayColorsButtons[uint32_t(i)]->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
            arrayButtonsMapper->setMapping(mArrayColorsButtons[uint32_t(i)], i);
            connect(mArrayColorsButtons[uint32_t(i)], SIGNAL(clicked(bool)), arrayButtonsMapper, SLOT(map()));

            QSizePolicy sizePolicy = mArrayColorsButtons[uint32_t(i)]->sizePolicy();
            sizePolicy.setRetainSizeWhenHidden(true);
            mArrayColorsButtons[uint32_t(i)]->setSizePolicy(sizePolicy);

            if (mType == EPaletteWidgetType::info) {
                mArrayLabels[uint32_t(i)]  = new QLabel(this);
                mArrayLabels[uint32_t(i)]->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
                mLayout->addWidget(mArrayLabels[uint32_t(i)],        h, w * 2);
                mLayout->addWidget(mArrayColorsButtons[uint32_t(i)], h, w * 2 + 1);
            } else {
                mLayout->addWidget(mArrayColorsButtons[uint32_t(i)], h, w);
            }
            ++i;
        }
    }
    connect(arrayButtonsMapper, SIGNAL(mapped(int)), this, SLOT(toggleArrayColor(int)));
    setLayout(mLayout);

    updateDevices(std::list<cor::Light>());
}

void LightVectorWidget::updateDevices(const std::list<cor::Light>& devices) {
    int i = 0;
    for (auto&& device : devices) {
        bool skip = mHideOffDevices && !device.isOn;
        if (i < mMaximumSize && !skip) {
            QJsonObject routineObject = lightToJson(device);
            mArrayColorsButtons[uint32_t(i)]->updateRoutine(routineObject);
            mArrayColorsButtons[uint32_t(i)]->setVisible(true);
            if (mType == EPaletteWidgetType::info) {
                if (device.name.length() > 11) {
                    QString shortenedName = device.name.mid(0, 8) + "...";
                    mArrayLabels[uint32_t(i)]->setText(shortenedName);
                } else {
                    mArrayLabels[uint32_t(i)]->setText(device.name);
                }
            }
        }
        ++i;
    }
    for (; i < mMaximumSize; ++i) {
        mArrayColorsButtons[uint32_t(i)]->setVisible(false);
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
    emit selectedCountChanged(int(selectedCount()));
}

void LightVectorWidget::enableButtonInteraction(bool enable) {
    for (uint32_t i = 0; i < mArrayColorsButtons.size(); ++i) {
        mArrayColorsButtons[i]->setAttribute(Qt::WA_TransparentForMouseEvents, !enable);
    }
}

}
