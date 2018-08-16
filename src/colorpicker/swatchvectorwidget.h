#ifndef SWATCH_VECTOR_WIDGET_H
#define SWATCH_VECTOR_WIDGET_H

#include "cor/slider.h"

#include <QWidget>
#include <QPushButton>

/*!
 * \copyright
 * Copyright (C) 2015 - 2018.
 * Released under the GNU General Public License.
 *
 *
 *
 * \brief The SwatchVectorWidget class is a widget that displays buttons that show solid colors. It is used
 *        in the color scheme picker and the custom color picker to choose colors to change and display the current settings.
 */
class SwatchVectorWidget: public QWidget
{
    Q_OBJECT
public:
    /// Constructor
    explicit SwatchVectorWidget(uint32_t width, uint32_t height, QWidget *parent);
    /*!
     * \brief updateDevices update the devices in the cor::Button to show the exact routine.
     * \param devices list of devices to display
     */
    void updateColors(const std::vector<QColor>& colors);

    /*!
     * \brief selectedCount number of selected devices by the ColorGrid
     * \return  number of selected devices.
     */
    uint32_t selectedCount();

    /// getter for the buttons used for the swatches
    const std::vector<QPushButton*>& buttons() { return mSwatches; }

    /// getter for the colors displayed in the swatch vector widget
    const std::vector<QColor>& colors() { return mColors; }

    /// update the selected lights with the given color
    void updateSelected(QColor color);

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
     * \brief toggleArrayColor when called, the multi color array color at the given index is seletected
     *        or deselected, depending on its current state.
     */
    void toggleArrayColor(int);

private:

    /// currently unused, but in place so that slider sizes match other layouts
    QLabel *mLabel;

    /*!
     * \brief mMaximumSize size of the multi color array, used to initialize
     *        and access vectors throughout this page.
     */
    uint32_t mMaximumSize;

    /// number of columns of palettes
    uint32_t mWidth;

    /// number of rows of palettes
    uint32_t mHeight;

    /// vector pushbuttons used for the multi layout
    std::vector<QPushButton*> mSwatches;

    /// stores the current colors of the swatches
    std::vector<QColor> mColors;

    /*!
     * \brief mLayout layout used to arrange the slider and the buttons.
     */
    QGridLayout *mLayout;
};

#endif // SWATCH_VECTOR_WIDGET_H
