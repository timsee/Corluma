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
          mSawtoothInButton{new QPushButton(this)},
          mSawtoothOutButton{new QPushButton(this)} {
        connect(mCheckBox,
                SIGNAL(clicked(ECheckboxState)),
                this,
                SLOT(checkBoxClicked(ECheckboxState)));

        mLinearButton->setCheckable(true);
        mSineButton->setCheckable(true);
        mBlinkButton->setCheckable(true);
        mSawtoothInButton->setCheckable(true);
        mSawtoothOutButton->setCheckable(true);

        highlightButton(ERoutine::singleFade, 0);

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
    /// handle when the checkbox is clicked
    void checkBoxClicked(ECheckboxState state) {
        if (state == ECheckboxState::unchecked) {
            mCheckBox->checkboxState(ECheckboxState::checked);
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
        highlightButton(ERoutine::singleFade, 0);
    }

    /// handle the sine button
    void sinePressed(bool) {
        mState.routine(ERoutine::singleFade);
        mState.param(1);
        mName->setText("Sine Fade");
        handleButtonPressed();
        highlightButton(ERoutine::singleFade, 1);
    }

    /// handle the blink button
    void blinkPressed(bool) {
        mState.routine(ERoutine::singleBlink);
        mName->setText("Blink");
        handleButtonPressed();
        highlightButton(ERoutine::singleBlink, 0);
    }

    /// handle the sawtooth in button
    void sawtoothInPressed(bool) {
        mState.routine(ERoutine::singleSawtoothFade);
        mState.param(1);
        mName->setText("Saw In");
        handleButtonPressed();
        highlightButton(ERoutine::singleSawtoothFade, 1);
    }

    /// handle the sawtooth out button
    void sawtoothOutPressed(bool) {
        mState.routine(ERoutine::singleSawtoothFade);
        mState.param(0);
        mName->setText("Saw Out");
        handleButtonPressed();
        highlightButton(ERoutine::singleSawtoothFade, 0);
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

        mBlinkButton->setGeometry(xPos, yPos, buttonSide, buttonSide);
        xPos += mBlinkButton->width() + buttonSpacer;
        mLinearButton->setGeometry(xPos, yPos, buttonSide, buttonSide);
        xPos += mLinearButton->width() + buttonSpacer;
        if (mProtocol == EProtocolType::arduCor) {
            mSineButton->setGeometry(xPos, yPos, buttonSide, buttonSide);
            xPos += mSineButton->width() + buttonSpacer;
        }
        mSawtoothInButton->setGeometry(xPos, yPos, buttonSide, buttonSide);
        xPos += mSawtoothInButton->width() + buttonSpacer;
        mSawtoothOutButton->setGeometry(xPos, yPos, buttonSide, buttonSide);
        xPos += mSawtoothOutButton->width() + buttonSpacer;

        addIconToButton(mSawtoothInButton, ":/images/sawtooth_in.png", buttonSide);
        addIconToButton(mSawtoothOutButton, ":/images/sawtooth_out.png", buttonSide);
    }

    /// sine fade isnt as clear on nanoleafs, remove the option.
    void updateProtocol() override {
        // hide sawtooth and sine for nanoleaf, as it only supports linear fades. remap those
        // buttons to linear in nanoleaf case.
        if (mProtocol == EProtocolType::nanoleaf) {
            mSineButton->setVisible(false);
        } else {
            mSineButton->setVisible(true);
        }
        resize();
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

    /// adds icon to button
    void addIconToButton(QPushButton* button, const QString& path, int buttonSide) {
        auto pixmap = QPixmap(path);
        button->setIconSize(QSize(buttonSide, buttonSide));
        button->setIcon(QIcon(
            pixmap.scaled(buttonSide, buttonSide, Qt::KeepAspectRatio, Qt::SmoothTransformation)));
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
