#ifndef SETTINGSBUTTON_H
#define SETTINGSBUTTON_H

#include <QLabel>
#include <QLayout>
#include <QWidget>

/*!
 * \copyright
 * Copyright (C) 2015 - 2019.
 * Released under the GNU General Public License.
 */

/*!
 * \brief The SettingsButton class is the widget used to display buttons on the settings page. This
 * button has one main title and an optional description.
 */
class SettingsButton : public QWidget {
    Q_OBJECT
public:
    /// constructor
    explicit SettingsButton(const QString& title, const QString& description, QWidget* parent);

    /*!
     * \brief shouldHightlght highlights or unhiglights the button.
     *
     * \param shouldHighlight true to highlight the button, false to unhighlight
     */
    void shouldHightlght(bool shouldHighlight);

    /// returns the title of the button
    QString text() { return mTitle->text(); }

signals:

    /// emits title when pressed
    void buttonPressed(QString title);

private slots:
    /// handles when the mouse is pressed down on a button
    void mousePressEvent(QMouseEvent* event);

    /// handles when the mouse is released on a button. This acts as clicking a button.
    void mouseReleaseEvent(QMouseEvent* event);

protected:
    /// handles when the widget is painted
    void paintEvent(QPaintEvent*);

private:
    /// title for button
    QLabel* mTitle;

    /// dscription for button
    QLabel* mDescription;

    /// layout for button
    QVBoxLayout* mLayout;

    /// true if highlight, false if not
    bool mIsHighlighted;
};

#endif // SETTINGSBUTTON_H
