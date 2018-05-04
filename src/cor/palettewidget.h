#ifndef COLORGRID_H
#define COLORGRID_H

#include <QWidget>
#include "cor/button.h"
#include "cor/slider.h"

namespace cor
{

/// type of widget
enum class EPaletteWidgetType {
    eStandard,
    eInfo
};

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
class PaletteWidget: public QWidget
{
    Q_OBJECT
public:
    /// Constructor
    explicit PaletteWidget(uint32_t width, uint32_t height,
                           std::vector<std::vector<QColor> > colorGroups,
                           EPaletteWidgetType type,
                           QWidget *parent = 0);
    /*!
     * \brief updateDevices update the devices in the cor::Button to show the exact routine.
     * \param devices list of devices to display
     */
    void updateDevices(const std::list<cor::Light>& devices);

    /*!
     * \brief enableButtonInteraction true to allow button interaction, false to
     *        ignore buttons and treat them as images instead. Clicks will go through buttons
     *        to the widget.
     * \param enable true to enable, false to disable.
     */
    void enableButtonInteraction(bool enable);

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

    /// getter for vector of buttons in widget.
    std::vector<cor::Button *> buttons() { return mArrayColorsButtons; }

    /*!
     * \brief manageMultiSelected handles the selected buttons in the Multi color layout. This
     *        includes deselecting indices that are larger than the mMultiUsed value, highlighting
     *        selected indices, and enabling/disabling the color wheel based on the count of selected
     *        indices.
     */
    void manageMultiSelected();

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
     * \brief selectArrayColor when called, the multi color array color at the given index is seletected
     *        or deselected, depending on its current state.
     */
    void selectArrayColor(int);

private:

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

    /// vector for storing labels for the palette
    std::vector<QLabel*> mArrayLabels;

    /// color groups used to display color palette state
    std::vector<std::vector<QColor> > mColorGroups;

    /// indices of selected colors in multi color layout.
    std::list<uint32_t> mSelectedIndices;

    /*!
     * \brief mLayout layout used to arrange the slider and the buttons.
     */
    QGridLayout *mLayout;

    /// type of widget
    EPaletteWidgetType mType;
};

}

#endif // COLORGRID_H
