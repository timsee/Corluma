#ifndef LEFTHANDMENUBUTTON_H
#define LEFTHANDMENUBUTTON_H

#include <QHBoxLayout>
#include <QJsonObject>
#include <QLabel>
#include <QWidget>

#include "cor/objects/lightstate.h"
#include "cor/protocols.h"

class LeftHandMenu;

/*!
 * \copyright
 * Copyright (C) 2015 - 2020.
 * Released under the GNU General Public License.
 *
 *
 * \brief The LeftHandButton class is used for buttons on the LeftHandMenu. These buttons are used
 * to change the main page displayed, so there is a button for pages like Settings, or the
 * Color Page.
 */
class LeftHandButton : public QWidget {
    Q_OBJECT
public:
    /// constructor with icon
    LeftHandButton(const QString& text,
                   EPage page,
                   const QString& iconResource,
                   LeftHandMenu* parent);

    /// constructor with Light State
    LeftHandButton(const QString& text,
                   EPage page,
                   const cor::LightState& state,
                   LeftHandMenu* parent);

    /// returns the title of the button
    QString text() { return mTitle->text(); }

    /// true if currently highlighted
    bool highlighted() { return mIsHighlighted; }

    /*!
     * \brief shouldHightlght highlights or unhiglights the button.
     *
     * \param shouldHighlight true to highlight the button, false to unhighlight
     */
    void shouldHightlght(bool shouldHighlight);

    /// update the icon of the button
    void updateIcon(const QString& iconResource);

    /// update the state of the button
    void updateState(const cor::LightState& state);

signals:

    /// emits page when pressed
    void pressed(EPage);

private slots:
    /// handles when the mouse is pressed down on a button
    void mousePressEvent(QMouseEvent* event);

    /// handles when the mouse is released on a button. This acts as clicking a button.
    void mouseReleaseEvent(QMouseEvent* event);

protected:
    /// handles when the widget is painted
    void paintEvent(QPaintEvent*);

    /// handles when the widget is resized
    void resizeEvent(QResizeEvent*);

    /// title for button
    QLabel* mTitle;

private:
    /// renders the icon of the button
    void renderButton();

    /// page index for the button to emit
    EPage mPage;

    /// pointer to menu to determine if its moving
    LeftHandMenu* mMenu;

    /// the icon used for displaying on the left of the menu button
    QLabel* mIcon;

    /// path to the resource for the icon
    QString mResourcePath;

    /// copy of the json data for the icon.
    cor::LightState mState;

    /// true if highlight, false if not
    bool mIsHighlighted;

    /// true if state, false if icon
    bool mIsState;

    /// resize programmatically
    void resize();
};

#endif // LEFTHANDMENUBUTTON_H
