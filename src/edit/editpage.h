#ifndef EDIT_EDITPAGE_H
#define EDIT_EDITPAGE_H

#include <QPushButton>
#include <QWidget>
#include "comm/commlayer.h"
#include "cor/objects/page.h"
#include "data/groupdata.h"
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

signals:

    /*!
     * \brief pressedClose emited when the close button is pressed.
     */
    void pressedClose();

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

    /// handles changing the page when signaled from the progress widget
    void changePageFromProgressWidget(std::uint32_t index);

private:
    /// handles sizing the close button
    void resizeCloseButton();

    /// shows a page and resizes it.
    void showAndResizePage(std::uint32_t i);

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

    /// vector of widgets to show
    std::vector<QWidget*> mWidgets;

    /// a progress widget that shows what page you're on and what pages are completed.
    EditProgressWidget* mProgressWidget;
};

} // namespace cor

#endif // EDIT_EDITPAGE_H
