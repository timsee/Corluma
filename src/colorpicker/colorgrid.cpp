/*!
 * \copyright
 * Copyright (C) 2015 - 2017.
 * Released under the GNU General Public License.
 */

#include "colorgrid.h"
#include <QSignalMapper>

ColorGrid::ColorGrid(QWidget *parent) : QWidget(parent)
{
    mMaximumSize = 10;
    mColorsUsed = 2;

    mCountSlider = new CorlumaSlider(this);
    mCountSlider->setSliderColorBackground(QColor(0, 0, 0));
    mCountSlider->slider->setRange(0, mMaximumSize * 10);
    mCountSlider->slider->blockSignals(true);
    mCountSlider->slider->setValue(20);
    mCountSlider->slider->blockSignals(false);
    mCountSlider->slider->setTickInterval(10);
    mCountSlider->setSnapToNearestTick(true);
    mCountSlider->slider->setTickPosition(QSlider::TicksBelow);
    mCountSlider->setMinimumPossible(true, 20);
    mCountSlider->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    mCountSlider->setSliderHeight(0.6f);
    connect(mCountSlider, SIGNAL(valueChanged(int)), this, SLOT(countSliderChanged(int)));
    connect(mCountSlider->slider, SIGNAL(sliderReleased()), this, SLOT(releasedSlider()));

    // --------------
    // Setup Layout
    // --------------

    mLayout = new QGridLayout;
    int halfway = mMaximumSize / 2;
    mLayout->addWidget(mCountSlider, 1, 0, 1, halfway + 1);

    // --------------
    // Setup Array Color Buttons
    // --------------

    mArrayColorsButtons = std::vector<QPushButton*>(mMaximumSize, nullptr);
    QSignalMapper *arrayButtonsMapper = new QSignalMapper(this);
    int row = 2;
    int col = 0;
    for (uint32_t i = 0; i < mMaximumSize; ++i) {
        mArrayColorsButtons[i] = new QPushButton;
        mArrayColorsButtons[i]->setStyleSheet("border: none;");
        mArrayColorsButtons[i]->setCheckable(true);
        mArrayColorsButtons[i]->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
        arrayButtonsMapper->setMapping(mArrayColorsButtons[i], i);
        connect(mArrayColorsButtons[i], SIGNAL(clicked(bool)), arrayButtonsMapper, SLOT(map()));
        int size = std::min(this->size().width(), this->size().height());
        size = size * 0.8f;
        mArrayColorsButtons[i]->setIconSize(QSize(size, size));
        QPixmap pixmap = createSolidColorIcon(QColor(0,0,0));
        mArrayColorsButtons[i]->setIcon(QIcon(pixmap.scaled(size,
                                                            size,
                                                            Qt::IgnoreAspectRatio,
                                                            Qt::SmoothTransformation)));
        mLayout->addWidget(mArrayColorsButtons[i], row, col);
        col++;
        if (((i + 1) % halfway) == 0) {
            row++;
            col = 0;
        }

    }
    connect(arrayButtonsMapper, SIGNAL(mapped(int)), this, SLOT(selectArrayColor(int)));
    setLayout(mLayout);
}

void ColorGrid::updateSelected(QColor color) {
    for (uint32_t i = 0; i < mColors.size(); ++i) {
        // check if in selected
        bool isSelected = false;
        for (auto&& index : mSelectedIndices) {
            if (index == i) {
                isSelected = true;
            }
        }
        if (isSelected) {
            // update data
            mColors[i] = color;
            // update UI
            QPixmap icon = createSolidColorIcon(mColors[i]);
            mArrayColorsButtons[i]->setIcon(icon);
        }
    }
    updateMultiColorSlider();
}

void ColorGrid::updateMultiColor(const std::vector<QColor> colors, int count) {
    mColorsUsed = count;
    if (mColorsUsed < 2) {
        mColorsUsed = 2;
    }
    mColors = colors;
    QPixmap greyIcon = createSolidColorIcon(QColor(140,140,140));

    for (uint32_t i = 0; i < mColorsUsed; ++i) {
        mArrayColorsButtons[i]->setEnabled(true);
        QPixmap icon = createSolidColorIcon(mColors[i]);
        mArrayColorsButtons[i]->setIcon(icon);
    }

    for (uint32_t i = mColorsUsed; i < mMaximumSize; ++i) {
        mArrayColorsButtons[i]->setIcon(greyIcon);
        mArrayColorsButtons[i]->setEnabled(false);
    }

    updateMultiColorSlider();
    manageMultiSelected();
}


void ColorGrid::selectArrayColor(int index) {
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


void ColorGrid::updateMultiColorSlider() {
    int r = 0;
    int g = 0;
    int b = 0;
    for (uint32_t i = 0; i < mColorsUsed; ++i) {
        r = r + mColors[i].red();
        g = g + mColors[i].green();
        b = b + mColors[i].blue();
    }
    QColor average(r / mColorsUsed,
                   g / mColorsUsed,
                   b / mColorsUsed);
    mCountSlider->setSliderColorBackground(average);
}

void ColorGrid::manageMultiSelected() {
    // check all selected devices, store all that are higher than multi used count.
    std::list<uint32_t> indicesToRemove;
    for (auto index : mSelectedIndices) {
        if (index >= mColorsUsed) {
            // index cant be selected cause its more the max count, remove from selected indices
            indicesToRemove.push_back(index);
        }
    }

    // if indices should be removed, remove them
    if (indicesToRemove.size() > 0) {
        for (auto&& index : indicesToRemove) {
            mSelectedIndices.remove(index);
            emit selectedCountChanged(mSelectedIndices.size());
        }
    }

    // now do the GUI work for highlighting...
    // deselect all, we'll be reselecting in here!
    for (uint32_t i = 0; i < mMaximumSize; ++i) {
        mArrayColorsButtons[i]->setChecked(false);
    }
    for (auto index : mSelectedIndices) {
        mArrayColorsButtons[index]->setChecked(true);
    }
}


void ColorGrid::releasedSlider() {
    emit multiColorCountChanged(mCountSlider->slider->value() / 10);
}

QPixmap ColorGrid::createSolidColorIcon(QColor color) {
    int iconSize = std::min(this->size().width(), this->size().height()) * 0.8f;
    QSize size(iconSize, iconSize);
    QPixmap pixmap(size);
    pixmap.fill(color);
    return pixmap;
}

void ColorGrid::countSliderChanged(int newValue) {
    emit multiColorCountChanged(newValue / 10);
}


void ColorGrid::resizeEvent(QResizeEvent *) {
    for (uint32_t i = 0; i < mMaximumSize; ++i) {
        int size = this->size().width() / 6;
        size = size * 0.66f;
        mArrayColorsButtons[i]->setIconSize(QSize(size, size));
        QPixmap pixmap = mArrayColorsButtons[i]->icon().pixmap(QSize(size,size));
        mArrayColorsButtons[i]->setIcon(QIcon(pixmap.scaled(size,
                                                            size,
                                                            Qt::IgnoreAspectRatio,
                                                            Qt::SmoothTransformation)));
    }
}
