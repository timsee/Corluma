#ifndef CHOOSEEDITPAGE_H
#define CHOOSEEDITPAGE_H

#include "cor/objects/page.h"

#include <QPushButton>
#include <QWidget>

enum class EChosenEditMode { add, edit, remove };
/*!
 * \copyright
 * Copyright (C) 2015 - 2020.
 * Released under the GNU General Public License.
 */


/*!
 * \brief The ChooseEditPage class chooses which type of edit action the user wants to take. The
 * user is presented with options such as add new data, edit existing data, or delete existing data.
 */
class ChooseEditPage : public QWidget, public cor::Page {
    Q_OBJECT
public:
    explicit ChooseEditPage(QWidget* parent);

    /// displays the page
    void pushIn(const QPoint& startPoint, const QPoint& endPoint);

    /// hides the page
    void pushOut(const QPoint& endPoint);

signals:

    /*!
     * \brief pressedClose emited when the close button is pressed.
     */
    void pressedClose();

    /// signals when a mode is selected
    void modeSelected(EChosenEditMode);

protected:
    /*!
     * \brief resizeEvent called whenever the widget resizes so that assets can be updated.
     */
    void resizeEvent(QResizeEvent*);

    /*!
     * \brief paintEvent used to draw the background of the widget.
     */
    void paintEvent(QPaintEvent*);

private slots:

    /*!
     * \brief closePressed called when close button is pressed. Checks if changes were made, asks
     * for user input if needed, and then closes the window.
     */
    void closePressed(bool);

    /// handles when the add button is pressed
    void addPressed(bool);

    /// handles when the edit button is pressed
    void editPressed(bool);

    /// handles when the delete button is pressed
    void deletePressed(bool);

private:
    /// handles sizing the close button
    void resizeCloseButton();

    /// top height doesnt change as the app resizes, so this is the cached version of the top height
    int mTopHeight;

    /// button placed at left hand side of widget
    QPushButton* mCloseButton;

    /// button for adding data
    QPushButton* mAddButton;

    /// button for editing data
    QPushButton* mEditButton;

    /// button for deleting data
    QPushButton* mDeleteButton;
};

#endif // CHOOSEEDITPAGE_H
