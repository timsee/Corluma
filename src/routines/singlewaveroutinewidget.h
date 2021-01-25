#ifndef SINGLEWAVEROUTINEWIDGET_H
#define SINGLEWAVEROUTINEWIDGET_H

#include <QWidget>
#include "cor/widgets/slider.h"
#include "routines/routinewidget.h"

/*!
 * \copyright
 * Copyright (C) 2015 - 2021.
 * Released under the GNU General Public License.
 *
 * \brief The SingleWaveRoutineWidget class is a routine widget that displays the single wave
 * routine. This routine has no additional options.
 *
 */
class SingleWaveRoutineWidget : public RoutineWidget {
    Q_OBJECT
public:
    explicit SingleWaveRoutineWidget(QWidget* parent)
        : RoutineWidget(parent, "Wave", ERoutine::singleWave) {
        connect(mCheckBox,
                SIGNAL(clicked(ECheckboxState)),
                this,
                SLOT(checkBoxClicked(ECheckboxState)));
    }

    /// update the color displayed by the widget
    void updateColor(const QColor& color) {
        mState.color(color);
        updateState(mState);
    }

signals:

    /// emits when the checkbox is clicked
    void clicked(cor::LightState);

private slots:
    /// handle when the checkbox is clicked
    void checkBoxClicked(ECheckboxState state) {
        if (state == ECheckboxState::unchecked) {
            mCheckBox->checkboxState(ECheckboxState::checked);
            mHighlight = true;
            emit clicked(mState);
            update();
        }
    }

private:
    /// handle when it resizes
    void resize() override { resizeTopLeftMenu(); }
};

#endif // SINGLEWAVEROUTINEWIDGET_H
