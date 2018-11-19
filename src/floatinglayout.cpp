/*!
 * \copyright
 * Copyright (C) 2015 - 2018.
 * Released under the GNU General Public License.
 */

#include "floatinglayout.h"
#include "mainwindow.h"
#include "cor/utils.h"

#include <QDebug>
#include <QSignalMapper>
#include <QStyleOption>
#include <QGraphicsOpacityEffect>
#include <QPainter>

FloatingLayout::FloatingLayout(bool makeVertical, QWidget *parent) : QWidget(parent) {
    mIsVertical = makeVertical;
    mRoutineIsTranslucent = false;
    mRoutineIsHighlighted = false;

    this->setContentsMargins(0,0,0,0);
    // setup up the layout
    if (mIsVertical) {
        mVerticalLayout = new QVBoxLayout;
        mVerticalLayout->setContentsMargins(0,0,0,0);
        mVerticalLayout->setSpacing(0);

        setLayout(mVerticalLayout);
    } else {
        mHorizontalLayout = new QHBoxLayout;
        mHorizontalLayout->setContentsMargins(0,0,0,0);
        mHorizontalLayout->setSpacing(0);

        setLayout(mHorizontalLayout);
    }

    mOriginalSize = cor::applicationSize();
}

void FloatingLayout::highlightButton(QString key) {
    this->blockSignals(true);

    auto result = std::find(mNames.begin(), mNames.end(), key);
    int index = int(std::distance(mNames.begin(), result));
    buttonPressed(index);

    this->blockSignals(false);
}

//--------------------------------
// Setup Buttons
//--------------------------------

