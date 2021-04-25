#include "palettewidget.h"
#include <QPainter>
#include <QStyleOption>
#include <algorithm>
#include "icondata.h"

namespace cor {

PaletteWidget::PaletteWidget(QWidget* parent)
    : QWidget(parent),
      mSkipOffLightStates{false},
      mIsSingleLine{false},
      mForceSquares{false} {}

void PaletteWidget::show(const std::vector<QColor>& colors) {
    mIsSolidColors = true;
    mSolidColors = colors;
    update();
}

void PaletteWidget::show(const std::vector<cor::LightState>& states) {
    auto statesCopy = states;
    // remove all off lights, if the widget is configured to do this
    if (mSkipOffLightStates) {
        std::vector<cor::LightState> nonOffLightStates;
        for (const auto& lightState : states) {
            if (lightState.isOn()) {
                nonOffLightStates.push_back(lightState);
            }
            statesCopy = nonOffLightStates;
        }
    }
    // sort by color
    std::sort(statesCopy.begin(),
              statesCopy.end(),
              [](const cor::LightState& a, const cor::LightState& b) -> bool {
                  double hueA;
                  if (a.routine() <= cor::ERoutineSingleColorEnd) {
                      hueA = a.color().hueF();
                  } else {
                      hueA = a.palette().averageColor().hueF();
                  }

                  double hueB;
                  if (b.routine() <= cor::ERoutineSingleColorEnd) {
                      hueB = b.color().hueF();
                  } else {
                      hueB = b.palette().averageColor().hueF();
                  }
                  return hueA < hueB;
              });

    mIsSolidColors = false;
    mStates = statesCopy;
    update();
}

bool PaletteWidget::isShowingAnything() {
    if (mIsSolidColors) {
        return !mSolidColors.empty();
    } else {
        return !mStates.empty();
    }
}

void PaletteWidget::paintEvent(QPaintEvent*) {
    auto gridSize = generateGridSize();

    QStyleOption opt;
    opt.initFrom(this);
    QPainter painter(this);

    std::uint32_t i = 0;
    if (mIsSolidColors) {
        for (const auto& color : mSolidColors) {
            auto renderRegion = generateRenderRegion(gridSize, i);
            drawSolidColor(painter, color, renderRegion);
            ++i;
        }
    } else {
        for (const auto& state : mStates) {
            auto renderRegion = generateRenderRegion(gridSize, i);
            drawLightState(painter, state, renderRegion);
            ++i;
        }
    }
}

void PaletteWidget::drawSolidColor(QPainter& painter,
                                   const QColor& color,
                                   const QRect& renderRect) {
    painter.fillRect(renderRect, QBrush(color));
}


void PaletteWidget::drawLightState(QPainter& painter,
                                   const cor::LightState& state,
                                   const QRect& renderRegion) {
    IconData icon;
    icon.setRoutine(state);
    painter.drawImage(renderRegion, icon.renderAsQImage());
}


QRect PaletteWidget::generateRenderRegion(const QSize& gridSize, std::uint32_t i) {
    auto height = i / gridSize.width();
    auto width = i % gridSize.width();

    auto itemWidth = this->width() / gridSize.width();
    auto itemHeight = this->height() / gridSize.height();
    QSize itemSize;
    QSize offset;
    if (mForceSquares) {
        auto side = std::min(itemWidth, itemHeight);
        itemSize = QSize(side, side);
        offset = QSize((this->width() - (gridSize.width() * side)) / 2,
                       (this->height() - (gridSize.height() * side)) / 2);
    } else {
        itemSize = QSize(itemWidth, itemHeight);
        offset = QSize(0, 0);
    }
    return QRect(offset.width() + itemSize.width() * width,
                 offset.height() + itemSize.height() * height,
                 itemSize.width(),
                 itemSize.height());
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
