#ifndef COLORPICKER_H
#define COLORPICKER_H

#include "cor/slider.h"

#include <QLabel>
#include <QLayout>
#include <QWidget>

#include "colorwheel.h"

/*!
 * \copyright
 * Copyright (C) 2015 - 2019.
 * Released under the GNU General Public License.
 */


/// type of colors allowed by selected lights
enum class EColorPickerType { dimmable, CT, color };


/*!
 * \brief The ColorPicker class is a GUI object designed to give the user ability to choose
 * RGB values in a variety of ways. The standard layout provides the user with a color wheel
 * and three sliders, one for red, one for green, and one for blue. All other layouts use
 * this style as a base but modify it slightly. For example, in the ambient layout, the RGB wheel is
 * replaced by a wheel that contains only shades of white and the RGB sliders are replaced by one
 * slider for choosing the temperature, and one slider for choosing the brightness.
 *
 * There also exists a multi color picker, which allows removes the sliders in favor of one slider
 * and two rows of buttons. This layout is good for modifying multiple colors at once.
 */
class ColorPicker : public QWidget {
    Q_OBJECT

public:
    /*!
     * \brief ColorPicker constructor
     * \param parent parent widget
     */
    explicit ColorPicker(QWidget* parent);

    /*!
     * \brief Destructor
     */
    ~ColorPicker() = default;

    /*!
     * \brief enable enables/disables the color picker. If the picker wheel is disabled, its faded
     * out and mouse events
     *        don't work. If its enabled, its not faded out and um, mouse events do work.
     * \param shouldEnable true to enable wheel, false otherwise.
     */
    virtual void enable(bool shouldEnable, EColorPickerType bestType) = 0;

    //------------------------------
    // Layout-Specific API
    //------------------------------

    /*!
     * \brief chooseAmbient programmatically set the ambient color picker. This will update
     *        the UI elements to reflect the values provided. By default it will also signal its
     *        changes, but a flag can be used to override the signal.
     * \param temperature desired temperature of the color.
     * \param brightness brightness of the color
     */
    void chooseAmbient(std::uint32_t temperature, std::uint32_t brightness);

signals:
    /*!
     * \brief colorUpdate should be connected to the slot of any other elements
     *        that utilize this color picker. Any time a color is chosen, it sends
     *        out the color using this signal.
     */
    void colorUpdate(const QColor&);

    /*!
     * \brief ambientUpdate emitted whenever the ambient picker has an update. First value is
     *        the color temperature (ranged between 153 and 500) and the second is the brightness
     *        (ranged between 0 and 100)
     */
    void ambientUpdate(std::uint32_t, std::uint32_t);

    /*!
     * \brief brightnessUpdate emitted whenever brightness changes from any layout that has a
     * brightness slider
     */
    void brightnessUpdate(uint32_t);

    /*!
     * \brief colorsUpdate update to a full color scheme
     */
    void colorsUpdate(std::vector<QColor>);

protected slots:

    /// updates whether or not the bottom menu should be enabled
    virtual void updateBottomMenuState(bool enable) = 0;

protected:
    /*!
     * \brief resize checks the geometry and resizes UI assets accordingly.
     */
    void resizeWheel();

    /*!
     * \brief mColorWheel the color wheel for the color picker. Uses an image asset,
     *        which needs to be included in the project. Used to pick the color with
     *        a single mouse event instead of setting 3 sliders.
     */
    ColorWheel* mColorWheel;

    /*!
     * \brief chooseColor programmatically set the color of the picker. this will update the
     *        UI elements to reflect this color. By default it wil also signal its changes
     *        a flag can be used to disable the signal.
     * \param color a QColor representation of the color you want to use.
     */
    void chooseColor(const QColor& color);


    /*!
     * \brief chooseBrightness programmatically set the brightness.This will update
     *        the UI elements to reflect the values provided. By default it will also signal its
     *        changes, but a flag can be used to override the signal.
     * \param brightness brightness of the color
     */
    void chooseBrightness(uint32_t brightness);

    //------------------------------
    // Miscellaneous
    //------------------------------

    /// stores the best possible type of colorpicker for situations where only white or ambient
    /// lights are connected and RGB colors can't be used.
    EColorPickerType mBestPossibleType;

    /*!
     * \brief mPlaceholder placeholder for the bottom layouts. These, such as the RGBSliders or the
     * ColorGrid get placed over  this mPlaceholder widget.
     */
    QWidget* mPlaceholder;

private:
    //------------------------------
    // Layout-Specific
    //------------------------------

    /*!
     * \brief fullLayout layout used when in the full layout mode.
     */
    QVBoxLayout* mFullLayout;
};

#endif // COLORPICKER_H
