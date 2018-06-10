#ifndef FLOATINGLAYOUT_H
#define FLOATINGLAYOUT_H

#include <QWidget>
#include <QPushButton>
#include <QHBoxLayout>
#include "cor/button.h"
#include "cor/presetpalettes.h"

/// size of buttons in widget
enum class EButtonSize {
    small,
    medium,
    rectangle
};

/*!
 * \copyright
 * Copyright (C) 2015 - 2018.
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
    explicit FloatingLayout(bool makeVertical, QWidget *parent);

    /*!
     * \brief setupButtons takees the vector of names and the size given and updates the buttons
     *        so that theres a button for every name
     * \param buttons names of buttons. these names will be signaled out whenever the button
     *        is clicked.
     * \param buttonSize preferred size for buttons
     */
    void setupButtons(std::vector<QString> buttons, EButtonSize buttonSize);

    /*!
     * \brief move move layout to a new location based off of the top right point provided.
     * \param topRightPoint the new top right point of the layout on its parent.
     */
    void move(QPoint topRightPoint);

    /*!
     * \brief updateRoutine show a routine on the button
     * \param routineObject the json representatino of the routine
     */
    void updateRoutine(const QJsonObject& routineObject);

    /*!
     * \brief updateDiscoveryButton update the icon of a discovery button. Shows the discovery state of the connection.
     * \param type comm type of discovery button
     * \param pixmap new pixmap for discovery button
     */
    void updateDiscoveryButton(EProtocolType type, QPixmap pixmap);

    /*!
     * \brief addMultiRoutineIcon add a multi routine icon
     * \param colors colors to use for multi routine icon.
     */
    void addMultiRoutineIcon(std::vector<QColor> colors);

    /*!
     * \brief updateColorPageButton update the color page button with given resource
     * \param resource the resource to draw on the color page button.
     */
    void updateColorPageButton(QString resource);

    /*!
     * \brief updateCollectionButton update the collection's edit or new icon.
     * \param resource the resource to draw on the collection button
     */
    void updateCollectionButton(QString resource);

    /*!
     * \brief highlightButton highlight the button with the given key. Unhighlights all other buttons.
     * \param key key of button to highlight.
     */
    void highlightButton(QString key);

    /*!
     * \brief enableButton programmatically enable or disable buttons
     * \param key key of button to enable or disable
     * \param enable true to enable, false to disable.
     */
    void enableButton(QString key, bool enable);

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

    /// preset data for palettes from ArduCor
    PresetPalettes mPalettes;

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

    /// original size of the application during initial load.
    QSize mOriginalSize;
};

#endif // FLOATINGLAYOUT_H
