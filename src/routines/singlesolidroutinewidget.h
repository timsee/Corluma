#ifndef SINGLESOLIDROUTINEWIDGET_H
#define SINGLESOLIDROUTINEWIDGET_H

#include <QWidget>
#include "routines/routinewidget.h"


/*!
 * \copyright
 * Copyright (C) 2015 - 2021.
 * Released under the GNU General Public License.
 *
 * \brief The SingleSolidRoutineWidget class is a routine widget that displays the single solid
 * routine. This routine has no additional options.
 *
 */
class SingleSolidRoutineWidget : public RoutineWidget {
    Q_OBJECT
public:
    explicit SingleSolidRoutineWidget(QWidget* parent)
        : RoutineWidget(parent, "Solid", ERoutine::singleSolid) {
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

    /// handles when the widget is clicked
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

#endif // SINGLESOLIDROUTINEWIDGET_H
