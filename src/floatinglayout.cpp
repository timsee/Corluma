/*!
 * \copyright
 * Copyright (C) 2015 - 2020.
 * Released under the GNU General Public License.
 */

#include "floatinglayout.h"
#include "connectionbutton.h"

#include <QDebug>
#include <QGraphicsOpacityEffect>
#include <QPainter>
#include <QStyleOption>

#include "mainwindow.h"
#include "utils/qt.h"

FloatingLayout::FloatingLayout(QWidget* parent) : QWidget(parent) {
    mRoutineIsTranslucent = false;
    mRoutineIsHighlighted = false;

    setContentsMargins(0, 0, 0, 0);
    // setup up the layout
    mLayout = new QHBoxLayout;
    mLayout->setContentsMargins(0, 0, 0, 0);
    mLayout->setSpacing(0);

    setLayout(mLayout);

    mOriginalSize = cor::applicationSize();
}

void FloatingLayout::highlightButton(const QString& key) {
    blockSignals(true);

    auto result = std::find(mNames.begin(), mNames.end(), key);
    int index = int(std::distance(mNames.begin(), result));
    handleButtonPressed(index);

    blockSignals(false);
}

//--------------------------------
// Setup Buttons
//--------------------------------

void FloatingLayout::setupButtons(const std::vector<QString>& buttons, EButtonSize buttonSize) {
    // set up the geometry
    QSize size = mOriginalSize;
    if (buttonSize == EButtonSize::small) {
        size = QSize(int(size.width() * 0.08f), int(size.height() * 0.08f));
    } else if (buttonSize == EButtonSize::medium) {
        size = QSize(int(size.width() * 0.1f), int(size.height() * 0.1f));
    }

    auto fixedWidth = size.height() * int(buttons.size());
    auto fixedHeight = size.height();

    setFixedSize(QSize(fixedWidth, fixedHeight));

    // setup the horizontal buttons
    if (!mButtons.empty()) {
        for (auto button : mButtons) {
            mLayout->removeWidget(button);
            delete button;
        }
    }
    mButtons = std::vector<QPushButton*>(buttons.size(), nullptr);
    mNames = buttons;

    for (std::size_t i = 0u; i < mNames.size(); ++i) {
        cor::LightState state;
        state.routine(ERoutine::singleSolid);
        state.color(QColor(255, 0, 0));
        state.isOn(true);


        bool foundMatch = false;
        if (mNames[i] == "HSV" || mNames[i] == "Temperature" || mNames[i] == "Settings"
            || mNames[i] == "Group_Lights" || mNames[i] == "Group_Details"
            || mNames[i] == "Group_Edit" || mNames[i] == "Discovery"
            || mNames[i] == "Select_Devices" || mNames[i] == "HueLightSearch"
            || mNames[i] == "New_Group" || mNames[i] == "Plus" || mNames[i] == "New_Palette"
            || mNames[i] == "Help" || mNames[i] == "Close") {
            foundMatch = true;
            mButtons[i] = new QPushButton(this);
            mButtons[i]->setCheckable(true);
            mButtons[i]->setMinimumSize(this->buttonSize());
        } else if (mNames[i] == "Off") {
            foundMatch = true;
            state.speed(100);
            state.paletteBrightness(0);
            state.isOn(false);
            auto lightsButton = new cor::Button(this, state);
            mButtons[i] = static_cast<QPushButton*>(lightsButton);
            Q_ASSERT(mButtons[i]);
        } else if (mNames[i] == "Preset") {
            foundMatch = true;
            cor::Palette palette(cor::UUID::invalidID(),
                                 "fake_name",
                                 {QColor(80, 0, 180),
                                  QColor(120, 0, 255),
                                  QColor(0, 0, 0),
                                  QColor(25, 0, 25),
                                  QColor(60, 60, 60),
                                  QColor(120, 0, 255),
                                  QColor(80, 0, 180),
                                  QColor(40, 0, 90),
                                  QColor(80, 0, 180)});

            state.routine(ERoutine::multiFade);
            state.palette(palette);
            state.speed(100);
            auto lightsButton = new cor::Button(this, state);
            mButtons[i] = static_cast<QPushButton*>(lightsButton);
            Q_ASSERT(mButtons[i]);
        } else if (mNames[i] == "Routine") {
            foundMatch = true;
            state.routine(ERoutine::singleGlimmer);
            state.color(QColor(0, 255, 0));
            auto lightsButton = new cor::Button(this, state);
            mButtons[i] = static_cast<QPushButton*>(lightsButton);
            Q_ASSERT(mButtons[i]);
        } else if (mNames[i] == "Discovery_ArduCor") {
            foundMatch = true;
            mButtons[i] = new ConnectionButton(this);
            mButtons[i]->setText("ArduCor");
        } else if (mNames[i] == "Discovery_Hue") {
            foundMatch = true;
            mButtons[i] = new ConnectionButton(this);
            mButtons[i]->setText("Hue");
        } else if (mNames[i] == "Discovery_NanoLeaf") {
            foundMatch = true;
            mButtons[i] = new ConnectionButton(this);
            mButtons[i]->setText("NanoLeaf");
        }

        if (foundMatch) {
            mButtons[i]->setMaximumWidth(this->buttonSize().width());
            mButtons[i]->setMaximumHeight(this->buttonSize().height());

            mButtons[i]->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
            mLayout->addWidget(mButtons[i]);

            connect(mButtons[i], &QPushButton::clicked, [this, i]() {
                handleButtonPressed(int(i));
            });

            if (!isALightsButton(i)) {
                // resize icon
                if (mNames[i] == "RGB") {
                    cor::resizeIcon(mButtons[i], ":/images/wheels/color_wheel_hsv.png");
                } else if (mNames[i] == "Temperature") {
                    cor::resizeIcon(mButtons[i], ":/images/wheels/color_wheel_ct.png");
                } else if (mNames[i] == "HSV") {
                    cor::resizeIcon(mButtons[i], ":/images/wheels/color_wheel_hs.png");
                } else if (mNames[i] == "Settings") {
                    cor::resizeIcon(mButtons[i], ":/images/settingsgear.png");
                } else if (mNames[i] == "Discovery") {
                    cor::resizeIcon(mButtons[i], ":/images/wifi.png");
                } else if (mNames[i] == "Select_Devices") {
                    cor::resizeIcon(mButtons[i], ":/mages/wheels/color_wheel_hsv.png");
                } else if (mNames[i] == "New_Group") {
                    cor::resizeIcon(mButtons[i], ":/images/plusIcon.png");
                } else if (mNames[i] == "Plus") {
                    cor::resizeIcon(mButtons[i], ":/images/plusIcon.png");
                } else if (mNames[i] == "New_Palette") {
                    cor::resizeIcon(mButtons[i], ":/images/plusIcon.png");
                } else if (mNames[i] == "Help") {
                    cor::resizeIcon(mButtons[i], ":/images/question_mark.png");
                } else if (mNames[i] == "Close") {
                    cor::resizeIcon(mButtons[i], ":/images/disabledX.png");
                } else if (mNames[i] == "HueLightSearch") {
                    cor::resizeIcon(mButtons[i], ":/images/plusIcon.png");
                } else if (mNames[i] == "Group_Lights") {
                    cor::resizeIcon(mButtons[i], ":/images/connectionIcon.png");
                } else if (mNames[i] == "Group_Details") {
                    cor::resizeIcon(mButtons[i], ":/images/wheels/color_wheel_hsv.png");
                } else if (mNames[i] == "Group_Edit") {
                    cor::resizeIcon(mButtons[i], ":/images/edit_icon.png");
                }
            }
        }
    }
}


