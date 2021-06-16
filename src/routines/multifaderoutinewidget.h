#ifndef MULTIFADEROUTINEWIDGET_H
#define MULTIFADEROUTINEWIDGET_H

#include <QWidget>
#include "routines/routinewidget.h"


/*!
 * \copyright
 * Copyright (C) 2015 - 2021.
 * Released under the GNU General Public License.
 *
 * \brief The MultiFadeRoutineWidget class is a routine widget that displays the multi fade
 * routine. This routine has no additional options.
 *
 */
class MultiFadeRoutineWidget : public RoutineWidget {
    Q_OBJECT
public:
    explicit MultiFadeRoutineWidget(QWidget* parent)
        : RoutineWidget(parent, "Fade", ERoutine::multiFade) {
        connect(mCheckBox,
                SIGNAL(clicked(ECheckboxState)),
                this,
                SLOT(checkBoxClicked(ECheckboxState)));
    }

    /// update the color displayed by the widget
    void updatePaletteColors(const std::vector<QColor>& colors, std::uint32_t brightness) {
        auto palette = cor::Palette::CustomPalette(colors);
        mState.paletteBrightness(brightness);
        mState.palette(palette);
        updateState(mState);
    }

signals:

    /// handles when the widget is clicked
    void clicked(cor::LightState);

private slots:
    /// handle when the checkbox is clicked
    void checkBoxClicked(ECheckboxState state) {
        if (state == ECheckboxState::unchecked) {
            mCheckBox->checkboxState(ECheckboxState::checked);
            emit clicked(mState);
            update();
        }
    }

private:
    /// handle when it resizes
    void resize() override { resizeTopLeftMenu(); }
};

#endif // MULTIFADEROUTINEWIDGET_H
