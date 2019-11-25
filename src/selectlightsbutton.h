#ifndef SELECTLIGHTSBUTTON_H
#define SELECTLIGHTSBUTTON_H

#include <QLabel>
#include <QWidget>

/*!
 * \copyright
 * Copyright (C) 2015 - 2019.
 * Released under the GNU General Public License.
 */

/*!
 * \brief The SelectLightsButton class is a simple button that prompts the user to select lights
 * when none are selected. This scrolls into the screen where selected lights normally are in cases
 * where the left menu is hidden but the rest of the app is greyed out, so that the user has
 * a clear place to click to continue using the app
 */
class SelectLightsButton : public QWidget {
    Q_OBJECT
public:
    /// constructor
    explicit SelectLightsButton(QWidget* parent);

    /// pushes in the widget for display
    void pushIn(int yPos);

    /// pushes out the widget from display
    void pushOut(int yPos);

    /// resize widget programmatically
    void resize(int yPos);

    /// true if in, false otherwise
    bool isIn() { return mIsIn; }

    /// getter of widgets text
    QString text() { return mLabel->text(); }

signals:

    /// emits when pressed
    void pressed();

protected:
    /// handles when the mouse is pressed down on a button
    void mousePressEvent(QMouseEvent* event);

    /// handles when the mouse is released on a button. This acts as clicking a button.
    void mouseReleaseEvent(QMouseEvent* event);

    /// handles when the widget is painted
    void paintEvent(QPaintEvent*);

    /// called when widget is resized
    void resizeEvent(QResizeEvent*);

private:
    /// true if in, false otherwsie
    bool mIsIn;

    /// true if highlighted, false otherwise
    bool mIsHighlighted;

    /// label for the button text
    QLabel* mLabel;
};

#endif // SELECTLIGHTSBUTTON_H
