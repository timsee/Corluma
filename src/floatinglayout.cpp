/*!
 * \copyright
 * Copyright (C) 2015 - 2017.
 * Released under the GNU General Public License.
 */

#include "floatinglayout.h"
#include "mainwindow.h"
#include "corlumautils.h"

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
        size = QSize(screen->size().width() * 0.07f, screen->size().height() * 0.02f);
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


    QSignalMapper *buttonsMapper = new QSignalMapper(this);
    for (uint32_t i = 0; i < mNames.size(); ++i) {
        bool foundMatch = false;
        if (mNames[i].compare("RGB") == 0) {
            foundMatch = true;
            mButtons[i] = new QPushButton(this);
            mButtons[i]->setCheckable(true);
            resizeIcon(mButtons[i], ":/images/colorWheel_icon.png");
        } else if (mNames[i].compare("Multi") == 0) {
            foundMatch = true;
            CorlumaButton *lightsButton = new CorlumaButton(this);
            lightsButton->button->setCheckable(true);
            lightsButton->setupAsStandardButton(ELightingRoutine::eSingleSolid, EColorGroup::eAll);
            lightsButton->updateIconSingleColorRoutine(ELightingRoutine::eSingleSolid, QColor(255,0,0));
            mButtons[i] = static_cast<QPushButton*>(lightsButton);
            Q_ASSERT(mButtons[i]);
        } else if (mNames[i].compare("Temperature") == 0) {
            foundMatch = true;
            mButtons[i] = new QPushButton(this);
            mButtons[i]->setCheckable(true);
            resizeIcon(mButtons[i], ":/images/hueRange_icon.png");
        } else if (mNames[i].compare("Routine") == 0) {
            foundMatch = true;
            CorlumaButton *lightsButton = new CorlumaButton(this);
            lightsButton->button->setCheckable(true);
            lightsButton->setupAsStandardButton(ELightingRoutine::eSingleGlimmer, EColorGroup::eAll);
            lightsButton->updateIconSingleColorRoutine(ELightingRoutine::eSingleGlimmer, QColor(0,255,0));
            mButtons[i] = static_cast<QPushButton*>(lightsButton);
            Q_ASSERT(mButtons[i]);
        } else if (mNames[i].compare("Settings") == 0) {
            foundMatch = true;
            mButtons[i] = new QPushButton(this);
            resizeIcon(mButtons[i], ":/images/settingsgear.png");
        } else if (mNames[i].compare("Discovery_Yun") == 0) {
            foundMatch = true;
            mButtons[i] = new QPushButton(this);
            mButtons[i]->setCheckable(true);
            mButtons[i]->setStyleSheet("text-align:left");
            mButtons[i]->setIconSize(QSize(mButtons[i]->size().height() * 0.9f,
                                           mButtons[i]->size().height() * 0.9f));
            mButtons[i]->setText("Yun");
        } else if (mNames[i].compare("Discovery_Serial") == 0) {
            foundMatch = true;
            mButtons[i] = new QPushButton(this);
            mButtons[i]->setCheckable(true);
            mButtons[i]->setStyleSheet("text-align:left");
            mButtons[i]->setIconSize(QSize(mButtons[i]->size().height() * 0.9f,
                                           mButtons[i]->size().height() * 0.9f));
            mButtons[i]->setText("Serial");
        } else if (mNames[i].compare("Discovery_Hue") == 0) {
            foundMatch = true;
            mButtons[i] = new QPushButton(this);
            mButtons[i]->setCheckable(true);
            mButtons[i]->setStyleSheet("text-align:left");
            mButtons[i]->setIconSize(QSize(mButtons[i]->size().height() * 0.9f,
                                           mButtons[i]->size().height() * 0.9f));
            mButtons[i]->setText("Hue");
        } else if (mNames[i].compare("Discovery") == 0) {
            foundMatch = true;
            mButtons[i] = new QPushButton(this);
            mButtons[i]->setCheckable(false);
            resizeIcon(mButtons[i], ":/images/wifi.png");
        } else if (mNames[i].compare("Select_Devices") == 0) {
            foundMatch = true;
            mButtons[i] = new QPushButton(this);
            mButtons[i]->setCheckable(true);
            resizeIcon(mButtons[i], ":/images/colorWheel_icon.png");
        } else if (mNames[i].compare("Select_Moods") == 0) {
            foundMatch = true;
            CorlumaButton *lightsButton = new CorlumaButton(this);
            lightsButton->button->setCheckable(true);
            lightsButton->setupAsStandardButton(ELightingRoutine::eSingleSolid, EColorGroup::eAll);
            lightsButton->updateIconSingleColorRoutine(ELightingRoutine::eSingleSolid, QColor(255,255,0));
            mButtons[i] = static_cast<QPushButton*>(lightsButton);
            Q_ASSERT(mButtons[i]);
        } else if (mNames[i].compare("New_Collection") == 0) {
            foundMatch = true;
            mButtons[i] = new QPushButton(this);
            mButtons[i]->setCheckable(false);
            resizeIcon(mButtons[i], ":/images/editIcon.png");
        } else if (mNames[i].compare("New_Mood") == 0) {
            foundMatch = true;
            mButtons[i] = new QPushButton(this);
            mButtons[i]->setCheckable(false);
            resizeIcon(mButtons[i], ":/images/editIcon.png");
        } else if (mNames[i].compare("Preset_Groups") == 0) {
            foundMatch = true;
            CorlumaButton *lightsButton = new CorlumaButton(this);
            lightsButton->button->setCheckable(true);
            lightsButton->setupAsStandardButton(ELightingRoutine::eSingleSolid, EColorGroup::eAll);
            lightsButton->updateIconSingleColorRoutine(ELightingRoutine::eSingleSolid, QColor(255,0,0));
            mButtons[i] = static_cast<QPushButton*>(lightsButton);
            Q_ASSERT(mButtons[i]);
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
            bool isLightsButton = isALightsButton(i);
            if (!isLightsButton) {
                connect(mButtons[i], SIGNAL(clicked(bool)), buttonsMapper, SLOT(map()));
                buttonsMapper->setMapping(mButtons[i], i);
            } else {
                CorlumaButton *lightsButton = static_cast<CorlumaButton*>(mButtons[i]);
                connect(lightsButton->button, SIGNAL(clicked(bool)), buttonsMapper, SLOT(map()));
                buttonsMapper->setMapping(lightsButton->button, i);
            }
        }
    }
    connect(buttonsMapper, SIGNAL(mapped(int)), this, SLOT(buttonPressed(int)));
}


