#ifndef ROUTINECONTAINER_H
#define ROUTINECONTAINER_H

#include <QLabel>
#include <QWidget>
#include "cor/objects/lightstate.h"
#include "cor/widgets/button.h"
#include "routines/singlefaderoutinewidget.h"
#include "routines/singleglimmerroutinewidget.h"
#include "routines/singlesolidroutinewidget.h"
#include "routines/singlewaveroutinewidget.h"

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

    /*!
     * \brief highlightRoutineButton highlights the button that implements
     *        the routine parameter. If it can't find a button that
     *        implements the lighting routine, then all buttons are unhighlighted
     *
     * \param label the name of the button
     */
    void highlightRoutineButton(const QString& label);

    /// update the color displayed by the widgets
    void changeColor(const QColor& color);

    /// change the palette displayed
    void changePalette(const cor::Palette& palette);

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

    /// find the label for a button given its state
    QString labelFromRoutine(ERoutine routine, int param);

    /// widget for displaying all lights as one solid color
    SingleSolidRoutineWidget* mSingleSolidRoutineWidget;

    /// widget for applying a glimmer to the lights (at random, certain lights are slightly dimmed)
    SingleGlimmerRoutineWidget* mSingleGlimmerRoutineWidget;

    /// widget for applying a fade to all lights (fade may be linear, sine, square, or sawtooth)
    SingleFadeRoutineWidget* mSingleFadeRoutineWidget;

    /// widget for dimming and turning on lights
    SingleWaveRoutineWidget* mSingleWaveRoutineWidget;

    /// vector of routines
    std::vector<std::pair<QString, cor::LightState>> mRoutines;

    /*!
     * \brief mSingleRoutineButtons pointers to all the main buttons
     */
    std::vector<cor::Button*> mRoutineButtons;

    /// slider to change the speed.
    cor::Slider* mSpeedSlider;

    /// vector of labels
    std::vector<QLabel*> mLabels;

    /// stored state for the single routine
    cor::LightState mState;

    /// the type of routines shown by the container.
    ERoutineGroup mRoutineGroup;
};

#endif // ROUTINECONTAINER_H
