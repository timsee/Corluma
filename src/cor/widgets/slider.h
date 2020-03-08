#ifndef COR_SLIDER_H
#define COR_SLIDER_H


#include <QLabel>
#include <QLayout>
#include <QSlider>
#include <QWidget>

namespace cor {

/// mode for slider
enum class ESliderType { vanilla, color, image, gradient };

/// standard background color of groove of slider
static QColor kSliderBackgroundColor(32, 31, 31);

/*!
 * \copyright
 * Copyright (C) 2015 - 2020.
 * Released under the GNU General Public License.
 *
 *
 * \brief The Slider class provides a slider with behaviors similiar
 * to sliders on mobile devices. Clicking on the slider jumps the slider's value
 * to the location you clicked instead of the QSlider default of jumping by the page
 * value. There is also the option to use a gradient instead of a solid color as the
 * background color for the slider.
 *
 * QSliders can have their tick marks break when the project uses a custom style sheet,
 * so tick marks are drawn using QPaint, if needed.
 *
 * There is also an option for developers to set a value that above the slider's minimum
 * as the "minmum possible." Users will see a slider that shows the minimum value, but they
 * won't below the value set as the minimum possible. This is used by the Array Colors Page
 * in order to prevent users from choosing less than 2 colors for the array routines.
 *
 */
class Slider : public QSlider {
    Q_OBJECT

public:
    /*!
     * \brief Constructor
     */
    explicit Slider(QWidget* parent);

    /*!
     * \brief setColor Does a dark to light gradient on the color provided on the background
     *        of the slider to the left of the thumb piece of the slider Uses a custom style sheet
     * to achieve this effect. \param color the color that will be put into a custom style sheet
     */
    void setColor(const QColor& color);

    /*!
     * \brief setImage Adds a background image to the slider instead of using a color with a
     * gradient. \param path path to the resource of the image
     */
    void setImage(const QString& path);

    /*!
     * \brief setGradient Adds a gradient from one color to another to the slider's background
     * \param leftColor starting color for lefthand side of the slider
     * \param rightColor ending color for righthand side of the slider
     */
    void setGradient(const QColor& leftColor, const QColor& rightColor);

    /*!
     * \brief setSnapToNearestTick set this if you want the slider to snap to the nearest tick
     * instead of snapping to the direct location that the user clicked.
     *
     * \param shouldSnap true if you want to it to snap, false otherwise.
     */
    void setSnapToNearestTick(bool shouldSnap);

    /*!
     * \brief setSliderHeight By default sliders take up all of the of the widget, but this covers
     * ticks and leaves no room inbewtween sliders that are stacked. This function takes a value
     * between 0.0 and 1.0 to  scale the slider down, while keeping it centered in the widget.
     *
     * \param percent a value between 0.0 and 1.0 that scales the slider to take up less of hte
     * widget.
     */
    void setHeightPercentage(float percent);

    /*!
     * \brief enable sets whether the slider is enabled or disabled. Disabling the slider adds
     * opacity, greys out the color, and disables user interaction.
     *
     * \param shouldEnable true if the asset should be enabled, false otherwise.
     */
    void enable(bool shouldEnable);

    /// set to true to draw ticks, set to false to hide them
    void setShouldDrawTickLabels(bool shouldDraw);

    /// resizes programmatically
    void resize();

    /// getter for slider color
    const QColor& color() const noexcept { return mSliderColor; }

signals:
    /*!
     * \brief Sends out the value that the slider has been set to after all the processing of
     * snapping it to a tick or accounting for its minium value or any of that.
     */
    void valueChanged(int);

private slots:
    /*!
     * \brief receivedValue called whenever the QSlider signals valueChanged().
     */
    void receivedValue(int);

    /*!
     * \brief releasedSlider uses the QSlider inside of the LightsSlider to pick up
     *        when the slider is released. This always sets the color of the color picker.
     *        This system is used to prevent an edge case with throttling with a timer.
     *        Without it, its possible to change the UI without updating the lights if you are
     *        quick enough.
     */
    void releasedSlider();

    /// slider moved
    void movedSlider(int);

protected:
    /*!
     * \brief resizeEvent makes sure that the QSlider resizes with the QWidget
     */
    void resizeEvent(QResizeEvent*);

    /*!
     * \brief paintEvent handles edge cases created by the having a QSlider and
     *        an elaborate stylesheet. Draws the application's background color
     *        and the slider's ticks.
     */
    void paintEvent(QPaintEvent*);

    /// picks up when a mouse is pressed down, which changes the value.z
    void mousePressEvent(QMouseEvent* event);

private:
    /*!
     * \brief mHeightScaleFactor used to scale the slider inside of its qwidget so it takes up
     *        less than 100% of the widget
     */
    float mHeightScaleFactor;

    /*!
     * \brief mShouldSnap true if the slider should snap to the nearest ticks, false
     *        if it should use the value specified by the user.
     */
    bool mShouldSnap;

    /*!
     * \brief mUseMinimumPossible true if the slider should have a minimum possible value it
     *        can be set to, which must be above its actual minimum. False otherwise.
     */
    bool mUseMinimumPossible;

    /*!
     * \brief mMinimumPossible true if it should use the minimum possible setting,
     *        false otherwise.
     */
    int mMinimumPossible;

    /// adjusts the stylesheet used by the slider
    void adjustStylesheet();

    /*!
     * \brief snapSliderToNearestTick checks the value that the slider is
     *        is getting changed to and, if its not exactly over one of the
     *        sliders ticks, it'll update the value to the value of the closest
     *        tick. This provides extra logic to jumpSliderToPosition, but only
     *        if its enabled by setSnapToNearestTick().
     *
     * \param slider the internal slider
     * \param pos the position determined by jumpSliderToPosition.
     * \return the final position of the slider.
     */
    int snapSliderToNearestTick(QSlider* slider, int pos);

    /*!
     * \brief mShouldDrawTickLabels if true, instead of ticks labels of the values at that
     *        region are drawn instead
     */
    bool mShouldDrawTickLabels;

    /*!
     * \brief mPath path to resource for background image for the slider.
     */
    QString mPath;

    /*!
     * \brief mSliderColor the current slider color
     */
    QColor mSliderColor;

    /// second color for the grident option
    QColor mColorGradient;

    /// size of the handle of the slider
    int mHandleSize;

    /// type of the slider
    ESliderType mType;
};

} // namespace cor
#endif // COR_SLIDER_H
