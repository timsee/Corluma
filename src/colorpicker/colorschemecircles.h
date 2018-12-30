#ifndef COLORSCHEMECIRCLES_H
#define COLORSCHEMECIRCLES_H

#include <QWidget>

/// info on a specific color picker selection
struct SPickerSelection {
    bool shouldShow;
    QPointF center;
    QColor color;
    float angle;
    float distance;
};

/*!
 * \copyright
 * Copyright (C) 2015 - 2019.
 * Released under the GNU General Public License.
 */

/*!
 * \brief The ColorSchemeCircles class provides a series of selecction circles to overlay on the color picker.
 *        Circles can both be moved by UI events and programmatically.
 */
class ColorSchemeCircles : public QWidget
{
    Q_OBJECT
public:
    /// constructor
    explicit ColorSchemeCircles(QImage renderedColorWheel, QWidget *parent);

    /*!
     * \brief positionIsUnderCircle gives index of picker selection that is over the given point, if one exists.
     *        if none exists, a -1 is returned.
     * \param newPos the position to check whether theres any selection indices over it.
     * \return the selection picker index if one exists, or -1 if there is none.
     */
    int positionIsUnderCircle(QPointF newPos);

    /*!
     * \brief updateColorScheme update the color scheme and the indices of the picker selections based off of the provided
     *        color scheme
     * \param colorScheme new values for the color scheme.
     */
    void updateColorScheme(const std::vector<QColor> colorScheme);

    /// update the count of colors to display
    void updateColorCount(uint32_t colorCount) { mColorCount = colorCount; }

    /*!
     * \brief moveCenterCircle move the center circle to a new position, update other circles if needed
     * \param newPos new position to move the center circle to
     * \param isOnOpenSpace true if this position was open space, false if its a minor update to the previous
     *        position of the center circle
     */
    void moveCenterCircle(QPointF newPos, bool isOnOpenSpace);

    /*!
     * \brief moveStandardCircle move a standard circle to a new position
     * \param i index of circle to move
     * \param newPos new position to move the circle to.
     */
    void moveStandardCircle(uint32_t i, QPointF newPos);

    /// getter for the varioujs circles used by this class
    std::vector<SPickerSelection> circles();

protected:
    /// called when rendering
    void paintEvent(QPaintEvent *event);

private:
    /// count of colors
    uint32_t mColorCount;

    /*!
     * \brief computeDistance compute length of line between point and the circle's center point
     * \param pos the point to calculate the distance for
     * \return the length of a line drawn from the given point to the center point
     */
    float computeDistance(QPointF pos);

    /*!
     * \brief computeAngle compute angle created by using main selection and the given point and the center as the points
     *        used to generate the angle
     * \param pos the input that you want to compute a angle for
     * \return an angle in degrees.
     */
    float computeAngle(QPointF pos);

    /*!
     * \brief findColorInWheel find a point that creates a color close to or identical to the colro given
     * \return a point on the widget that has a given color similar to the color provided.
     */
    QPointF findColorInWheel(QColor);

    /*!
     * \brief moveCircles move circles, based off the rules provided by the specific given circle index
     * \param i the circle index that is calling all other circles moving.
     */
    void moveCircles(int i);

    /// radius of circles
    int mRadius;

    /// change in angle during movement
    float mDeltaAngle;

    /// size of shadow for the circles
    int mShadowSize;

    /// "main" circle that is used to move all the other circles
    SPickerSelection mMainSelection;

    /// rendered version of color wheel for choosing colors
    QImage mRenderedColorWheel;

    /// the rest of the circles that show if multiple lights are selected.
    std::vector<SPickerSelection> mCircles;
};

#endif // COLORSCHEMECIRCLES_H
