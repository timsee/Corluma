/*!
 * \copyright
 * Copyright (C) 2015 - 2019.
 * Released under the GNU General Public License.
 */

#include "swatchvectorwidget.h"

#include <QSignalMapper>

SwatchVectorWidget::SwatchVectorWidget(uint32_t width, uint32_t height, QWidget* parent)
    : QWidget(parent) {
    mWidth = width;
    mHeight = height;
    mMaximumSize = width * height;
    mColors = std::vector<QColor>(10, QColor(140, 140, 140));

    // --------------
    // Setup Layout
    // --------------

    mLayout = new QGridLayout;
    mLayout->setContentsMargins(0, 0, 0, 0);
    mLayout->setHorizontalSpacing(0);

    // --------------
    // Setup Array Color Buttons
    // --------------

    mSwatches = std::vector<QPushButton*>(mMaximumSize, nullptr);
    auto arrayButtonsMapper = new QSignalMapper(this);
    uint32_t i = 0;
    for (uint32_t h = 0; h < mHeight; ++h) {
        for (uint32_t w = 0; w < mWidth; ++w) {
            mSwatches[i] = new QPushButton(this);
            mSwatches[i]->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
            mSwatches[i]->setCheckable(false);
            arrayButtonsMapper->setMapping(mSwatches[i], int(i));
            connect(mSwatches[i], SIGNAL(clicked(bool)), arrayButtonsMapper, SLOT(map()));

            QSizePolicy sizePolicy = mSwatches[i]->sizePolicy();
            sizePolicy.setRetainSizeWhenHidden(true);
            mSwatches[i]->setSizePolicy(sizePolicy);

            mLayout->addWidget(mSwatches[i], int(h), int(w));
            ++i;
        }
    }
    connect(arrayButtonsMapper, SIGNAL(mapped(int)), this, SLOT(toggleArrayColor(int)));
    setLayout(mLayout);
}

void SwatchVectorWidget::updateColors(const std::vector<QColor>& colors) {
    mColors = colors;

    uint32_t i = 0;
    for (const auto& color : colors) {
        if (i < mMaximumSize) {
            int size =
                std::min(int(mSwatches[i]->width() * 0.8f), int(mSwatches[i]->height() * 0.8f));
            QImage image(size, size, QImage::Format_RGB32);
            image.fill(color);
            mSwatches[i]->setIcon(QIcon(QPixmap::fromImage(image)));
            mSwatches[i]->setIconSize(QSize(size, size));
        }
        ++i;
    }

    for (; i < mMaximumSize; ++i) {
        int size = std::min(int(mSwatches[i]->width() * 0.8f), int(mSwatches[i]->height() * 0.8f));
        QImage image(size, size, QImage::Format_RGB32);
        image.fill(QColor(140, 140, 140));
        mSwatches[i]->setIcon(QIcon(QPixmap::fromImage(image)));
        mSwatches[i]->setIconSize(QSize(size, size));
    }
}

uint32_t SwatchVectorWidget::selectedCount() {
    uint32_t i = 0;
    for (const auto& widget : mSwatches) {
        if (widget->isChecked()) {
            ++i;
        }
    }
    return i;
}

void SwatchVectorWidget::toggleArrayColor(int) {
    emit selectedCountChanged(int(selectedCount()));
}

void SwatchVectorWidget::updateSelected(const QColor& color) {
    for (uint32_t i = 0; i < mSwatches.size(); ++i) {
        if (mSwatches[i]->isChecked()) {
            int size =
                std::min(int(mSwatches[i]->width() * 0.8f), int(mSwatches[i]->height() * 0.8f));
            QImage image(size, size, QImage::Format_RGB32);
            image.fill(color);
            mSwatches[i]->setIcon(QIcon(QPixmap::fromImage(image)));
            mSwatches[i]->setVisible(true);
            mSwatches[i]->setIconSize(QSize(size, size));
            mColors[i] = color;
        }
    }
}
