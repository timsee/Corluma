#ifndef CHOOSESTATEWIDGET_H
#define CHOOSESTATEWIDGET_H
/*!
 * \copyright
 * Copyright (C) 2015 - 2020.
 * Released under the GNU General Public License.
 */

#include <QWidget>
#include "colorpicker/singlecolorpicker.h"
#include "cor/objects/lightstate.h"
#include "cor/presetpalettes.h"
#include "floatinglayout.h"
#include "palettescrollarea.h"
#include "routines/routinecontainer.h"

/*!
 * \brief The ChooseStateWidget class condenses all the widgets that can be used to change the state
 * of a light into a single widget. This is ideal for cases where you want to give the user full
 * control over the color, routine, and palette of the state. For instance, in a page where you are
 * editing a mood, it may be useful to set a state for an entire group of lights.
 */
class ChooseStateWidget : public QWidget {
    Q_OBJECT
public:
    explicit ChooseStateWidget(QWidget* parent);

    /// true if should be enabled, false if user cannot interact
    void enable(bool shouldEnable);

    /// update the state and the protocol used for showing the state
    void updateState(const cor::LightState& state, EProtocolType protocol);

    /// resize programmatically
    void resize();

    /// getter for state
    cor::LightState state() { return mState; }

signals:
    /// emits whenever the state changes
    void stateChanged(cor::LightState);

protected:
    /// handles when the widget is painted
    void paintEvent(QPaintEvent*);

    /// handles when the widget is resized
    void resizeEvent(QResizeEvent*);
private slots:

    /// handles color changes from the color picker
    void colorChanged(const QColor& color);

    /// handles ambient colro changes from the color picker
    void ambientUpdateReceived(std::uint32_t newAmbientValue, std::uint32_t newBrightness);

    /// handles when a palette button is clicked
    void paletteButtonClicked(cor::Palette);

    /// handles when a routine button is clicked
    void routineChanged(ERoutine, int, int);

    /// handles when the floating layout is pressed
    void floatingLayoutButtonPressed(QString);

private:
    /// show the colorpicker to choose HSV
    void changeToHSV();

    /// show the colorpicker to choose ambient colors
    void changeToTemperature();

    /// show the palette page to choose different multi color palettes
    void changeToPalette();

    /// show the routines to choose different lighting routines
    void changeToRoutine();

    /// handles when the lighting protocol type changes, this changes what menus are available
    void handleProtocol(EProtocolType protocol);

    /// fixes the highlighted button after "Off" is clicked
    void fixButtonHighlight();

    /// color picker for choosing colors
    SingleColorPicker* mColorPicker;

    /// palette scroll area for choosing palettes
    PaletteScrollArea* mPaletteScrollArea;

    /// routines buttons for choosing routines
    RoutineContainer* mRoutinesWidget;

    /// floating layout at the top of the widget
    FloatingLayout* mTopFloatingLayout;

    /// stores preset palettes to switch from EPalette enum to Palette objects
    PresetPalettes mPresetPalettes;

    /// protocol type, impacts what features are available
    EProtocolType mProtocol;

    /// stored state, modified by all the menus in the widget
    cor::LightState mState;
};

#endif // CHOOSESTATEWIDGET_H
