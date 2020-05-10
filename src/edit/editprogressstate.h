#ifndef EDITPROGRESSSTATE_H
#define EDITPROGRESSSTATE_H

#include <QMetaType>
/*!
 * \copyright
 * Copyright (C) 2015 - 2020.
 * Released under the GNU General Public License.
 */

//// the progress state of each edit page
enum class EEditProgressState { locked, incomplete, completed };
Q_DECLARE_METATYPE(EEditProgressState)

#endif // EDITPROGRESSSTATE_H
