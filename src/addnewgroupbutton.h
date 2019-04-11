#ifndef ADDNEWGROUPBUTTON_H
#define ADDNEWGROUPBUTTON_H

#include <QWidget>
#include <QLabel>
/*!
 * \copyright
 * Copyright (C) 2015 - 2019.
 * Released under the GNU General Public License.
 *
 * \brief The AddNewGroupButton class is a simple button for prompting the user to add a new
 *        group.
 */
class AddNewGroupButton : public QWidget
{
    Q_OBJECT
public:
    /// constructor
    explicit AddNewGroupButton(QWidget *parent);

    /// resize widget programmatically
    void resize(int yPos, const QSize& size);

signals:

    /// emits when pressed
    void pressed();

protected:
    /// handles when the mouse is pressed down on a button
    void mousePressEvent(QMouseEvent* event);

    /// handles when the mouse is released on a button. This acts as clicking a button.
    void mouseReleaseEvent(QMouseEvent* event);

    /// handles when the widget is painted
    void paintEvent(QPaintEvent *);

    /// called when widget is resized
    void resizeEvent(QResizeEvent *);

private:

    /// true if in, false otherwsie
    bool mIsIn;

    /// true if highlighted, false otherwise
    bool mIsHighlighted;

    /// label for the button text
    QLabel *mLabel;
};

#endif // ADDNEWGROUPBUTTON_H
