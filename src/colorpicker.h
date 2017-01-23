#ifndef COLORPICKER_H
#define COLORPICKER_H

#include "lightsslider.h"

#include <QWidget>
#include <QTimer>
#include <QTime>
#include <QLabel>
#include <QMouseEvent>
#include <QLayout>

/*!
 * \copyright
 * Copyright (C) 2015 - 2017.
 * Released under the GNU General Public License.
 */

/*!
 * \brief The ELayoutColorPicker enum contains
 * all possible layouts for the color picker.
 * By default, it uses its Full Layout.
 */
enum class ELayoutColorPicker {
    /*!
     * The color wheel is displayed on top of
     * the sliders and when the window resizes,
     * the sliders will resize horizontally but the
     * wheel grows vertically.
     */
    eFullLayout,
    /*!
     * The color wheel is displayed to the left
     * of the sliders
     */
    eCondensedLayout,
    /*!
     * only the wheel is displayed.
     */
    eWheelOnlyLayout,
    /*!
     * only the sliders are displayed.
     */
    eSlidersOnlyLayout
};

/*!
 * \brief The ColorPicker class is an object that combines three LightsSliders
 * and a color wheel to create an interface for choosing colors.
 *
 * Whenever a color is selected, it signals out a QColor of to all connected slots.
 * Colors can be selected by by moving the sliders or by clicking anywhere on the
 * color wheel. A timer is used to throttle the signals to prevent sending messages
 * too frequently.
 *
 * A layout can be set during its construction, but cannot be changed on the fly. This
 * layout uses ELayoutColorPicker as a parameter and this can be used to hide the
 * sliders, hide the color wheel, or move the color wheel from the top of the sliders
 * to the left of them.
 */
class ColorPicker : public QWidget
{
    Q_OBJECT

public:
    /*!
     * \brief ColorPicker constructor
     * \param parent parent widget
     */
    explicit ColorPicker(QWidget *parent = 0);

    /*!
     * \brief Deconstructor
     */
    ~ColorPicker();

    /*!
     * \brief chooseLayout sets the layout using the available layout modes. This allows
     *        you to hide the sliders or color wheel, and change how things are ordered
     *        so they can fit more window sizes. This should be called only once as Qt doesn't
     *        seem to like it when you change between horizontal and vertical layouts.
     * \param layout the layout you want to use.
     */
    void chooseLayout(ELayoutColorPicker layout);


    /*!
     * \brief chooseColor programmatically set the color of the picker. this will update the
     *        UI elements to reflect this color. By default it wil also signal its changes
     *        a flag can be used to disable the signal.
     * \param color a QColor representation of the color you want to use.
     * \param shouldSignal Defaulted to true, this flag
     */
    void chooseColor(QColor color, bool shouldSignal = true);

    /*!
     * \brief colorWheel the color wheel for the color picker. Uses an image asset,
     *        which needs to be included in the project. Used to pick the color with
     *        a single mouse event instead of setting 3 sliders.
     */
    QLabel *colorWheel;

    /*!
     * \brief rSlider slider used for choosing the strength of the red LEDs
     */
    LightsSlider *rSlider;
    /*!
     * \brief gSlider slider used for choosing the strength of the green LEDs
     */
    LightsSlider *gSlider;
    /*!
     * \brief bSlider slider used for choosing the strength of the blue LEDs
     */
    LightsSlider *bSlider;

    /*!
     * \brief rLabel label for the rSlider, puts that little "R" in front of the slider.
     */
    QLabel *rLabel;
    /*!
     * \brief gLabel label for the gSlider, puts that little "G" in front of the slider.
     */
    QLabel *gLabel;
    /*!
     * \brief bLabel label for the bSlider, puts that little "B" in front of the slider.
     */
    QLabel *bLabel;

    /*!
     * \brief useHueWheel uses a wheel with more desaturated colors since a hue bulb has a
     *        different color gamut than standard RGB LEDs.
     * \param shouldUseHueWheel true if it should use a hue wheel, false if it should use the
     *        standard wheel.
     */
    void useHueWheel(bool shouldUseHueWheel);

signals:
    /*!
     * \brief colorUpdate should be connected to the slot of any other elements
     *        that utilize this color picker. Any time a color is chosen, it sends
     *        out the color using this signal.
     */
    void colorUpdate(QColor);

protected:
    /*!
     * \brief resizeEvent called whenever the window resizes. This is used to override
     *        the resizing of the color wheel to use our custom logic.
     */
    virtual void resizeEvent(QResizeEvent *);

