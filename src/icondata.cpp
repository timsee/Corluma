/*!
 * \copyright
 * Copyright (C) 2015 - 2020.
 * Released under the GNU General Public License.
 */

#include "icondata.h"

IconData::IconData()
    : mWidth{4},
      mHeight{4},
      mDataLength{mWidth * mHeight * 3},
      mBuffer(mDataLength, 0u) {
    // Our "random individual is a lie". It uses
    // this array to determine its "random" values,
    // and if the number is too large for that particular
    // color group's size, then it defaults to either 0
    // or 1, since all color groups are at least 2 in size.
    mRandomIndividual = {0, 5, 5, 1, 0, 3, 6, 2, 6, 0, 0, 5, 7, 1, 2, 6};
}


void IconData::setRoutine(const cor::LightState& state) {
    ERoutine routine = state.routine();
    std::vector<QColor> colors = state.palette().colors();
    QColor color = state.color();
    auto brightness = color.valueF() / 2.0;
    color.setHsvF(color.hueF(), color.saturationF(), 0.5 + brightness);

    int param = state.param();

    // catch edge case where off lights display funny
    if (!state.isOn()) {
        setSolidColor(QColor(0, 0, 0));
        return;
    }

    switch (routine) {
        case ERoutine::singleSolid:
            setSolidColor(color);
            break;
        case ERoutine::singleBlink:
            setSolidColor(color);
            addBlink();
            break;
        case ERoutine::singleWave:
            setSolidColor(color);
            addWave();
            break;
        case ERoutine::singleGlimmer:
            setSolidColor(color);
            addGlimmer();
            break;
        case ERoutine::singleFade:
            setSolidColor(color);
            if (param == 1) {
                addSineFade();
            } else {
                addLinearFade();
            }
            break;
        case ERoutine::singleSawtoothFade:
            setSolidColor(color);
            if (param == 1) {
                addSawtoothIn();
            } else {
                addSawtoothOut();
            }
            break;
        case ERoutine::multiGlimmer:
            setMultiGlimmer(colors);
            break;
        case ERoutine::multiFade:
            setMultiFade(colors, false);
            break;
        case ERoutine::multiRandomSolid:
            setMultiRandomSolid(colors);
            break;
        case ERoutine::multiRandomIndividual:
            setMultiRandomIndividual(colors);
            break;
        case ERoutine::multiBars:
            setBars(colors);
            break;
        default:
            // if a routine doesn't exist, just draw things black.
            setSolidColor(QColor(0, 0, 0));
            break;
    }
}

void IconData::setSolidColor(const QColor& color) {
    for (std::uint32_t i = 0; i < mDataLength; i = i + 3) {
        mBuffer[i] = std::uint8_t(color.red());
        mBuffer[i + 1] = std::uint8_t(color.green());
        mBuffer[i + 2] = std::uint8_t(color.blue());
    }
}


void IconData::setMultiGlimmer(const std::vector<QColor>& colors) {
    auto colorCount = std::uint32_t(colors.size());

    int j = 0;
    for (std::uint32_t i = 0; i < mDataLength; i = i + 3) {
        // the third element and the 8th element get their
        // color changed to simulate the multi glimmer effect.
        if (j == 3) {
            uint32_t color = 1;
            if (colorCount > 2) {
                color = colorCount - 1;
            }
            mBuffer[i] = std::uint8_t(colors[color].red());
            mBuffer[i + 1] = std::uint8_t(colors[color].green());
            mBuffer[i + 2] = std::uint8_t(colors[color].blue());
        } else if (j == 8) {
            uint32_t color = 1;
            // only use this value if the colorCount allows it
            if (colorCount > 2) {
                color = 2;
            }
            mBuffer[i] = std::uint8_t(colors[color].red());
            mBuffer[i + 1] = std::uint8_t(colors[color].green());
            mBuffer[i + 2] = std::uint8_t(colors[color].blue());
        } else {
            mBuffer[i] = std::uint8_t(colors[0].red());
            mBuffer[i + 1] = std::uint8_t(colors[0].green());
            mBuffer[i + 2] = std::uint8_t(colors[0].blue());
        }
        j++;
    }
    addGlimmer(); // this already draws to output.
}

