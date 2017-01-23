#ifndef FLOATINGLAYOUT_H
#define FLOATINGLAYOUT_H

#include <QWidget>
#include <QPushButton>
#include <QHBoxLayout>

/*!
 * \copyright
 * Copyright (C) 2015 - 2017.
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
     * \param parent constructor
     */
    explicit FloatingLayout(QWidget *parent = 0);

    /*!
     * \brief setupButtons takees the vector of names and the size given and updates the buttons
     *        so that theres a button for every name
     * \param buttonNames names of buttons provided. these names will be signaled out whenever the button
     *        is clicked.
     * \param size how large to make the floating layout.
     */
    void setupButtons(std::vector<QString> buttonNames, QSize size);

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
     * \brief buttonClicked handles whenever any button is clicked. Converts the click into a signal of the button's
     *        name.
     */
    void buttonClicked(int);

private:

    /*!
     * \brief mButtons vector of buttons
     */
    std::vector<QPushButton*> mButtons;

    /*!
     * \brief mNames vector of names of those buttons.
     */
    std::vector<QString> mNames;

    /*!
     * \brief mFloatingLayout layout for the buttons
     */
    QHBoxLayout *mFloatingLayout;

};

#endif // FLOATINGLAYOUT_H
