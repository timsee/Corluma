#ifndef PALETTESCROLLAREA_H
#define PALETTESCROLLAREA_H

#include <QObject>
#include <QScrollArea>

#include "cor/listlayout.h"
#include "cor/objects/palettegroup.h"
#include "menu/menugenericgroupcontainer.h"
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
    enum EState { palettes, parents, palettesForParent };

    /// constructor
    explicit PaletteScrollArea(QWidget* parent, const std::vector<cor::Palette>& palettes);

    /// sets the height of a widget on the menu.
    void widgetHeight(int height) {
        mWidgetHeight = height;
        mPaletteContainer->widgetHeight(height);
    }

    /// programmatically resize
    void resize();

    /// clears all widgets in the scroll area
    void clear();

    /// displays the state of the widget.
    EState state() { return mState; }

    /// adds palettes to scroll area.
    void showPalettes(std::vector<cor::Palette> palettes);

    /// adds palette groups to the scroll area.
    void showPalettesWithParents(const std::vector<cor::PaletteGroup>& groups);

signals:

    /// emits the palette clicked when a button is clicked
    void paletteClicked(cor::Palette);

private slots:

    /// handles a button click and converts it to a signal.
    void buttonClicked(cor::Palette);

    /// handles when the top "parent" is clicked, which minimizes that parent and displays all other
    /// parents.
    void topParentClicked(QString);

    /// handles when a parent is clicked.
    void parentClicked(QString);

protected:
    /// called whenever it is resized
    void resizeEvent(QResizeEvent*) { resize(); }

private:
    /// current state of the widget.
    EState mState;

    /// height of a widget.
    int mWidgetHeight;

    /// scroll area for the widget
    QScrollArea* mScrollArea;

    /// scroll area for showing the MenuGenericGroupContainer
    QScrollArea* mParentScrollArea;

    /// container for all the known palettes.
    MenuPaletteContainer* mPaletteContainer;

    /// stores the parents of the palette groups.
    MenuGenericGroupContainer* mParentContainer;

    /// dropdown top widget used to display the parent group widget
    cor::GroupButton* mParentWidget;

    /// stores the palette groups for lookup.
    std::vector<cor::PaletteGroup> mPaletteGroups;
};

#endif // PALETTESCROLLAREA_H
