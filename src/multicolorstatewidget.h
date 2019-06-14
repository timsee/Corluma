#ifndef MULTICOLORSTATEWIDGET_H
#define MULTICOLORSTATEWIDGET_H

#include <QWidget>
#include "colorpicker/swatchvectorwidget.h"
#include "cor/protocols.h"
#include "cor/widgets/button.h"
#include "syncwidget.h"
/*!
 * \copyright
 * Copyright (C) 2015 - 2019.
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

protected:
    /// called when widget resizes
    void resizeEvent(QResizeEvent*);

private:
    /// widget for displaying sync state
    SyncWidget* mSyncWidget;

    /// swatch vector widget for showing the colors picked in the color scheme
    SwatchVectorWidget* mSwatchWidget;
};

#endif // MULTICOLORSTATEWIDGET_H
