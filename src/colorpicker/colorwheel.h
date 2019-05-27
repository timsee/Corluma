#ifndef COLORWHEEL_H
#define COLORWHEEL_H

#include <QLabel>

/*!
 * \copyright
 * Copyright (C) 2015 - 2019.
 * Released under the GNU General Public License.
 */

/// type of color wheel
enum class EWheelType { RGB, HS, CT };

namespace cor {
using CirclePoint = QPointF;
}

/*!
 * \brief The ColorWheel class renders the color wheel for the ColorPicker.
 */
class ColorWheel : public QLabel {
    Q_OBJECT
public:
    /// constructor
    explicit ColorWheel(QWidget* parent);

    /// update brightness for CT and HS wheel
    void updateBrightness(std::uint32_t brightness);

    /// chang the type for the color wheel
    void changeType(EWheelType);

    /// getter for wheel's brightness
    std::uint32_t brightness() const noexcept { return mBrightness; }

    /// resize programmatically
    void resize();

    /// enable or disable
    void enable(bool shouldEnable);

    /*!
     * \brief findPixelByColor find the pixel for a color in the color wheel. Useful for
     * programmatically setting the locations of assets overlaid on the color wheel
     *
     * \param color color to look for in color wheel
     * \return the closest approximation to the color's location in the colorwheel. If there is no
     * close approximation, it returns the center point.
     */
    cor::CirclePoint findPixelByColor(const QColor& color);

    /*!
     * \brief findColorByPixel converts a normalized cor::CirclePoint to the color it represents
     * \param point a normalized point.
     * \return a color from the wheel on that normalize point.
     */
    QColor findColorByPixel(const cor::CirclePoint& point);

    /*!
     * \brief checkIfPointIsOverWheel checcks if a point in non-normalized space is directly over
     * the color wheel. This check looks in a circular region and does not include corners from the
     * bounding box.
     *
     * \param point point ot test
     * \return true if point is over wheel, false otherwise.
     */
    bool checkIfPointIsOverWheel(const QPointF& point);

    /// center of wheel in non-normalized space
    QPoint center();

    /// bounding rect for wheel. Useful if widget is not square, since it accounts for the offset.
    QRect wheelRect();

    /*!
     * \brief handleMouseEvent handles all the mouse events used in the color wheel.
     *        The mousePressEvent and mouseReleaseEvent both map to this function.
     * \param event the mouse event that is getting processed.
     */
    void handleMouseEvent(QMouseEvent* event);

signals:

    /// emits when a color changes
    void changeColor(QColor);

    /// emits when a color temperature and brightness changes
    void changeCT(std::uint32_t brightness, std::uint32_t temperature);

protected:
    /*!
     * \brief mousePressEvent called only onnce when a mouse press is clicked down. Events not
     *        directly on top of the color wheel are ignored.
     */
    void mousePressEvent(QMouseEvent*);
    /*!
     * \brief mouseMoveEvent called when a mouse press is clicked down and moves. This event gets
     *        called very frequently, so the events are thorttled by a timer. Also, events not
     *        directly on top of the color wheel are ignored.
     */
    void mouseMoveEvent(QMouseEvent*);

    /// used to render the color wheel
    void paintEvent(QPaintEvent*);

private:
    /*!
     * \brief checkIfColorIsValid returns true if the color passes checks that make sure
     *        its not a background color or one of the illegal colors for the API to send out.
     * \param color color to check.
     * \return true if color is valid, false otherwise.
     */
    bool checkIfColorIsValid(const QColor& color);

    /*!
     * \brief eventIsOverWheel true if the event is over the color wheel itself, false otherwise
     * \param event mouse event being tested
     * \return true if the event is over the color wheel, false otherwise.
     */
    bool eventIsOverWheel(QMouseEvent* event);

    /// stores the color wheel when it is rendered
    QImage* mImage;

    /// tracks what color wheel to use
    EWheelType mWheelType;

    /// tracks how bright the color wheel should be
    std::uint32_t mBrightness;

    /// true if wheel is usable, false if disabled
    bool mIsEnabled;

    /// true if should repaint, false otherwise
    bool mRepaint;
};

namespace cor {

inline QPoint circlePointToDenormalizedPoint(const cor::CirclePoint& point,
                                             const QRect& fullRect,
                                             const QRect& wheelRect) {
    auto offsetX = (fullRect.width() - wheelRect.width()) / 2;
    auto offsetY = (fullRect.height() - wheelRect.height()) / 2;
    return QPoint(int(point.x() * wheelRect.width() + offsetX),
                  int(point.y() * wheelRect.height() + offsetY));
}

inline cor::CirclePoint denormalizedPointToCirclePoint(const QPointF& point,
                                                       const QRect& fullRect,
                                                       const QRect& wheelRect) {
    auto offsetX = (fullRect.width() - wheelRect.width()) / 2;
    auto offsetY = (fullRect.height() - wheelRect.height()) / 2;
    return QPointF((point.x() - offsetX) / wheelRect.width(),
                   (point.y() - offsetY) / wheelRect.height());
}

} // namespace cor

#endif // COLORWHEEL_H
