#ifndef COLORSCHEMEGRID_H
#define COLORSCHEMEGRID_H
/*!
 * \copyright
 * Copyright (C) 2015 - 2018.
 * Released under the GNU General Public License.
 */

#include <QWidget>
#include <QPushButton>
#include <QLayout>
#include <QLabel>

/*!
 * \brief The ColorSchemeGrid class provides color swatches that show
 *        the current colors in a color scheme. The user cannot interact
 *        with these color swatches.
 */
class ColorSchemeGrid : public QWidget
{
    Q_OBJECT
public:
    /// constructor
    explicit ColorSchemeGrid(QWidget *parent = 0);

    /// update the count of colors to display
    void updateColorCount(uint32_t colorCount) { mColorCount = colorCount; }

    /*!
     * \brief updateColorScheme update the colors displayed on the grid
     * \param colorScheme the nwe colors to display
     */
    void updateColorScheme(const std::vector<QColor> colorScheme);

    /*!
     * \brief setColor set a color at the given index
     * \param i index to set a color for
     * \param color a new color for that index
     */
    void setColor(int i, QColor color);

private slots:
    /// resize the grid and its assets.
    void resizeEvent(QResizeEvent *);

private:
    /// count of colors
    uint32_t mColorCount;

    /// labels that show the colors in the grid
    std::vector<QLabel *> mColorLabels;

    /// stored colors of grid
    std::vector<QColor> mColors;

    /// layout
    QHBoxLayout *mLayout;

    /// maximum number of colors for the grid.
    uint32_t mMaximumSize;
};

#endif // COLORSCHEMEGRID_H
