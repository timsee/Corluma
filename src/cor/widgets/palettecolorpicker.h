#ifndef PALETTECOLORPICKER_H
#define PALETTECOLORPICKER_H

#include <QPushButton>
#include <QWidget>

#include "utils/exception.h"
/*!
 * \copyright
 * Copyright (C) 2015 - 2021.
 * Released under the GNU General Public License.
 */


/*!
 * \brief The PaletteColorPicker class is a simple class for choosing colors in a palette
 */
class PaletteColorPicker : public QWidget {
    Q_OBJECT
public:
    explicit PaletteColorPicker(QWidget* parent)
        : QWidget(parent),
          mMaxColors{6},
          mButtons{std::vector<QPushButton*>(mMaxColors, nullptr)},
          mColors{std::vector<QColor>(mMaxColors, QColor(140, 140, 140))},
          mColorCount{2u} {
        for (auto i = 0u; i < mMaxColors; ++i) {
            mButtons[i] = new QPushButton(this);
            mButtons[i]->setCheckable(true);

            switch (i) {
                case 0:
                    connect(mButtons[i], SIGNAL(clicked()), this, SLOT(buttonPressed0()));
                    break;
                case 1:
                    connect(mButtons[i], SIGNAL(clicked()), this, SLOT(buttonPressed1()));
                    break;
                case 2:
                    connect(mButtons[i], SIGNAL(clicked()), this, SLOT(buttonPressed2()));
                    break;
                case 3:
                    connect(mButtons[i], SIGNAL(clicked()), this, SLOT(buttonPressed3()));
                    break;
                case 4:
                    connect(mButtons[i], SIGNAL(clicked()), this, SLOT(buttonPressed4()));
                    break;
                case 5:
                    connect(mButtons[i], SIGNAL(clicked()), this, SLOT(buttonPressed5()));
                    break;
                default:
                    break;
            }

            QSizePolicy sizePolicy = mButtons[i]->sizePolicy();
            sizePolicy.setRetainSizeWhenHidden(true);
            mButtons[i]->setSizePolicy(sizePolicy);
        }

        changeColorCount(mColorCount);
        /// pick the first color by default
        mButtons[0]->setChecked(true);
    }


    /// update the colors in the palette.
    void updateColors(const std::vector<QColor>& colors) {
        if (colors.size() > mMaxColors) {
            THROW_EXCEPTION("too many colors in update!");
        }
        if (colors.size() != mColorCount) {
            changeColorCount(colors.size());
        }
        for (auto i = 0u; i < mColorCount; ++i) {
            mColors[i] = colors[i];
        }


        for (auto i = 0u; i < colors.size(); ++i) {
            int size =
                std::min(int(mButtons[i]->width() * 0.8f), int(mButtons[i]->height() * 0.8f));
            QImage image(size, size, QImage::Format_RGB32);
            image.fill(mColors[i]);
            mButtons[i]->setIcon(QIcon(QPixmap::fromImage(image)));
            mButtons[i]->setIconSize(QSize(size, size));
        }
    }

    /// getter for colors
    std::vector<QColor> colors() {
        std::vector<QColor> colors(colorCount());
        for (auto i = 0u; i < colorCount(); ++i) {
            colors[i] = mColors[i];
        }
        return colors;
    }

    /// getter for selected index
    std::uint32_t selectedIndex() {
        for (auto i = 0u; i < mButtons.size(); ++i) {
            if (mButtons[i]->isChecked()) {
                return i;
            }
        }
        return 0;
    }

    /// getter for color count
    std::uint32_t colorCount() const noexcept { return mColorCount; }

    /// getter for max number of colors supported by the widget
    std::uint32_t maxColors() const noexcept { return mMaxColors; }

    /// change the count of colors.
    void changeColorCount(std::uint32_t i) {
        if (i > mMaxColors) {
            THROW_EXCEPTION("too many colors requested!");
        }
        mColorCount = i;

        if (selectedIndex() >= mColorCount - 1) {
            buttonPressed0();
        }

        for (auto i = 0u; i < mButtons.size(); ++i) {
            if (i >= mColorCount) {
                mButtons[i]->setVisible(false);
            } else {
                mButtons[i]->setVisible(true);
            }
        }
        updateColors(colors());
    }

    void updateSelected(const QColor& color);

protected:
    void resizeEvent(QResizeEvent*) { resize(); }

private slots:
    void buttonPressed0() { deselectLights(0u); }

    void buttonPressed1() { deselectLights(1u); }

    void buttonPressed2() { deselectLights(2u); }

    void buttonPressed3() { deselectLights(3u); }

    void buttonPressed4() { deselectLights(4u); }

    void buttonPressed5() { deselectLights(5u); }

signals:

    void selectionChanged(QColor, std::uint32_t);

private:
    /// programmatically resize
    void resize() {
        auto buttonWidth = this->width() / mMaxColors;
        auto xPos = 0;
        for (auto i = 0u; i < mButtons.size(); ++i) {
            mButtons[i]->setGeometry(xPos, 0, buttonWidth, height());
            xPos += mButtons[i]->width();
        }
    }

    /// getter for the currently selected color
    std::uint32_t currentlySelectedColor() {
        for (auto i = 0u; i < mButtons.size(); ++i) {
            if (mButtons[i]->isChecked()) {
                return i;
            }
        }
        return 0;
    }

    /// deselect all lights except the one at the index provided.
    void deselectLights(std::uint32_t keepSelected) {
        for (auto i = 0u; i < mButtons.size(); ++i) {
            if (i != keepSelected) {
                mButtons[i]->setChecked(false);
            }
        }
    }

    /// the total number of colors supported by the widget
    const std::uint32_t mMaxColors;

    /// vector of buttons used by the widget
    std::vector<QPushButton*> mButtons;

    /// vector of stored colors
    std::vector<QColor> mColors;

    /// count of uesd colors.
    std::uint32_t mColorCount;
};

#endif // PALETTECOLORPICKER_H
