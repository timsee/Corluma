/*!
 * \copyright
 * Copyright (C) 2015 - 2020.
 * Released under the GNU General Public License.
 */

#include "floatinglayout.h"

#include <QDebug>
#include <QGraphicsOpacityEffect>
#include <QPainter>
#include <QStyleOption>

#include "mainwindow.h"
#include "utils/qt.h"

FloatingLayout::FloatingLayout(bool makeVertical, QWidget* parent) : QWidget(parent) {
    mIsVertical = makeVertical;
    mRoutineIsTranslucent = false;
    mRoutineIsHighlighted = false;

    setContentsMargins(0, 0, 0, 0);
    // setup up the layout
    if (mIsVertical) {
        mVerticalLayout = new QVBoxLayout;
        mVerticalLayout->setContentsMargins(0, 0, 0, 0);
        mVerticalLayout->setSpacing(0);

        setLayout(mVerticalLayout);
    } else {
        mHorizontalLayout = new QHBoxLayout;
        mHorizontalLayout->setContentsMargins(0, 0, 0, 0);
        mHorizontalLayout->setSpacing(0);

        setLayout(mHorizontalLayout);
    }

    mOriginalSize = cor::applicationSize();
}

void FloatingLayout::highlightButton(const QString& key) {
    blockSignals(true);

    auto result = std::find(mNames.begin(), mNames.end(), key);
    int index = int(std::distance(mNames.begin(), result));
    buttonPressed(index);

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
    } else if (buttonSize == EButtonSize::rectangle) {
        // extremely thin aspect ratios will likely make the rectangle still pretty thin, so account
        // for that
        auto ratio = float(size.height()) / size.width();
        auto width = size.width() * 0.22f;
        auto height = size.height() * 0.08f;
        if (ratio > 1.3f) {
            width = size.width() * 0.33f;
            height = size.height() * 0.1f;
        }
        size = QSize(int(width), int(height));
    }

    int fixedWidth;
    int fixedHeight;
    if (buttonSize == EButtonSize::rectangle) {
        if (mIsVertical) {
            fixedWidth = size.width();
            fixedHeight = size.height() * int(buttons.size());
        } else {
            fixedWidth = size.width() * int(buttons.size());
            fixedHeight = size.height();
        }
    } else {
        if (mIsVertical) {
            fixedWidth = size.width();
            fixedHeight = size.height() * int(buttons.size());
        } else {
            fixedWidth = size.height() * int(buttons.size());
            fixedHeight = size.height();
        }
    }
    setFixedSize(QSize(fixedWidth, fixedHeight));

    // setup the horizontal buttons
    if (!mButtons.empty()) {
        for (auto button : mButtons) {
            if (mIsVertical) {
                mVerticalLayout->removeWidget(button);
            } else {
                mHorizontalLayout->removeWidget(button);
            }
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
            || mNames[i] == "New_Group" || mNames[i] == "Plus" || mNames[i] == "Close") {
            foundMatch = true;
            mButtons[i] = new QPushButton(this);
            mButtons[i]->setCheckable(true);
            mButtons[i]->setMinimumSize(this->buttonSize());
        } else if (mNames[i] == "Off") {
            foundMatch = true;
            state.routine(ERoutine::multiFade);
            state.palette(mPalettes.palette(EPalette::poison));
            state.speed(100);
            state.paletteBrightness(0);
            state.isOn(false);
            auto lightsButton = new cor::Button(this, state);
            mButtons[i] = static_cast<QPushButton*>(lightsButton);
            Q_ASSERT(mButtons[i]);
        } else if (mNames[i] == "Preset") {
            foundMatch = true;
            state.routine(ERoutine::multiFade);
            state.palette(mPalettes.palette(EPalette::poison));
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
            mButtons[i] = new QPushButton(this);
            mButtons[i]->setCheckable(true);
            mButtons[i]->setStyleSheet("text-align:left");
            mButtons[i]->setIconSize(QSize(int(mButtons[i]->size().height() * 0.9f),
                                           int(mButtons[i]->size().height() * 0.9f)));
            mButtons[i]->setText("ArduCor");
        } else if (mNames[i] == "Discovery_Hue") {
            foundMatch = true;
            mButtons[i] = new QPushButton(this);
            mButtons[i]->setCheckable(true);
            mButtons[i]->setStyleSheet("text-align:left");
            mButtons[i]->setIconSize(QSize(int(mButtons[i]->size().height() * 0.9f),
                                           int(mButtons[i]->size().height() * 0.9f)));
            mButtons[i]->setText("Hue");
        } else if (mNames[i] == "Discovery_NanoLeaf") {
            foundMatch = true;
            mButtons[i] = new QPushButton(this);
            mButtons[i]->setCheckable(true);
            mButtons[i]->setStyleSheet("text-align:left");
            mButtons[i]->setIconSize(QSize(int(mButtons[i]->size().height() * 0.9f),
                                           int(mButtons[i]->size().height() * 0.9f)));
            mButtons[i]->setText("NanoLeaf");
        }

        if (foundMatch) {
            mButtons[i]->setMaximumWidth(this->buttonSize().width());
            mButtons[i]->setMaximumHeight(this->buttonSize().height());

            mButtons[i]->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
            if (mIsVertical) {
                mVerticalLayout->addWidget(mButtons[i]);
            } else {
                mHorizontalLayout->addWidget(mButtons[i]);
            }

            connect(mButtons[i], &QPushButton::clicked, [this, i]() { buttonPressed(int(i)); });

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
                } else if (mNames[i] == "Close") {
                    cor::resizeIcon(mButtons[i], ":/images/disabledX.png");
                } else if (mNames[i] == "HueLightSearch") {
                    cor::resizeIcon(mButtons[i], ":/images/plusIcon.png");
                } else if (mNames[i] == "Group_Lights") {
                    cor::resizeIcon(mButtons[i], ":/images/connectionIcon.png");
                } else if (mNames[i] == "Group_Details") {
                    cor::resizeIcon(mButtons[i], ":/images/wheels/color_wheel_hsv.png");
                } else if (mNames[i] == "Group_Edit") {
                    cor::resizeIcon(mButtons[i], ":/images/editIcon.png");
                }
            }
        }
    }

    if (buttonSize == EButtonSize::rectangle) {
        handleRectangleFontSize();
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

void FloatingLayout::updateDiscoveryButton(EProtocolType type, const QPixmap& pixmap) {
    QString label;
    switch (type) {
        case EProtocolType::hue:
            label = "Discovery_Hue";
            break;
        case EProtocolType::arduCor:
            label = "Discovery_ArduCor";
            break;
        case EProtocolType::nanoleaf:
            label = "Discovery_NanoLeaf";
            break;
        default:
            break;
    }
    for (std::size_t i = 0u; i < mButtons.size(); ++i) {
        if (mNames[i] == label && !pixmap.isNull()) {
            int size = int(mButtons[i]->size().height() * 0.5f);
            mButtons[i]->setIcon(
                QIcon(pixmap.scaled(size, size, Qt::IgnoreAspectRatio, Qt::SmoothTransformation)));
            mButtons[i]->setIconSize(QSize(size, size));
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
    auto width = buttonSize().width() * mButtons.size();
    // add floating region to far right of screen under main icon menu
    setGeometry(topRightPoint.x() - width, topRightPoint.y(), width, height());
}


//--------------------------------
// Slots
//--------------------------------

void FloatingLayout::buttonPressed(int buttonIndex) {
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

//--------------------------------
// Helpers
//--------------------------------


bool FloatingLayout::isALightsButton(std::uint32_t index) {
    return (mNames[index] == "Preset" || mNames[index] == "Routine" || mNames[index] == "Off");
}

QSize FloatingLayout::buttonSize() {
    if (mIsVertical) {
        return {height() / int(mButtons.size()), height() / int(mButtons.size())};
    } else {
        return {width() / int(mButtons.size()), width() / int(mButtons.size())};
    }
}

void FloatingLayout::handleRectangleFontSize() {
    QPushButton* widget = nullptr;
    for (auto button : mButtons) {
        if (button->text() == "NanoLeaf") {
            widget = button;
        }
    }
    // no nanoleaf, no need to resize text
    if (widget == nullptr) {
        return;
    }

#ifdef IOS_BUILD
    const auto& text = QString("NanoLeaf");
    // calcuate the text's size
    auto systemFontWidth = widget->fontMetrics().boundingRect(text).width();
    // calculate the button's size
    auto buttonWidth = (widget->width() - widget->iconSize().width()) * 0.95;
    // check if font needs to be resized
    if (systemFontWidth > buttonWidth) {
        auto fontPtSize = widget->font().pointSize();
        QFont font(widget->font());
        for (auto i = fontPtSize - 1; i > 0; --i) {
            font.setPointSize(i);
            widget->setFont(font);
            auto newFontWidth = widget->fontMetrics().boundingRect(text).width();
            if (newFontWidth < buttonWidth) {
                // font is small enough to fit
                break;
            }
        }

        for (auto button : mButtons) {
            button->setFont(font);
        }
    }
#endif // IOS_BUILD
}
