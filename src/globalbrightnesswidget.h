#ifndef GLOBALBRIGHTNESSWIDGET_H
#define GLOBALBRIGHTNESSWIDGET_H

#include "cor/lightlist.h"
#include "cor/widgets/slider.h"
#include "cor/widgets/switch.h"

#include <QWidget>

/*!
 * \copyright
 * Copyright (C) 2015 - 2019.
 * Released under the GNU General Public License.
 *
 *
 * \brief The GlobalBrightnessWidget is a widget that controls the brightness of all selected
 * lights. It is set to display the brightness of all lights averaged together, and when it changes,
 * it sets all lights to tis new brightness. It also has an on/off switch.
 */
class GlobalBrightnessWidget : public QWidget {
    Q_OBJECT
public:
    /// constructor
    explicit GlobalBrightnessWidget(const QSize& size,
                                    bool isLeftAlwaysOpen,
                                    cor::LightList* data,
                                    QWidget* parent);

    /// update the state of the widget
    void updateColor(const QColor& color);

    /// updates the brightness of the widget
    void updateBrightness(int brightness);

    /// getter for brightness
    int brightness() { return mBrightnessSlider->slider()->value(); }

    /// resize the widget programmatically
    void resize();

    /// push in the widget
    void pushIn();

    /// push out the widget
    void pushOut();

    /// true if in, false otherwise
    bool isIn() const noexcept { return mIsIn; }

    /// called when the count changes to reflect the light's state better
    void lightCountChanged(bool isOn, const QColor& color, std::size_t count);

signals:
    /// the new value of the brightness slider
    void brightnessChanged(std::uint32_t newValue);

private slots:
    /*!
     * \brief brightnessSliderChanged Connected to the the slider at the top, this takeas a value
     * between 0-100 and sends that value to the lights to control how bright they are.
     */
    void brightnessSliderChanged(int);

    /*!
     * \brief changedSwitchState Connected to the button in the top left of the GUI at all times.
     * Toggles between running the current routine at current settings, and off.
     */
    void changedSwitchState(bool state);

private:
    /// true if in, false if out
    bool mIsIn;

    /// current color of slider
    QColor mColor;

    /// size used for determining widget size
    QSize mSize;

    /// true if left menu is always open in app, false otherwise
    bool mIsLeftAlwaysOpen;

    /// x position of widget
    int mPositionX;

    /// how many pixels of space between widget and top
    int mTopSpacer;

    /// data to change
    cor::LightList* mData;

    /*!
     * \brief mBrightnessSlider slider for adjusting the brightness of all selected devices.
     */
    cor::Slider* mBrightnessSlider;

    /// switch for turning all selected lights on and off.
    cor::Switch* mOnOffSwitch;
};

#endif // GLOBALBRIGHTNESSWIDGET_H
