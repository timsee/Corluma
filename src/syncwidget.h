#ifndef SYNCWIDGET_H
#define SYNCWIDGET_H

/*!
 * \copyright
 * Copyright (C) 2015 - 2020.
 * Released under the GNU General Public License.
 */

#include <QLabel>
#include <QWidget>

/// state of the sync widget
enum class ESyncState { synced, syncing, notSynced, hidden };

/*!
 * \brief The SyncWidget class is a basic widget that shows whether or not two things are in sync.
 */
class SyncWidget : public QWidget {
    Q_OBJECT
public:
    /// constructor
    explicit SyncWidget(QWidget* parent);

    /// programmatically change the state
    void changeState(ESyncState state);

    /// request the current state
    ESyncState state() { return mState; }

protected:
    /// called when widget is resized
    void resizeEvent(QResizeEvent*);

private:
    /// current state
    ESyncState mState;

    /// label for displaying static images
    QLabel* mLabel;

    /// label for displaying the syncing movie
    QLabel* mMovieLabel;

    /// movie for diplaying moving images
    QMovie* mMovie;
};

#endif // SYNCWIDGET_H
