#ifndef SINGLEFADEROUTINEWIDGET_H
#define SINGLEFADEROUTINEWIDGET_H

#include <QWidget>
#include "cor/widgets/slider.h"
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
          mLinearButton{new QPushButton("Linear", this)},
          mSineButton{new QPushButton("Sine", this)},
          mBlinkButton{new QPushButton("Blink", this)},
          mSawtoothInButton{new QPushButton("Saw In", this)},
          mSawtoothOutButton{new QPushButton("Saw Out", this)} {
        connect(mCheckBox,
                SIGNAL(clicked(ECheckboxState)),
                this,
                SLOT(checkBoxClicked(ECheckboxState)));

        connect(mLinearButton, SIGNAL(clicked(bool)), this, SLOT(linearPressed(bool)));
        connect(mSineButton, SIGNAL(clicked(bool)), this, SLOT(sinePressed(bool)));
        connect(mBlinkButton, SIGNAL(clicked(bool)), this, SLOT(blinkPressed(bool)));
        connect(mSawtoothInButton, SIGNAL(clicked(bool)), this, SLOT(sawtoothInPressed(bool)));
        connect(mSawtoothOutButton, SIGNAL(clicked(bool)), this, SLOT(sawtoothOutPressed(bool)));
    }

    /// update the color displayed by the widget
    void updateColor(const QColor& color) {
        mState.color(color);
        updateState(mState);
    }

    void selectRoutine(ERoutine routine) override {
        mHighlight = (routine == ERoutine::singleBlink) || (routine == ERoutine::singleFade)
                     || (routine == ERoutine::singleSawtoothFade);
        if (mHighlight) {
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
    /// handle when the checkbox is clicked
    void checkBoxClicked(ECheckboxState state) {
        if (state == ECheckboxState::unchecked) {
            mCheckBox->checkboxState(ECheckboxState::checked);
            mHighlight = true;
            emit clicked(mState);
            update();
        }
    }

    /// handle the linear button
    void linearPressed(bool) {
        mState.routine(ERoutine::singleFade);
        mState.param(0);
        mName->setText("Linear Fade");
        handleButtonPressed();
    }

    /// handle the sine button
    void sinePressed(bool) {
        mState.routine(ERoutine::singleFade);
        mState.param(1);
        mName->setText("Sine Fade");
        handleButtonPressed();
    }

    /// handle the blink button
    void blinkPressed(bool) {
        mState.routine(ERoutine::singleBlink);
        mName->setText("Blink");
        handleButtonPressed();
    }

    /// handle the sawtooth in button
    void sawtoothInPressed(bool) {
        mState.routine(ERoutine::singleSawtoothFade);
        mState.param(1);
        mName->setText("Saw In");
        handleButtonPressed();
    }

    /// handle the sawtooth out button
    void sawtoothOutPressed(bool) {
        mState.routine(ERoutine::singleSawtoothFade);
        mState.param(0);
        mName->setText("Saw Out");
        handleButtonPressed();
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

        mBlinkButton->setGeometry(xPos, yPos, buttonSide, buttonSide);
        xPos += mBlinkButton->width() + buttonSpacer;
        mLinearButton->setGeometry(xPos, yPos, buttonSide, buttonSide);
        xPos += mLinearButton->width() + buttonSpacer;
        mSineButton->setGeometry(xPos, yPos, buttonSide, buttonSide);
        xPos += mSineButton->width() + buttonSpacer;
        mSawtoothInButton->setGeometry(xPos, yPos, buttonSide, buttonSide);
        xPos += mSawtoothInButton->width() + buttonSpacer;
        mSawtoothOutButton->setGeometry(xPos, yPos, buttonSide, buttonSide);
        xPos += mSawtoothOutButton->width() + buttonSpacer;
    }

    /// handle when a button is pressed
    void handleButtonPressed() {
        if (mCheckBox->checkboxState() != ECheckboxState::checked) {
            mCheckBox->checkboxState(ECheckboxState::checked);
            mHighlight = true;
            update();
        }
        updateStateIcon();
        emit clicked(mState);
    }

    /// linear button
    QPushButton* mLinearButton;

    /// sine button
    QPushButton* mSineButton;

    /// blink button
    QPushButton* mBlinkButton;

    /// sawtooth in button
    QPushButton* mSawtoothInButton;

    /// sawtooth out button
    QPushButton* mSawtoothOutButton;
};

#endif // SINGLEFADEROUTINEWIDGET_H
