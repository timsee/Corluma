#ifndef STYLESHEETS_H
#define STYLESHEETS_H

#include <QString>
/*!
 * \copyright
 * Copyright (C) 2015 - 2021.
 * Released under the GNU General Public License.
 */
namespace cor {

/// transparent widget
const QString kTransparentStylesheet = "background-color: rgba(0,0,0,0);";
/// transparent widget with bold texxt
const QString kTransparentAndBoldStylesheet = "font: bold; background-color: rgba(0,0,0,0);";
/// gradient between 33,32,32 dark grey to transparent
const QString kGradientStylesheet =
    "font: bold; background: QLinearGradient( x1: 0, y1: 0, x2: 1, y2: 0, "
    " stop: 0.0 rgba(33,32,32,1), stop: 0.7 rgba(33,32,32,0.5), stop: 1 rgba(33,32,32,0));";

/// stanardard grey background
const QString kStandardGreyBackground = "background-color:rgb(48, 47, 47);";
/// darker grey background
const QString kDarkerGreyBackground = "background-color:rgb(33, 32, 32);";
/// lighter grey background
const QString kLighterGreyBackground = "background-color:rgb(69, 67, 67);";

/// red-ish background for delete buttons
const QString kDeleteButtonBackground = "background-color:rgb(110,30,30);";
/// green-ish background for edit buttons
const QString kEditButtonBackground = "background-color:rgb(30,110,30);";

/// stylesheets that use primary colors.
const QString kDebugStylesheetRed = "background-color: rgb(255,0,0);";
const QString kDebugStylesheetGreen = "background-color: rgb(0,255,0);";
const QString kDebugStylesheetBlue = "background-color: rgb(0,0,255);";

} // namespace cor

#endif // STYLESHEETS_H
