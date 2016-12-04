/*!
 * \copyright
 * Copyright (C) 2015 - 2016.
 * Released under the GNU General Public License.
 */

#include "lightsbutton.h"

#include <QDebug>
#include <QGraphicsOpacityEffect>

LightsButton::LightsButton(QWidget *parent) : QWidget(parent) {
    mLightingRoutine = ELightingRoutine::eLightingRoutine_MAX;
    mColorGroup = EColorGroup::eColorGroup_MAX;
    mSetupHasBeenCalled = false;
    mIsMenuButton = false;

    button = new QPushButton(this);
    button->setCheckable(true);
    connect(button, SIGNAL(clicked(bool)), this, SLOT(handleButton()));
}


void LightsButton::setupAsMenuButton(int pageNumber, DataLayer *dataLayer) {
    mIconData = IconData(256, 256, dataLayer);
    if (pageNumber == 0) {
        mIconData.setSolidColor(QColor(0,255,0));
    } else if (pageNumber == 1) {
        mIconData.setMultiFade(EColorGroup::eCustom, true);
    } else {
        mIconData.setMultiFade(EColorGroup::eSevenColor);
    }
    button->setIcon(mIconData.renderAsQPixmap());
    mDataLayer = dataLayer;
    mSetupHasBeenCalled = true;
    mIsMenuButton = true;
    mPageNumber = pageNumber;
    mLayout = new QVBoxLayout;
    mLayout->setSpacing(0);
    mLayout->setContentsMargins(0,0,0,0);
    mLayout->addWidget(button);
    button->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    setLayout(mLayout);
}

void LightsButton::setupAsStandardButton(ELightingRoutine routine, EColorGroup colorGroup, DataLayer *dataLayer, QString label) {
    mIconData = IconData(256, 256, dataLayer);
    mSetupHasBeenCalled = true;
    bool renderIcon = false;
    switch(routine) {
        case ELightingRoutine::eMultiGlimmer:
        case ELightingRoutine::eMultiFade:
        case ELightingRoutine::eMultiRandomSolid:
        case ELightingRoutine::eMultiRandomIndividual:
        case ELightingRoutine::eMultiBarsSolid:
        case ELightingRoutine::eMultiBarsMoving:
            renderIcon = true;
            break;
        default:
            break;
    }

    if (renderIcon) {
        mIconData.setLightingRoutine(routine, colorGroup);
        button->setIcon(mIconData.renderAsQPixmap());
    }


    mLightingRoutine = routine;
    mColorGroup = colorGroup;
    mDataLayer = dataLayer;

    mLayout = new QVBoxLayout;
    mLayout->setSpacing(2);
    mLayout->setContentsMargins(0,0,0,0);
    mLayout->addWidget(button);
    mLabel = label;
    if (mLabel.compare(QString("")) != 0) {
        buttonLabel = new QLabel;
        buttonLabel->setText(label);
        buttonLabel->setFont(QFont(buttonLabel->font().styleName(), 10, 0));

        buttonLabel->setAlignment(Qt::AlignCenter);
        mLayout->addWidget(buttonLabel);
        button->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Minimum);
    }
    button->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    mLayout->setStretch(0, 50);
    mLayout->setStretch(1, 1);
    setLayout(mLayout);
}

ELightingRoutine LightsButton::lightingRoutine() {
    return mLightingRoutine;
}

EColorGroup LightsButton::colorGroup() {
    return mColorGroup;
}

void LightsButton::enable(bool shouldEnable) {
    if(shouldEnable) {
        QGraphicsOpacityEffect *effect = new QGraphicsOpacityEffect(button);
        effect->setOpacity(1.0);
        button->setGraphicsEffect(effect);
        this->setEnabled(true);
    } else {
        QGraphicsOpacityEffect *effect = new QGraphicsOpacityEffect(button);
        effect->setOpacity(0.1);
        button->setGraphicsEffect(effect);
        this->setEnabled(false);
    }
}


void LightsButton::resizeEvent(QResizeEvent *event) {
    Q_UNUSED(event);
    int size = (int)(std::min(this->size().width(), this->size().height()));
    if (mIsMenuButton) {
        button->setMinimumSize(size,size);        
        button->setIconSize(QSize(this->size().height() * 0.8f,
                                  this->size().height() * 0.8f));
    } else {
        if (mLabel.compare(QString("")) != 0) {
            buttonLabel->setMinimumSize(0, buttonLabel->size().height());
            button->setIconSize(QSize(size * 0.8f,
                                      (size - buttonLabel->size().height()) * 0.8f));
        } else {
            button->setMinimumSize(size, size);
            button->setIconSize(QSize(size * 0.8f,
                                      size * 0.8f));
        }
    }
}

void LightsButton::updateIcon() {
    if (!mIsMenuButton) {
        mIconData.setLightingRoutine(mLightingRoutine, mColorGroup);
        button->setIcon(mIconData.renderAsQPixmap());
    } else {
        if (mPageNumber == 0) {
            mIconData.setSolidColor(mDataLayer->mainColor());
        } else if (mPageNumber == 1) {
            mIconData.setMultiFade(EColorGroup::eCustom, true);
        } else {
            mIconData.setMultiFade(mDataLayer->currentColorGroup());
        }
        button->setIcon(mIconData.renderAsQPixmap());
    }
}

void LightsButton::updateIconSingleColorRoutine(ELightingRoutine lightingRoutine, QColor color) {
    mIconData.setSingleLightingRoutine(lightingRoutine, color);
    button->setIcon(mIconData.renderAsQPixmap());
}

void LightsButton::updateIconPresetColorRoutine(ELightingRoutine lightingRoutine, EColorGroup colorGroup) {
    mIconData.setLightingRoutine((ELightingRoutine)lightingRoutine, (EColorGroup)colorGroup);
    button->setIcon(mIconData.renderAsQPixmap());
}


void LightsButton::handleButton() {
    if (mSetupHasBeenCalled) {
        if (mIsMenuButton) {
            emit menuButtonClicked(mPageNumber);
        } else {
            emit buttonClicked((int)mLightingRoutine,(int)mColorGroup);
        }
    }
}
