/*!
 * \copyright
 * Copyright (C) 2015 - 2017.
 * Released under the GNU General Public License.
 */

#include "floatinglayout.h"
#include "mainwindow.h"
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

void FloatingLayout::setupButtons(std::vector<QString> buttons) {
    // set up the geometry
    QScreen *screen = QApplication::screens().at(0);
#ifdef MOBILE_BUILD
    QSize size = QSize(screen->availableSize().width() * 0.12f, screen->availableSize().height() * 0.12f);
#else
    QSize size = QSize(screen->size().width() * 0.025f, screen->size().height() * 0.025f);
#endif

    if (mIsVertical) {
        this->setMinimumWidth(size.width());
        this->setMaximumWidth(size.width());

        this->setMinimumHeight(size.width() * buttons.size());
        this->setMaximumHeight(size.width() * buttons.size());
    } else {
        this->setMinimumWidth(size.width() * buttons.size());
        this->setMaximumWidth(size.width() * buttons.size());

        this->setMinimumHeight(size.width());
        this->setMaximumHeight(size.width());

    }

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
            LightsButton *lightsButton = new LightsButton(this);
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
            LightsButton *lightsButton = new LightsButton(this);
            lightsButton->button->setCheckable(true);
            lightsButton->setupAsStandardButton(ELightingRoutine::eSingleGlimmer, EColorGroup::eAll);
            lightsButton->updateIconSingleColorRoutine(ELightingRoutine::eSingleGlimmer, QColor(0,255,0));
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
                LightsButton *lightsButton = static_cast<LightsButton*>(mButtons[i]);
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
            LightsButton *lightsButton = static_cast<LightsButton*>(mButtons[i]);
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
            LightsButton *lightsButton = static_cast<LightsButton*>(mButtons[i]);
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


void FloatingLayout::addMultiRoutineIcon(std::vector<QColor> colors) {
    for (uint32_t i = 0; i < mButtons.size(); ++i) {
        if (mNames[i].compare("Multi") == 0) {
            LightsButton *lightsButton = static_cast<LightsButton*>(mButtons[i]);
            Q_ASSERT(lightsButton);
            lightsButton->updateIconPresetColorRoutine(ELightingRoutine::eMultiBarsMoving, EColorGroup::eSevenColor, colors);
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
            LightsButton *lightsButton = static_cast<LightsButton*>(mButtons[i]);
            Q_ASSERT(lightsButton);
            lightsButton->button->setChecked(false);
        }
    }

    // check the proper button
    QString label = mNames[buttonIndex];
    if (isALightsButton(buttonIndex)) {
        LightsButton *lightsButton = static_cast<LightsButton*>(mButtons[buttonIndex]);
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
            LightsButton *lightsButton = static_cast<LightsButton*>(mButtons[i]);
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
            || mNames[index].compare("Routine") == 0) {
        return true;
    }
    return false;
}

QSize FloatingLayout::buttonSize() {
    return QSize(this->width() / mButtons.size(), this->width() / mButtons.size());
}
