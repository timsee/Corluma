#include "palettewidget.h"
#include <QEvent>
#include <QGraphicsOpacityEffect>
#include <QPainter>
#include <QStyleOption>
#include <algorithm>
#include "icondata.h"

namespace cor {

PaletteWidget::PaletteWidget(QWidget* parent)
    : QWidget(parent),
      mPreferPalettesOverRoutines{false},
      mSkipOffLightStates{false},
      mIsSingleLine{false},
      mForceSquares{false},
      mBrightnessMode{EBrightnessMode::none} {}

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
        auto brightness = calculateBrightness(mSolidColors, mBrightnessMode);
        for (const auto& color : mSolidColors) {
            auto renderRegion = generateRenderRegion(gridSize, i);
            renderRegion =
                correctRenderRegionEdgeCases(renderRegion, geometry(), i, mSolidColors.size());
            drawSolidColor(painter, color, brightness, renderRegion);
            ++i;
        }
    } else {
        auto brightness = calculateBrightness(mStates, mBrightnessMode);
        std::vector<std::pair<QColor, int>> colorAndSizeVector;
        for (const auto& state : mStates) {
            auto renderRegion = generateRenderRegion(gridSize, i);
            renderRegion =
                correctRenderRegionEdgeCases(renderRegion, geometry(), i, mStates.size());

            if (mPreferPalettesOverRoutines) {
                auto results = convertStateColors(state, brightness, renderRegion);
                colorAndSizeVector.insert(colorAndSizeVector.begin(),
                                          results.begin(),
                                          results.end());
            } else {
                drawLightState(painter, state, brightness, renderRegion);
            }
            ++i;
        }

        if (mPreferPalettesOverRoutines) {
            drawColorsFromStates(painter, colorAndSizeVector);
        }
    }
}

QRect PaletteWidget::correctRenderRegionEdgeCases(QRect rect,
                                                  const QRect& boundingRect,
                                                  std::uint32_t i,
                                                  std::uint32_t lightCount) {
    if (mIsSingleLine && (i == lightCount - 1) && (rect.topRight().x() != boundingRect.width())) {
        auto diff = width() - rect.topRight().x();
        rect = QRect(rect.x(), rect.y(), rect.width() + diff, rect.height());
    }
    return rect;
}

void PaletteWidget::changeEvent(QEvent*) {
    // qDebug() << " event " << event->type();

    // TODO: this lags on mobile...
    //    if (event->type() == QEvent::EnabledChange && isEnabled()) {
    //        QGraphicsOpacityEffect* effect = new QGraphicsOpacityEffect(this);
    //        effect->setOpacity(1.0);
    //        this->setGraphicsEffect(effect);
    //    } else if (event->type() == QEvent::EnabledChange && !isEnabled()) {
    //        QGraphicsOpacityEffect* effect = new QGraphicsOpacityEffect(this);
    //        effect->setOpacity(0.5);
    //        this->setGraphicsEffect(effect);
    //    }
}

void PaletteWidget::drawSolidColor(QPainter& painter,
                                   const QColor& color,
                                   float brightness,
                                   const QRect& renderRect) {
    auto colorCopy = color;
    if (mBrightnessMode != EBrightnessMode::none) {
        colorCopy = QColor::fromHsvF(color.hueF(), color.saturationF(), brightness);
    }
    painter.fillRect(renderRect, QBrush(colorCopy));
}

namespace {

bool sortVectorByHue(const std::pair<QColor, int>& lhs, const std::pair<QColor, int>& rhs) {
    if (lhs.first.hue() == rhs.first.hue()) {
        if (lhs.first.saturation() == rhs.first.saturation()) {
            return (lhs.first.saturationF() > rhs.first.saturationF());
        } else {
            return (lhs.first.valueF() > rhs.first.valueF());
        }
    } else {
        return (lhs.first.hueF() < rhs.first.hueF());
    }
}

} // namespace

void PaletteWidget::drawColorsFromStates(QPainter& painter,
                                         std::vector<std::pair<QColor, int>> inputVector) {
    auto xPos = 0;
    std::sort(inputVector.begin(), inputVector.end(), sortVectorByHue);
    auto count = 0u;
    for (const auto& colorAndSizePair : inputVector) {
        painter.fillRect(QRect(xPos, 0, colorAndSizePair.second, height()),
                         QBrush(colorAndSizePair.first));
        xPos += colorAndSizePair.second;
        ++count;
        if (count == inputVector.size() - 1) {
            if (xPos < width()) {
                auto remainingArea = width() - xPos;
                painter.fillRect(QRect(xPos, 0, remainingArea, height()),
                                 QBrush(colorAndSizePair.first));
            }
        }
    }
}

