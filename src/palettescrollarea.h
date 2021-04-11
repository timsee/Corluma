#ifndef PALETTESCROLLAREA_H
#define PALETTESCROLLAREA_H

#include <QObject>
#include <QScrollArea>

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
class PaletteScrollArea : public QScrollArea {
    Q_OBJECT
public:
    /// constructor
    explicit PaletteScrollArea(QWidget* parent);

    /*!
     * \brief highlightRoutineButton highlights the button that implements
     * the palette. If it can't find a button that
     * implements this lighting routine, then all buttons are unhighlighted
     *
     * \param palette the color group that the highlighted button implements.
     */
    void highlightButton(EPalette palette);

    /// programmatically resize
    void resize();

signals:

    /// emits the palette clicked when a button is clicked
    void paletteClicked(EPalette);

private slots:

    /// handles a button click and converts it to a signal.
    void buttonClicked(EPalette);

private:
    /// widget used as main widget of QScrollArea.
    QWidget* mScrollWidget;

    /*!
     * \brief mPaletteWidgets vector of all palettes widgets getting displayed in the
     * scroll area.
     */
    std::vector<StoredPaletteWidget*> mPaletteWidgets;

    /*!
     * \brief mPresetHueLayout layout of all hue preset widgets.
     */
    QGridLayout* mLayout;
};

#endif // PALETTESCROLLAREA_H
