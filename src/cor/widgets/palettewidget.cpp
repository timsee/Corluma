#include "palettewidget.h"
#include <QPainter>
#include <QStyleOption>
#include "icondata.h"

namespace cor {

PaletteWidget::PaletteWidget(QWidget* parent)
    : QWidget(parent),
      mSkipOffLightStates{false},
      mIsSingleLine{false} {}

void PaletteWidget::show(const std::vector<QColor>& colors) {
    mIsSolidColors = true;
    mSolidColors = colors;
}

void PaletteWidget::show(const std::vector<cor::LightState>& states) {
    auto statesCopy = states;
    if (mSkipOffLightStates) {
        std::vector<cor::LightState> nonOffLightStates;
        for (const auto& lightState : states) {
            if (lightState.isOn()) {
                nonOffLightStates.push_back(lightState);
            }
            statesCopy = nonOffLightStates;
        }
    }
    mIsSolidColors = false;
    mStates = statesCopy;
}


void PaletteWidget::paintEvent(QPaintEvent*) {
    auto gridSize = generateGridSize();
    auto itemSize = QSize(width() / gridSize.width(), height() / gridSize.height());

    QStyleOption opt;
    opt.initFrom(this);
    QPainter painter(this);

    std::uint32_t i = 0;
    if (mIsSolidColors) {
        for (const auto& color : mSolidColors) {
            auto offset = generateOffset(itemSize, gridSize, i);
            drawSolidColor(painter, color, itemSize, offset);
            ++i;
        }
    } else {
        for (const auto& state : mStates) {
            auto offset = generateOffset(itemSize, gridSize, i);
            drawLightState(painter, state, itemSize, offset);
            ++i;
        }
    }
}

void PaletteWidget::drawSolidColor(QPainter& painter,
                                   const QColor& color,
                                   const QSize& rectSize,
                                   const QPoint& offset) {
    painter.fillRect(QRect(offset, rectSize), QBrush(color));
}


void PaletteWidget::drawLightState(QPainter& painter,
                                   const cor::LightState& state,
                                   const QSize& rectSize,
                                   const QPoint& offset) {
    IconData icon;
    icon.setRoutine(state);
    painter.drawImage(QRect(offset, rectSize), icon.renderAsQImage());
}


QPoint PaletteWidget::generateOffset(const QSize& itemSize,
                                     const QSize& gridSize,
                                     std::uint32_t i) {
    auto height = i / gridSize.width();
    auto width = i % gridSize.width();
    return QPoint(itemSize.width() * width, itemSize.height() * height);
}


QSize PaletteWidget::generateGridSize() {
    auto lightCount = mStates.size();
    if (mIsSolidColors) {
        lightCount = mSolidColors.size();
    }

    if (mIsSingleLine) {
        return QSize(lightCount, 1);
    } else {
        switch (lightCount) {
            case 1:
                return QSize(1, 1);
            case 2:
                return QSize(2, 1);
            case 3:
                return QSize(3, 1);
            case 4:
                return QSize(2, 2);
            case 5:
            case 6:
                return QSize(3, 2);
            case 7:
            case 8:
            case 9:
                return QSize(3, 3);
            case 10:
            case 11:
            case 12:
                return QSize(4, 3);
            case 13:
            case 14:
            case 15:
            case 16:
                return QSize(4, 4);
            case 17:
            case 18:
            case 19:
            case 20:
                return QSize(5, 4);
            default:
                return QSize(lightCount, 1);
        }
    }
}


} // namespace cor
