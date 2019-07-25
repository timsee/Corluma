#ifndef SINGLECOLORSTATEWIDGET_H
#define SINGLECOLORSTATEWIDGET_H

#include <QWidget>

#include "cor/objects/light.h"
#include "cor/widgets/button.h"
#include "syncwidget.h"
/*!
 * \copyright
 * Copyright (C) 2015 - 2019.
 * Released under the GNU General Public License.
 */


/*!
 * \brief The SingleColorStateWidget class shows the state of the SingleColorPicker and whether or
 * not it is currently in sync
 */
class SingleColorStateWidget : public QWidget {
    Q_OBJECT
public:
    /// constructor
    explicit SingleColorStateWidget(QWidget* parent);

    /// update the desired state of the single color picker
    void updateState(const QColor& color, ERoutine routine);

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
    /// cached version of the light
    cor::Light mLight;

    /// button for displaying the state
    cor::Button* mState;

    /// widget for displaying sync state
    SyncWidget* mSyncWidget;
};

#endif // SINGLECOLORSTATEWIDGET_H
