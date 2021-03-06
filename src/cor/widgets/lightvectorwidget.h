#ifndef COLORGRID_H
#define COLORGRID_H

#include <QWidget>

#include "cor/objects/light.h"
#include "cor/widgets/button.h"
#include "cor/widgets/slider.h"

namespace cor {

/*!
 * \copyright
 * Copyright (C) 2015 - 2020.
 * Released under the GNU General Public License.
 *
 *
 *
 * \brief The LightVectorWidget class is a bottom layout for the color picker that allows the user
 * to choose up to 10 colors at once and change them all to a new color. It is used for multi color
 * routines. A slider is also on top to determine how many colors are used for this routine.
 */
class LightVectorWidget : public QWidget {
    Q_OBJECT
public:
    /// Constructor
    explicit LightVectorWidget(std::uint32_t width,
                               std::uint32_t height,
                               bool fillFromLeft,
                               QWidget* parent);
    /*!
     * \brief updateDevices update the lights in the cor::Button to show the exact routine.
     * \param devices list of devices to display
     */
    void updateLights(const std::vector<cor::Light>& lights);

    /// count of how many lights are shown
    std::uint32_t lightCount() { return mWidth * mHeight; }

    /// set whether or not to hide off lights
    void hideOffLights(bool shouldHide) { mHideOffLights = shouldHide; }

    /// set the percent of an icon that each button should take up.
    void setButtonIconPercent(float percent) {
        for (auto button : mArrayColorsButtons) {
            button->setIconPercent(percent);
        }
    }

    /*!
     * \brief enableButtonInteraction true to allow button interaction, false to
     *        ignore buttons and treat them as images instead. Clicks will go through buttons
     *        to the widget.
     * \param enable true to enable, false to disable.
     */
    void enableButtonInteraction(bool enable);

    /*!
     * \brief selectedCount number of selected lights by the ColorGrid
     * \return  number of selected devices.
     */
    uint32_t selectedCount();

    /// getter for vector of buttons in widget.
    std::vector<cor::Button*> buttons() { return mArrayColorsButtons; }

signals:
    /*!
     * \brief selectedCountChanged a button was pressed so the selected count has changed. emits new
     * count.
     */
    void selectedCountChanged(int);

private slots:
    /*!
     * \brief toggleArrayColor when called, the multi color array color at the given index is
     * seletected or deselected, depending on its current state.
     */
    void toggleArrayColor();

protected:
    /// called when the widget resizes
    virtual void resizeEvent(QResizeEvent*);

private:
    /// true if widget should fill new entries from left, false if it should fill from right
    bool mFillFromLeft;

    /// hide devices if they are off
    bool mHideOffLights;

    /*!
     * \brief mMaximumSize size of the multi color array, used to initialize
     *        and access vectors throughout this page.
     */
    std::uint32_t mMaximumSize;

    /// number of columns of palettes
    std::uint32_t mWidth;

    /// number of rows of palettes
    std::uint32_t mHeight;

    /// vector pushbuttons used for the multi layout
    std::vector<cor::Button*> mArrayColorsButtons;

    /// vector for storing labels for the palette
    std::vector<QLabel*> mArrayLabels;

    /// color groups used to display color palette state
    std::vector<std::vector<QColor>> mColorGroups;

    /*!
     * \brief mLayout layout used to arrange the slider and the buttons.
     */
    QGridLayout* mLayout;
};

} // namespace cor

#endif // COLORGRID_H
