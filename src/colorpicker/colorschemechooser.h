#ifndef COLORSCHEMECHOOSER_H
#define COLORSCHEMECHOOSER_H

#include <QHBoxLayout>
#include <QWidget>

#include "colorpicker/colorschemebutton.h"
/*!
 * \copyright
 * Copyright (C) 2015 - 2020.
 * Released under the GNU General Public License.
 *
 *
 * \brief The ColorSchemeChooser class is a widget that holds the color scheme buttons
 *        and is used for selecting the current color scheme.
 */
class ColorSchemeChooser : public QWidget {
    Q_OBJECT
public:
    /// constructor
    explicit ColorSchemeChooser(QWidget* parent);

    /// enables or disables a button of the given type
    void enableButton(EColorSchemeType, bool);

    /// adjust selection of schemes, unselecting and selecting as required.
    void adjustSelection();

    /// current scheme selected by the chooser
    EColorSchemeType currentScheme() const noexcept { return mCurrentKey; }

signals:

    /// emits when a scheme is changed
    void schemeChanged(EColorSchemeType);

private slots:

    /// emits when a scheme is clicked
    void schemeClicked(EColorSchemeType, bool);

protected:
    /// called when widget is resized
    void resizeEvent(QResizeEvent*);

private:
    /// stores the key of the currently selected group
    EColorSchemeType mCurrentKey;

    /// stores the group button widgets
    std::vector<ColorSchemeButton*> mButtons;
};

#endif // COLORSCHEMECHOOSER_H