//--------------------------------
// Update Icons
//--------------------------------

void FloatingLayout::updateRoutineSingleColor(ELightingRoutine routine, QColor color) {
    for (uint32_t i = 0; i < mButtons.size(); ++i) {
        if (mNames[i].compare("Routine") == 0) {
            CorlumaButton *lightsButton = static_cast<CorlumaButton*>(mButtons[i]);
            Q_ASSERT(lightsButton);
            if ((int)routine <= (int)utils::ELightingRoutineSingleColorEnd) {
                lightsButton->updateIconSingleColorRoutine(routine, color);
                if (mRoutineIsTranslucent) {
                    mRoutineIsTranslucent = false;
                    QGraphicsOpacityEffect *effect = new QGraphicsOpacityEffect(lightsButton);
                    effect->setOpacity(1.0f);
                    lightsButton->setGraphicsEffect(effect);
                }
            } else {
                if (!mRoutineIsTranslucent) {
                    mRoutineIsTranslucent = true;
                    QGraphicsOpacityEffect *effect = new QGraphicsOpacityEffect(lightsButton);
                    effect->setOpacity(0.333f);
                    lightsButton->setGraphicsEffect(effect);
                }
            }
        }
    }
}

void FloatingLayout::updateRoutineMultiColor(ELightingRoutine routine, std::vector<QColor> colors, int colorCount) {
    for (uint32_t i = 0; i < mButtons.size(); ++i) {
        if (mNames[i].compare("Routine") == 0) {
            CorlumaButton *lightsButton = static_cast<CorlumaButton*>(mButtons[i]);
            Q_ASSERT(lightsButton);
            if ((int)routine > (int)utils::ELightingRoutineSingleColorEnd) {
                lightsButton->updateIconPresetColorRoutine(routine, EColorGroup::eCustom, colors, colorCount);
                if (mRoutineIsTranslucent) {
                    mRoutineIsTranslucent = false;
                    QGraphicsOpacityEffect *effect = new QGraphicsOpacityEffect(lightsButton);
                    effect->setOpacity(1.0f);
                    lightsButton->setGraphicsEffect(effect);
                }
            } else {
                if (!mRoutineIsTranslucent) {
                    mRoutineIsTranslucent = true;
                    QGraphicsOpacityEffect *effect = new QGraphicsOpacityEffect(lightsButton);
                    effect->setOpacity(0.333f);
                    lightsButton->setGraphicsEffect(effect);
                }
            }
        }
    }
}