//--------------------------------
// Update Icons
//--------------------------------

void FloatingLayout::updateRoutine(const cor::LightState& state) {
    for (std::size_t i = 0u; i < mButtons.size(); ++i) {
        if (mNames[i] == "Routine") {
            auto lightsButton = dynamic_cast<cor::Button*>(mButtons[i]);
            Q_ASSERT(lightsButton);
            lightsButton->updateRoutine(state);
        }
    }
}


void FloatingLayout::updateColorPageButton(const QString& resource) {
    for (std::size_t i = 0u; i < mButtons.size(); ++i) {
        if (mNames[i] == "Colors_Page") {
            cor::resizeIcon(mButtons[i], resource);
        }
    }
}

void FloatingLayout::updateCollectionButton(const QString& resource) {
    for (std::size_t i = 0u; i < mButtons.size(); ++i) {
        if (mNames[i] == "New_Group") {
            cor::resizeIcon(mButtons[i], resource);
        }
    }
}

void FloatingLayout::enableButton(const QString& key, bool enable) {
    for (std::size_t i = 0u; i < mButtons.size(); ++i) {
        if (mNames[i] == key) {
            mButtons[i]->setEnabled(enable);
        }
    }
}

bool FloatingLayout::isKeyHighlighted(const QString& key) {
    for (std::size_t i = 0u; i < mButtons.size(); ++i) {
        if (mNames[i] == key) {
            return mButtons[i]->isEnabled();
        }
    }
    return false;
}

//--------------------------------
// Moving
//--------------------------------


void FloatingLayout::move(QPoint topRightPoint) {
    // add floating region to far right of screen under main icon menu
    setGeometry(topRightPoint.x() - width(), topRightPoint.y(), width(), height());
}


//--------------------------------
// Slots
//--------------------------------

void FloatingLayout::handleButtonPressed(int buttonIndex) {
    // uncheck all other buttons
    for (std::size_t i = 0u; i < mButtons.size(); ++i) {
        mButtons[i]->setChecked(false);
        if (isALightsButton(i)) {
            auto lightsButton = dynamic_cast<cor::Button*>(mButtons[i]);
            Q_ASSERT(lightsButton);
            lightsButton->setChecked(false);
        }
    }

    // check the proper button
    auto index = std::uint32_t(buttonIndex);
    if (index < mButtons.size()) {
        QString label = mNames[index];
        if (isALightsButton(index)) {
            auto lightsButton = dynamic_cast<cor::Button*>(mButtons[index]);
            Q_ASSERT(lightsButton);
            lightsButton->setChecked(true);
        } else {
            mButtons[index]->setChecked(true);
        }


        // emit label of checked box
        emit buttonPressed(label);
    }
}

void FloatingLayout::useDarkTheme() {
    const QString stylesheet = "QPushButton { background-color:rgb(33,32,32); } "
                               "QPushButton:checked { background-color:#4A4949; }";
    for (auto button : mButtons) {
        button->setStyleSheet(stylesheet);
    }
}


//--------------------------------
// Helpers
//--------------------------------


bool FloatingLayout::isALightsButton(std::uint32_t index) {
    return (mNames[index] == "Preset" || mNames[index] == "Routine" || mNames[index] == "Off");
}

QSize FloatingLayout::buttonSize() {
    return {width() / int(mButtons.size()), height()};
}
