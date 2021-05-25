#ifndef PRESETGROUPWIDGET_H
#define PRESETGROUPWIDGET_H

#include <QLabel>
#include <QObject>
#include <QWidget>

#include "cor/objects/light.h"
#include "cor/widgets/button.h"
#include "cor/widgets/palettewidget.h"

/*!
 * \copyright
 * Copyright (C) 2015 - 2020.
 * Released under the GNU General Public License.
 *
 *
 * \brief The PaletteWidget is a widget used on the PalettePage to display the name and all the
 * colors for a palette.
 */
class StoredPaletteWidget : public QWidget {
    Q_OBJECT
public:
    explicit StoredPaletteWidget(const cor::Palette& palette, QWidget* parent);

    /// updates the palette displayed by the widget.
    void updatePalette(const cor::Palette& palette);

    /*!
     * \brief setChecked acts similarly to the setChecked of a standard QPushButton, but
     * allows you to call the specific button of the PresetGroupWidget by its lighting
     * palette.
     *
     * \param palette palette to use
     */
    void setChecked(cor::Palette palette);

    /// resize this widget and all subwidgets
    void resize();

signals:

    /*!
     * \brief paletteButtonClicked signal that any of the lightsbuttons were pressed and emitting
     *        their own signal.
     */
    void paletteButtonClicked(cor::Palette);

protected:
    /// handles when the mouse is released on a button. This acts as clicking a button.
    void mouseReleaseEvent(QMouseEvent*) { emit paletteButtonClicked(mPalette); }

    /// paint the widget
    void paintEvent(QPaintEvent*);

private:
    /// vector that shows the colors in a palette
    cor::PaletteWidget* mLightVector;

    /*!
     * \brief mLabel label for name of preset group.
     */
    QLabel* mLabel;

    /// palette enum
    cor::Palette mPalette;

    /// true if checked, false otherwise
    bool mIsChecked;
};


#endif // PRESETGROUPWIDGET_H
