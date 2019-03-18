#ifndef LEFTHANDMENUBUTTON_H
#define LEFTHANDMENUBUTTON_H

#include <QWidget>
#include <QLabel>
#include <QHBoxLayout>
#include <QJsonObject>

#include "topmenu.h"

/*!
 * \copyright
 * Copyright (C) 2015 - 2019.
 * Released under the GNU General Public License.
 *
 *
 * \brief The LeftHandButton class is used for buttons on the LeftHandMenu. These buttons are used
 *        to change the main page displayed, so there is a button for pages like Settings, or the Color Page.
 */
class LeftHandButton : public QWidget
{
    Q_OBJECT
public:
    /// constructor with icon
   LeftHandButton(const QString& text, EPage page, const QString& iconResource, QWidget *parent);

   /// constructor with json data
   LeftHandButton(const QString& text, EPage page, const QJsonObject& jsonObject, QWidget *parent);

    /// returns the title of the button
    QString text() { return mTitle->text(); }

    /*!
     * \brief shouldHightlght highlights or unhiglights the button.
     * \param shouldHighlight true to highlight the button, false to unhighlight
     */
    void shouldHightlght(bool shouldHighlight);

    /// update the icon of the button
    void updateIcon(const QString& iconResource);

    /// update the json of the button
    void updateJSON(const QJsonObject& jsonObject);

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
    void paintEvent(QPaintEvent *);

private:

    /// page index for the button to emit
    EPage mPage;

    /// layout for button
    QHBoxLayout *mLayout;

    /// the icon used for displaying on the left of the menu button
    QLabel *mIcon;

    /// path to the resource for the icon
    QString mResourcePath;

    /// copy of the json data for the icon.
    QJsonObject mJsonObject;

    /// title for button
    QLabel *mTitle;

    /// true if highlight, false if not
    bool mIsHighlighted;
};

#endif // LEFTHANDMENUBUTTON_H