void FloatingLayout::setupButtons(std::vector<QString> buttons, EButtonSize eButtonSize) {
    // set up the geometry
    QSize size = mOriginalSize;
    if (eButtonSize == EButtonSize::small) {
        size = QSize(int(size.width() * 0.12f),
                     int(size.height() * 0.12f));
    } else if (eButtonSize == EButtonSize::medium) {
        size = QSize(int(size.width() * 0.15f),
                     int(size.height() * 0.15f));
    } else if (eButtonSize == EButtonSize::rectangle) {
        size = QSize(int(size.width() * 0.3f),
                     int(size.height() * 0.09f));
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
            fixedWidth = size.width();
            fixedHeight = size.width() * int(buttons.size());
        } else {
            fixedWidth = size.width() * int(buttons.size());
            fixedHeight = size.width();
        }
    }
    this->setFixedSize(QSize(fixedWidth, fixedHeight));

    // setup the horizontal buttons
    if (mButtons.size() > 0) {
        for (uint32_t i = 0; i < mButtons.size(); ++i) {
            if (mIsVertical) {
                mVerticalLayout->removeWidget(mButtons[i]);
            } else {
                mHorizontalLayout->removeWidget(mButtons[i]);
            }
            delete mButtons[i];
        }
    }
    mButtons = std::vector<QPushButton*>(buttons.size(), nullptr);
    mNames = buttons;

    QSignalMapper *buttonsMapper = new QSignalMapper(this);
    for (uint32_t i = 0; i < mNames.size(); ++i) {
        cor::Light light(mNames[i], "NO_CONTROLLER", ECommType::MAX);
        light.routine = ERoutine::singleSolid;
        light.color = QColor(255, 0, 0);

        bool foundMatch = false;
        if (mNames[i].compare("RGB") == 0) {
            foundMatch = true;
            mButtons[i] = new QPushButton(this);
            mButtons[i]->setCheckable(true);
            mButtons[i]->setMinimumSize(buttonSize());
        } else if (mNames[i].compare("Multi") == 0) {
            foundMatch = true;
            light.routine = ERoutine::multiFade;
            light.palette = mPalettes.palette(EPalette::cool);
            light.speed   = 100;
            QJsonObject routineObject = lightToJson(light);
            cor::Button *lightsButton = new cor::Button(this, routineObject);
            mButtons[i] = static_cast<QPushButton*>(lightsButton);
            Q_ASSERT(mButtons[i]);
        } else if (mNames[i].compare("ColorScheme") == 0) {
            foundMatch = true;
            mButtons[i] = new QPushButton(this);
            mButtons[i]->setCheckable(true);
            mButtons[i]->setMinimumSize(buttonSize());
        } else if (mNames[i].compare("Temperature") == 0) {
            foundMatch = true;
            mButtons[i] = new QPushButton(this);
            mButtons[i]->setCheckable(true);
            mButtons[i]->setMinimumSize(buttonSize());
        } else if (mNames[i].compare("Routine") == 0) {
            foundMatch = true;
            light.routine = ERoutine::singleGlimmer;
            light.color   = QColor(0, 255, 0);
            QJsonObject routineObject = lightToJson(light);
            cor::Button *lightsButton = new cor::Button(this, routineObject);
            mButtons[i] = static_cast<QPushButton*>(lightsButton);
            Q_ASSERT(mButtons[i]);
        } else if (mNames[i].compare("Settings") == 0) {
            foundMatch = true;
            mButtons[i] = new QPushButton(this);
            mButtons[i]->setMinimumSize(buttonSize());
        } else if (mNames[i].compare("Lights_Page") == 0) {
            foundMatch = true;
            mButtons[i] = new QPushButton(this);
            mButtons[i]->setCheckable(true);
            mButtons[i]->setMinimumSize(buttonSize());
        } else if (mNames[i].compare("Colors_Page") == 0) {
            foundMatch = true;
            mButtons[i] = new QPushButton(this);
            mButtons[i]->setCheckable(true);
            mButtons[i]->setMinimumSize(buttonSize());
        } else if (mNames[i].compare("Palette_Page") == 0) {
            foundMatch = true;
            light.routine = ERoutine::multiBars;
            light.palette = mPalettes.palette(EPalette::water);
            light.speed   = 100;
            QJsonObject routineObject = lightToJson(light);
            cor::Button *lightsButton = new cor::Button(this, routineObject);
            lightsButton->setCheckable(true);
            mButtons[i] = static_cast<QPushButton*>(lightsButton);
            Q_ASSERT(mButtons[i]);
        } else if (mNames[i].compare("Discovery_ArduCor") == 0) {
            foundMatch = true;
            mButtons[i] = new QPushButton(this);
            mButtons[i]->setCheckable(true);
            mButtons[i]->setStyleSheet("text-align:left");
            mButtons[i]->setIconSize(QSize(int(mButtons[i]->size().height() * 0.9f),
                                           int(mButtons[i]->size().height() * 0.9f)));
            mButtons[i]->setText("ArduCor");
        } else if (mNames[i].compare("Discovery_Hue") == 0) {
            foundMatch = true;
            mButtons[i] = new QPushButton(this);
            mButtons[i]->setCheckable(true);
            mButtons[i]->setStyleSheet("text-align:left");
            mButtons[i]->setIconSize(QSize(int(mButtons[i]->size().height() * 0.9f),
                                           int(mButtons[i]->size().height() * 0.9f)));
            mButtons[i]->setText("Hue");
        } else if (mNames[i].compare("Discovery_NanoLeaf") == 0) {
            foundMatch = true;
            mButtons[i] = new QPushButton(this);
            mButtons[i]->setCheckable(true);
            mButtons[i]->setStyleSheet("text-align:left");
            mButtons[i]->setIconSize(QSize(int(mButtons[i]->size().height() * 0.9f),
                                           int(mButtons[i]->size().height() * 0.9f)));
            mButtons[i]->setText("NanoLeaf");
        } else if (mNames[i].compare("Discovery") == 0) {
            foundMatch = true;
            mButtons[i] = new QPushButton(this);
            mButtons[i]->setCheckable(false);
            mButtons[i]->setMinimumSize(buttonSize());
        } else if (mNames[i].compare("Select_Devices") == 0) {
            foundMatch = true;
            mButtons[i] = new QPushButton(this);
            mButtons[i]->setCheckable(true);
            mButtons[i]->setMinimumSize(buttonSize());
        } else if (mNames[i].compare("HueLightSearch") == 0) {
            foundMatch = true;
            mButtons[i] = new QPushButton(this);
            mButtons[i]->setCheckable(true);
            mButtons[i]->setMinimumSize(buttonSize());
        } else if (mNames[i].compare("Mood_Page") == 0) {
            foundMatch = true;
            light.routine = ERoutine::multiFade;
            light.palette = mPalettes.palette(EPalette::fire);
            light.speed   = 100;
            QJsonObject routineObject = lightToJson(light);
            cor::Button *lightsButton = new cor::Button(this, routineObject);
            mButtons[i] = static_cast<QPushButton*>(lightsButton);
            Q_ASSERT(mButtons[i]);
        } else if (mNames[i].compare("New_Group") == 0) {
            foundMatch = true;
            mButtons[i] = new QPushButton(this);
            mButtons[i]->setCheckable(false);
            mButtons[i]->setMinimumSize(buttonSize());
        } else if (mNames[i].compare("Plus") == 0) {
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
                if (mNames[i].compare("RGB") == 0) {
                    cor::resizeIcon(mButtons[i], ":/images/colorWheel_icon.png");
                } else if (mNames[i].compare("Temperature") == 0) {
                    cor::resizeIcon(mButtons[i], ":/images/hueRange_icon.png");
                } else if (mNames[i].compare("Settings") == 0) {
                    cor::resizeIcon(mButtons[i], ":/images/settingsgear.png");
                } else if (mNames[i].compare("Discovery") == 0) {
                    cor::resizeIcon(mButtons[i], ":/images/wifi.png");
                } else if (mNames[i].compare("Select_Devices") == 0) {
                    cor::resizeIcon(mButtons[i], ":/images/colorWheel_icon.png");
                } else if (mNames[i].compare("New_Group") == 0) {
                    cor::resizeIcon(mButtons[i], ":/images/plusIcon.png");
                } else if (mNames[i].compare("Plus") == 0) {
                    cor::resizeIcon(mButtons[i], ":/images/plusIcon.png");
                } else if (mNames[i].compare("ColorScheme") == 0) {
                    cor::resizeIcon(mButtons[i], ":/images/ColorSchemePicker_icon.png");
                } else if (mNames[i].compare("HueLightSearch") == 0) {
                    cor::resizeIcon(mButtons[i], ":/images/plusIcon.png");
                } else if (mNames[i].compare("Lights_Page") == 0) {
                    cor::resizeIcon(mButtons[i], ":/images/connectionIcon.png");
                } else if (mNames[i].compare("Colors_Page") == 0) {
                    cor::resizeIcon(mButtons[i], ":/images/colorWheel_icon.png");
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
        if (mNames[i].compare("Routine") == 0) {
            cor::Button *lightsButton = static_cast<cor::Button*>(mButtons[i]);
            Q_ASSERT(lightsButton);
            lightsButton->updateRoutine(routineObject);
        }
    }
}


void FloatingLayout::updateColorPageButton(QString resource) {
    for (uint32_t i = 0; i < mButtons.size(); ++i) {
        if (mNames[i].compare("Colors_Page") == 0) {
            cor::resizeIcon(mButtons[i], resource);
        }
    }
}

void FloatingLayout::updateCollectionButton(QString resource) {
    for (uint32_t i = 0; i < mButtons.size(); ++i) {
        if (mNames[i].compare("New_Group") == 0) {
            cor::resizeIcon(mButtons[i], resource);
        }
    }
}

void FloatingLayout::updateDiscoveryButton(EProtocolType type, QPixmap pixmap) {
    QString label;
    switch (type)
    {
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
    for (uint32_t i = 0; i < mButtons.size(); ++i) {
        if (mNames[i].compare(label) == 0) {
            int size = int(mButtons[i]->size().height() * 0.5f);
            mButtons[i]->setIcon(QIcon(pixmap.scaled(size,
                                                size,
                                                Qt::IgnoreAspectRatio,
                                                Qt::SmoothTransformation)));
            mButtons[i]->setIconSize(QSize(size,
                                           size));
        }
    }

}

void FloatingLayout::enableButton(QString key, bool enable) {
    for (uint32_t i = 0; i < mButtons.size(); ++i) {
        if (mNames[i].compare(key) == 0) {
            mButtons[i]->setEnabled(enable);
        }
    }
}

//--------------------------------
// Moving
//--------------------------------


void FloatingLayout::move(QPoint topRightPoint) {
    // add floating region to far right of screen under main icon menu
    this->setGeometry(topRightPoint.x() - this->width(), topRightPoint.y(), this->width(), this->height());
}


//--------------------------------
// Slots
//--------------------------------

void FloatingLayout::buttonPressed(int buttonIndex) {
    // uncheck all other buttons
    for (uint32_t i = 0; i < mButtons.size(); ++i) {
        mButtons[i]->setChecked(false);
        if (isALightsButton(i)) {
            cor::Button *lightsButton = static_cast<cor::Button*>(mButtons[i]);
            Q_ASSERT(lightsButton);
            lightsButton->setChecked(false);
        }
    }

    // check the proper button
    uint32_t index = uint32_t(buttonIndex);
    if (index < mButtons.size()) {
        QString label = mNames[index];
        if (isALightsButton(index)) {
            cor::Button *lightsButton = static_cast<cor::Button*>(mButtons[index]);
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
    if (mNames[index].compare("Multi") == 0
            || mNames[index].compare("Routine") == 0
            || mNames[index].compare("Mood_Page") == 0
            || mNames[index].compare("Palette_Page") == 0) {
        return true;
    }
    return false;
}

QSize FloatingLayout::buttonSize() {
    return QSize(this->width() / int(mButtons.size()),
                 this->width() / int(mButtons.size()));
}
