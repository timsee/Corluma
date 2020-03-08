#ifndef GLOBALBRIGHTNESSWIDGET_H
#define GLOBALBRIGHTNESSWIDGET_H

#include "comm/commlayer.h"
#include "cor/lightlist.h"
#include "cor/widgets/slider.h"
#include "cor/widgets/switch.h"

#include <QWidget>

/*!
 * \copyright
 * Copyright (C) 2015 - 2020.
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
                                    CommLayer* comm,
                                    cor::LightList* data,
                                    QWidget* parent);

    /// programmatically set if on
    bool isOn(bool);

    /// update the state of the widget
    void updateColor(const QColor& color);

    /// updates the brightness of the widget
    void updateBrightness(int brightness);

    /// getter for brightness
    int brightness() { return mBrightnessSlider->value(); }

    /// resize the widget programmatically
    void resize();

    /// push in the widget
    void pushIn();

    /// push out the widget
    void pushOut();

    /// true if in, false otherwise
    bool isIn() const noexcept { return mIsIn; }

    /// called when the count changes to reflect the light's state better
    void lightCountChanged(bool isOn,
                           const QColor& color,
                           std::uint32_t brightness,
                           std::size_t count);

signals:
    /// the new value of the brightness slider
    void brightnessChanged(std::uint32_t newValue);

    /// the switch for turning all the lights on or off has been toggled
    void isOnUpdate(bool isOn);

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

    /// called from a timer to check whether or not the all lights are currently on.
    void checkifOn();

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

    /*!
     * \brief mBrightnessSlider slider for adjusting the brightness of all selected devices.
     */
    cor::Slider* mBrightnessSlider;

    /// switch for turning all selected lights on and off.
    cor::Switch* mOnOffSwitch;

    /// tracks how long its been since an update
    QElapsedTimer mElapsedTime;

    /// set whenever there is any interaction with the on/off switch, acts as a cooldown for
    /// updates.
    bool mAnyInteraction;

    /// pointer to CommLayer to check on/off state
    CommLayer* mComm;

    /// pointer to currently selected lights for checking on/off state
    cor::LightList* mData;

    /// timer for checking whether or not the widget is on or off
    QTimer* mOnOffCheckTimer;
};

#endif // GLOBALBRIGHTNESSWIDGET_H
