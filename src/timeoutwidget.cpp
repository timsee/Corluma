#include "timeoutwidget.h"

#include "utils/qt.h"

TimeoutWidget::TimeoutWidget(QWidget* parent,
                             int defaultTimeoutValue,
                             bool timeoutEnabled,
                             int spacerPixels)
    : QWidget(parent),
      mSpacerPixels{spacerPixels} {
    const QString transparentStyleSheet = "background-color: rgba(0,0,0,0)";
    //-----------
    // CheckBoxes
    //-----------

    if (timeoutEnabled) {
        mCheckBox = new cor::CheckBox(this, "Timeout Enabled");
    } else {
        mCheckBox = new cor::CheckBox(this, "Timeout Disabled");
    }
    connect(mCheckBox,
            SIGNAL(boxChecked(bool)),
            parentWidget(),
            SLOT(timeoutCheckboxPressed(bool)));

    //-----------
    // Sliders
    //-----------
    mSlider = new cor::Slider(this);
    mSlider->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    mSlider->slider()->setRange(0, 300);
    mSlider->slider()->setTickInterval(100);
    mSlider->slider()->setValue(defaultTimeoutValue);
    mSlider->setHeightPercentage(0.5f);
    mSlider->slider()->setTickPosition(QSlider::TicksBelow);
    mSlider->setColor(QColor(255, 127, 0));
    mSlider->setShouldDrawTickLabels(true);
    connect(mSlider, SIGNAL(valueChanged(int)), parentWidget(), SLOT(timeoutChanged(int)));

    //----------
    // Buttons
    //----------
    mMinutesButton = new QPushButton(this);
    mMinutesButton->setCheckable(true);
    mMinutesButton->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    connect(mMinutesButton, SIGNAL(clicked(bool)), this, SLOT(minutesClicked(bool)));
    mMinutesButton->setText("Minutes");

    mHoursButton = new QPushButton(this);
    mHoursButton->setCheckable(true);
    mHoursButton->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    connect(mHoursButton, SIGNAL(clicked(bool)), this, SLOT(hoursClicked(bool)));
    mHoursButton->setText("Hours");

    mCheckBox->setChecked(timeoutEnabled);
    show(timeoutEnabled);

    if (defaultTimeoutValue < 60) {
        mMode = ETimeoutMode::hours;
        handleMode(ETimeoutMode::minutes);
    } else {
        mMode = ETimeoutMode::minutes;
        handleMode(ETimeoutMode::hours);
    }
}

void TimeoutWidget::minutesClicked(bool) {
    handleMode(ETimeoutMode::minutes);
}

void TimeoutWidget::hoursClicked(bool) {
    handleMode(ETimeoutMode::hours);
}

void TimeoutWidget::handleMode(ETimeoutMode newMode) {
    switch (newMode) {
        case ETimeoutMode::hours:
            mMinutesButton->setChecked(false);
            mHoursButton->setChecked(true);
            break;
        case ETimeoutMode::minutes:
            mMinutesButton->setChecked(true);
            mHoursButton->setChecked(false);
            break;
    }
    if (mMode != newMode) {
        mMode = newMode;
        if (mMode == ETimeoutMode::hours) {
            mSlider->slider()->setRange(0, 300);
            mSlider->slider()->setTickInterval(100);
        } else if (mMode == ETimeoutMode::minutes) {
            auto currentValue = mSlider->slider()->value();
            if (currentValue > 60) {
                mSlider->slider()->setValue(60);
            }
            mSlider->slider()->setRange(0, 60);
            mSlider->slider()->setTickInterval(20);
        }
        mSlider->resize();
    }
}

void TimeoutWidget::changeTimeoutLabel(const QString& title) {
    mCheckBox->setTitle(title);
}

int TimeoutWidget::timeoutValue() {
    return mSlider->slider()->value();
}

void TimeoutWidget::show(bool shouldShow) {
    mSlider->setVisible(shouldShow);
    mMinutesButton->setVisible(shouldShow);
    mHoursButton->setVisible(shouldShow);
    resize(mLastWidgetHeight);
}

int TimeoutWidget::resize(int widgetHeight) {
    mLastWidgetHeight = widgetHeight;
    auto parentWidth = parentWidget()->width();

    // resize the checkboxes widths, if needed
    mCheckBox->downsizeTextWidthToFit(int(parentWidth * 0.5f));

    int currentY = 0;
    auto sliderWidth = int(width() * 0.7f);
    auto sliderHeight = int(widgetHeight * 2.75f);

    auto buttonWidth = width() * 0.15;
    const auto& appSize = cor::applicationSize();
    auto ratio = float(appSize.height()) / appSize.width();

    if (ratio > 1.3f) {
        buttonWidth = int(width() * 0.3f);
    }
    auto buttonHeight = buttonWidth / 2;
    auto checkBoxHeight = buttonHeight;

    mCheckBox->setGeometry(mSpacerPixels, mSpacerPixels, int(width() * 0.5), checkBoxHeight);

    currentY += mCheckBox->height() + mSpacerPixels;

    if (mCheckBox->checked()) {
        mSlider->setGeometry(2 * mSpacerPixels, currentY, sliderWidth, sliderHeight);
        currentY += mSlider->height() + mSpacerPixels;

        mMinutesButton->setGeometry(2 * mSpacerPixels, currentY, buttonWidth, buttonHeight);
        mHoursButton->setGeometry(buttonWidth + 3 * mSpacerPixels,
                                  currentY,
                                  buttonWidth,
                                  buttonHeight);
        currentY += mMinutesButton->height();
    }

    setGeometry(0, 0, parentWidth, currentY);
    return currentY;
}
