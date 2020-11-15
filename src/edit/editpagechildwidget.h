#ifndef EDITPAGECHILDWIDGET_H
#define EDITPAGECHILDWIDGET_H

#include <QWidget>
#include "edit/editbottombuttons.h"
#include "edit/editprogressstate.h"
/*!
 * \copyright
 * Copyright (C) 2015 - 2020.
 * Released under the GNU General Public License.
 *
 *
 * \brief The EditPageChildWidget class is the class used for children of an EditPage. Each child
 * has buttons on the bototm that are used to flip forward and backwards, and signals their state
 * with a predefined API.
 */
class EditPageChildWidget : public QWidget {
    Q_OBJECT
public:
    explicit EditPageChildWidget(QWidget* parent)
        : QWidget(parent),
          mBottomButtons{new EditBottomButtons(this)} {}

    /// getter for the bottom buttons. Only the forward button is used in this widget.
    EditBottomButtons* bottomButtons() { return mBottomButtons; }

    /// setter for the index of the page. This is emitted when its state changes.
    void index(std::uint32_t i) noexcept { mIndex = i; }

    /// true if the page has had any changes, false if it is in its original state.
    virtual bool hasEdits() = 0;

signals:
    /// signals that the state of one of its widgets has changed.
    void stateChanged(std::uint32_t index, EEditProgressState state);

    /// emits that any data changed.
    void dataChanged();

    /// signals that the UI elements on other pages should update to new group information. Use this
    /// when a new group is made, or a group is deleted, or an existing group is altered.
    void updateGroups();

    /// signals that it wants to close its parent's page. This reverts the app back to whatever it
    /// was doing before it opened the edit page.
    void forceClosePage();

protected:
    /// buttons for going forward and backward on the bottom of the page.
    EditBottomButtons* mBottomButtons;

    /// the index of a page. Used when emitting the state of the widget to its parent to identify
    /// itself.
    std::uint32_t mIndex;
};

#endif // EDITPAGECHILDWIDGET_H
