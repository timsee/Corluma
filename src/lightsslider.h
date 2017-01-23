
#ifndef LIGHTSSLIDER_H
#define LIGHTSSLIDER_H

#include "datalayer.h"

#include <QApplication>
#include <QWidget>
#include <QSlider>
#include <QLabel>
#include <QLayout>
#include <memory>

/*!
 * \copyright
 * Copyright (C) 2015 - 2017.
 * Released under the GNU General Public License.
 *
 *
 * \brief The LightsSlider class provides a slider with behaviors similiar
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
class LightsSlider : public QWidget
{
    Q_OBJECT

public:
    /*!
     * \brief Constructor
     */
    explicit LightsSlider(QWidget *parent = 0);

    /*!
     * \brief slider The actual and factual QSlider in this slider class
     */
    QSlider *slider;

    /*!
     * \brief label label for the slider, allows you to label the slider, but by default its an empty value.
     */
    QLabel *label;

    /*!
     * \brief setSliderColorBackground Does a dark to light gradient on the color provided on the background
     *        of the slider to the left of the thumb piece of the slider Uses a custom style sheet to achieve this effect.
     * \param color the color that will be put into a custom style sheet
     */
    void setSliderColorBackground(QColor color);

    /*!
     * \brief setSliderImageBackground Adds a background image to the slider instead of using a color with a gradient.
     * \param path path to the resource of the image
     * \TODO clean this up more
     */
    void setSliderImageBackground(QString path);

    /*!
     * \brief setSnapToNearestTick set this if you want the slider to snap to the nearest tick instead
     *        of snapping to the direct location that the user clicked.
     * \param shouldSnap true if you want to it to snap, false otherwise.
     */
    void setSnapToNearestTick(bool shouldSnap);

    /*!
     * \brief setMinimumPossible allows the user to set a value other than the minimum as the minimum possible
     *        value the slider can hit.
     * \param useMinimumPossible tue if you want to use this new minimum, false otherwise.
     * \param minimumPossible the value that you want to be a minimum.
     */
    void setMinimumPossible(bool useMinimumPossible, int minimumPossible);

    /*!
     * \brief setSliderHeight By default sliders take up all of the of the widget, but this covers ticks
     *        and leaves no room inbewtween sliders that are stacked. This function takes a value between
     *        0.0 and 1.0 to  scale the slider down, while keeping it centered in the widget.
     * \param percent a value between 0.0 and 1.0 that scales the slider to take up less of hte widget.
     */
    void setSliderHeight(float percent);

    /*!
     * \brief enable sets whether the slider is enabled or disabled. Disabling the slider adds opacity,
     *        greys out the color, and disables user interaction.
     * \param shouldEnable true if the asset should be enabled, false otherwise.
     */
    void enable(bool shouldEnable);

signals:
    /*!
     * \brief Sends out the value that the slider has been set to after all the processing of snapping
     *        it to a tick or accounting for its minium value or any of that.
     */
    void valueChanged(int);

private slots:
    /*!
     * \brief receivedValue called whenever the QSlider signals valueChanged().
     */
    void receivedValue(int);

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


protected:
    /*!
     * \brief resizeEvent makes sure that the QSlider resizes with the QWidget
     */
    void resizeEvent(QResizeEvent *);
    /*!
     * \brief paintEvent handles edge cases created by the having a QSlider and
     *        an elaborate stylesheet. Draws the application's background color
     *        and the slider's ticks.
     */
    void paintEvent(QPaintEvent *);

    /*!
     * \brief showEvent used to turn on the throttle timer
     */
    void showEvent(QShowEvent *);

    /*!
     * \brief hideEvent used to turn off the throttle timer
     */
    void hideEvent(QHideEvent *);

private:

    /*!
     * \brief mHeightScaleFactor used to scale the slider inside of its qwidget so it takes up
     *        less than 100% of the widget
     */
    float mHeightScaleFactor;

    /*!
     * \brief mLayout layout of a lights slider
     */
    QVBoxLayout *mLayout;

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

    /*!
     * \brief Makes it so that by default, the slider jumps to the position the user
     *        clicks instead of jumping up by a page value. This gives the slider a more
     *        "mobile like" experience than the standard QSlider.
     *
     *        solution based on this stack overflow response:
     *        http://stackoverflow.com/a/15321654
     *
     * \param slider the slider that was clicked
     * \param newPos the position that user clicked.
     * \return the new position of the slider.
     */
    int jumpSliderToPosition(QSlider *slider, int newPos);

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
    int snapSliderToNearestTick(QSlider *slider, int pos);

    /*!
     * \brief mThrottleTimer throttles the speed that the lights sliders update,
     *        since without this it updates on each redraw which can really clog
     *        nearly all communication streams.
     */
    QTimer *mThrottleTimer;

    /*!
     * \brief mOpacity opacity of the lights slider, which gets changed when its
     *        enabled or disabled.
     */
    float mOpacity;

    /*!
     * \brief mThrottleFlag flag used to enforced the throttle timer's throttle.
     */
    bool mThrottleFlag;

    /*!
     * \brief mSliderColorSet true if the slider color has been changed, false otherwise
     */
    bool mSliderColorSet;

    /*!
     * \brief mSliderImageSet true if slider has a background image, false otherwise
     */
    bool mSliderImageSet;

    /*!
     * \brief mPath path to resource for background image for the slider.
     */
    QString mPath;

    /*!
     * \brief mSliderColor the current slider color
     */
    QColor mSliderColor;
};

#endif // LIGHTSSLIDER_H
