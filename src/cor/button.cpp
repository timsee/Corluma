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
        mIconData.setMultiFade(EPalette::eCustom, group, true);
    } else {
        mIconData.setMultiFade(EPalette::eSevenColor, group);
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

void Button::setupAsStandardButton(QJsonObject routineObject, QString label, const std::vector<QColor>& group) {
    ERoutine routine = stringToRoutine(routineObject["routine"].toString());
    EPalette palette = EPalette::ePalette_MAX;
    if (routineObject["palette"].isString()) {
        palette = stringToPalette(routineObject["palette"].toString());
    }
    mKey = label;
    mIconData = IconData(256, 256);
    mSetupHasBeenCalled = true;
    bool renderIcon = false;
    switch(routine) {
        case ERoutine::eMultiGlimmer:
        case ERoutine::eMultiFade:
        case ERoutine::eMultiRandomSolid:
        case ERoutine::eMultiRandomIndividual:
        case ERoutine::eMultiBars:
            renderIcon = true;
            break;
        default:
            break;
    }

    if (renderIcon) {
        cor::Light light;
        light.routine = routine;
        light.palette = palette;
        light.speed = 100;
        QJsonObject routineObject = cor::lightToJson(light);
        mIconData.setRoutine(routineObject, group);
        button->setIcon(mIconData.renderAsQPixmap());
    }

    mRoutineObject = routineObject;

    mLayout = new QVBoxLayout;
    mLayout->setSpacing(0);
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
        button->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    }

    button->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    mLayout->setStretch(0, 5);
    mLayout->setStretch(1, 1);
    setLayout(mLayout);
    resizeIcon();
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


void Button::updateRoutine(const QJsonObject& routineObject, const std::vector<QColor>& colors) {
    mIconData.setRoutine(routineObject, colors);
    button->setIcon(mIconData.renderAsQPixmap());
    resizeIcon();
}


void Button::handleButton() {
    if (mSetupHasBeenCalled) {
        if (mIsMenuButton) {
            emit menuButtonClicked(mPageNumber);
        } else {
            emit buttonClicked(mRoutineObject);
        }
    }
}

void Button::hideContent() {
    button->setVisible(false);
}

QString Button::label() {
    return mKey;
}

}
