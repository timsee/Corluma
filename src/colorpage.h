
#ifndef SINGLECOLORPAGE_H
#define SINGLECOLORPAGE_H

#include <QWidget>

#include "colorpicker/colorpicker.h"
#include "cor/objects/page.h"
#include "routines/routinecontainer.h"

/*!
 * \copyright
 * Copyright (C) 2015 - 2020.
 * Released under the GNU General Public License.
 *
 *
 * \brief The ColorPage provides a way to change the color of the lights selected in the app.
 *
 * The page contains a ColorPicker that allows the user to choose colors. The color picker has
 * two main modes: an HSV mode, which requires the user to use a slider to change the brightness
 * and an ambient mode, which allows the user to choose between different temperatures of white.
 *
 * While the user is on this page, there is also a pop up menu on the bottom that allows the user to
 * choose routines for lights that can be individually indexed, such as arduinos and nanoleafs.
 */
class ColorPage : public QWidget, public cor::Page {
    Q_OBJECT

public:
    /*!
     * \brief Constructor
     */
    explicit ColorPage(QWidget* parent);

    /*!
     * \brief changePageType change the hue page type to display a different color picker.
     *
     * \param page the new page type for the hue page.
     */
    void changePageType(EColorPickerMode page) {
        if (page != pageType()) {
            switch (page) {
                case EColorPickerMode::HSV:
                    mColorPicker->changeMode(EColorPickerMode::HSV);
                    mColorPicker->enable(mColorPicker->isEnabled(), EColorPickerType::color);
                    break;
                case EColorPickerMode::ambient:
                    mColorPicker->changeMode(EColorPickerMode::ambient);
                    mColorPicker->enable(mColorPicker->isEnabled(), EColorPickerType::CT);
                    break;
                case EColorPickerMode::multi:
                    mColorPicker->changeMode(EColorPickerMode::multi);
                    mColorPicker->enable(mColorPicker->isEnabled(), EColorPickerType::color);
                    break;
                default:
                    break;
            }
        }
    }

    /// getter for current type of color page (ambiance, RGB, etc.)
    EColorPickerMode pageType() { return mColorPicker->mode(); }

    /// programmatically updates brightness
    void updateBrightness(std::uint32_t brightness);

    /// called when the app state is updated (IE when the selected lights changed)
    void update(const QColor& color,
                const std::vector<QColor>& colorScheme,
                std::uint32_t brightness,
                std::size_t lightCount,
                EColorPickerType bestType);

    /// getter for currently selected color
    const QColor color() const noexcept { return mColor; }

    /// pointer to the colorpicker
    ColorPicker* colorPicker() { return mColorPicker; }

signals:

    /// sent out whenever a routine update is triggered
    void colorUpdate(QColor);

    /// sent out whenever an ambient color update is triggered
    void ambientUpdate(std::uint32_t, std::uint32_t);

public slots:

    /*!
     * \brief colorChanged signaled whenever the ColorPicker chooses a new color.
     */
    void colorChanged(const QColor&);

protected:
    /*!
     * \brief resizeEvent called every time the main window is resized.
     */
    void resizeEvent(QResizeEvent*);

private slots:

    /*!
     * \brief ambientUpdateReceived called whenever the colorpicker gives back an ambient update.
     *        Gives the color temperature value first, followed by the brightness.
     */
    void ambientUpdateReceived(std::uint32_t, std::uint32_t);

private:
    /*!
     * \brief setupButtons sets up the routine buttons.
     */
    void setupButtons();

    /// stores last value for the color
    QColor mColor;

    /// vector for multi color pickers.
    std::vector<QColor> mScheme;

    /// best possible type of color picker allowed by selected lights
    EColorPickerType mBestType;

    /// main feature of widget, this allows the user to select colors for the LEDs
    ColorPicker* mColorPicker;
};

#endif // SINGLECOLORPAGE_H
