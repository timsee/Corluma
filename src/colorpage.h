
#ifndef SINGLECOLORPAGE_H
#define SINGLECOLORPAGE_H

#include <QWidget>

#include "colorpicker/singlecolorpicker.h"
#include "cor/objects/page.h"

/*!
 * \copyright
 * Copyright (C) 2015 - 2020.
 * Released under the GNU General Public License.
 *
 *
 * \brief The ColorPage provides a way to change the color of the lights selected in the app.
 *
 * The page contains a ColorPicker that allows the user to choose colors. The color picker has
 * various modes such as standard, which gives full range of RGB, and ambient, which allows the
 * user to choose different shades of white.
 *
 * For arduino-based projects, there is also a pop up menu on the bottom that allows the user to
 * choose routines for the arduino. To get the single color routines, the user should be on the
 * standard color picker. To get the multi color routines, the user should be on the multi color
 * picker.
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
    void changePageType(ESingleColorPickerMode page) { mColorPicker->changeMode(page); }

    /// getter for current type of color page (ambiance, RGB, etc.)
    ESingleColorPickerMode pageType() { return mColorPicker->mode(); }

    /// programmatically updates brightness
    void updateBrightness(std::uint32_t brightness);

    /// called when the app state is updated (IE when the selected lights changed)
    void update(const QColor& color, std::size_t lightCount, EColorPickerType bestType);

    /// getter for currently selected color
    const QColor color() const noexcept { return mColor; }

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

    /// best possible type of color picker allowed by selected lights
    EColorPickerType mBestType;

    /// main feature of widget, this allows the user to select colors for the LEDs
    SingleColorPicker* mColorPicker;

    /// layout for widget
    QVBoxLayout* mLayout;
};

#endif // SINGLECOLORPAGE_H
