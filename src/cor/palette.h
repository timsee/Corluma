#ifndef COLORGRID_H
#define COLORGRID_H

#include <QWidget>
#include "cor/button.h"
#include "cor/slider.h"

namespace cor
{

/*!
 * \copyright
 * Copyright (C) 2015 - 2018.
 * Released under the GNU General Public License.
 *
 *
 *
 * \brief The ColorGrid class is a bottom layout for the color picker that allows the user to choose up to
 *        10 colors at once and change them all to a new color. It is used for multi color routines. A slider
 *        is also on top to determine how many colors are used for this routine.
 */
class Palette: public QWidget
{
    Q_OBJECT
public:
    /// Constructor
    explicit Palette(uint32_t width, uint32_t height, bool showSlider, QWidget *parent = 0);

    /*!
     * \brief updateMultiColor programmatically set the colors in the multi color picker. This will update
     *        the UI elements to reflect the values provided. By default it will also signal its
     *        changes, but a flag can be used to override the signal.
     * \param colors the colors for the array in the multi color picker
     * \param count the number of colors to use from the vector
     */
    void updateMultiColor(const std::vector<QColor>& colors, int count);

    /*!
     * \brief setupButtons setup buttons with device data
     * \param devices devices to use the buttons
     */
    void setupButtons(const std::list<cor::Light>& devices);

    /*!
     * \brief updateDevices update the devices in the cor::Button to show the exact routine.
     * \param devices list of devices to display
     */
    void updateDevices(const std::list<cor::Light>& devices);

    /*!
     * \brief loadColorGroups
     * \param colorGroups
     */
    void loadColorGroups(std::vector<std::vector<QColor> > colorGroups);

    /*!
     * \brief enableButtonInteraction true to allow button interaction, false to
     *        ignore buttons and treat them as images instead. Clicks will go through buttons
     *        to the widget.
     * \param enable true to enable, false to disable.
     */
    void enableButtonInteraction(bool enable);

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

protected:
    /// resize the grid and its assets.
    void resizeEvent(QResizeEvent *);

private:

    /*!
     * \brief mCountSlider slider that determines how many colors are used.
     */
    cor::Slider *mCountSlider;

    /// currently unused, but in place so that slider sizes match other layouts
    QLabel *mLabel;

    /*!
     * \brief mMaximumSize size of the multi color array, used to initialize
     *        and access vectors throughout this page.
     */
    uint32_t mMaximumSize;

    /// true show count slider, false to hide
    bool mShowSlider;

    /// number of columns of palettes
    uint32_t mWidth;

    /// number of rows of palettes
    uint32_t mHeight;

    /// vector pushbuttons used for the multi layout
    std::vector<cor::Button *> mArrayColorsButtons;

    /// color values for the multi layout
    std::vector<QColor> mColors;

    /// the maximum number of colors that can be used
    uint32_t mColorsUsed;

    /// color groups used to display color palette state
    std::vector<std::vector<QColor> > mColorGroups;

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

}

#endif // COLORGRID_H
