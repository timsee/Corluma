#ifndef CUSTOMCOLORPICKER_H
#define CUSTOMCOLORPICKER_H

#include "cor/palettewidget.h"
#include "cor/slider.h"

#include <QWidget>
/*!
 * \copyright
 * Copyright (C) 2015 - 2018.
 * Released under the GNU General Public License.
 *
 *
 * \brief The CustomColorPicker class is a color picker that allows the user to choose
 *        multiple colors in a palette
 */
class CustomColorPicker : public QWidget
{
    Q_OBJECT
public:
    /// constructor
    explicit CustomColorPicker(QWidget *parent = nullptr);

    /*!
     * \brief updateMultiColor programmatically set the colors in the multi color picker. This will update
     *        the UI elements to reflect the values provided. By default it will also signal its
     *        changes, but a flag can be used to override the signal.
     * \param colors the colors for the array in the multi color picker
     * \param count the number of colors to use from the vector
     */
    void updateMultiColor(const std::vector<QColor>& colors, int count);

    /*!
     * \brief updateSelected progrmatically update the selected buttons with a new color. Does not emit
     *        a signal when complete
     * \param new color for selected indices
     */
    void updateSelected(QColor color);

    /// getter for pointer of color palette
    cor::PaletteWidget *palette() { return mColorGrid; }

signals:
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
     * \brief releasedSlider uses the QSlider inside of the LightsSlider to pick up
     *        when the slider is released. This always sets the color of the color picker.
     *        This system is used to prevent an edge case with throttling with a timer.
     *        Without it, its possible to change the UI without updating the lights if you are
     *        quick enough.
     */
    void releasedSlider();

private:

    /// layout
    QVBoxLayout *mLayout;

    /// color palette widget
    cor::PaletteWidget *mColorGrid;

    /*!
     * \brief mCountSlider slider that determines how many colors are used.
     */
    cor::Slider *mCountSlider;

    /// color values for the multi layout
    std::vector<QColor> mColors;

    /// the maximum number of colors that can be used
    uint32_t mColorsUsed;

    /*!
     * \brief mMaximumSize size of the multi color array, used to initialize
     *        and access vectors throughout this page.
     */
    uint32_t mMaximumSize;


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

};

#endif // CUSTOMCOLORPICKER_H
