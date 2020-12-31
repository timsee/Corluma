/*!
 * \copyright
 * Copyright (C) 2015 - 2020.
 * Released under the GNU General Public License.
 */

#include "palettewidget.h"

#include <QPainter>
#include <QStyleOption>
#include "cor/presetpalettes.h"
#include "utils/qt.h"

PaletteWidget::PaletteWidget(const QString& name, EPalette palette, QWidget* parent)
    : QWidget(parent),
      mLightVector{new cor::LightVectorWidget(3, 3, true, this)},
      mLabel{new QLabel(name, this)},
      mPalette{palette} {
    mLabel->setWordWrap(true);
    mLabel->setStyleSheet("background-color:rgba(0,0,0,0);");

    PresetPalettes palettes;
    cor::LightState state;
    state.isOn(true);
    state.palette(palettes.palette(palette));
    state.speed(100);

    auto paletteColors = palettes.palette(palette).colors();
    std::vector<cor::Light> lights;
    for (auto color : paletteColors) {
        cor::Light light;
        cor::LightState state;
        state.isOn(true);
        state.routine(ERoutine::singleSolid);
        state.color(color);
        light.state(state);
        lights.push_back(light);
    }
    state.routine(ERoutine::multiBars);

    mLightVector->enableButtonInteraction(false);
    mLightVector->updateLights(lights);
}

void PaletteWidget::setChecked(EPalette palette) {
    mIsChecked = palette == mPalette;
    update();
}

void PaletteWidget::resize() {
    auto yPos = 0u;
    auto rowHeight = height() / 5;

    mLabel->setGeometry(0, yPos, width(), rowHeight);
    yPos += mLabel->height();

    mLightVector->setGeometry(0, yPos, width(), rowHeight * 4);
    yPos += mLightVector->height();
}


void PaletteWidget::paintEvent(QPaintEvent*) {
    QStyleOption opt;
    opt.initFrom(this);
    QPainter painter(this);

    // handle highlight
    if (mIsChecked) {
        painter.fillRect(rect(), QBrush(QColor(61, 142, 201)));
    } else {
        painter.fillRect(rect(), QBrush(QColor(32, 31, 31)));
    }
}
