
#ifndef ICONDATA_H
#define ICONDATA_H

#include "datalayer.h"

#include <QPixmap>
#include <QImage>

#include <stdio.h>
#include <memory>


/*!
 * \copyright
 * Copyright (C) 2015 - 2016.
 * Released under the GNU General Public License.
 *
 * \brief The IconData class creates all the pixel-y square icons that are seen throughout
 * the app. The grids are made by using a very small buffer of RGB values to do the
 * computation. For display, this buffer gets upscaled to the desired icon size.
 *
 */
class IconData
{
public:
    /*!
     * \brief Constructor
     */
    IconData();
    /*!
     * \brief Standard Constructor, values must  be a multiple of four.
     *
     * \param width the width
     * \param height the height
     */
    IconData(int width, int height);

    /*!
     * \brief Required constructor when the icons use the color array.
     *
     * \param width the width
     * \param height the height
     * \param data pointer to the data layer to access array data.
     */
    IconData(int width, int height, DataLayer *data);

    /*!
     * \brief setLightingRoutine use a lighting routine and color group to set up the IconData.
     * \param routine the routine that you want to use for the icon
     * \param colorGroup the group that you want to use for the icon.
     */
    void setLightingRoutine(ELightingRoutine routine, EColorGroup colorGroup = EColorGroup::eCustom);

    /*!
     * \brief setSingleLightingRoutine use a lighting routine and a color to set up the IconData
     *        for a single color routine.
     * \param routine lighting routine to use for the icon.
     * \param color color to use for the icon.
     */
    void setSingleLightingRoutine(ELightingRoutine routine, QColor color);

    /*!
     * \brief setSolidColor sets the icon as a solid color
     */
    void setSolidColor(QColor color);

    /*!
     * \brief setMultiColors shows all array colors, repeating if necessary
     */
    void setMultiColors(EColorGroup group);

    /*!
     * \brief setMultiGlimmer sets as mostly mainColor, but adds random colors
     *        as a glimmer effect
     */
    void setMultiGlimmer(EColorGroup group);

    /*!
     * \brief setMultiFade regions slowly fade from one array color to another
     * \param colorGroup the colorGroup used for the IconData
     * \param set to false in nearly all cases, this only gets set to true for the menu bar
     *        so the custom array shows a few more colors than 2 when defaulted to 2.
     */
    void setMultiFade(EColorGroup group, bool showMore = false);

    /*!
     * \brief setMultiRandomSolid sets icon as 4 colors from array
     */
    void setMultiRandomSolid(EColorGroup group);

    /*!
     * \brief setMultiRandomIndividual sets each region a random color from the array
     */
    void setMultiRandomIndividual(EColorGroup group);

    /*!
     * \brief setMultiBarsSolid draws the bars with region sizes of 2
     */
    void setMultiBarsSolid(EColorGroup group);

    /*!
     * \brief setMultiBarsSolid draws the bars with region sizes of 2 but slightly offset
     *        'cause its hard to show motion in a static icon.
     */
    void setMultiBarsMoving(EColorGroup group);

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
    uint dataLength();
    /*!
     * \brief width getter for the data buffer's width.
     * \return the width of the IconData.
     */
    uint width();
    /*!
     * \brief height getter for the data buffer's height.
     * \return the height of the IconData.
     */
    uint height();

    /*!
     * \brief renderAsQImage takes the data and outputs it as a QImage
     * \return a QImage representation of the data
     */
    QImage renderAsQImage();
    /*!
     * \brief renderAsQPixmap takes the dat and outputs it as a QPixmap
     * \return a QPixmap representation of the data
     */
    QPixmap renderAsQPixmap();

private:

    /*!
     * \brief setup used by the constructors to set up the data buffers.
     * \param width the width of the data buffers.
     * \param height the height of the data buffers.
     */
    void setup(int width, int height);

    /*!
     * \brief mDataLayer a pointer to the data layer.
     */
    DataLayer *mDataLayer;

    /*!
     * the full data used when rendering an image.
     * It matches in size the size of the image
     */
    QVector<uint8_t> mData;
    /*!
     * \brief mWidth the width of the resulting icon
     */
    uint mWidth;
    /*!
     * \brief mHeight the height of the resulting icon.
     */
    uint mHeight;
    /*!
     * \brief mDataLength the number of bytes needed to
     *        to fill mWidth * mHeight * 3.
     */
    uint mDataLength;

    /*!
     * a buffer that is used when doing processing.
     * set of RGB values in the buffer represents an entire
     * region of the mData. A function bufferToOutput() must
     * be called in any function to map mBuffer to mData.
     */
    QVector<uint8_t> mBuffer;
    /*!
     * \brief mBufferWidth the buffer's width.
     */
    uint mBufferWidth;
    /*!
     * \brief mBufferHeight the buffer's height.
     */
    uint mBufferHeight;
    /*!
     * \brief mBufferLength the number of bytes needed to fill
     *        mBufferWidth * mBufferHeight * 3.
     */
    uint mBufferLength;

    /*!
     * \brief bufferToOutput required at the end of any set of processing
     *        this maps the mBuffer values to mData, scaling mBuffer up.
     */
    void bufferToOutput();

    /*!
     * \brief getMiddleColor helper that takes two colors and computes
     *        the color in the middle of those colors by averaging their
     *        values.
     * \param first The first color being used.
     * \param second The second color being used.
     * \return A new QColor based on adding the two colors together and dividing
     *         by two.
     */
    QColor getMiddleColor(QColor first, QColor second);

    /*!
     * \brief mRandomIndividual used by random
     */
    int mRandomIndividual[16];
};

#endif // ICONDATA_H
