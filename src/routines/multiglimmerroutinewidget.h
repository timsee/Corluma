#ifndef MULTIGLIMMERROUTINEWIDGET_H
#define MULTIGLIMMERROUTINEWIDGET_H

#include <QWidget>
#include "cor/widgets/slider.h"
#include "routines/routinewidget.h"


/*!
 * \copyright
 * Copyright (C) 2015 - 2021.
 * Released under the GNU General Public License.
 *
 * \brief The MultiGlimmerRoutineWidget class is a routine widget that displays the multi glimmer
 * routine. This has a slider that determines the frequency of the glimmer.
 *
 */
class MultiGlimmerRoutineWidget : public RoutineWidget {
    Q_OBJECT
public:
    explicit MultiGlimmerRoutineWidget(QWidget* parent)
        : RoutineWidget(parent, "Glimmer", ERoutine::multiGlimmer),
          mPercentLabel{new QLabel(this)},
          mPercentSlider{new cor::Slider(this)} {
        auto defaultValue = 15;
        mState.param(defaultValue);

        mPercentSlider->setColor(QColor(255, 0, 0));
        mPercentSlider->setRange(1, 100);
        mPercentSlider->setValue(defaultValue);
        mPercentSlider->setBackgroundColor(QColor(48, 47, 47));
        connect(mPercentSlider, SIGNAL(valueChanged(int)), this, SLOT(percentSliderChanged(int)));

        mPercentLabel->setWordWrap(true);
        mPercentLabel->setAlignment(Qt::AlignVCenter);
        mPercentLabel->setText("Frequency:<br>" + QString::number(defaultValue) + "%");
        mPercentLabel->setStyleSheet(cor::kTransparentStylesheet);

        connect(mCheckBox,
                SIGNAL(clicked(ECheckboxState)),
                this,
                SLOT(checkBoxClicked(ECheckboxState)));
    }

    /// update the color displayed by the widget
    void updatePaletteColors(const std::vector<QColor>& colors, std::uint32_t brightness) {
        auto palette = cor::Palette(paletteToString(EPalette::custom), colors, brightness);
        mState.palette(palette);
        mPercentSlider->setColor(palette.averageColor());
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
            emit clicked(mState);
            update();
        }
    }

    /// handles when the percent slider changes.
    void percentSliderChanged(int sliderValue) {
        mPercentLabel->setText("Frequency:<br>" + QString::number(sliderValue) + "%");
        mState.param(sliderValue);
        emit clicked(mState);
    }

private:
    /// handle when it resizes
    void resize() override {
        resizeTopLeftMenu();

        auto xSpacer = width() * 0.02;
        auto ySpacer = height() * 0.02;

        auto rowHeight = height() - ySpacer * 2;
        auto yPos = ySpacer;
        auto xPos = xSpacer + mLeftWidgetSize.width();
        auto workableWidth = width() - mLeftWidgetSize.width() - xSpacer * 2;
        auto sliderWidth = 0.7 * workableWidth;
        auto labelWidth = 0.3 * workableWidth;
        auto sliderPadding = rowHeight * 0.3;
        auto sliderHeight = rowHeight * 0.4;
        mPercentLabel->setGeometry(xPos, yPos, labelWidth, rowHeight);
        xPos += mPercentLabel->width();
        mPercentSlider->setGeometry(xPos, yPos + sliderPadding, sliderWidth, sliderHeight);
    }

    /// label that has the glimmer percent value
    QLabel* mPercentLabel;

    /// slider to change the percent.
    cor::Slider* mPercentSlider;
};

#endif // MULTIGLIMMERROUTINEWIDGET_H
