/*!
 * \copyright
 * Copyright (C) 2015 - 2019.
 * Released under the GNU General Public License.
 */

#include <QDebug>
#include <QGraphicsOpacityEffect>

#include "colorschemechooser.h"
#include "utils/exception.h"

ColorSchemeChooser::ColorSchemeChooser(QWidget* parent) : QWidget(parent) {
    std::vector<std::pair<EColorSchemeType, QString>> nameResourcePairs
        = {{EColorSchemeType::custom, ":images/schemes/custom.png"},
           {EColorSchemeType::similar, ":images/schemes/similar.png"},
           {EColorSchemeType::complement, ":images/schemes/complement.png"},
           {EColorSchemeType::triad, ":images/schemes/triad.png"},
           {EColorSchemeType::compound, ":images/schemes/compound.png"}};

    mLayout = new QHBoxLayout;
    mLayout->setContentsMargins(0, 0, 0, 0);
    mLayout->setSpacing(0);
    for (const auto& scheme : nameResourcePairs) {
        auto button = new ColorSchemeButton(scheme.first, scheme.second, this);
        connect(button,
                SIGNAL(clicked(EColorSchemeType, bool)),
                this,
                SLOT(schemeClicked(EColorSchemeType, bool)));
        button->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
        mButtons.push_back(button);
        mLayout->addWidget(button);
        if (button->key() == EColorSchemeType::similar) {
            mCurrentKey = button->key();
            button->select(true);
        }
    }
    this->setLayout(mLayout);
}

void ColorSchemeChooser::schemeClicked(EColorSchemeType key, bool) {
    for (const auto& button : mButtons) {
        if (key == button->key()) {
            button->select(true);
        }
    }
    mCurrentKey = key;
    emit schemeChanged(key);
    for (const auto& button : mButtons) {
        if (key != button->key()) {
            button->select(false);
        }
    }
}

void ColorSchemeChooser::enableButton(EColorSchemeType key, bool enable) {
    for (auto button : mButtons) {
        if (button->key() == key) {
            button->enable(enable);
        }
    }
}

void ColorSchemeChooser::resizeEvent(QResizeEvent*) {
    for (auto button : mButtons) {
        button->resize();
    }
}

void ColorSchemeChooser::adjustSelection() {
    for (const auto& button : mButtons) {
        if (mCurrentKey == button->key() && !button->isEnabled()) {
            schemeClicked(EColorSchemeType::custom, true);
        }
    }
}
