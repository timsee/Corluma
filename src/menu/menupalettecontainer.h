#ifndef MENUPALETTECONTAINER_H
#define MENUPALETTECONTAINER_H

#include <QWidget>
#include "cor/listlayout.h"
#include "cor/objects/palette.h"

/*!
 * \copyright
 * Copyright (C) 2015 - 2021.
 * Released under the GNU General Public License.
 */


/*!
 * \brief The MenuPaletteContainer class is an object that defines its own height and displays an
 * arbitrary count of palettes.
 */
class MenuPaletteContainer : public QWidget {
    Q_OBJECT
public:
    explicit MenuPaletteContainer(QWidget* parent);

    /// updates the widget to show new palettes, deleting all current palettes
    void showPalettes(const std::vector<cor::Palette>& palettes);

    /// getter for all displayed paletets
    const std::vector<cor::Palette>& palettes() { return mPalettes; }

    /// height of the widget's rows
    void widgetHeight(int height) { mHeight = height; }

    /// remove all widgets from the container, reseting it to an empty state
    void clear();

    /// resize programmatically
    void resize();

signals:
    /// signasl when a palette is selected.
    void paletteSelected(cor::Palette);

private slots:

    /// called when a palette is clicked
    void selectPalette(cor::Palette);

    void resizeEvent(QResizeEvent*) { resize(); }

private:
    /// layout for widget
    cor::ListLayout mListLayout;

    /// all palettes currently displayed.
    std::vector<cor::Palette> mPalettes;

    /// height for any palette widget.
    int mHeight;
};

#endif // MENUPALETTECONTAINER_H
