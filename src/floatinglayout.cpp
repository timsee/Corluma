/*!
 * \copyright
 * Copyright (C) 2015 - 2019.
 * Released under the GNU General Public License.
 */

#include "floatinglayout.h"

#include <QDebug>
#include <QGraphicsOpacityEffect>
#include <QPainter>
#include <QSignalMapper>
#include <QStyleOption>

#include "mainwindow.h"
#include "utils/qt.h"

FloatingLayout::FloatingLayout(bool makeVertical, QWidget* parent) : QWidget(parent) {
    mIsVertical = makeVertical;
    mRoutineIsTranslucent = false;
    mRoutineIsHighlighted = false;

    this->setContentsMargins(0, 0, 0, 0);
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
    this->blockSignals(true);

    auto result = std::find(mNames.begin(), mNames.end(), key);
    int index = int(std::distance(mNames.begin(), result));
    buttonPressed(index);

    this->blockSignals(false);
}

//--------------------------------
// Setup Buttons
//--------------------------------

void FloatingLayout::setupButtons(const std::vector<QString>& buttons, EButtonSize eButtonSize) {
    // set up the geometry
    QSize size = mOriginalSize;
    if (eButtonSize == EButtonSize::small) {
        size = QSize(int(size.width() * 0.08f), int(size.height() * 0.08f));
    } else if (eButtonSize == EButtonSize::medium) {
        size = QSize(int(size.width() * 0.1f), int(size.height() * 0.1f));
    } else if (eButtonSize == EButtonSize::rectangle) {
        // extremely thin aspect ratios will likely make the rectangle still pretty thin, so account
        // for that
        auto ratio = float(size.height()) / size.width();
        auto width = size.width() * 0.22f;
        auto height = size.height() * 0.08f;
        if (ratio > 1.3f) {
            width = size.width() * 0.33f;
            height = size.height() * 0.1f;
        }
        size = QSize(width, height);
    }

    int fixedWidth;
    int fixedHeight;
    if (eButtonSize == EButtonSize::rectangle) {
        if (mIsVertical) {
            fixedWidth = size.width();
            fixedHeight = size.height() * int(buttons.size());
        } else {
            fixedWidth = size.width() * int(buttons.size());
            fixedHeight = size.height();
        }
    } else {
        if (mIsVertical) {
            fixedWidth = size.height();
            fixedHeight = size.height() * int(buttons.size());
        } else {
            fixedWidth = size.height() * int(buttons.size());
            fixedHeight = size.height();
        }
    }
    this->setFixedSize(QSize(fixedWidth, fixedHeight));

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

    auto buttonsMapper = new QSignalMapper(this);
    for (uint32_t i = 0; i < mNames.size(); ++i) {
        cor::Light light;
        light.routine = ERoutine::singleSolid;
        light.color = QColor(255, 0, 0);

        bool foundMatch = false;
        if (mNames[i] == "RGB") {
            foundMatch = true;
            mButtons[i] = new QPushButton(this);
            mButtons[i]->setCheckable(true);
            mButtons[i]->setMinimumSize(buttonSize());
        } else if (mNames[i] == "HSV") {
            foundMatch = true;
            mButtons[i] = new QPushButton(this);
            mButtons[i]->setCheckable(true);
            mButtons[i]->setMinimumSize(buttonSize());
        } else if (mNames[i] == "Preset") {
            foundMatch = true;
            light.routine = ERoutine::multiFade;
            light.palette = mPalettes.palette(EPalette::poison);
            light.speed = 100;
            QJsonObject routineObject = lightToJson(light);
            auto lightsButton = new cor::Button(this, routineObject);
            mButtons[i] = static_cast<QPushButton*>(lightsButton);
            Q_ASSERT(mButtons[i]);
        } else if (mNames[i] == "Temperature") {
            foundMatch = true;
            mButtons[i] = new QPushButton(this);
            mButtons[i]->setCheckable(true);
            mButtons[i]->setMinimumSize(buttonSize());
        } else if (mNames[i] == "Routine") {
            foundMatch = true;
            light.routine = ERoutine::singleGlimmer;
            light.color = QColor(0, 255, 0);
            QJsonObject routineObject = lightToJson(light);
            auto lightsButton = new cor::Button(this, routineObject);
            mButtons[i] = static_cast<QPushButton*>(lightsButton);
            Q_ASSERT(mButtons[i]);
        } else if (mNames[i] == "Settings") {
            foundMatch = true;
            mButtons[i] = new QPushButton(this);
            mButtons[i]->setMinimumSize(buttonSize());
        } else if (mNames[i] == "Group_Lights") {
            foundMatch = true;
            mButtons[i] = new QPushButton(this);
            mButtons[i]->setCheckable(true);
            mButtons[i]->setMinimumSize(buttonSize());
        } else if (mNames[i] == "Group_Details") {
            foundMatch = true;
            mButtons[i] = new QPushButton(this);
            mButtons[i]->setCheckable(true);
            mButtons[i]->setMinimumSize(buttonSize());
        } else if (mNames[i] == "Group_Edit") {
            foundMatch = true;
            mButtons[i] = new QPushButton(this);
            mButtons[i]->setCheckable(true);
            mButtons[i]->setMinimumSize(buttonSize());
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
        } else if (mNames[i] == "Discovery") {
            foundMatch = true;
            mButtons[i] = new QPushButton(this);
            mButtons[i]->setCheckable(false);
            mButtons[i]->setMinimumSize(buttonSize());
        } else if (mNames[i] == "Select_Devices") {
            foundMatch = true;
            mButtons[i] = new QPushButton(this);
            mButtons[i]->setCheckable(true);
            mButtons[i]->setMinimumSize(buttonSize());
        } else if (mNames[i] == "HueLightSearch") {
            foundMatch = true;
            mButtons[i] = new QPushButton(this);
            mButtons[i]->setCheckable(true);
            mButtons[i]->setMinimumSize(buttonSize());
        } else if (mNames[i] == "New_Group") {
            foundMatch = true;
            mButtons[i] = new QPushButton(this);
            mButtons[i]->setCheckable(false);
            mButtons[i]->setMinimumSize(buttonSize());
        } else if (mNames[i] == "Plus") {
            foundMatch = true;
            mButtons[i] = new QPushButton(this);
            mButtons[i]->setCheckable(false);
            mButtons[i]->setMinimumSize(buttonSize());
        }

        if (foundMatch) {
            mButtons[i]->setMaximumWidth(buttonSize().width());
            mButtons[i]->setMaximumHeight(buttonSize().height());

            mButtons[i]->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
            if (mIsVertical) {
                mVerticalLayout->addWidget(mButtons[i]);
            } else {
                mHorizontalLayout->addWidget(mButtons[i]);
            }

            connect(mButtons[i], SIGNAL(clicked(bool)), buttonsMapper, SLOT(map()));
            buttonsMapper->setMapping(mButtons[i], int(i));

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
    connect(buttonsMapper, SIGNAL(mapped(int)), this, SLOT(buttonPressed(int)));
}


//--------------------------------
// Update Icons
//--------------------------------

void FloatingLayout::updateRoutine(const QJsonObject& routineObject) {
    for (uint32_t i = 0; i < mButtons.size(); ++i) {
        if (mNames[i] == "Routine") {
            auto lightsButton = dynamic_cast<cor::Button*>(mButtons[i]);
            Q_ASSERT(lightsButton);
            lightsButton->updateRoutine(routineObject);
        }
    }
}


void FloatingLayout::updateColorPageButton(const QString& resource) {
    for (uint32_t i = 0; i < mButtons.size(); ++i) {
        if (mNames[i] == "Colors_Page") {
            cor::resizeIcon(mButtons[i], resource);
        }
    }
}

void FloatingLayout::updateCollectionButton(const QString& resource) {
    for (uint32_t i = 0; i < mButtons.size(); ++i) {
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
    for (auto i = 0u; i < mButtons.size(); ++i) {
        if (mNames[i] == label && !pixmap.isNull()) {
            int size = int(mButtons[i]->size().height() * 0.5f);
            mButtons[i]->setIcon(
                QIcon(pixmap.scaled(size, size, Qt::IgnoreAspectRatio, Qt::SmoothTransformation)));
            mButtons[i]->setIconSize(QSize(size, size));
        }
    }
}

void FloatingLayout::enableButton(const QString& key, bool enable) {
    for (uint32_t i = 0; i < mButtons.size(); ++i) {
        if (mNames[i] == key) {
            mButtons[i]->setEnabled(enable);
        }
    }
}

bool FloatingLayout::isKeyHighlighted(const QString& key) {
    for (uint32_t i = 0; i < mButtons.size(); ++i) {
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
    this->setGeometry(topRightPoint.x() - this->width(),
                      topRightPoint.y(),
                      this->width(),
                      this->height());
}


//--------------------------------
// Slots
//--------------------------------

void FloatingLayout::buttonPressed(int buttonIndex) {
    // uncheck all other buttons
    for (uint32_t i = 0; i < mButtons.size(); ++i) {
        mButtons[i]->setChecked(false);
        if (isALightsButton(i)) {
            auto lightsButton = dynamic_cast<cor::Button*>(mButtons[i]);
            Q_ASSERT(lightsButton);
            lightsButton->setChecked(false);
        }
    }

    // check the proper button
    auto index = uint32_t(buttonIndex);
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


bool FloatingLayout::isALightsButton(uint32_t index) {
    return (mNames[index] == "Preset" || mNames[index] == "Routine");
}

QSize FloatingLayout::buttonSize() {
    return {this->width() / int(mButtons.size()), this->width() / int(mButtons.size())};
}