void FloatingLayout::updateGroupPageButtons(const std::vector<std::vector<QColor> >& colors, int colorCount) {
    for (uint32_t i = 0; i < mButtons.size(); ++i) {
        if (mNames[i].compare("Preset_Groups") == 0) {
            CorlumaButton *lightsButton = static_cast<CorlumaButton*>(mButtons[i]);
            lightsButton->updateIconPresetColorRoutine(ELightingRoutine::eMultiRandomIndividual,
                                                       EColorGroup::eWater,
                                                       colors[(int)EColorGroup::eWater]);
            Q_ASSERT(lightsButton);
        } else if (mNames[i].compare("Select_Moods") == 0) {
            CorlumaButton *lightsButton = static_cast<CorlumaButton*>(mButtons[i]);
            Q_ASSERT(lightsButton);
            lightsButton->updateIconPresetColorRoutine(ELightingRoutine::eMultiFade,
                                                       EColorGroup::eSevenColor,
                                                       colors[(int)EColorGroup::eSevenColor]);
        }
    }
}

void FloatingLayout::addMultiRoutineIcon(std::vector<QColor> colors) {
    for (uint32_t i = 0; i < mButtons.size(); ++i) {
        if (mNames[i].compare("Multi") == 0) {
            CorlumaButton *lightsButton = static_cast<CorlumaButton*>(mButtons[i]);
            Q_ASSERT(lightsButton);
            lightsButton->updateIconPresetColorRoutine(ELightingRoutine::eMultiBarsMoving, EColorGroup::eSevenColor, colors);
        }
    }
}
void FloatingLayout::updateDiscoveryButton(ECommType type, QPixmap pixmap) {
    QString label;
    switch (type)
    {
        case ECommType::eHue:
            label = "Discovery_Hue";
            break;
#ifndef MOBILE_BUILD
        case ECommType::eSerial:
            label = "Discovery_Serial";
            break;
#endif
        case ECommType::eUDP:
            label = "Discovery_Yun";
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

//--------------------------------
// Resizing and Moving
//--------------------------------


void FloatingLayout::resizeIcon(QPushButton *button, QString iconPath) {
    QPixmap pixmap(iconPath);
    int size = buttonSize().width() * 0.66f;
    button->setIcon(QIcon(pixmap.scaled(size,
                                        size,
                                        Qt::IgnoreAspectRatio,
                                        Qt::SmoothTransformation)));
    button->setIconSize(QSize(size, size));
}

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
            CorlumaButton *lightsButton = static_cast<CorlumaButton*>(mButtons[i]);
            Q_ASSERT(lightsButton);
            lightsButton->button->setChecked(false);
        }
    }

    // check the proper button
    QString label = mNames[buttonIndex];
    if (isALightsButton(buttonIndex)) {
        CorlumaButton *lightsButton = static_cast<CorlumaButton*>(mButtons[buttonIndex]);
        Q_ASSERT(lightsButton);
        // handle edge case of routine button, its the only button that toggles independently...
        if (mNames[buttonIndex].compare("Routine") == 0) {
            highlightRoutineButton(!mRoutineIsHighlighted);
        } else {
            lightsButton->button->setChecked(true);
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
            CorlumaButton *lightsButton = static_cast<CorlumaButton*>(mButtons[i]);
            Q_ASSERT(lightsButton);
            if (shouldHighlight) {
                lightsButton->button->setChecked(true);
            } else {
                lightsButton->button->setChecked(false);
            }
            mRoutineIsHighlighted = shouldHighlight;
        }
    }
}

bool FloatingLayout::isALightsButton(int index) {
    if (mNames[index].compare("Multi") == 0
            || mNames[index].compare("Routine") == 0
            || mNames[index].compare("Preset_Groups") == 0
            || mNames[index].compare("Select_Moods") == 0) {
        return true;
    }
    return false;
}

QSize FloatingLayout::buttonSize() {
    return QSize(this->width() / mButtons.size(), this->width() / mButtons.size());
}