    /*!
     * \brief mousePressEvent called only onnce when a mouse press is clicked down. Events not
     *        directly on top of the color wheel are ignored.
     */
    virtual void mousePressEvent(QMouseEvent *);
    /*!
     * \brief mouseMoveEvent called when a mouse press is clicked down and moves. This event gets
     *        called very frequently, so the events are thorttled by a timer. Also, events not
     *        directly on top of the color wheel are ignored.
     */
    virtual void mouseMoveEvent(QMouseEvent *);
    /*!
     * \brief mouseReleaseEvent called when a mouse press is released Events not
     *        directly on top of the color wheel are ignored.
     */
    virtual void mouseReleaseEvent(QMouseEvent *);

    /*!
     * \brief showEvent used to turn on the throttle timer
     */
    void showEvent(QShowEvent *);

    /*!
     * \brief hideEvent used to turn off the throttle timer
     */
    void hideEvent(QHideEvent *);


private slots:

    /*!
     * \brief rSliderChanged called whenever the slider in the red LightsSlider class
     *        changes its value.
     */
    void rSliderChanged(int);

    /*!
     * \brief gSliderChanged called whenever the slider in the green LightsSlider class
     *        changes its value.
     */
    void gSliderChanged(int);

    /*!
     * \brief bSliderChanged called whenever the slider in the blue LightsSlider class
     *        changes its value.
     */
    void bSliderChanged(int);

    /*!
     * \brief resetThrottleFlag called by the throttle timer to allow commands to
     *        be sent again. This gets called on a loop whenever color picker is being
     *        used in order to prevent clogging the communication stream.
     */
    void resetThrottleFlag();

    /*!
     * \brief releasedSlider uses the QSlider inside of the LightsSlider to pick up
     *        when the slider is released. This always sets the color of the color picker.
     *        This system is used to prevent an edge case with throttling with a timer.
     *        Without it, its possible to change the UI without updating the lights if you are
     *        quick enough.
     */
    void releasedSlider();


private:
    /*!
     * \brief mColor the current color that the color picker is displaying
     */
    QColor mColor;

    /*!
     * \brief mThrottleTimer This timer is only active while the user is actively using
     *        the color wheel or moving the sliders. It prevents the picker from sending
     *        too many signals and clogging sending streams by throttling them to a max
     *        speed of 20 signals per second
     */
    QTimer *mThrottleTimer;

    /*!
     * \brief mPressTime used by the throttle timer to ignore clicks and to focus
     *        on long presses.
     */
    QTime mPressTime;

    /*!
     * \brief mThrottleFlag flag used to enforced the throttle timer's throttle.
     */
    bool mThrottleFlag;

    /*!
     * \brief mShowSliders flag used when determining the layout. If its set to true,
     *        the picker will show the sliders, otherwise, it won't show them.
     */
    bool mShowSliders;

    /*!
     * \brief mShowWheel flag used when determine the layout. If its set to true,
     *        the picker will show the color wheel, otherwise it won't show them.
     */
    bool mShowWheel;

    /*!
     * \brief mUseHueWheel true if using standard wheel, false otherwise.
     */
    bool mUseHueWheel;

    /*!
     * \brief sliderLayout layout used to arrange the RGB sliders.
     */
    QGridLayout *mSliderLayout;

    /*!
     * \brief fullLayout layout used when in the full layout mode.
     */
    QVBoxLayout *mFullLayout;

    /*!
     * \brief condensedLayout layout used when in the condensed layout mode
     */
    QHBoxLayout *mCondensedLayout;

    /*!
     * \brief mCurrentLayout The current layout of the color picker. Used
     *        to determine whether the mFullLayout or mCondensedLayout is
     *        in use.
     */
    ELayoutColorPicker mCurrentLayoutColorPicker;

    /*!
     * \brief handleMouseEvent handles all the mouse events used in the color wheel.
     *        The mousePressEvent and mouseReleaseEvent both map to this function.
     * \param event the mouse event that is getting processed.
     */
    void handleMouseEvent(QMouseEvent *event);

    /*!
     * \brief map map function from arduino: https://www.arduino.cc/en/Reference/Map
     * \param x the value getting the map applied
     * \param in_min the original minimum possible value of x
     * \param in_max the original maximum possible value of x
     * \param out_min the new minimum possible value of x
     * \param out_max the new maximum possibel value of x
     * \return x mapped from the range of in_min->in_max to the range of out_min->out_max
     */
    int map(int x, int in_min, int in_max, int out_min, int out_max);
};

#endif // COLORPICKER_H
