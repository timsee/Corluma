#ifndef ROUTINEBUTTONSWIDGET_H
#define ROUTINEBUTTONSWIDGET_H

#include <QWidget>

#include "cor/objects/light.h"
#include "cor/widgets/button.h"


/// different types of routinebuttonswidgets
enum class EWidgetGroup { singleRoutines, multiRoutines, both };

/*!
 * \copyright
 * Copyright (C) 2015 - 2020.
 * Released under the GNU General Public License.
 *
 * \brief The RoutineButtonsWidget class is a basic widget that holds a grid of lightsbuttons that
 * each map to a set of routines. There are two versions of this widget: eSingleRoutines, which
 * shows the single color routines, and eMultiRoutines, which shows the multi color routine. This
 * widget is much smaller than the size of the application and is best used as an overlay for a
 * larger page.
 */
class RoutineButtonsWidget : public QWidget {
    Q_OBJECT
public:
    /*!
     * \brief RoutineButtonsWidget constructor
     *
     * \param parent parent of this widget
     */
    explicit RoutineButtonsWidget(QWidget* parent);

    /*!
     * \brief highlightRoutineButton highlights the button that implements
     *        the routine parameter. If it can't find a button that
     *        implements the lighting routine, then all buttons are unhighlighted
     *
     * \param label the name of the button
     */
    void highlightRoutineButton(const QString& label);

    /*!
     * \brief singleRoutineColorChanged can only be used by a eSingleRoutines version of the widget,
     * this updates the color of of all of the icons to reflect the new color.
     *
     * \param newColor new color for all of the icons.
     */
    void singleRoutineColorChanged(const QColor& newColor);

    /*!
     * \brief multiRoutineColorsChanged can only be used by eMultiRotuines version of this widget,
     * this updates all the colors of all the icons to reflect the new group of custom colors.
     *
     * \param colors full group of custom colors
     */
    void multiRoutineColorsChanged(const std::vector<QColor>& colors);

    /*!
     * \brief resize resize widget to given size
     *
     * \param size new size for widget.
     */
    void resize(int x, QSize size);

    /// resize the widget when it is a static page. no arguments needed
    void resizeStaticPage();

    /// change the routines being shown by the widget
    void changeRoutines(EWidgetGroup);

    /// push widget in, displaying one of the widget groups
    void pushIn();

    /// push widget out
    void pushOut();

    /// true if open, false if hidden
    bool isOpen() const noexcept { return mIsOpen; }

    /// last routine clicked on the single routine options
    ERoutine singleRoutine() { return mSingleState.routine(); }

    /// last routine clicke don the multi routine options
    ERoutine multiRoutine() { return mMultiState.routine(); }

    /// last parameter used
    std::uint32_t parameter() { return state().param(); }

signals:

    /// emits when a routine is selected, sends just the routine itself
    void newRoutineSelected(ERoutine);

private slots:

    /*!
     * \brief routineChanged signaled whenever a routine button is pressed, used to
     * to tell the LEDs to update.
     */
    void routineChanged(const cor::LightState&);

private:
    /// getter for current state based off of widget group.
    const cor::LightState& state() const noexcept {
        if (mWidgetGroup == EWidgetGroup::multiRoutines) {
            return mMultiState;
        } else {
            return mSingleState;
        }
    }

    /// initializes the widgets used in the single routine version of the widget
    void initSingleRoutineButtons();

    /// initailizes the widgets used in the multi routine version of the widget
    void initMultiRoutinesButtons();

    /// find the label for a button given its state
    QString labelFromState(const cor::LightState& state);

    /// true if showing, false if hidden
    bool mIsOpen;

    /// widget to display the single routine options
    QWidget* mSingleWidget;

    /*!
     * \brief mSingleLayout layout for single widget
     */
    QGridLayout* mSingleLayout;

    /// widget to display the multi routine options
    QWidget* mMultiWidget;

    /*!
     * \brief mMultiLayout layout for multi widget
     */
    QGridLayout* mMultiLayout;

    /// vector of single routines
    std::vector<std::pair<QString, cor::LightState>> mSingleRoutines;

    /// vector of multi routines
    std::vector<std::pair<QString, cor::LightState>> mMultiRoutines;

    /*!
     * \brief mSingleRoutineButtons pointers to all the main buttons
     */
    std::vector<cor::Button*> mSingleRoutineButtons;

    /*!
     * \brief mMultiRoutineButtons pointers to all the main buttons
     */
    std::vector<cor::Button*> mMultiRoutineButtons;

    /// vector of single labels
    std::vector<QLabel*> mSingleLabels;

    /// vector of multi labels
    std::vector<QLabel*> mMultiLabels;

    /// stored state for the single routine
    cor::LightState mSingleState;

    /// stored state for the multi routine
    cor::LightState mMultiState;

    /// current group being displayed
    EWidgetGroup mWidgetGroup;
};

#endif // ROUTINEBUTTONSWIDGET_H
