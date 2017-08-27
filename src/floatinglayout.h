#ifndef FLOATINGLAYOUT_H
#define FLOATINGLAYOUT_H

#include <QWidget>
#include <QPushButton>
#include <QHBoxLayout>
#include "corlumabutton.h"

enum class EButtonSize {
    eSmall,
    eMedium,
    eRectangle
};

/*!
 * \copyright
 * Copyright (C) 2015 - 2017.
 * Released under the GNU General Public License.
 */

/*!
 * \brief The FloatingLayout class QWidget that floats in the top right corner of the screen instead
 *        of using the standard layout methods. This layout provides only two buttons. Its position
 *        must be managed by resizeEvent calls as standard layouts do not apply to it.
 */
class FloatingLayout : public QWidget
{
    Q_OBJECT
public:
    /*!
     * Constructor
     * \param makeVertical makes a vertical layout if true, makes a horizontal layout if false.
     * \param parent parent
     */
    explicit FloatingLayout(bool makeVertical = false, QWidget *parent = 0);

    /*!
     * \brief setupButtons takees the vector of names and the size given and updates the buttons
     *        so that theres a button for every name
     * \param buttons names of buttons. these names will be signaled out whenever the button
     *        is clicked.
     * \param buttonSize preferred size for buttons
     */
    void setupButtons(std::vector<QString> buttons, EButtonSize buttonSize = EButtonSize::eSmall);

    /*!
     * \brief move move layout to a new location based off of the top right point provided.
     * \param topRightPoint the new top right point of the layout on its parent.
     */
    void move(QPoint topRightPoint);

    /*!
     * \brief updateRoutineSingleColor update routine icon when a single color routine is changed.
     * \param routine new routine for icon.
     * \param color new color for icon.
     */
    void updateRoutineSingleColor(ELightingRoutine routine, QColor color);

    /*!
     * \brief updateRoutineMultiColor update routine icon with multi color routine.
     * \param routine new routine for icon.
     * \param colors the full array of colors
     * \param colorCount the number of colors to use from the array.
     */
    void updateRoutineMultiColor(ELightingRoutine routine, std::vector<QColor> colors, int colorCount);

    /*!
     * \brief updateGroupPageButtons update group page buttons using colors only found in the datalayer
     * \param colors mData->colors()
     */
    void updateGroupPageButtons(const std::vector<std::vector<QColor> >& colors);

    /*!
     * \brief updateDiscoveryButton update the icon of a discovery button. Shows the discovery state of the connection.
     * \param type comm type of discovery button
     * \param pixmap new pixmap for discovery button
     */
    void updateDiscoveryButton(ECommType type, QPixmap pixmap);

    /*!
     * \brief addMultiRoutineIcon add a multi routine icon
     * \param colors colors to use for multi routine icon.
     */
    void addMultiRoutineIcon(std::vector<QColor> colors);

    /*!
     * \brief highlightButton highlight the button with the given key. Unhighlights all other buttons.
     * \param key key of button to highlight.
     */
    void highlightButton(QString key);

    /*!
     * \brief highlightRoutineButton highlights the routine button. Allows the page that owns the floating layout
     *        to override its behavior, if it changes state.
     * \param shouldHighlight true to highlight the button, false to leave the button unhighlighted.
     */
    void highlightRoutineButton(bool shouldHighlight);

    /*!
     * \brief buttonCount returns the number of buttons in the floating layout
     * \return the number of buttons in the floating layout
     */
    uint32_t buttonCount() { return mButtons.size(); }

signals:
    /*!
     * \brief buttonPressed emitted whenever a button is pressed with a QString representing the
     *        the button that was pressed.
     */
    void buttonPressed(QString);

private slots:

    /*!
     * \brief buttonPressed handles whenever any button is clicked. Converts the click into a signal of the button's
     *        name.
     */
    void buttonPressed(int);

private:

    /*!
     * \brief isALightsButton true if button at index is a lights button, false if its not or if the index is out of range
     * \param index index of button that you want to check if its a lightsbutton
     * \return true if button at index is a lights button, false if its not or if the index is out of range
     */
    bool isALightsButton(int index);

    /*!
     * \brief buttonSize size of any individual button. All buttons are always the same size
     * \return size of any individual button
     */
    QSize buttonSize();

    /*!
     * \brief mButtons vector of buttons
     */
    std::vector<QPushButton*> mButtons;

    /*!
     * \brief mNames vector of names of those buttons.
     */
    std::vector<QString> mNames;

    /*!
     * \brief mHorizontalLayout layout for horizontal widgets
     */
    QHBoxLayout *mHorizontalLayout;

    /*!
     * \brief mVerticalLayout layout for vertical widgets
     */
    QVBoxLayout *mVerticalLayout;

    /// true if vertical floating layout, false if horizontal.
    bool mIsVertical;

    /// the routine button is made translucent if somethings not in sync, this is true if that is the case.
    bool mRoutineIsTranslucent;

    /// true if routine button is currently highlighted, false otherwise
    bool mRoutineIsHighlighted;
};

#endif // FLOATINGLAYOUT_H
