#ifndef ROUTINECONTAINER_H
#define ROUTINECONTAINER_H

#include <QLabel>
#include <QWidget>
#include "cor/objects/lightstate.h"
#include "cor/widgets/button.h"
#include "routines/multibarsroutinewidget.h"
#include "routines/multifaderoutinewidget.h"
#include "routines/multiglimmerroutinewidget.h"
#include "routines/multirandomroutinewidget.h"
#include "routines/singlefaderoutinewidget.h"
#include "routines/singleglimmerroutinewidget.h"
#include "routines/singlesolidroutinewidget.h"
#include "routines/singlewaveroutinewidget.h"
#include "routines/speedslider.h"

/// different types of routinebuttonswidgets
enum class ERoutineGroup { single, multi, all };

/*!
 * \copyright
 * Copyright (C) 2015 - 2020.
 * Released under the GNU General Public License.
 *
 *
 * \brief The RoutineContainer class is a container that contains all the widgets related
 * to displaying routines.
 */
class RoutineContainer : public QWidget {
    Q_OBJECT
public:
    explicit RoutineContainer(QWidget* parent, ERoutineGroup routineGroup);

    /// highlight a specific routine button in the container based off of the enum and parameter
    void highlightRoutine(ERoutine routine, int param);

    /// update the color displayed by the widgets
    void changeColor(const QColor& color);

    /// change the palette displayed
    void changeColorScheme(const std::vector<QColor>& colors);

    /// change the speed displayed by the speed slider
    void changeSpeed(int speed);

    /// getter for state for the widget
    const cor::LightState& state() { return mState; }

signals:
    /// emits when a routine is selected, sends just the routine itself
    void newRoutineSelected(ERoutine, int, int);

protected:
    /*!
     * \brief resizeEvent called every time the main window is resized.
     */
    void resizeEvent(QResizeEvent*) { resize(); }

    /// handles hwen the widget is painted
    void paintEvent(QPaintEvent*);

private slots:

    /*!
     * \brief routineChanged signaled whenever a routine button is pressed, used to
     * to tell the LEDs to update.
     */
    void routineChanged(const cor::LightState&);

    /// handles when the time slder changes.
    void speedSliderChanged(int sliderValue);

private:
    /// init the buttons used by the widget
    void initButtons();

    /// resize programmatically
    void resize();

    /// widget for displaying all lights as one solid color
    SingleSolidRoutineWidget* mSingleSolidRoutineWidget;

    /// widget for applying a glimmer to the lights (at random, certain lights are slightly dimmed)
    SingleGlimmerRoutineWidget* mSingleGlimmerRoutineWidget;

    /// widget for applying a fade to all lights (fade may be linear, sine, square, or sawtooth)
    SingleFadeRoutineWidget* mSingleFadeRoutineWidget;

    /// widget for dimming and turning on lights
    SingleWaveRoutineWidget* mSingleWaveRoutineWidget;

    /// widget for bars that scroll across the light
    MultiBarsRoutineWidget* mMultiBarsRoutineWidget;

    /// widget for fading between colors
    MultiFadeRoutineWidget* mMultiFadeRoutineWidget;

    /// widget for randomly showing colors
    MultiRandomRoutineWidget* mMultiRandomRoutineWidget;

    /// widget for using one color as a base and showing other colors within it
    MultiGlimmerRoutineWidget* mMultiGlimmerRoutineWidget;

    /// vector of all single routine widgets
    std::vector<RoutineWidget*> mSingleRoutineWidgets;

    /// vector of all multi routine widgets
    std::vector<RoutineWidget*> mMultiRoutineWidgets;

    /// vector of all widgets displayed by the container
    std::vector<RoutineWidget*> mAllRoutineWidgets;

    /// slider for controlling the speed.
    SpeedSlider* mSpeedSlider;

    /// stored state for the single routine
    cor::LightState mState;

    /// the type of routines shown by the container.
    ERoutineGroup mRoutineGroup;
};

#endif // ROUTINECONTAINER_H
