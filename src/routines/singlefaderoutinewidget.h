#ifndef SINGLEFADEROUTINEWIDGET_H
#define SINGLEFADEROUTINEWIDGET_H

#include <QWidget>
#include "cor/widgets/slider.h"
#include "routines/fadebutton.h"
#include "routines/routinewidget.h"

/*!
 * \copyright
 * Copyright (C) 2015 - 2021.
 * Released under the GNU General Public License.
 *
 * \brief The SingleFadeRoutineWidget class is a routine widget that displays the single fade
 * routines. This is a series of routines, singleFade, singleSawtoothFade, and singleBlink.
 *
 */
class SingleFadeRoutineWidget : public RoutineWidget {
    Q_OBJECT
public:
    explicit SingleFadeRoutineWidget(QWidget* parent)
        : RoutineWidget(parent, "Linear Fade", ERoutine::singleFade),
          mLinearButton{new cor::FadeButton(this, "Linear", ":/images/fade/fade_linear_icon.png")},
          mSineButton{new cor::FadeButton(this, "Sine", ":/images/fade/fade_sine_icon.png")},
          mBlinkButton{new cor::FadeButton(this, "Blink", ":/images/fade/fade_blink_icon.png")},
          mSawtoothInButton{
              new cor::FadeButton(this, "Saw In", ":/images/fade/fade_sawtooth_in_icon.png")},
          mSawtoothOutButton{
              new cor::FadeButton(this, "Saw Out", ":/images/fade/fade_sawtooth_out_icon.png")} {
        connect(mCheckBox,
                SIGNAL(clicked(ECheckboxState)),
                this,
                SLOT(checkBoxClicked(ECheckboxState)));

        highlightButton(ERoutine::singleFade, 0);

        connect(mLinearButton, SIGNAL(pressed(QString)), this, SLOT(buttonPressed(QString)));
        connect(mSineButton, SIGNAL(pressed(QString)), this, SLOT(buttonPressed(QString)));
        connect(mBlinkButton, SIGNAL(pressed(QString)), this, SLOT(buttonPressed(QString)));
        connect(mSawtoothInButton, SIGNAL(pressed(QString)), this, SLOT(buttonPressed(QString)));
        connect(mSawtoothOutButton, SIGNAL(pressed(QString)), this, SLOT(buttonPressed(QString)));
    }

    /// update the color displayed by the widget
    void updateColor(const QColor& color) {
        mState.color(color);
        updateState(mState);
    }

    /// overrides the select routine to handle the multiple routines and parameters handled in this
    /// widget.
    void selectRoutine(ERoutine routine, int param) override {
        auto shouldCheck = (routine == ERoutine::singleBlink) || (routine == ERoutine::singleFade)
                           || (routine == ERoutine::singleSawtoothFade);
        if (shouldCheck) {
            highlightButton(routine, param);
            // change the highlight based off of the param
            mCheckBox->checkboxState(ECheckboxState::checked);
        } else {
            mCheckBox->checkboxState(ECheckboxState::unchecked);
        }
        update();
    }

signals:

    /// emits when the checkbox is clicked
    void clicked(cor::LightState);

private slots:

    void buttonPressed(QString key) {
        if (key == "Linear") {
            mState.routine(ERoutine::singleFade);
            mState.param(0);
            highlightButton(ERoutine::singleFade, 0);
        } else if (key == "Sine") {
            mState.routine(ERoutine::singleFade);
            mState.param(1);
            highlightButton(ERoutine::singleFade, 1);
        } else if (key == "Blink") {
            mState.routine(ERoutine::singleBlink);
            highlightButton(ERoutine::singleBlink, 0);
        } else if (key == "Saw In") {
            mState.routine(ERoutine::singleSawtoothFade);
            mState.param(1);
            highlightButton(ERoutine::singleSawtoothFade, 1);
        } else if (key == "Saw Out") {
            mState.routine(ERoutine::singleSawtoothFade);
            mState.param(0);
            highlightButton(ERoutine::singleSawtoothFade, 0);
        }

        mName->setText(key);
        handleButtonPressed();
    }

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
    void resize() override {
        resizeTopLeftMenu();
        auto buttonSpacer = width() * 0.01;
        auto buttonsWidth = width() - buttonSpacer;
        auto xPos = buttonSpacer;
        auto yPos = mLeftWidgetSize.height();
        auto buttonSide = int((buttonsWidth / 5) - buttonSpacer);
        auto usableHeight = height() - mLeftWidgetSize.height() - height() * 0.03;
        if (buttonSide > usableHeight) {
            buttonSide = usableHeight;
        }

        mLinearButton->setGeometry(xPos, yPos, buttonSide, buttonSide);
        xPos += mLinearButton->width() + buttonSpacer;
        mSawtoothInButton->setGeometry(xPos, yPos, buttonSide, buttonSide);
        xPos += mSawtoothInButton->width() + buttonSpacer;
        mSawtoothOutButton->setGeometry(xPos, yPos, buttonSide, buttonSide);
        xPos += mSawtoothOutButton->width() + buttonSpacer;
        mSineButton->setGeometry(xPos, yPos, buttonSide, buttonSide);
        xPos += mSineButton->width() + buttonSpacer;
        mBlinkButton->setGeometry(xPos, yPos, buttonSide, buttonSide);
        xPos += mBlinkButton->width() + buttonSpacer;
    }

    /// handle when a button is pressed
    void handleButtonPressed() {
        if (mCheckBox->checkboxState() != ECheckboxState::checked) {
            mCheckBox->checkboxState(ECheckboxState::checked);
            update();
        }
        updateStateIcon();
        emit clicked(mState);
    }

    /// highlights a button based on the routine and parameter provided.
    void highlightButton(ERoutine routine, int param) {
        mLinearButton->setChecked(false);
        mSineButton->setChecked(false);
        mBlinkButton->setChecked(false);
        mSawtoothInButton->setChecked(false);
        mSawtoothOutButton->setChecked(false);

        if (routine == ERoutine::singleBlink) {
            mBlinkButton->setChecked(true);
        } else if (routine == ERoutine::singleFade && param == 0) {
            mLinearButton->setChecked(true);
        } else if (routine == ERoutine::singleFade && param == 1) {
            mSineButton->setChecked(true);
        } else if (routine == ERoutine::singleSawtoothFade && param == 1) {
            mSawtoothInButton->setChecked(true);
        } else if (routine == ERoutine::singleSawtoothFade && param == 0) {
            mSawtoothOutButton->setChecked(true);
        }
    }

    /// linear button
    cor::FadeButton* mLinearButton;

    /// sine button
    cor::FadeButton* mSineButton;

    /// blink button
    cor::FadeButton* mBlinkButton;

    /// sawtooth in button
    cor::FadeButton* mSawtoothInButton;

    /// sawtooth out button
    cor::FadeButton* mSawtoothOutButton;
};

#endif // SINGLEFADEROUTINEWIDGET_H
