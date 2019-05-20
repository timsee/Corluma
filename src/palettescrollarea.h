#ifndef PALETTESCROLLAREA_H
#define PALETTESCROLLAREA_H

#include <QObject>
#include <QScrollArea>

#include "presetgroupwidget.h"

/*!
 * \copyright
 * Copyright (C) 2015 - 2016.
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
     * \brief setupButtons sets up the routine buttons.
     */
    void setupButtons(bool isArduino);

    /*!
     * \brief highlightRoutineButton highlights the button that implements
     * the routine parameter. If it can't find a button that
     * implements this lighting routine, then all buttons are unhighlighted
     *
     * \param routine the lighting routine the highlighted button implements.
     * \param palette the color group that the highlighted button implements.
     */
    void highlightRoutineButton(ERoutine routine, EPalette palette);

    /// programmatically resize
    void resize();

private:
    /// widget used as main widget of QScrollArea.
    QWidget* mScrollWidget;

    /*!
     * \brief mPresetWidgets vector of all preset widgets getting displayed in the
     * scroll area.
     */
    std::vector<PresetGroupWidget*> mPresetWidgets;

    /*!
     * \brief mPresetHueLayout layout of all hue preset widgets.
     */
    QGridLayout* mLayout;
};

#endif // PALETTESCROLLAREA_H
