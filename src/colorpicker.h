#ifndef COLORPICKER_H
#define COLORPICKER_H

#include "lightsslider.h"
#include "lightdevice.h"

#include <QWidget>
#include <QTimer>
#include <QTime>
#include <QLabel>
#include <QMouseEvent>
#include <QLayout>
#include <QPushButton>

#include <QGraphicsOpacityEffect>
#include <QPropertyAnimation>
#include <QParallelAnimationGroup>

/*!
 * \copyright
 * Copyright (C) 2015 - 2017.
 * Released under the GNU General Public License.
 */

/*!
 * \brief The ELayoutColorPicker enum contains
 * all possible layouts for the color picker.
 * By default, it uses eStandardLayout.
 */
enum class ELayoutColorPicker {
    /*!
     * The color wheel is displayed on top of
     * the sliders and when the window resizes,
     * the sliders will resize horizontally but the
     * wheel grows vertically.
     */
    eStandardLayout,
    /*!
     * only the wheel is displayed.
     */
    eWheelOnlyLayout,
    /*!
     * The color wheel is changed to shades of white.
     * Can choose between the a blue-ish white or an
     * orange-ish white and everything in between.
     * Top slider provides the ability to choose the
     * color via slider, second slider allows you
     * to adjust brightness.
     */
    eAmbientLayout,
    /*!
     * A dumb layout, but one that adds a bit more
     * "uniformity" or some word like that to the design.
     * This layout shows up only when hue white bulbs are
     * the only selected bulbs. It is similar to the Ambient
     * Layout, but without the blueish or orangeish bit, its just
     * white. White bulbs can only control brightness so theres a
     * wheel and a single slider that both do that.
     */
    eBrightnessLayout,
    /*!
     * The full color wheel is shown, but instead of
     * three sliders underneath theres one and 2 rows
     * of five boxes. This version of the color picker
     * allows the user to change colors in an array for
     * things that use multiple colors simultaneously.
     */
    eMultiColorLayout
};


/*!
 * \brief The ColorPicker class is a GUI object designed to give the user ability to choose
 *        RGB values in a variety of ways. The standard layout provides the user with a color wheel
 *        and three sliders, one for red, one for green, and one for blue. All other layouts use this
 *        style as a base but modify it slightly. For example, in the ambient layout, the RGB wheel is
 *        replaced by a wheel that contains only shades of white and the RGB sliders are replaced by
 *        one slider for choosing the temperature, and one slider for choosing the brightness.
 *
 *        There also exists a multi color picker, which allows removes the sliders in favor of one slider
 *        and two rows of buttons. This layout is good for modifying multiple colors at once.
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
     * \brief changeLayout sets the layout using the available layout modes. This allows
     *        you to hide the sliders or color wheel, and change how things are ordered
     *        so they can fit more window sizes. This should be called only once as Qt doesn't
     *        seem to like it when you change between horizontal and vertical layouts.
     * \param layout the layout you want to use.
     * \param skipAnimation if true, skips animation
     */
    void changeLayout(ELayoutColorPicker layout, bool skipAnimation = false);

    /*!
     * \brief enableWheel enables/disables the color wheel. If the color wheel is disabled, its faded out and mouse events
     *        don't work. If its enabled, its not faded out and um, mouse events do work.
     * \param shouldEnable true to enable wheel, false otherwise.
     */
    void enableWheel(bool shouldEnable);

    //------------------------------
    // Layout-Specific API
    //------------------------------

    /*!
     * \brief chooseColor programmatically set the color of the picker. this will update the
     *        UI elements to reflect this color. By default it wil also signal its changes
     *        a flag can be used to disable the signal.
     * \param color a QColor representation of the color you want to use.
     * \param shouldSignal true to signal, false to skip this signal.
     */
    void chooseColor(QColor color, bool shouldSignal = true);

    /*!
     * \brief chooseAmbient programmatically set the ambient color picker. This will update
     *        the UI elements to reflect the values provided. By default it will also signal its
     *        changes, but a flag can be used to override the signal.
     * \param temperature desired temperature of the color.
     * \param brightness brightness of the color
     * \param shouldSignal true to signal, false to skip this signal
     */
    void chooseAmbient(int temperature, int brightness, bool shouldSignal = true);

    /*!
     * \brief chooseBrightness programmatically set the brightness.This will update
     *        the UI elements to reflect the values provided. By default it will also signal its
     *        changes, but a flag can be used to override the signal.
     * \param brightness brightness of the color
     * \param shouldSignal true to signal, false to skip this signal
     */
    void chooseBrightness(int brightness, bool shouldSignal = true);

    /*!
     * \brief updateMultiColor programmatically set the colors in the multi color picker. This will update
     *        the UI elements to reflect the values provided. By default it will also signal its
     *        changes, but a flag can be used to override the signal.
     * \param colors the colors for the array in the multi color picker
     * \param count the number of colors to use from the vector
     */
    void updateMultiColor(const std::vector<QColor> colors, int count);

