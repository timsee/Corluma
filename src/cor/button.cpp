/*!
 * \copyright
 * Copyright (C) 2015 - 2018.
 * Released under the GNU General Public License.
 */

#include "cor/button.h"

#include <QDebug>
#include <QGraphicsOpacityEffect>

namespace cor
{

Button::Button(QWidget *parent) : QPushButton(parent) {
    mLightingRoutine = ELightingRoutine::eLightingRoutine_MAX;
    mColorGroup = EColorGroup::eColorGroup_MAX;
    mSetupHasBeenCalled = false;
    mIsMenuButton = false;

    button = new QPushButton(this);
    button->setCheckable(true);
    button->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    connect(button, SIGNAL(clicked(bool)), this, SLOT(handleButton()));
}


void Button::setupAsMenuButton(int pageNumber, const std::vector<QColor>& group) {
    mIconData = IconData(32, 32);
    if (pageNumber == 0) {
        mIconData.setSolidColor(QColor(0,255,0));
    } else if (pageNumber == 1) {
        mIconData.setMultiFade(EColorGroup::eCustom, group, true);
    } else {
        mIconData.setMultiFade(EColorGroup::eSevenColor, group);
    }
    button->setIcon(mIconData.renderAsQPixmap());
    mSetupHasBeenCalled = true;
    mIsMenuButton = true;
    mPageNumber = pageNumber;
    mLayout = new QVBoxLayout;
    mLayout->setSpacing(0);
    mLayout->setContentsMargins(0,0,0,0);
    mLayout->addWidget(button);
    setLayout(mLayout);
    resizeIcon();
}

void Button::setupAsStandardButton(ELightingRoutine routine, EColorGroup colorGroup, QString label, const std::vector<QColor>& group) {
    mIconData = IconData(256, 256);
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
        mIconData.setMultiLightingRoutine(routine, colorGroup, group);
        button->setIcon(mIconData.renderAsQPixmap());
    }

    mLightingRoutine = routine;
    mColorGroup = colorGroup;

    mLayout = new QVBoxLayout;
    mLayout->setSpacing(2);
    mLayout->setContentsMargins(0,0,0,0);
    mLayout->addWidget(button);
    mLabel = label;
    if (mLabel.compare(QString("")) != 0) {
        buttonLabel = new QLabel(this);
        if (label.size() > 12) {
            label = label.mid(0, 9);
            label += "...";
        }
        buttonLabel->setText(label);
        buttonLabel->setStyleSheet("font: 8pt;");
        buttonLabel->setAlignment(Qt::AlignCenter);
        mLayout->addWidget(buttonLabel);
        button->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Minimum);
    }
    button->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    mLayout->setStretch(0, 5);
    mLayout->setStretch(1, 1);
    setLayout(mLayout);
    resizeIcon();
}

ELightingRoutine Button::lightingRoutine() {
    return mLightingRoutine;
}

EColorGroup Button::colorGroup() {
    return mColorGroup;
}

void Button::enable(bool shouldEnable) {
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


void Button::resizeEvent(QResizeEvent *event) {
    Q_UNUSED(event);
    resizeIcon();
}

void Button::resizeIcon() {
    int size;
    if (mIsMenuButton) {
        size = (int)(std::min(this->size().width(), this->size().height()));
    } else {
        size = (int)(std::min(this->size().width(), (int)(this->size().height() / 1.33f)));
    }
    button->setMinimumSize(size, size);
    QSize newSize = QSize(size * 0.8f,
                         size * 0.8f);
    QPixmap pixmap = mIconData.renderAsQPixmap();
    pixmap = pixmap.scaled(this->size().width() * 0.8f,
                           this->size().height() * 0.8f,
                           Qt::KeepAspectRatio,
                           Qt::FastTransformation);
    button->setIcon(QIcon(pixmap));
    button->setIconSize(newSize);
}

void Button::updateIconSingleColorRoutine(ELightingRoutine lightingRoutine, QColor color) {
    mIconData.setSingleLightingRoutine(lightingRoutine, color);
    button->setIcon(mIconData.renderAsQPixmap());
    resizeIcon();
}

void Button::updateIconPresetColorRoutine(ELightingRoutine lightingRoutine, EColorGroup colorGroup, const std::vector<QColor>& colors, int colorMax) {
    mIconData.setMultiLightingRoutine(lightingRoutine, colorGroup, colors, colorMax);
    button->setIcon(mIconData.renderAsQPixmap());
    resizeIcon();
}


void Button::handleButton() {
    if (mSetupHasBeenCalled) {
        if (mIsMenuButton) {
            emit menuButtonClicked(mPageNumber);
        } else {
            emit buttonClicked((int)mLightingRoutine,(int)mColorGroup);
        }
    }
}

void Button::hideContent() {
    button->setVisible(false);
}

}
