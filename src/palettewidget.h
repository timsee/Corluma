#ifndef PRESETGROUPWIDGET_H
#define PRESETGROUPWIDGET_H

#include <QLabel>
#include <QObject>
#include <QWidget>

#include "cor/objects/light.h"
#include "cor/widgets/button.h"
#include "cor/widgets/lightvectorwidget.h"

/*!
 * \copyright
 * Copyright (C) 2015 - 2020.
 * Released under the GNU General Public License.
 *
 *
 * \brief The PaletteWidget is a widget used on the PalettePage to display the name and all the
 * colors for a palette.
 */
class PaletteWidget : public QWidget {
    Q_OBJECT
public:
    explicit PaletteWidget(const QString& name, EPalette palette, QWidget* parent);

    /*!
     * \brief setChecked acts similarly to the setChecked of a standard QPushButton, but
     * allows you to call the specific button of the PresetGroupWidget by its lighting
     * palette.
     *
     * \param palette palette to use
     */
    void setChecked(EPalette palette);

    /// resize this widget and all subwidgets
    void resize();

signals:

    /*!
     * \brief paletteButtonClicked signal that any of the lightsbuttons were pressed and emitting
     *        their own signal.
     */
    void paletteButtonClicked(EPalette);

protected:
    /// handles when the mouse is released on a button. This acts as clicking a button.
    void mouseReleaseEvent(QMouseEvent*) { emit paletteButtonClicked(mPalette); }

    /// paint the widget
    void paintEvent(QPaintEvent*);

private:
    /// vector that shows the colors in a palette
    cor::LightVectorWidget* mLightVector;

    /*!
     * \brief mLabel label for name of preset group.
     */
    QLabel* mLabel;

    /// palette enum
    EPalette mPalette;

    /// true if checked, false otherwise
    bool mIsChecked;
};


#endif // PRESETGROUPWIDGET_H
