#ifndef SINGLECOLORSTATEWIDGET_H
#define SINGLECOLORSTATEWIDGET_H

#include <QWidget>

#include "cor/objects/lightstate.h"
#include "cor/widgets/button.h"
#include "syncwidget.h"
/*!
 * \copyright
 * Copyright (C) 2015 - 2020.
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

    /// true if in, false if widget is out
    bool isIn() const noexcept { return mIsIn; }

    /// programmatically move the single color state widget in
    void pushIn(const QPoint&);

    /// programmatically move the single color state widget out
    void pushOut(const QPoint&);

protected:
    /// called when widget resizes
    void resizeEvent(QResizeEvent*);

private:
    /// stores if widget is in or not
    bool mIsIn;

    /// cached version of the light
    cor::LightState mState;

    /// button for displaying the state
    cor::Button* mButton;

    /// widget for displaying sync state
    SyncWidget* mSyncWidget;
};

#endif // SINGLECOLORSTATEWIDGET_H