void IconData::setMultiFade(const std::vector<QColor>& colors, bool showMore) {
    auto colorCount = std::uint32_t(colors.size());
    if (showMore) {
        colorCount = 10;
    }

    std::uint32_t k = 0;
    int tempIndex = -1;
    std::vector<std::uint32_t> arrayIndices(8);
    while (k < 8) {
        tempIndex = (tempIndex + 1) % int(colorCount);
        arrayIndices[k] = std::uint32_t(tempIndex);
        k++;
    }

    std::uint32_t count = 16;
    std::vector<QColor> output(count);
    std::uint32_t colorIndex = 0;
    for (auto i = 0u; i < count - 2; i = i + 2) {
        output[i] = colors[arrayIndices[colorIndex]];
        output[i + 1] =
            getMiddleColor(colors[arrayIndices[colorIndex]], colors[arrayIndices[colorIndex + 1]]);
        colorIndex++;
    }

    // wrap the last value around
    output[count - 2] = colors[arrayIndices[colorIndex]];
    output[count - 1] = getMiddleColor(colors[arrayIndices[colorIndex]], colors[arrayIndices[0]]);
    std::uint32_t j = 0;
    for (std::uint32_t i = 0; i < mDataLength; i = i + 3) {
        mBuffer[i] = std::uint8_t(output[j].red());
        mBuffer[i + 1] = std::uint8_t(output[j].green());
        mBuffer[i + 2] = std::uint8_t(output[j].blue());
        j++;
    }
}

void IconData::setMultiRandomSolid(const std::vector<QColor>& colors) {
    auto colorCount = std::uint32_t(colors.size());

    int k = 0;
    for (std::uint32_t i = 0; i < mDataLength; i = i + 12) {
        QColor randomColor;
        if (k == 0) {
            randomColor = colors[1];
        } else if (k == 1) {
            randomColor = colors[0];
        } else if (k == 2) {
            if (colorCount > 3) {
                randomColor = colors[2];
            } else if (colorCount > 2) {
                randomColor = colors[1];
            } else {
                randomColor = colors[0];
            }
        } else {
            if (colorCount > 2) {
                randomColor = colors[1];
            } else {
                randomColor = colors[0];
            }
        }
        for (std::uint32_t j = 0; j < 12; j = j + 3) {
            mBuffer[i + j] = std::uint8_t(randomColor.red());
            mBuffer[i + j + 1] = std::uint8_t(randomColor.green());
            mBuffer[i + j + 2] = std::uint8_t(randomColor.blue());
        }
        k++;
    }
}

void IconData::setMultiRandomIndividual(const std::vector<QColor>& colors) {
    auto colorCount = int(colors.size());

    std::uint32_t j = 0;
    for (std::uint32_t i = 0; i < mDataLength; i = i + 3) {
        uint32_t index;
        if (mRandomIndividual[j] >= colorCount) {
            index = j % 2; // even number use 0, odd numbers use 1
        } else {
            index = std::uint32_t(mRandomIndividual[j]);
        }
        mBuffer[i] = std::uint8_t(colors[index].red());
        mBuffer[i + 1] = std::uint8_t(colors[index].green());
        mBuffer[i + 2] = std::uint8_t(colors[index].blue());
        j++;
    }
}

void IconData::setBars(const std::vector<QColor>& colors) {
    auto colorCount = std::uint32_t(colors.size());

    std::uint32_t colorIndex = 0;
    QColor color;
    for (std::uint32_t i = 3; i < mDataLength; i = i + 12) {
        color = colors[colorIndex % colorCount];
        for (std::uint32_t j = 0; j < 6; j = j + 3) {
            mBuffer[i + j] = std::uint8_t(color.red());
            mBuffer[i + j + 1] = std::uint8_t(color.green());
            mBuffer[i + j + 2] = std::uint8_t(color.blue());
        }
        color = colors[(colorIndex + 1) % colorCount];
        for (std::uint32_t j = 6; j < 12; j = j + 3) {
            if (i + j + 2 < mDataLength) {
                mBuffer[i + j] = std::uint8_t(color.red());
                mBuffer[i + j + 1] = std::uint8_t(color.green());
                mBuffer[i + j + 2] = std::uint8_t(color.blue());
            }
        }
        colorIndex = (colorIndex + 2) % colorCount;
    }
    color = colors[colorIndex % colorCount];
    mBuffer[0] = std::uint8_t(color.red());
    mBuffer[1] = std::uint8_t(color.green());
    mBuffer[2] = std::uint8_t(color.blue());
    mBuffer[45] = std::uint8_t(color.red());
    mBuffer[46] = std::uint8_t(color.green());
    mBuffer[47] = std::uint8_t(color.blue());
}

void IconData::addGlimmer() {
    int j = 0;
    for (uint i = 0; i < mDataLength; i = i + 3) {
        // the same colors get assigned the glimmer
        // each time this routine is ran.
        bool shouldGlimmer;
        switch (j) {
            case 0:
            case 5:
            case 6:
            case 10:
            case 12:
            case 15:
                shouldGlimmer = true;
                break;
            default:
                shouldGlimmer = false;
                break;
        }
        if (shouldGlimmer) {
            // 12 and 10 receive a strong glimmer.
            if (j == 12 || j == 10) {
                mBuffer[i] = mBuffer[i] / 4;
                mBuffer[i + 1] = mBuffer[i + 1] / 4;
                mBuffer[i + 2] = mBuffer[i + 2] / 4;
            } else {
                mBuffer[i] = mBuffer[i] / 2;
                mBuffer[i + 1] = mBuffer[i + 1] / 2;
                mBuffer[i + 2] = mBuffer[i + 2] / 2;
            }
        }
        j++;
    }
}

