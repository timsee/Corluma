#ifndef MULTIBARSROUTINEWIDGET_H
#define MULTIBARSROUTINEWIDGET_H

#include <QComboBox>
#include <QWidget>
#include "routines/routinewidget.h"

/*!
 * \copyright
 * Copyright (C) 2015 - 2021.
 * Released under the GNU General Public License.
 *
 * \brief The MultiBarsRoutineWidget class is a routine widget that displays the multi bars
 * routine. This routine has one parameter determining how large groups of lights should be.
 *
 */
class MultiBarsRoutineWidget : public RoutineWidget {
    Q_OBJECT
public:
    explicit MultiBarsRoutineWidget(QWidget* parent)
        : RoutineWidget(parent, "Scrolling", ERoutine::multiBars),
          mLightCountLabel{new QLabel("Light Count", this)},
          mDropdownLightCount{new QComboBox(this)} {
        connect(mCheckBox,
                SIGNAL(clicked(ECheckboxState)),
                this,
                SLOT(checkBoxClicked(ECheckboxState)));

        mLightCountLabel->setStyleSheet(mTransparentStylesheet);

        mDropdownLightCount->setEditable(false);
        for (auto i = 1; i <= 5; ++i) {
            mDropdownLightCount->addItem(QString::number(i));
        }
        mDropdownLightCount->setCurrentIndex(3);
        connect(mDropdownLightCount,
                SIGNAL(currentTextChanged(QString)),
                this,
                SLOT(paramChanged(QString)));
        mState.param(4);
    }

    /// update the color displayed by the widget
    void updatePaletteColors(const std::vector<QColor>& colors, std::uint32_t brightness) {
        auto palette = cor::Palette(paletteToString(EPalette::custom), colors, brightness);
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

    /// handles when the param changes.
    void paramChanged(QString param) {
        mState.param(param.toInt());
        updateState(mState);
        emit clicked(mState);
    }

private:
    /// label for the count of lights
    QLabel* mLightCountLabel;

    /// drop down list to choose light count
    QComboBox* mDropdownLightCount;

    /// handle when it resizes
    void resize() override {
        resizeTopLeftMenu();
        auto xSpacer = width() * 0.02;
        auto ySpacer = height() * 0.02;
        auto yPos = ySpacer;
        auto xPos = mLeftWidgetSize.width() + xSpacer;
        auto workableWidth = width() - mLeftWidgetSize.width() - xSpacer * 2;

        auto labelHeight = height() * 0.96 / 3;

        mLightCountLabel->setGeometry(xPos, yPos, workableWidth, labelHeight);
        yPos += mLightCountLabel->height();
        mDropdownLightCount->setGeometry(xPos, yPos, workableWidth, labelHeight * 2);
    }
};

#endif // MULTIBARSROUTINEWIDGET_H
