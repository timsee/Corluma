#ifndef MULTIRANDOMROUTINEWIDGET_H
#define MULTIRANDOMROUTINEWIDGET_H

#include <QComboBox>
#include <QWidget>
#include "routines/routinewidget.h"


/*!
 * \copyright
 * Copyright (C) 2015 - 2021.
 * Released under the GNU General Public License.
 *
 * \brief The MultiRandomRoutineWidget class is a routine widget that displays the multi random
 * routines. Each routine ahs no additional options, but this can choose either
 * multiRandomIndividual or multiRandomSolid.
 *
 */
class MultiRandomRoutineWidget : public RoutineWidget {
    Q_OBJECT
public:
    explicit MultiRandomRoutineWidget(QWidget* parent)
        : RoutineWidget(parent, "Random", ERoutine::multiRandomIndividual),
          mStateLabel{new QLabel(this)},
          mDropdownMenu{new QComboBox(this)},
          mRandomIndividualString{"One color per light."},
          mRandomSolidString{"One color for all lights."} {
        connect(mCheckBox,
                SIGNAL(clicked(ECheckboxState)),
                this,
                SLOT(checkBoxClicked(ECheckboxState)));

        mStateLabel->setStyleSheet(mTransparentStylesheet);

        mDropdownMenu->setEditable(false);
        mDropdownMenu->addItem(mRandomIndividualString);
        mDropdownMenu->addItem(mRandomSolidString);
        connect(mDropdownMenu,
                SIGNAL(currentTextChanged(QString)),
                this,
                SLOT(menuChanged(QString)));
    }

    /// update the color displayed by the widget
    void updatePaletteColors(const std::vector<QColor>& colors, std::uint32_t brightness) {
        auto palette = cor::Palette(paletteToString(EPalette::custom), colors, brightness);
        mState.palette(palette);
        updateState(mState);
    }

    /// overrides the select routine to handle the multiple routines and parameters handled in this
    /// widget.
    void selectRoutine(ERoutine routine, int) override {
        auto shouldCheck =
            (routine == ERoutine::multiRandomIndividual) || (routine == ERoutine::multiRandomSolid);
        if (shouldCheck) {
            handleComboBox(routine);
            // change the highlight based off of the param
            mCheckBox->checkboxState(ECheckboxState::checked);
        } else {
            mCheckBox->checkboxState(ECheckboxState::unchecked);
        }
        update();
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

    /// handles when the menu is changed
    void menuChanged(QString value) {
        if (value == mRandomIndividualString) {
            mState.routine(ERoutine::multiRandomIndividual);
        } else if (value == mRandomSolidString) {
            mState.routine(ERoutine::multiRandomSolid);
        }
        emit clicked(mState);
    }

private:
    /// handles the combo box when a routine is selected
    void handleComboBox(ERoutine routine) {
        if (routine == ERoutine::multiRandomIndividual) {
            mDropdownMenu->setCurrentIndex(0);
        } else if (routine == ERoutine::multiRandomSolid) {
            mDropdownMenu->setCurrentIndex(1);
        }
    }

    /// state label
    QLabel* mStateLabel;

    /// dropdown menu to choose a version
    QComboBox* mDropdownMenu;

    /// string for random individual
    QString mRandomIndividualString;

    /// string for random solid
    QString mRandomSolidString;

    /// handle when it resizes
    void resize() override {
        resizeTopLeftMenu();
        auto xSpacer = width() * 0.02;
        auto ySpacer = height() * 0.02;
        auto yPos = ySpacer;
        auto xPos = mLeftWidgetSize.width() + xSpacer;
        auto workableWidth = width() - mLeftWidgetSize.width() - xSpacer * 2;

        auto labelHeight = height() * 0.96 / 3;

        mStateLabel->setGeometry(xPos, yPos, workableWidth, labelHeight);
        yPos += mStateLabel->height();
        mDropdownMenu->setGeometry(xPos, yPos, workableWidth, labelHeight * 2);
    }
};

#endif // MULTIRANDOMROUTINEWIDGET_H
