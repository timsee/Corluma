#ifndef EDITPAGETOPMENU_H
#define EDITPAGETOPMENU_H

#include <QGridLayout>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QWidget>

/*!
 * \copyright
 * Copyright (C) 2015 - 2020.
 * Released under the GNU General Public License.
 *
 *
 * \brief The EditPageTopMenu class is the top menu for the edit page. This is kept as as a separate
 * widget as to not make the layout and the function of the edit page toooo confusing.
 */
class EditPageTopMenu : public QWidget {
    Q_OBJECT
public:
    /// conustructor
    explicit EditPageTopMenu(QWidget* parent);

    /// getter for close button
    QPushButton* closeButton() { return mCloseButton; }

    /// getter for save button
    QPushButton* saveButton() { return mSaveButton; }

    /// getter for delete button
    QPushButton* deleteButton() { return mDeleteButton; }

    /// getter for reset button
    QPushButton* resetButton() { return mResetButton; }

    /// getter for name edit.
    QLineEdit* nameEdit() { return mNameEdit; }

    /// getter for help label
    QLabel* helpLabel() { return mHelpLabel; }

private:
    /// vertical layout for widget
    QGridLayout* mLayout;

    /// label for top of box
    QLabel* mHelpLabel;

    /// close button
    QPushButton* mCloseButton;

    /// button to save group
    QPushButton* mSaveButton;

    /// button to delete group
    QPushButton* mDeleteButton;

    /// button to reset to original settings
    QPushButton* mResetButton;

    /// line edit for changing the name of a group
    QLineEdit* mNameEdit;
};

#endif // EDITPAGETOPMENU_H
