/*!
 * \copyright
 * Copyright (C) 2015 - 2019.
 * Released under the GNU General Public License.
 */

#include <QDebug>

#include "page.h"

namespace cor
{

Page::~Page() {}

void Page::isOpen(bool open) {
    mIsOpen = open;
}

bool Page::isOpen() {
    return mIsOpen;
}



}
