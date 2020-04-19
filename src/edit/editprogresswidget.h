#ifndef EDITPROGRESSWIDGET_H
#define EDITPROGRESSWIDGET_H

#include <QWidget>

//// the progress state of each edit page
enum class EEditProgressState { locked, incomplete, completed };

/*!
 * \copyright
 * Copyright (C) 2015 - 2020.
 * Released under the GNU General Public License.
 *
 *
 * \brief The EditProgressWidget class is a widget that shows the progress of different editing
 * pages, and allows the user to flip between edit pages.
 */
class EditProgressWidget : public QWidget {
    Q_OBJECT
public:
    /// constructor
    explicit EditProgressWidget(QWidget* parent, std::uint32_t numberOfPages);

    /// programmatically change to a specific page
    void changeToPage(std::uint32_t pageNumber);

    /// update the state of a page to a different state
    void updateState(std::uint32_t index, EEditProgressState state);

    /// getter for the current page
    std::uint32_t currentPage() const noexcept { return mCurrentPage; }

    /// getter for the number of pages
    std::uint32_t numberOfPages() const noexcept { return mNumberOfPages; }

signals:

    /// signals when a page is changed from interacting with the EditProgressWidget
    void changePage(std::uint32_t);

protected:
    /*!
     * \brief paintEvent used to draw the background of the widget.
     */
    void paintEvent(QPaintEvent*);

    /// handles when a mouse is relased
    void mouseReleaseEvent(QMouseEvent*);

private:
    /// stores the number of pages, used for drawing the widget
    std::uint32_t mNumberOfPages;

    /// stores the current page, used for drawing the widget
    std::uint32_t mCurrentPage;

    /// stores the state of each page, used while drawing the widget
    std::vector<EEditProgressState> mPageState;
};

#endif // EDITPROGRESSWIDGET_H
