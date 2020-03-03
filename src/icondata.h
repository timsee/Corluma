
#ifndef ICONDATA_H
#define ICONDATA_H

#include <QPixmap>

#include "cor/objects/lightstate.h"

/*!
 * \copyright
 * Copyright (C) 2015 - 2020.
 * Released under the GNU General Public License.
 *
 *
 * \brief The IconData class creates all the pixel-y square icons that are seen throughout
 * the app. The grids are made by using a very small buffer of RGB values to do the
 * computation.
 *
 */
class IconData {
public:
    /*!
     * \brief Constructor
     */
    IconData();

    /*!
     * \brief setRoutine sets the icon as a lighting routine. This takes a routine object and
     * the associated palette for a routine, if one exists.
     *
     * \param routineObject the json object that represents the routine
     */
    void setRoutine(const cor::LightState& state);

    /*!
     * \brief setSolidColor sets the icon as a solid color
     */
    void setSolidColor(const QColor& color);

    /*!
     * \brief setMultiGlimmer sets as mostly mainColor, but adds random colors
     * as a glimmer effect
     */
    void setMultiGlimmer(const std::vector<QColor>& colors);

    /*!
     * \brief setMultiFade regions slowly fade from one array color to another
     * \param colorGroup the colorGroup used for the IconData
     * \param set to false in nearly all cases, this only gets set to true for the menu bar
     * so the custom array shows a few more colors than 2 when defaulted to 2.
     */
    void setMultiFade(const std::vector<QColor>& colors, bool showMore = false);

    /*!
     * \brief setMultiRandomSolid sets icon as 4 colors from array
     */
    void setMultiRandomSolid(const std::vector<QColor>& colors);

    /*!
     * \brief setMultiRandomIndividual sets each region a random color from the array
     */
    void setMultiRandomIndividual(const std::vector<QColor>& colors);

    /*!
     * \brief setBars draws the bars with region sizes of 2 but slightly offset
     *        'cause its hard to show motion in a static icon.
     */
    void setBars(const std::vector<QColor>& colors);

    /*!
     * \brief addWave similar to a linear fade, but offset a bit since otherwise
     *        they would have the same static icon..
     */
    void addWave();

    /*!
     * \brief addGlimmer adds glimmer lighting effect to the preexisting data
     */
    void addGlimmer();

    /*!
     * \brief addBlink turns half the columns of preexisting data black to give blink effect
     *        in static image
     */
    void addBlink();

    /*!
     * \brief addLinearFade fades from no color to full color back to full color with constant
     *        intervals between each pixel.
     */
    void addLinearFade();

    /*!
     * \brief addSawtoothIn fades from no color to full color and then repeats the fade again.
     */
    void addSawtoothIn();

    /*!
     * \brief addSawtoothOut fades from full color to no color and then repeats the fade again.
     */
    void addSawtoothOut();

    /*!
     * \brief addSineFade similar to a linear fade in that it fades from no color to full color
     *        then back to no color, but this one uses a sine wave to to determine the intervals
     *        between the colors instead of a constant value.
     */
    void addSineFade();

    /*!
     * \brief getters for private values
     */
    std::uint32_t dataLength();

    /*!
     * \brief width getter for the data buffer's width.
     *
     * \return the width of the IconData.
     */
    std::uint32_t width();
    /*!
     * \brief height getter for the data buffer's height.
     *
     * \return the height of the IconData.
     */
    std::uint32_t height();

    /*!
     * \brief renderAsQPixmap takes the dat and outputs it as a QPixmap
     *
     * \return a QPixmap representation of the data
     */
    const QPixmap renderAsQPixmap();

private:
    /*!
     * \brief mWidth the width of the resulting icon
     */
    std::uint32_t mWidth;
    /*!
     * \brief mHeight the height of the resulting icon.
     */
    std::uint32_t mHeight;
    /*!
     * \brief mDataLength the number of bytes needed to
     *        to fill mWidth * mHeight * 3.
     */
    std::uint32_t mDataLength;

    /*!
     * a buffer that stores the RGB values of the icon
     */
    std::vector<std::uint8_t> mBuffer;

    /*!
     * \brief getMiddleColor helper that takes two colors and computes
     *        the color in the middle of those colors by averaging their
     *        values.
     *
     * \param first The first color being used.
     * \param second The second color being used.
     * \return A new QColor based on adding the two colors together and dividing
     *         by two.
     */
    QColor getMiddleColor(const QColor& first, const QColor& second);

    /*!
     * \brief mRandomIndividual used by random
     */
    std::vector<int> mRandomIndividual;
};

#endif // ICONDATA_H
