#ifndef CUSTOMCOLORPICKER_H
#define CUSTOMCOLORPICKER_H

#include "cor/slider.h"

#include "swatchvectorwidget.h"

#include <QWidget>

/*!
 * \copyright
 * Copyright (C) 2015 - 2019.
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
    explicit CustomColorPicker(QWidget *parent);

    /*!
     * \brief updateSelected progrmatically update the selected buttons with a new color. Does not emit
     *        a signal when complete
     * \param new color for selected indices
     */
    void updateSelected(QColor color);

    /// update the colors used by the buttons
    void updateMultiColor(const std::vector<QColor>& colors);

    /// getter for pointer of color palette
    SwatchVectorWidget *palette() { return mColorGrid; }

    /// getter for the colors used by the buttons
    const std::vector<QColor>& colors() { return mColorGrid->colors(); }

signals:
    /*!
     * \brief multiColorCountChanged number of colors to use during multi color routines changed.
     */
    void multiColorCountChanged(int);

protected:

    /// called when widget is resized
    void resizeEvent(QResizeEvent *);

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

    /// change the selected max count
    void selectedMaxChanged(uint32_t maxCount);

    /// layout
    QVBoxLayout *mLayout;

    /// color palette widget
    SwatchVectorWidget *mColorGrid;

    /*!
     * \brief mCountSlider slider that determines how many colors are used.
     */
    cor::Slider *mCountSlider;

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
