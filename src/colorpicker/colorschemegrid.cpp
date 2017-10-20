/*!
 * \copyright
 * Copyright (C) 2015 - 2017.
 * Released under the GNU General Public License.
 */
#include "colorschemegrid.h"
#include <QDebug>

ColorSchemeGrid::ColorSchemeGrid(QWidget *parent) : QWidget(parent) {
    mMaximumSize = 5;

    // --------------
    // Setup Layout
    // --------------

    mLayout = new QHBoxLayout;

    // --------------
    // Setup Array Color Buttons
    // --------------

    mColorLabels = std::vector<QLabel*>(mMaximumSize, nullptr);
    mColors = std::vector<QColor>(mMaximumSize, QColor(0,0,0));
    for (uint32_t i = 0; i < mMaximumSize; ++i) {
        mColorLabels[i] = new QLabel;
        mColorLabels[i]->setAlignment(Qt::AlignCenter);
        mColorLabels[i]->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
        mLayout->addWidget(mColorLabels[i]);
    }

    setLayout(mLayout);
}

void ColorSchemeGrid::updateColorScheme(const std::vector<QColor> colorScheme) {
    int smallest = std::min(mColorCount, mMaximumSize);
    for (uint32_t i = 0; i < mMaximumSize; ++i) {
        if (i < smallest) {
            mColors[i] = colorScheme[i];
            int count = std::min((uint32_t)mMaximumSize, (uint32_t)colorScheme.size());
            count = std::max(count, 3);
            int size = this->size().width() / (count + 1);
            size = size * 0.66f;
            QPixmap pixmap(size, this->height() * 0.8f);
            pixmap.fill(colorScheme[i]);
            mColorLabels[i]->setPixmap(pixmap);
            mColorLabels[i]->setVisible(true);
        } else {
            mColorLabels[i]->setVisible(false);
        }
    }
}

void ColorSchemeGrid::resizeEvent(QResizeEvent *) {
    for (uint32_t i = 0; i < mMaximumSize; ++i) {
        int size = this->size().width() / 6;
        size = size * 0.66f;
        QPixmap pixmap(size, size * 2);
        pixmap.fill(mColors[i]);
        mColorLabels[i]->setPixmap(pixmap);
    }
}


void ColorSchemeGrid::setColor(int i, QColor color) {
    mColors[i] = color;
    int size = this->size().width() / 6;
    size = size * 0.66f;
    QPixmap pixmap(size, size * 2);
    pixmap.fill(color);
    mColorLabels[i]->setPixmap(pixmap);
}
