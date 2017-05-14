#ifndef COLORGRID_H
#define COLORGRID_H

#include <QWidget>
#include "corlumabutton.h"
#include "corlumaslider.h"

/*!
 * \copyright
 * Copyright (C) 2015 - 2017.
 * Released under the GNU General Public License.
 *
 *
 *
 * \brief The ColorGrid class is a bottom layout for the color picker that allows the user to choose up to
 *        10 colors at once and change them all to a new color. It is used for multi color routines. A slider
 *        is also on top to determine how many colors are used for this routine.
 */
class ColorGrid: public QWidget
{
    Q_OBJECT
public:
    /// Constructor
    explicit ColorGrid(QWidget *parent = 0);

    /*!
     * \brief updateMultiColor programmatically set the colors in the multi color picker. This will update
     *        the UI elements to reflect the values provided. By default it will also signal its
     *        changes, but a flag can be used to override the signal.
     * \param colors the colors for the array in the multi color picker
     * \param count the number of colors to use from the vector
     */
    void updateMultiColor(const std::vector<QColor> colors, int count);

    /*!
     * \brief updateSelected progrmatically update the selected buttons with a new color. Does not emit
     *        a signal when complete
     * \param new color for selected indices
     */
    void updateSelected(QColor color);

    /*!
     * \brief selectedCount number of selected devices by the ColorGrid
     * \return  number of selected devices.
     */
    uint32_t selectedCount() { return mSelectedIndices.size(); }

    /*!
     * \brief selected list of all selected indices
     * \return list of indices
     */
    std::list<uint32_t> selected() { return mSelectedIndices; }

signals:
    /*!
     * \brief multiColorChanged emitted whenever the multi color picker has an update for any individual color.
     */
    void multiColorChanged(QColor, int);

    /*!
     * \brief multiColorCountChanged number of colors to use during multi color routines changed.
     */
    void multiColorCountChanged(int);

    /*!
     * \brief selectedCountChanged a button was pressed so the selected count has changed. emits new count.
     */
    void selectedCountChanged(int);

private slots:

    /*!
     * \brief countSliderChanged handles when the count slider changes its value.
     */
    void countSliderChanged(int);

    /*!
     * \brief selectArrayColor when called, the multi color array color at the given index is seletected
     *        or deselected, depending on its current state.
     */
    void selectArrayColor(int);

    /*!
     * \brief releasedSlider uses the QSlider inside of the LightsSlider to pick up
     *        when the slider is released. This always sets the color of the color picker.
     *        This system is used to prevent an edge case with throttling with a timer.
     *        Without it, its possible to change the UI without updating the lights if you are
     *        quick enough.
     */
    void releasedSlider();
private:

    /*!
     * \brief createSolidColorIcon creates a QPushButton that has an icon that is a solid color that matches the QColor
     *        that is provided as input.
     * \param color the color of the QPushButton's icon.
     * \return  a pixmap that is a single solid color that matches the input.
     */
    QPixmap createSolidColorIcon(QColor color);

    /*!
     * \brief mCountSlider slider that determines how many colors are used.
     */
    CorlumaSlider *mCountSlider;

    /// currently unused, but in place so that slider sizes match other layouts
    QLabel *mLabel;

    /*!
     * \brief mMaximumSize size of the multi color array, used to initialize
     *        and access vectors throughout this page.
     */
    uint32_t mMaximumSize;

    /// vector pushbuttons used for the multi layout
    std::vector<QPushButton *> mArrayColorsButtons;

    /// color values for the multi layout
    std::vector<QColor> mColors;

    /// the maximum number of colors that can be used
    uint32_t mColorsUsed;

    /// indices of selected colors in multi color layout.
    std::list<uint32_t> mSelectedIndices;

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

    /*!
     * \brief mLayout layout used to arrange the slider and the buttons.
     */
    QGridLayout *mLayout;
};

#endif // COLORGRID_H
