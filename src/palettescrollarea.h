#ifndef PALETTESCROLLAREA_H
#define PALETTESCROLLAREA_H

#include <QObject>
#include <QScrollArea>

#include "cor/listlayout.h"
#include "menu/menupalettecontainer.h"
#include "storedpalettewidget.h"

/*!
 * \copyright
 * Copyright (C) 2015 - 2020.
 * Released under the GNU General Public License.
 *
 *
 * \brief The PaletteScrollArea class is a QScrollArea that contains a series of buttons that
 * can be used to signal for using either palettes, or palette+routine combintations.
 */
class PaletteScrollArea : public QWidget {
    Q_OBJECT
public:
    /// constructor
    explicit PaletteScrollArea(QWidget* parent, const std::vector<cor::Palette>& palettes);

    /*!
     * \brief highlightRoutineButton highlights the button that implements
     * the palette. If it can't find a button that
     * implements this lighting routine, then all buttons are unhighlighted
     *
     * \param palette the color group that the highlighted button implements.
     */
    void highlightButton(cor::Palette palette);

    /// sets the height of a widget on the menu.
    void widgetHeight(int height) { mPaletteContainer->widgetHeight(height); }

    /// programmatically resize
    void resize();

    /// clears all widgets in the scroll area
    void clear();

    /// adds palettes to scroll area.
    void addPalettes(std::vector<cor::Palette> palettes);

signals:

    /// emits the palette clicked when a button is clicked
    void paletteClicked(cor::Palette);

private slots:

    /// handles a button click and converts it to a signal.
    void buttonClicked(cor::Palette);

protected:
    /// called whenever it is resized
    void resizeEvent(QResizeEvent*) { resize(); }

private:
    /// scroll area for the widget
    QScrollArea* mScrollArea;

    /// container for all the known palettes.
    MenuPaletteContainer* mPaletteContainer;
};

#endif // PALETTESCROLLAREA_H
