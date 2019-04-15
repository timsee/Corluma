#ifndef COR_PAGE_H
#define COR_PAGE_H

#include <QTimer>

#include <memory>

namespace cor
{

/*!
 * \copyright
 * Copyright (C) 2015 - 2019.
 * Released under the GNU General Public License.
 *
 *
 * \brief Inherited by many of the largest Pages, provides a link to the backend.
 */
class Page {

public:

    /*!
     * \brief ~Page Destructor
     */
    virtual ~Page() = default;

    /// setter for open flag
    void isOpen(bool open);

    /// true if open, false otherwise
    bool isOpen();

protected:

    /*!
     * \brief mRenderThread timer that calls a renderUI() function on each of the pages.
     *        This function is used to render more expensive assets if and only if they
     *        received a change in state since the previous renderUI call.
     */
    QTimer *mRenderThread;

    /// true if page is open, false if hidden.
    bool mIsOpen;

    /*!
     * \brief mRenderInterval number of msec between each of the UI events.
     */
    int mRenderInterval = 100;
};

}
#endif // COR_PAGE_H