signals:
    /*!
     * \brief colorUpdate should be connected to the slot of any other elements
     *        that utilize this color picker. Any time a color is chosen, it sends
     *        out the color using this signal.
     */
    void colorUpdate(QColor);

    /*!
     * \brief multiColorChanged emitted whenever the multi color picker has an update for any individual color.
     */
    void multiColorChanged(int, QColor);

    /*!
     * \brief ambientUpdate emitted whenever the ambient picker has an update. First value is
     *        the color temperature (ranged between 153 and 500) and the second is the brightness
     *        (ranged between 0 and 100)
     */
    void ambientUpdate(int, int);

    /*!
     * \brief brightnessUpdate emitted whenever brightness changes from any layout that has a brightness slider
     */
    void brightnessUpdate(int);

    /*!
     * \brief multiColorCountChanged number of colors to use during multi color routines changed.
     */
    void multiColorCountChanged(int);

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
     * \brief topSliderChanged called whenever the slider in the top position
     *        changes its value.
     */
    void topSliderChanged(int);

    /*!
     * \brief midSliderChanged called whenever the slider in the mid position
     *        changes its value.
     */
    void midSliderChanged(int);

    /*!
     * \brief bottomSliderChanged called whenever the slider in the bottom position
     *        changes its value.
     */
    void bottomSliderChanged(int);

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

    /*!
     * \brief selectArrayColor when called, the multi color array color at the given index is seletected
     *        or deselected, depending on its current state.
     */
    void selectArrayColor(int);

    /*!
     * \brief hideTempWheel hides the mTempWheel. Called by animations when its done fading the temp wheel out.
     */
    void hideTempWheel();

