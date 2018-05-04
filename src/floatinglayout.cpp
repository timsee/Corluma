/*!
 * \copyright
 * Copyright (C) 2015 - 2018.
 * Released under the GNU General Public License.
 */

#include "floatinglayout.h"
#include "mainwindow.h"
#include "cor/utils.h"

#include <QDebug>
#include <QApplication>
#include <QScreen>
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
}

void FloatingLayout::highlightButton(QString key) {
   this->blockSignals(true);
    bool found = false;
    for (uint32_t i = 0; i < mButtons.size(); ++i) {
        if (mNames[i].compare(key) == 0) {
            found = true;
            buttonPressed(i);
        }
    }
    // in the case of no found, default to given values.
    if (!found) {
        for (uint32_t i = 0; i < mButtons.size(); ++i) {
            if (mNames[i].compare("RGB") == 0) {
                buttonPressed(i);
            }
        }
    }
    this->blockSignals(false);
}

//--------------------------------
// Setup Buttons
//--------------------------------

void FloatingLayout::setupButtons(std::vector<QString> buttons, EButtonSize eButtonSize) {
    // set up the geometry
    QScreen *screen = QApplication::screens().at(0);

    QSize size;
    if (eButtonSize == EButtonSize::eSmall) {
        size = QSize(screen->size().width() * 0.025f, screen->size().height() * 0.025f);
    } else if (eButtonSize == EButtonSize::eMedium) {
        size = QSize(screen->size().width() * 0.03f, screen->size().height() * 0.03f);
    } else if (eButtonSize == EButtonSize::eRectangle) {
#ifdef MOBILE_BUILD
        size = QSize(screen->size().width() * 0.06f, screen->size().height() * 0.02f);
#else
        size = QSize(screen->size().width() * 0.07f, screen->size().height() * 0.05f);
#endif
    }
#ifdef MOBILE_BUILD
    size = QSize(size.width() * 5, size.height() * 5);
#endif

    int fixedWidth;
    int fixedHeight;
    if (eButtonSize == EButtonSize::eRectangle) {
        if (mIsVertical) {
            fixedWidth = size.width();
            fixedHeight = size.height() * buttons.size();
        } else {
            fixedWidth = size.width() * buttons.size();
            fixedHeight = size.height();
        }
    } else {
        if (mIsVertical) {
            fixedWidth = size.width();
            fixedHeight = size.width() * buttons.size();
        } else {
            fixedWidth = size.width() * buttons.size();
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


    cor::Light light;
    light.routine = ERoutine::eSingleSolid;
    light.color = QColor(255, 0, 0);

    QSignalMapper *buttonsMapper = new QSignalMapper(this);
    for (uint32_t i = 0; i < mNames.size(); ++i) {
        bool foundMatch = false;
        if (mNames[i].compare("RGB") == 0) {
            foundMatch = true;
            mButtons[i] = new QPushButton(this);
            mButtons[i]->setCheckable(true);
            mButtons[i]->setMinimumSize(buttonSize());
        } else if (mNames[i].compare("Multi") == 0) {
            foundMatch = true;
            QJsonObject routineObject = lightToJson(light);
            cor::Button *lightsButton = new cor::Button(routineObject, std::vector<QColor>(), this);
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
            light.routine = ERoutine::eSingleGlimmer;
            light.color   = QColor(0, 255, 0);
            QJsonObject routineObject = lightToJson(light);
            cor::Button *lightsButton = new cor::Button(routineObject, std::vector<QColor>(), this);
            mButtons[i] = static_cast<QPushButton*>(lightsButton);
            Q_ASSERT(mButtons[i]);
        } else if (mNames[i].compare("Settings") == 0) {
            foundMatch = true;
            mButtons[i] = new QPushButton(this);
            mButtons[i]->setMinimumSize(buttonSize());
        } else if (mNames[i].compare("Connection_Page") == 0) {
            foundMatch = true;
            mButtons[i] = new QPushButton(this);
            mButtons[i]->setCheckable(true);
            mButtons[i]->setMinimumSize(buttonSize());
        } else if (mNames[i].compare("Colors_Page") == 0) {
            foundMatch = true;
            mButtons[i] = new QPushButton(this);
            mButtons[i]->setCheckable(true);
            mButtons[i]->setMinimumSize(buttonSize());
        } else if (mNames[i].compare("Multi_Colors_Page") == 0) {
            foundMatch = true;
            QJsonObject routineObject = lightToJson(light);
            cor::Button *lightsButton = new cor::Button(routineObject, std::vector<QColor>(), this);
            lightsButton->setCheckable(true);
            mButtons[i] = static_cast<QPushButton*>(lightsButton);
            Q_ASSERT(mButtons[i]);
        } else if (mNames[i].compare("Discovery_ArduCor") == 0) {
            foundMatch = true;
            mButtons[i] = new QPushButton(this);
            mButtons[i]->setCheckable(true);
            mButtons[i]->setStyleSheet("text-align:left");
            mButtons[i]->setIconSize(QSize(mButtons[i]->size().height() * 0.9f,
                                           mButtons[i]->size().height() * 0.9f));
            mButtons[i]->setText("ArduCor");
        } else if (mNames[i].compare("Discovery_Hue") == 0) {
            foundMatch = true;
            mButtons[i] = new QPushButton(this);
            mButtons[i]->setCheckable(true);
            mButtons[i]->setStyleSheet("text-align:left");
            mButtons[i]->setIconSize(QSize(mButtons[i]->size().height() * 0.9f,
                                           mButtons[i]->size().height() * 0.9f));
            mButtons[i]->setText("Hue");
        } else if (mNames[i].compare("Discovery_NanoLeaf") == 0) {
            foundMatch = true;
            mButtons[i] = new QPushButton(this);
            mButtons[i]->setCheckable(true);
            mButtons[i]->setStyleSheet("text-align:left");
            mButtons[i]->setIconSize(QSize(mButtons[i]->size().height() * 0.9f,
                                           mButtons[i]->size().height() * 0.9f));
            mButtons[i]->setText("NanoLeaf");
        } else if (mNames[i].compare("Discovery") == 0) {
            foundMatch = true;
            mButtons[i] = new QPushButton(this);
            mButtons[i]->setCheckable(false);
            mButtons[i]->setMinimumSize(buttonSize());
        } else if (mNames[i].compare("Rooms") == 0) {
            foundMatch = true;
            mButtons[i] = new QPushButton(this);
            mButtons[i]->setCheckable(true);
            mButtons[i]->setMinimumSize(buttonSize());
        } else if (mNames[i].compare("Groups") == 0) {
            foundMatch = true;
            mButtons[i] = new QPushButton(this);
            mButtons[i]->setCheckable(true);
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
        } else if (mNames[i].compare("Select_Moods") == 0) {
            foundMatch = true;
            light.color   = QColor(255, 255, 0);
            QJsonObject routineObject = lightToJson(light);
            cor::Button *lightsButton = new cor::Button(routineObject, std::vector<QColor>(), this);
            mButtons[i] = static_cast<QPushButton*>(lightsButton);
            Q_ASSERT(mButtons[i]);
        } else if (mNames[i].compare("New_Group") == 0) {
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
            if (isALightsButton(i)) {
                cor::Button *lightsButton = static_cast<cor::Button*>(mButtons[i]);
                connect(lightsButton, SIGNAL(clicked(bool)), buttonsMapper, SLOT(map()));
                buttonsMapper->setMapping(lightsButton, i);
            } else {
                connect(mButtons[i], SIGNAL(clicked(bool)), buttonsMapper, SLOT(map()));
                buttonsMapper->setMapping(mButtons[i], i);
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
                } else if (mNames[i].compare("ColorScheme") == 0) {
                    cor::resizeIcon(mButtons[i], ":/images/ColorSchemePicker_icon.png");
                } else if (mNames[i].compare("HueLightSearch") == 0) {
                    cor::resizeIcon(mButtons[i], ":/images/plusIcon.png");
                } else if (mNames[i].compare("Groups") == 0) {
                    cor::resizeIcon(mButtons[i], ":/images/groupsIcon.png");
                } else if (mNames[i].compare("Rooms") == 0) {
                    cor::resizeIcon(mButtons[i], ":/images/roomIcon.png");
                } else if (mNames[i].compare("Connection_Page") == 0) {
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

void FloatingLayout::updateRoutine(const QJsonObject& routineObject, std::vector<QColor> colors) {
    for (uint32_t i = 0; i < mButtons.size(); ++i) {
        if (mNames[i].compare("Routine") == 0) {
            cor::Button *lightsButton = static_cast<cor::Button*>(mButtons[i]);
            Q_ASSERT(lightsButton);
            lightsButton->updateRoutine(routineObject, colors);
        }
    }
}

void FloatingLayout::updateGroupPageButtons(const std::vector<std::vector<QColor> >& colors) {
    for (uint32_t i = 0; i < mButtons.size(); ++i) {
        if (mNames[i].compare("Select_Moods") == 0) {
            cor::Button *lightsButton = static_cast<cor::Button*>(mButtons[i]);
            Q_ASSERT(lightsButton);
            cor::Light light;
            light.routine = ERoutine::eMultiFade;
            light.palette = EPalette::eSevenColor;
            light.speed   = 100;
            QJsonObject routineObject = lightToJson(light);
            lightsButton->updateRoutine(routineObject, colors[(int)EPalette::eSevenColor]);
        }
    }
}


void FloatingLayout::updateMultiPageButton(const std::vector<std::vector<QColor> >& colors) {
    for (uint32_t i = 0; i < mButtons.size(); ++i) {
        if (mNames[i].compare("Multi") == 0) {
            cor::Button *lightsButton = static_cast<cor::Button*>(mButtons[i]);
            Q_ASSERT(lightsButton);
            cor::Light light;
            light.routine = ERoutine::eMultiFade;
            light.palette = EPalette::eCool;
            light.speed   = 100;
            QJsonObject routineObject = lightToJson(light);
            lightsButton->updateRoutine(routineObject, colors[(int)EPalette::eCool]);
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


void FloatingLayout::addMultiRoutineIcon(std::vector<QColor> colors) {
    for (uint32_t i = 0; i < mButtons.size(); ++i) {
        if (mNames[i].compare("Multi_Colors_Page") == 0) {
            cor::Button *lightsButton = static_cast<cor::Button*>(mButtons[i]);
            Q_ASSERT(lightsButton);
            cor::Light light;
            light.routine = ERoutine::eMultiBars;
            light.palette = EPalette::eWater;
            light.speed   = 100;
            QJsonObject routineObject = lightToJson(light);
            lightsButton->updateRoutine(routineObject, colors);
        }
    }
}

void FloatingLayout::updateDiscoveryButton(EProtocolType type, QPixmap pixmap) {
    QString label;
    switch (type)
    {
        case EProtocolType::eHue:
            label = "Discovery_Hue";
            break;
        case EProtocolType::eArduCor:
            label = "Discovery_ArduCor";
            break;
        case EProtocolType::eNanoleaf:
            label = "Discovery_NanoLeaf";
            break;
        default:
            break;
    }
    for (uint32_t i = 0; i < mButtons.size(); ++i) {
        if (mNames[i].compare(label) == 0) {
            int size = mButtons[i]->size().width() * 0.25f;
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
    QString label = mNames[buttonIndex];
    if (isALightsButton(buttonIndex)) {
        cor::Button *lightsButton = static_cast<cor::Button*>(mButtons[buttonIndex]);
        Q_ASSERT(lightsButton);
        // handle edge case of routine button, its the only button that toggles independently...
        if (mNames[buttonIndex].compare("Routine") == 0) {
            highlightRoutineButton(!mRoutineIsHighlighted);
        } else {
            lightsButton->setChecked(true);
        }
    } else {
        mButtons[buttonIndex]->setChecked(true);
    }


    // emit label of checked box
    emit buttonPressed(label);
}

//--------------------------------
// Helpers
//--------------------------------

void FloatingLayout::highlightRoutineButton(bool shouldHighlight) {
    for (uint32_t i = 0; i < mButtons.size(); ++i) {
        if (mNames[i].compare("Routine") == 0) {
            cor::Button *lightsButton = static_cast<cor::Button*>(mButtons[i]);
            Q_ASSERT(lightsButton);
            if (shouldHighlight) {
                lightsButton->setChecked(true);
            } else {
                lightsButton->setChecked(false);
            }
            mRoutineIsHighlighted = shouldHighlight;
        }
    }
}

bool FloatingLayout::isALightsButton(int index) {
    if (mNames[index].compare("Multi") == 0
            || mNames[index].compare("Routine") == 0
            || mNames[index].compare("Select_Moods") == 0
            || mNames[index].compare("Multi_Colors_Page") == 0) {
        return true;
    }
    return false;
}

QSize FloatingLayout::buttonSize() {
    return QSize(this->width() / mButtons.size(), this->width() / mButtons.size());
}
