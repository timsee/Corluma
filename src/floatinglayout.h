#ifndef FLOATINGLAYOUT_H
#define FLOATINGLAYOUT_H

#include <QWidget>
#include <QPushButton>
#include <QHBoxLayout>

/*!
 * \copyright
 * Copyright (C) 2015 - 2016.
 * Released under the GNU General Public License.
 */

/*!
 * \brief The FloatingLayout class QWidget that floats in the top right corner of the screen instead
 *        of using the standard layout methods. This layout provides only two buttons. Its position
 *        must be managed by resizeEvent calls as standard layouts do not apply to it.
 */
class FloatingLayout : public QWidget
{
    Q_OBJECT
public:
    /*!
     * Constructor
     * \param size the desired size of the widget
     * \param parent constructor
     */
    explicit FloatingLayout(QSize size, QWidget *parent = 0);

    /*!
     * \brief move move layout to a new location based off of the top right point provided.
     * \param topRightPoint the new top right point of the layout on its parent.
     */
    void move(QPoint topRightPoint);

signals:
    /*!
     * \brief buttonPressed emitted whenever a button is pressed with a QString representing the
     *        the button that was pressed.
     */
    void buttonPressed(QString);

private slots:
    /*!
     * \brief singleButtonClicked the button that opens the single color page.
     */
    void singleButtonClicked(bool);

    /*!
     * \brief multiButtonClicked the button that opens the multi color page.
     */
    void multiButtonClicked(bool);

private:

    /*!
     * \brief mFloatingLayout layout for the buttons
     */
    QHBoxLayout *mFloatingLayout;
    /*!
     * \brief mSingleButton button for changing to the single color page.
     */
    QPushButton *mSingleButton;
    /*!
     * \brief mMultiButton button for changing to the multi color page.
     */
    QPushButton *mMultiButton;

    /*!
     * \brief mFloatingLayoutSingleSelected true if the last button pressed was the single color button,
     *        false if it was the multi color button.
     */
    bool mFloatingLayoutSingleSelected;
};

#endif // FLOATINGLAYOUT_H
