#ifndef EDIT_EDITPAGE_H
#define EDIT_EDITPAGE_H

#include <QPushButton>
#include <QWidget>
#include "comm/commlayer.h"
#include "cor/objects/page.h"
#include "data/groupdata.h"
#include "edit/editpagechildwidget.h"
#include "edit/editprogresswidget.h"

namespace cor {
/*!
 * \copyright
 * Copyright (C) 2015 - 2020.
 * Released under the GNU General Public License.
 *
 *
 * \brief The EditPage class allows the user to add and edit groups and moods. This page is the base
 * page of a variety of edit pages. Each edit page will keep the same progress widget and layout,
 * but will have different widgets that are shown in the middle of the widget.
 */
class EditPage : public QWidget, public cor::Page {
    Q_OBJECT
public:
    /// constructor
    explicit EditPage(QWidget* parent, CommLayer* layer, GroupData* parser);

    /// displays the discovery page
    void pushIn(const QPoint& startPoint, const QPoint& endPoint);

    /// hides the discovery page
    void pushOut(const QPoint& endPoint);

    /// shows a specific page by index
    void showPage(std::uint32_t pageIndex);

    /// reset the page to its default state, removing all edits that have happened.
    void reset();

    /*!
     * \brief widgets All edit pages have a group of widgets that the user must go through to
     * complete an edit. This is a vector of all widgets contained by the EditPage
     * \return vector of all widgets contained by the edit page
     */
    std::vector<EditPageChildWidget*> widgets() { return mWidgets; }

    /*!
     * \brief editProgressWidget All edit pages have a progress widget in the top right taht can be
     * used to see how many steps are left in the edit or to change what step you are on. This is a
     * getter for the EditProgressWidget
     * \return getter for EditProgressWidget
     */
    EditProgressWidget* editProgressWidget() { return mProgressWidget; }

    /// programmatically set the height of rows in scroll widgets.
    virtual void changeRowHeight(int height) = 0;

signals:

    /*!
     * \brief pressedClose emited when the close button is pressed.
     */
    void pressedClose();

    /// signals when a group has been edited in any capacity. This signal is listened to by widgets
    /// that require to update to edited data.
    void updateGroups();

protected:
    /*!
     * \brief setupWidgets initializes the widget vector and hooks up its signals and slots. This
     * must be called from the constructor of a derived class in order to have the class function
     * properly.
     */
    void setupWidgets(std::vector<EditPageChildWidget*>);

    /*!
     * \brief pageChanged handles whenever a page changes in a derived class. The UI elements of a
     * page changing are already handled, but this function exists to do tasks such as piping data
     * from one widget to another, or doing any state changes that require knowledge of multiple
     * widgets.
     * \param i index of the new page to change to.
     */
    virtual void pageChanged(std::uint32_t i) = 0;

    /*!
     * \brief resizeEvent called whenever the widget resizes so that assets can be updated.
     */
    void resizeEvent(QResizeEvent*);

    /*!
     * \brief paintEvent used to draw the background of the widget.
     */
    void paintEvent(QPaintEvent*);

private slots:
    /// moves the edit page one page forwrad
    void pageForwards();

    /// moves the edit page one page backward
    void pageBackwards();

    /// handles when close is pressed from a child widget
    void closeFromPagePressed();

    /// a child page has sent an updateGroups signal, which is intercepted here and turned into an
    /// EditPage signal.
    void updateGroupsFromPagePressed();

    /// handles when a widget's state is changed.
    void widgetChangedState(std::uint32_t, EEditProgressState);

    /*!
     * \brief closePressed called when close button is pressed. Checks if changes were made, asks
     * for user input if needed, and then closes the window.
     */
    void closePressed(bool);

    /// handles changing the page when signaled from the progress widget
    void changePageFromProgressWidget(std::uint32_t index);

private:
    /// handles sizing the close button
    void resizeCloseButton();

    /// shows a page and resizes it.
    void showAndResizePage(std::uint32_t i);

    /// computes the state of the review page. the review page's state is based off of the state of
    /// the other widgets.
    void computeStateOfReviewPage();

    /// top height doesnt change as the app resizes, so this is the cached version of the top height
    int mTopHeight;

    /*!
     * \brief communication pointer to communication object
     *        for sending comannds to the lights
     */
    CommLayer* mComm;

    /// groups parser
    GroupData* mGroups;

    /// placeholder for the main page of the widget
    QWidget* mPlaceholder;

    /// button placed at left hand side of widget
    QPushButton* mCloseButton;

    /// a progress widget that shows what page you're on and what pages are completed.
    EditProgressWidget* mProgressWidget;

    /// stores all the widgets that the user flips through on the EditPage.
    std::vector<EditPageChildWidget*> mWidgets;
};

} // namespace cor

#endif // EDIT_EDITPAGE_H
