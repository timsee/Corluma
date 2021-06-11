#ifndef PALETTEDETAILEDWIDGET_H
#define PALETTEDETAILEDWIDGET_H

#include <QLabel>
#include <QMessageBox>
#include <QPushButton>
#include <QWidget>
#include "cor/objects/page.h"
#include "cor/objects/palette.h"
#include "cor/widgets/palettewidget.h"
#include "syncwidget.h"

/*!
 * \copyright
 * Copyright (C) 2015 - 2021.
 * Released under the GNU General Public License.
 */


/*!
 * \brief The PaletteDetailedWidget class displays the details of a palette and allows the user to
 * sync the palette to the selected lights or edit the palette.
 */
class PaletteDetailedWidget : public QWidget, public cor::Page {
    Q_OBJECT
public:
    explicit PaletteDetailedWidget(QWidget* parent);

    /// update the palette in the widget
    void update(const cor::Palette& palette);

    /// pushes in the widget
    void pushIn();

    /// pushes out the widget
    void pushOut();

    /// resizes widget programmatically
    void resize();

    /// update the sync status
    void updateSyncStatus(ESyncState);

signals:

    /*!
     * \brief pressedClose emited when the close button is pressed.
     */
    void pressedClose();

    /// signals when the palette is going to be selected
    void enablePalette(std::uint64_t);

    /// emits whenever the palette syncs
    void syncPalette(cor::Palette);

    /// emits when a palette should be deleted.
    void deletePalette(cor::Palette);

protected:
    /*!
     * \brief paintEvent used to draw the background of the widget.
     */
    void paintEvent(QPaintEvent*);

    /*!
     * \brief resizeEvent called whenever the widget resizes
     */
    void resizeEvent(QResizeEvent*) { resize(); }

private slots:

    /// handles when the close button is pressed
    void closeButtonPressed(bool) { emit pressedClose(); }

    /// handles when the sync button is pressed
    void syncButtonPressed(bool) { emit syncPalette(mPalette); }

    /// handles when a delete button is pressed.
    void deleteButtonPressed(bool) {
        QMessageBox::StandardButton reply;
        QString text = "Delete " + mPalette.name() + "? This will remove it from the app memory.";

        reply = QMessageBox::question(this, "Delete?", text, QMessageBox::Yes | QMessageBox::No);
        if (reply == QMessageBox::Yes) {
            // signal to remove from app
            emit deletePalette(mPalette);
        }
    }

private:
    /// resizes icons on the top of the widget.
    void resizeIcons();

    /// button placed at left hand side of widget
    QPushButton* mCloseButton;

    /// button to sync lights
    QPushButton* mSyncButton;

    /// button to edit palette
    QPushButton* mEditButton;

    /// button to delete palettes
    QPushButton* mDeleteButton;

    /// name of palette
    QLabel* mName;

    /// sync widget to show syncing state
    SyncWidget* mSyncWidget;

    /// palette widget to show the colors in the palette
    cor::PaletteWidget* mPaletteWidget;

    /// paletet shown by widget
    cor::Palette mPalette;
};

#endif // PALETTEDETAILEDWIDGET_H