void PaletteWidget::drawLightState(QPainter& painter,
                                   const cor::LightState& state,
                                   float brightness,
                                   const QRect& renderRegion) {
    auto stateCopy = state;
    if (mBrightnessMode != EBrightnessMode::none) {
        if (state.routine() <= cor::ERoutineSingleColorEnd) {
            stateCopy.color(
                QColor::fromHsvF(state.color().hueF(), state.color().saturationF(), brightness));
        } else {
            auto paletteCopy = stateCopy.palette();
            stateCopy.paletteBrightness(brightness * 100.0f);
        }
    }
    IconData icon;
    icon.setRoutine(stateCopy);
    painter.drawImage(renderRegion, icon.renderAsQImage());
}


std::vector<std::pair<QColor, int>> PaletteWidget::convertStateColors(const cor::LightState& state,
                                                                      float brightness,
                                                                      const QRect& renderRect) {
    auto stateCopy = state;
    if (mBrightnessMode != EBrightnessMode::none) {
        if (state.routine() <= cor::ERoutineSingleColorEnd) {
            stateCopy.color(
                QColor::fromHsvF(state.color().hueF(), state.color().saturationF(), brightness));
        } else {
            auto paletteCopy = stateCopy.palette();
            stateCopy.paletteBrightness(brightness * 100.0f);
        }
    }

    // TODO: should palette brightness be used here?
    if (!state.isOn()) {
        return {std::make_pair(QColor(0, 0, 0), renderRect.width())};
    } else if (state.routine() <= cor::ERoutineSingleColorEnd) {
        return {std::make_pair(stateCopy.color(), renderRect.width())};
    } else {
        auto colorCount = stateCopy.palette().colors().size();
        auto colorWidth = renderRect.width() / colorCount;
        std::uint32_t i = 0;
        std::vector<std::pair<QColor, int>> colorAndSizeVector;
        for (auto color : stateCopy.palette().colors()) {
            QRect rect(renderRect.x() + i * colorWidth,
                       renderRect.y(),
                       colorWidth,
                       renderRect.height());
            // rect = correctRenderRegionEdgeCases(rect, renderRect, i, colorCount);
            colorAndSizeVector.push_back(std::make_pair(color, rect.width()));
            ++i;
        }
        return colorAndSizeVector;
    }
}


float PaletteWidget::calculateBrightness(const std::vector<QColor>& colors, EBrightnessMode mode) {
    if (mode == EBrightnessMode::average) {
        float bright = 0.0f;
        for (const auto& color : colors) {
            bright += color.valueF();
        }
        return bright / colors.size();
    } else if (mode == EBrightnessMode::max) {
        float bright = 0.0f;
        for (const auto& color : colors) {
            if (color.valueF() > bright) {
                bright = color.valueF();
            }
        }
        return bright;
    } else {
        return 0.0;
    }
}

float PaletteWidget::calculateBrightness(const std::vector<cor::LightState>& states,
                                         EBrightnessMode mode) {
    if (mode == EBrightnessMode::average) {
        float bright = 0.0f;
        std::uint32_t onLights = 0u;
        for (const auto& state : states) {
            if (state.isOn()) {
                ++onLights;
                if (state.routine() <= cor::ERoutineSingleColorEnd) {
                    bright += state.color().valueF();
                } else {
                    bright += state.paletteBrightness() / 100.0f;
                }
            }
        }
        if (onLights > 0) {
            return bright / onLights;
        } else {
            return 0.0;
        }
    } else if (mode == EBrightnessMode::max) {
        float bright = 0.0f;
        for (const auto& state : states) {
            if (state.routine() <= cor::ERoutineSingleColorEnd) {
                if (state.color().valueF() > bright) {
                    bright = state.color().valueF();
                }
            } else {
                if (state.paletteBrightness() > bright) {
                    bright = state.paletteBrightness() / 100.0f;
                }
            }
        }
        return bright;
    } else {
        return 0.0;
    }
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