void IconData::addBlink() {
    bool isOn = false;
    for (std::uint32_t i = 0; i < mDataLength; i = i + 3) {
        if (isOn) {
            mBuffer[i] = 0;
            mBuffer[i + 1] = 0;
            mBuffer[i + 2] = 0;
        }
        isOn = !isOn;
    }
}

void IconData::addWave() {
    float k = 0.0f;
    int j = 3;
    float max = (mDataLength / 3.0f) - 1.0f;
    bool skip = false;
    for (std::uint32_t i = 0; i < mDataLength; i = i + 3) {
        k = j / max;
        if (k <= 0.5f) {
            k = k / 0.5f;
        } else {
            k = 1.0f - (k - 0.5f) / 0.5f;
        }
        mBuffer[i] = std::uint8_t(mBuffer[i] * k);
        mBuffer[i + 1] = std::uint8_t(mBuffer[i + 1] * k);
        mBuffer[i + 2] = std::uint8_t(mBuffer[i + 2] * k);
        if (!skip) {
            j++;
        }
        skip = !skip;
    }
}

void IconData::addLinearFade() {
    float k = 0.0f;
    int j = 0;
    float max = (mDataLength / 3.0f) - 1.0f;
    for (std::uint32_t i = 0; i < mDataLength; i = i + 3) {
        k = j / max;
        if (k < 0.5f) {
            k = k / 0.5f;
        } else {
            k = 1.0f - (k - 0.5f) / 0.5f;
        }
        mBuffer[i] = std::uint8_t(mBuffer[i] * k);
        mBuffer[i + 1] = std::uint8_t(mBuffer[i + 1] * k);
        mBuffer[i + 2] = std::uint8_t(mBuffer[i + 2] * k);
        j++;
    }
}

void IconData::addSawtoothIn() {
    float k = 0.0f;
    int j = 0;
    float max = (mDataLength / 3.0f) - 1.0f;
    for (std::uint32_t i = 0; i < mDataLength; i = i + 3) {
        k = j / max;
        if (k < 0.5f) {
            k = k / 0.5f;
        } else {
            k = (k - 0.5f) / 0.5f;
        }
        mBuffer[i] = std::uint8_t(mBuffer[i] * k);
        mBuffer[i + 1] = std::uint8_t(mBuffer[i + 1] * k);
        mBuffer[i + 2] = std::uint8_t(mBuffer[i + 2] * k);
        j++;
    }
}

void IconData::addSawtoothOut() {
    float k = 0.0f;
    int j = 0;
    float max = (mDataLength / 3.0f) - 1.0f;
    for (std::uint32_t i = 0; i < mDataLength; i = i + 3) {
        k = j / max;
        if (k < 0.5f) {
            k = 1.0f - k / 0.5f;
        } else {
            k = 1.0f - (k - 0.5f) / 0.5f;
        }
        mBuffer[i] = std::uint8_t(mBuffer[i] * k);
        mBuffer[i + 1] = std::uint8_t(mBuffer[i + 1] * k);
        mBuffer[i + 2] = std::uint8_t(mBuffer[i + 2] * k);
        j++;
    }
}

void IconData::addSineFade() {
    double k(0.0);
    int j = 0;
    double max = (mDataLength / 3.0) - 1;
    for (std::uint32_t i = 0; i < mDataLength; i = i + 3) {
        k = cbrt((sin(((j / max) * 6.28) - 1.67) + 1) / 2.0);
        mBuffer[i] = std::uint8_t(mBuffer[i] * k);
        mBuffer[i + 1] = std::uint8_t(mBuffer[i + 1] * k);
        mBuffer[i + 2] = std::uint8_t(mBuffer[i + 2] * k);
        j++;
    }
}

std::uint32_t IconData::dataLength() {
    return mDataLength;
}

std::uint32_t IconData::width() {
    return mWidth;
}

std::uint32_t IconData::height() {
    return mHeight;
}

const QPixmap IconData::renderAsQPixmap() {
    return QPixmap::fromImage(
        QImage(mBuffer.data(), int(mWidth), int(mHeight), QImage::Format_RGB888));
}

QColor IconData::getMiddleColor(const QColor& first, const QColor& second) {
    return {abs(first.red() - second.red()) / 2 + std::min(first.red(), second.red()),
            abs(first.green() - second.green()) / 2 + std::min(first.green(), second.green()),
            abs(first.blue() - second.blue()) / 2 + std::min(first.blue(), second.blue())};
}
