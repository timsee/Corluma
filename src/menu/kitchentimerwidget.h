#ifndef KITCHENTIMERWIDGET_H
#define KITCHENTIMERWIDGET_H

#include <QLabel>
#include <QPushButton>
#include <QWidget>

enum class EKitchenTimerMode { minutes, hours };
/*!
 * \copyright
 * Copyright (C) 2015 - 2020.
 * Released under the GNU General Public License.
 *
 *
 * \brief The KitchenTimerWidget class provides a kitchen timer for choosing how long until
 * something times out. It is designed after a kitchen timer.
 */
class KitchenTimerWidget : public QWidget {
    Q_OBJECT
public:
    explicit KitchenTimerWidget(QWidget* parent);

    /// programmatically set the value of the timer
    void setValue(int value);

    /// programmatically set the value and mode of the timer
    void setValue(int value, EKitchenTimerMode mode);

    /// getter for the value
    int value() { return mValue; }

signals:

    /// emits when the value changes
    void valueChanged(int);

protected:
    /*!
     * \brief mousePressEvent called only onnce when a mouse press is clicked down. Events not
     *        directly on top of the kitchen timer are ignored.
     */
    void mousePressEvent(QMouseEvent*);
    /*!
     * \brief mouseMoveEvent called when a mouse press is clicked down and moves. This event gets
     *        called very frequently, so the events are thorttled by a timer. Also, events not
     *        directly on top of the kitchen timer are ignored.
     */
    void mouseMoveEvent(QMouseEvent*);

    /// used to render the kitchen timer
    void paintEvent(QPaintEvent*);

    /// called when widget is resized
    void resizeEvent(QResizeEvent*);

private slots:

    /// handles when minutes button is clicked
    void minutesClicked(bool);

    /// handles when the hours button is clicked
    void hoursClicked(bool);

private:
    /*!
     * \brief handleMouseEvent handles all the mouse events used in the kitchen timer.
     *        The mousePressEvent and mouseReleaseEvent both map to this function.
     * \param event the mouse event that is getting processed.
     */
    void handleMouseEvent(QMouseEvent* event);

    /*!
     * \brief eventIsOverHalfCircle true if the event is over the half circle itself, false
     * otherwise.
     *
     * \param event mouse event being tested
     * \return true if the event is over the half circle, false otherwise.
     */
    bool eventIsOverHalfCircle(QMouseEvent* event);

    /// converts angle to value
    float angleToValue(float angle);

    /// converts a value to an angle
    float valueToAngle(float value);

    /// updates the widget value
    void updateWidgetValue(int value);

    /// changes the timeout mode
    void handleMode(EKitchenTimerMode newMode);

    /// updates the line based off of the stored value
    void updateLine();

    /// label that displays the value
    QLabel* mValueLabel;

    /// button for switching into mintues mode
    QPushButton* mMinutesButton;

    /// button for switching into hours mode
    QPushButton* mHoursButton;

    /// mode for the kitchen timer
    EKitchenTimerMode mMode;

    /// line drawn on the kitchen timer
    QLineF mLine;

    /// value displayed by the widget
    int mValue;
};

#endif // KITCHENTIMERWIDGET_H
