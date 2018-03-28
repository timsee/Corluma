#ifndef SINGLECOLORROUTINEWIDGET_H
#define SINGLECOLORROUTINEWIDGET_H

#include <QWidget>
#include "cor/button.h"


/// different types of routinebuttonswidgets
enum class EWidgetGroup {
    eSingleRoutines,
    eMultiRoutines
};

/*!
 * \copyright
 * Copyright (C) 2015 - 2018.
 * Released under the GNU General Public License.
 *
 * \brief The RoutineButtonsWidget class is a basic widget that holds a grid of lightsbuttons that each map to a set of routines.
 *        There are two versions of this widget: eSingleRoutines, which shows the single color routines, and eMultiRoutines, which
 *        shows the multi color routine. This widget is much smaller than the size of the application and is best used as an overlay
 *        for a larger page.
 */
class RoutineButtonsWidget : public QWidget
{
    Q_OBJECT
public:
    /*!
     * \brief RoutineButtonsWidget constructor
     * \param widgetGroup type of widgetgroup. can be either single color routines or multi color routines
     * \param colors the colors to use for multi color routines. Not necessary for single color routines.
     * \param parent parent of this widget
     */
    explicit RoutineButtonsWidget(EWidgetGroup widgetGroup, std::vector<QColor> colors = std::vector<QColor>(), QWidget *parent = 0);

    /*!
     * \brief highlightRoutineButton highlights the button that implements
     *        the routine parameter. If it can't find a button that
     *        implements the lighting routine, then all buttons are unhighlighted
     * \param routine the mode that the highlighted button implements.
     */
    void highlightRoutineButton(ELightingRoutine routine);

    /*!
     * \brief singleRoutineColorChanged can only be used by a eSingleRoutines version of the widget, this updates the color of
     *        of all of the icons to reflect the new color.
     * \param newColor new color for all of the icons.
     */
    void singleRoutineColorChanged(QColor newColor);

    /*!
     * \brief multiRoutineColorsChanged can only be used by eMultiRotuiens version of this widget, this updates all the colors
     *        of all the icons to reflect the new group of custom colors.
     * \param colors full group of custom colors
     * \param colorCount how many custom colors to use from that group.
     */
    void multiRoutineColorsChanged(const std::vector<QColor>& colors, int colorCount);

    /*!
     * \brief resize resize widget to given size
     * \param size new size for widget.
     */
    void resize(QSize size);

signals:

    /*!
     * \brief newRoutineSelected emitted whenever a button is pressed with the int representation
     *        of the routine that it represents.
     */
    void newRoutineSelected(ELightingRoutine);

protected:
    /*!
     * \brief paintEvent paint event for rendering. used to overwrite the background
     *        color of the widget so that it hides everything behind it.
     */
    void paintEvent(QPaintEvent *);

private slots:

    /*!
     * \brief routineChanged signaled whenever a routine button is pressed, used to
     *        to tell the LEDs to update.
     */
    void routineChanged(ELightingRoutine, EColorGroup);

private:

    /*!
     * \brief mLayout layout for widget
     */
    QGridLayout *mLayout;

    /*!
     * \brief mRoutineButtons pointers to all the main buttons, used
     *        to iterate through them quickly.
     */
    std::vector<cor::Button*> mRoutineButtons;
};

#endif // SINGLECOLORROUTINEWIDGET_H