private:
    //------------------------------
    // General
    //------------------------------

    /*!
     * \brief mTopSlider top slider of the bottom layout. Used for red in standard mode, but is also
     *        used in many of the other layouts for different functionality.
     */
    LightsSlider *mTopSlider;

    /*!
     * \brief mMidSlider middle slider in the bottom layout. Used for green in standard mode, but is also
     *        used in some of the other layouts for different functionality.
     */
    LightsSlider *mMidSlider;

    /*!
     * \brief mBottomSlider bottom slider in the bottom layout. Only used for blue in standard mode, is hidden
     *        in all other modes.
     */
    LightsSlider *mBottomSlider;

    /*!
     * \brief mTopLabel label for the topSlider in RGB modes, puts that little "R" in front of the slider.
     */
    QLabel *mTopLabel;
    /*!
     * \brief mMidLabel label for the midSLider in RGB modes, puts that little "G" in front of the slider.
     */
    QLabel *mMidLabel;
    /*!
     * \brief mBottomSlider label for the bottomSlider in RGB modes, puts that little "B" in front of the slider.
     */
    QLabel *mBottomLabel;

    /*!
     * \brief mColorWheel the color wheel for the color picker. Uses an image asset,
     *        which needs to be included in the project. Used to pick the color with
     *        a single mouse event instead of setting 3 sliders.
     */
    QLabel *mColorWheel;

    //------------------------------
    // Standard Layout-Specific
    //------------------------------

    /*!
     * \brief mColor the current color that the color picker is displaying
     */
    QColor mColor;

    //------------------------------
    // Brightness Layout-Specific
    //------------------------------

    /// brightness used in the brightness layout.
    uint32_t mWhiteBrightness;

    //------------------------------
    // Ambient Layout-Specific
    //------------------------------

    /// color temperature used during ambient layout
    int mAmbientTemperature;

    /// brightness used doing ambient layout
    int mAmbientBrightness;

    //------------------------------
    // Multi Layout-Specific
    //------------------------------

    /*!
     * \brief mMultiArraySize size of the multi color array, used to initialize
     *        and access vectors throughout this page.
     */
    uint32_t mMultiArraySize;

    /// vector pushbuttons used for the multi layout
    std::vector<QPushButton *> mArrayColorsButtons;

    /// color values for the multi layout
    std::vector<QColor> mMultiColors;

    /// the number colors that are enabled in the multi color picker.
    uint32_t mMultiUsed;

    /// indices of selected colors in multi color layout.
    std::list<uint32_t> mMultiSelected;

    /*!
     * \brief updateMultiColorSlider averages all colors with indices less than mMultiUsed
     *        and sets the top slider to be that averaged color.
     */
    void updateMultiColorSlider();

    /*!
     * \brief manageMultiSelected handles the selected buttons in the Multi color layout. This
     *        includes deselecting indices that are larger than the mMultiUsed value, highlighting
     *        selected indices, and enabling/disabling the color wheel based on the count of selected
     *        indices.
     */
    void manageMultiSelected();

    //------------------------------
    // Layout-Specific
    //------------------------------

    /*!
     * \brief mBottomLayout layout used to arrange the RGB sliders.
     */
    QGridLayout *mBottomLayout;

    /*!
     * \brief fullLayout layout used when in the full layout mode.
     */
    QVBoxLayout *mFullLayout;


    //------------------------------
    // Mouse Events
    //------------------------------

    /*!
     * \brief mPressTime used by the throttle timer to ignore clicks and to focus
     *        on long presses.
     */
    QTime mPressTime;

    /*!
     * \brief handleMouseEvent handles all the mouse events used in the color wheel.
     *        The mousePressEvent and mouseReleaseEvent both map to this function.
     * \param event the mouse event that is getting processed.
     */
    void handleMouseEvent(QMouseEvent *event);

    /*!
     * \brief eventIsOverWheel true if the event is over the color wheel itself, false otherwise
     * \param event mouse event being tested
     * \return true if the event is over the color wheel, false otherwise.
     */
    bool eventIsOverWheel(QMouseEvent *event);

    //------------------------------
    // Helpers
    //------------------------------

    /*!
     * \brief changeColorWheel resizes the color wheel and changes it between two layouts. Will animate
     *        changing the color wheel, if needed.
     * \param oldLayout color wheel for old layout.
     * \param newLayout color wheel for the new layout.
     * \param skipAnimation if true, skips animation
     */
    void changeColorWheel(ELayoutColorPicker oldLayout, ELayoutColorPicker newLayout, bool skipAnimation = false);

    /*!
     * \brief checkIfColorIsValid returns true if the color passes checks that make sure
     *        its not a background color or one of the illegal colors for the API to send out.
     * \param color color to check.
     * \return true if color is valid, false otherwise.
     */
    bool checkIfColorIsValid(QColor color);

    /*!
     * \brief getWheelPixmapPath retrieves the proper path to the asset required for the current cvolor wheel
     * \param layout the layout that the colorpicker is using. Certain layouts have different wheels
     * \return the proper path to the resource that should be used for the color wheel
     */
    QString getWheelPixmapPath(ELayoutColorPicker layout);

    /*!
     * \brief resize checks the geometry and resizes UI assets accordingly.
     */
    void resize();

    /*!
     * \brief createSolidColorIcon creates a QPushButton that has an icon that is a solid color that matches the QColor
     *        that is provided as input.
     * \param color the color of the QPushButton's icon.
     * \return  a pixmap that is a single solid color that matches the input.
     */
    QPixmap createSolidColorIcon(QColor color);

    //------------------------------
    // Miscellaneous
    //------------------------------

    /*!
     * \brief mThrottleTimer This timer is only active while the user is actively using
     *        the color wheel or moving the sliders. It prevents the picker from sending
     *        too many signals and clogging sending streams by throttling them to a max
     *        speed of 20 signals per second
     */
    QTimer *mThrottleTimer;

    /*!
     * \brief mTempWheel temp label, used for animations where wheels fade in and out.
     */
    QLabel *mTempWheel;

    /// opacity of mColorWheel
    float mWheelOpacity;

    /*!
     * \brief mThrottleFlag flag used to enforced the throttle timer's throttle.
     */
    bool mThrottleFlag;

    /// true if wheel is enabled, false othwerise.
    bool mWheelIsEnabled;

    /*!
     * \brief mCurrentLayout The current layout of the color picker. Used
     *        to determine whether the mFullLayout or mCondensedLayout is
     *        in use.
     */
    ELayoutColorPicker mCurrentLayoutColorPicker;

};

#endif // COLORPICKER_H
