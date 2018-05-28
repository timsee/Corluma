/*!
 * \copyright
 * Copyright (C) 2015 - 2018.
 * Released under the GNU General Public License.
 */

#include <QSignalMapper>

#include "palettewidget.h"
#include "cor/utils.h"

namespace cor
{

PaletteWidget::PaletteWidget(uint32_t width, uint32_t height,
                             std::vector<std::vector<QColor> > colorGroups,
                             EPaletteWidgetType type,
                             QWidget *parent) : QWidget(parent) {
    mWidth = width;
    mHeight = height;
    mMaximumSize = width * height;
    mColorGroups = colorGroups;
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
            cor::Light light;
            light.routine = ERoutine::eSingleSolid;
            light.color = QColor(0,0,0);
            QJsonObject routineObject = lightToJson(light);
            mArrayColorsButtons[i] = new cor::Button(routineObject,
                                                     std::vector<QColor>(),
                                                     this);
            mArrayColorsButtons[i]->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
            arrayButtonsMapper->setMapping(mArrayColorsButtons[i], i);
            connect(mArrayColorsButtons[i], SIGNAL(clicked(bool)), arrayButtonsMapper, SLOT(map()));

            QSizePolicy sizePolicy = mArrayColorsButtons[i]->sizePolicy();
            sizePolicy.setRetainSizeWhenHidden(true);
            mArrayColorsButtons[i]->setSizePolicy(sizePolicy);

            if (mType == EPaletteWidgetType::eInfo) {
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
    connect(arrayButtonsMapper, SIGNAL(mapped(int)), this, SLOT(selectArrayColor(int)));
    setLayout(mLayout);

    updateDevices(std::list<cor::Light>());
}

void PaletteWidget::updateDevices(const std::list<cor::Light>& devices) {
    uint32_t i = 0;
    for (auto&& device : devices) {
        if (i < mMaximumSize) {
            QJsonObject routineObject = lightToJson(device);
            mArrayColorsButtons[i]->updateRoutine(routineObject, mColorGroups[(int)device.palette]);
            mArrayColorsButtons[i]->setVisible(true);
            if (mType == EPaletteWidgetType::eInfo) {
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

void PaletteWidget::selectArrayColor(int index) {
    // check if new index is already in list
    auto result = std::find(mSelectedIndices.begin(), mSelectedIndices.end(), index);
    if (result != mSelectedIndices.end()) {
        // if is found, remove it and deselect, but only if it isn't the only selected object
        mSelectedIndices.remove(index);
    } else {
        // if it isn't found, add it and select
        mSelectedIndices.push_back(index);
    }
    emit selectedCountChanged(mSelectedIndices.size());
    manageMultiSelected();
}


void PaletteWidget::manageMultiSelected() {
    // now do the GUI work for highlighting...
    // deselect all, we'll be reselecting in here!
    for (uint32_t i = 0; i < mMaximumSize; ++i) {
        mArrayColorsButtons[i]->setChecked(false);
    }
    for (auto index : mSelectedIndices) {
        mArrayColorsButtons[index]->setChecked(true);
    }
}


void PaletteWidget::enableButtonInteraction(bool enable) {
    for (uint32_t i = 0; i < mArrayColorsButtons.size(); ++i) {
        mArrayColorsButtons[i]->setAttribute(Qt::WA_TransparentForMouseEvents, !enable);
    }
}

}
