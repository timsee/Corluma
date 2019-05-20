#ifndef SINGLECOLORROUTINEWIDGET_H
#define SINGLECOLORROUTINEWIDGET_H

#include <QWidget>
#include "cor/button.h"
#include "cor/light.h"


/// different types of routinebuttonswidgets
enum class EWidgetGroup { singleRoutines, multiRoutines };

/*!
 * \copyright
 * Copyright (C) 2015 - 2019.
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
     * \param widgetGroup type of widgetgroup. can be either single color routines or multi color
     * routines
     * \param colors the colors to use for multi color routines. Not necessary for single
     * color routines.
     * \param parent parent of this widget
     */
    explicit RoutineButtonsWidget(EWidgetGroup widgetGroup,
                                  const std::vector<QColor>& colors,
                                  QWidget* parent);

    /*!
     * \brief highlightRoutineButton highlights the button that implements
     *        the routine parameter. If it can't find a button that
     *        implements the lighting routine, then all buttons are unhighlighted
     *
     * \param label the name of the button
     */
    void highlightRoutineButton(const QString& label);

    /*!
     * \brief jsonToButtonName takes a json object that represents a routine as input and if it
     * matches any of the json objects used by routine buttons, it returns the name. Otherwise, it
     * returns an empty string
     *
     * \param routineObject the routine object to match to a button
     * \return the name of the json button.
     */
    QString jsonToButtonName(const QJsonObject& routineObject);

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
    void resize(QSize size);

    /// getter for routines
    const std::vector<std::pair<QString, QJsonObject>> routines() { return mRoutines; }

    /// true to show widget, false to hide it
    void showWidget(bool shouldShow);

    /// true if open, false if hidden
    bool isOpen() const noexcept { return mIsOpen; }

signals:

    /*!
     * \brief newRoutineSelected emitted whenever a button is pressed with the int representation
     *        of the routine that it represents.
     */
    void newRoutineSelected(QJsonObject);

protected:
    /*!
     * \brief paintEvent paint event for rendering. used to overwrite the background
     * color of the widget so that it hides everything behind it.
     */
    void paintEvent(QPaintEvent*);

private slots:

    /*!
     * \brief routineChanged signaled whenever a routine button is pressed, used to
     * to tell the LEDs to update.
     */
    void routineChanged(const QJsonObject&);

private:
    /*!
     * \brief mLayout layout for widget
     */
    QGridLayout* mLayout;

    /// vector of routines
    std::vector<std::pair<QString, QJsonObject>> mRoutines;

    /// true if showing, false if hidden
    bool mIsOpen;

    /*!
     * \brief mRoutineButtons pointers to all the main buttons, used
     * to iterate through them quickly.
     */
    std::vector<cor::Button*> mRoutineButtons;

    /// vector of labels
    std::vector<QLabel*> mLabels;
};

#endif // SINGLECOLORROUTINEWIDGET_H
