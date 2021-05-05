#ifndef MULTICOLORSTATEWIDGET_H
#define MULTICOLORSTATEWIDGET_H

#include <QWidget>

#include "colorpicker/swatchvectorwidget.h"
#include "cor/protocols.h"
#include "cor/widgets/button.h"
#include "cor/widgets/palettewidget.h"
#include "syncwidget.h"

/*!
 * \copyright
 * Copyright (C) 2015 - 2020.
 * Released under the GNU General Public License.
 */

/*!
 * \brief The MultiColorStateWidget class shows the state of the MultiColorPicker and whether or
 * not it is currently in sync
 */
class MultiColorStateWidget : public QWidget {
    Q_OBJECT
public:
    /// constructor
    explicit MultiColorStateWidget(QWidget* parent);

    /// update the desired state of the multi color picker
    void updateState(std::vector<QColor> colors);

    /// update the sync status
    void updateSyncStatus(ESyncState state);

    /// getter for the sync status
    ESyncState syncState() { return mSyncWidget->state(); }

    /// programmatically resize
    void resize();

    /// true if widget is in, false if its hidden
    bool isIn() const noexcept { return mIsIn; }

    /// programmatically push the multi color state widget in
    void pushIn(const QPoint&);

    /// programmatically push the multi color state widget out
    void pushOut(const QPoint&);

protected:
    /// called when widget resizes
    void resizeEvent(QResizeEvent*);

private:
    /// stores whether or not the widget is in or not
    bool mIsIn;

    /// widget for displaying sync state
    SyncWidget* mSyncWidget;

    /// palette widget for showing the colors picked in the color scheme
    cor::PaletteWidget* mPaletteWidget;
};

#endif // MULTICOLORSTATEWIDGET_H
