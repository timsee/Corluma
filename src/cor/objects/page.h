#ifndef COR_PAGE_H
#define COR_PAGE_H

namespace cor {

/*!
 * \copyright
 * Copyright (C) 2015 - 2020.
 * Released under the GNU General Public License.
 *
 *
 * \brief Inherited by many of the largest Pages, provides a link to the backend.
 */
class Page {
public:
    /// Destructor
    virtual ~Page() = default;

    /// setter for open flag
    void isOpen(bool open) noexcept { mIsOpen = open; }

    /// true if open, false otherwise
    bool isOpen() const noexcept { return mIsOpen; }

protected:
    /// true if page is open, false if hidden.
    bool mIsOpen;
};

} // namespace cor
#endif // COR_PAGE_H
