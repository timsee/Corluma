/*!
 * \copyright
 * Copyright (C) 2015 - 2016.
 * Released under the GNU General Public License.
 */

#include "icondata.h"

#include <QDebug>

#include <stdlib.h>
#include <ctime>
#include <algorithm>
#include <math.h>

IconData::IconData() {
    setup(64, 64);
}

IconData::IconData(int width, int height) {
    setup(width, height);
}

IconData::IconData(int width, int height, DataLayer *data) {
    mDataLayer = data;
    setup(width, height);
}

void IconData::setup(int width, int height) {
    srand(time(NULL));

    // first, check that both are a multiple of four
    bool printError = false;
    while ((width % 4)) {
        width = width - 1;
        printError = true;
    }
    while((height % 4)) {
        height = height - 1;
        printError = true;
    }

    if (printError) {
        qDebug() << "Warning: a value was given to icon data that wasn't a multiple of four. The value has been adjusted!";
    }

    // always uses RGB_888 so multiply width and height by 3
    mWidth = width;
    mHeight = height;
    mDataLength = width * height * 3;
    mData = QVector<uint8_t>(mDataLength);

    mBufferWidth = 4;
    mBufferHeight = 4;
    mBufferLength = mBufferWidth * mBufferHeight * 3;
    mBuffer = QVector<uint8_t>(mBufferLength);

    // zero out the arrays
    for (uint i = 0; i < mBufferLength; i ++) {
        mBuffer[i] = 0;
    }
    for (uint i = 0; i < mDataLength; i ++) {
        mData[i] = 0;
    }

    // Our "random individual is a lie". It uses
    // this array to determine its "random" values,
    // and if the number is too large for that particular
    // color group's size, then it defaults to either 0
    // or 1, since all color groups are at least 2 in size.
    mRandomIndividual[0] = 0;
    mRandomIndividual[1] = 5;
    mRandomIndividual[2] = 5;
    mRandomIndividual[3] = 1;
    mRandomIndividual[4] = 0;
    mRandomIndividual[5] = 3;
    mRandomIndividual[6] = 6;
    mRandomIndividual[7] = 2;
    mRandomIndividual[8] = 6;
    mRandomIndividual[9] = 0;
    mRandomIndividual[10] = 0;
    mRandomIndividual[11] = 5;
    mRandomIndividual[12] = 7;
    mRandomIndividual[13] = 1;
    mRandomIndividual[14] = 2;
    mRandomIndividual[15] = 6;
}

void IconData::bufferToOutput() {
    uint j = 0;
    uint k = 0;
    for (uint i = 0; i < mDataLength; i = i + 3) {
        if ((i % (mWidth * 3)) < mWidth * 3 / 4) {
            mData[i] = mBuffer[j];
            mData[i + 1] = mBuffer[j + 1];
            mData[i + 2] = mBuffer[j + 2];
        }
        else if ((i % (mWidth * 3)) < mWidth * 6 / 4) {
             mData[i] = mBuffer[j + 3];
             mData[i + 1] = mBuffer[j + 4];
             mData[i + 2] = mBuffer[j + 5];
        }
        else if ((i % (mWidth * 3)) < mWidth * 9 / 4) {
             mData[i] = mBuffer[j + 6];
             mData[i + 1] = mBuffer[j + 7];
             mData[i + 2] = mBuffer[j + 8];
        }
        else if ((i % (mWidth * 3)) < mWidth * 3) {
             mData[i] = mBuffer[j + 9];
             mData[i + 1] = mBuffer[j + 10];
             mData[i + 2] = mBuffer[j + 11];
        }

        if (!((i + 3) % (mWidth * 3))) {
            k++;
        }
        if (k == (mHeight / 4)) {
            j = j + 4 * 3;
            k = 0;
        }
    }
}

void IconData::setLightingRoutine(ELightingRoutine routine, EColorGroup colorGroup) {
    switch (routine)
    {
        case ELightingRoutine::eSingleSolid:
            setSolidColor(mDataLayer->mainColor());
            break;
        case ELightingRoutine::eSingleBlink:
            setSolidColor(mDataLayer->mainColor());
            addBlink();
            break;
        case ELightingRoutine::eSingleWave:
            setSolidColor(mDataLayer->mainColor());
            addWave();
            break;
        case ELightingRoutine::eSingleGlimmer:
            setSolidColor(mDataLayer->mainColor());
            addGlimmer();
            break;
        case ELightingRoutine::eSingleLinearFade:
            setSolidColor(mDataLayer->mainColor());
            addLinearFade();
            break;
        case ELightingRoutine::eSingleSawtoothFadeIn:
            setSolidColor(mDataLayer->mainColor());
            addSawtoothIn();
            break;
        case ELightingRoutine::eSingleSawtoothFadeOut:
            setSolidColor(mDataLayer->mainColor());
            addSawtoothOut();
            break;
        case ELightingRoutine::eSingleSineFade:
            setSolidColor(mDataLayer->mainColor());
            addSineFade();
            break;
        case ELightingRoutine::eMultiGlimmer:
            setMultiGlimmer(colorGroup);
            break;
        case ELightingRoutine::eMultiFade:
            setMultiFade(colorGroup);
            break;
        case ELightingRoutine::eMultiRandomSolid:
            setMultiRandomSolid(colorGroup);
            break;
        case ELightingRoutine::eMultiRandomIndividual:
            setMultiRandomIndividual(colorGroup);
            break;
        case ELightingRoutine::eMultiBarsSolid:
            setMultiBarsSolid(colorGroup);
            break;
        case ELightingRoutine::eMultiBarsMoving:
            setMultiBarsMoving(colorGroup);
            break;
        default:
            break;
    }
}

void IconData::setSingleLightingRoutine(ELightingRoutine routine, QColor color) {
    switch (routine)
    {
        case ELightingRoutine::eSingleSolid:
            setSolidColor(color);
            break;
        case ELightingRoutine::eSingleBlink:
            setSolidColor(color);
            addBlink();
            break;
        case ELightingRoutine::eSingleWave:
            setSolidColor(color);
            addWave();
            break;
        case ELightingRoutine::eSingleGlimmer:
            setSolidColor(color);
            addGlimmer();
            break;
        case ELightingRoutine::eSingleLinearFade:
            setSolidColor(color);
            addLinearFade();
            break;
        case ELightingRoutine::eSingleSawtoothFadeIn:
            setSolidColor(color);
            addSawtoothIn();
            break;
        case ELightingRoutine::eSingleSawtoothFadeOut:
            setSolidColor(color);
            addSawtoothOut();
            break;
        case ELightingRoutine::eSingleSineFade:
            setSolidColor(color);
            addSineFade();
            break;
        default:
            qDebug() << "WARNING: used single light routine function with multi light routine.";
            break;
    }
}

void IconData::setSolidColor(QColor color) {
    for (uint i = 0; i < mBufferLength; i = i + 3) {
        mBuffer[i] = color.red();
        mBuffer[i + 1] = color.green();
        mBuffer[i + 2] = color.blue();
    }
    bufferToOutput();
}


void IconData::setMultiColors(EColorGroup group) {
    if (mDataLayer == NULL) {
        qDebug() << "ERROR: the data layer is not set, cannot use the arary colors!";
    }
    int colorCount = mDataLayer->groupSize(group);
    QColor *colors = mDataLayer->colorGroup(group);

    int j = 0;
    for (uint i = 0; i < mBufferLength; i = i + 3) {
        mBuffer[i] = colors[j].red();
        mBuffer[i + 1] = colors[j].green();
        mBuffer[i + 2] = colors[j].blue();
        j = (j + 1) % colorCount;
    }
    bufferToOutput();
}



void IconData::setMultiGlimmer(EColorGroup group) {
    int colorCount = mDataLayer->groupSize(group);
    QColor *colors = mDataLayer->colorGroup(group);
    int j = 0;
    for (uint i = 0; i < mBufferLength; i = i + 3) {
        // the third element and the 8th element get their
        // color changed to simulate the multi glimmer effect.
        if (j == 3) {
            int color = 1;
            if (colorCount > 2) {
                color = colorCount - 1;
            }
            mBuffer[i]     = colors[color].red();
            mBuffer[i + 1] = colors[color].green();
            mBuffer[i + 2] = colors[color].blue();
        } else if (j == 8) {
            int color = 1;
            // only use this value if the colorCount allows it
            if (colorCount > 2) {
                color = 2;
            }
            mBuffer[i]     = colors[color].red();
            mBuffer[i + 1] = colors[color].green();
            mBuffer[i + 2] = colors[color].blue();
        } else {
            mBuffer[i]     = colors[0].red();
            mBuffer[i + 1] = colors[0].green();
            mBuffer[i + 2] = colors[0].blue();
        }
        j++;
    }
    addGlimmer(); // this already draws to output.
}

void IconData::setMultiFade(EColorGroup group, bool showMore) {
    // By default, showMore is set to false because everything shows its
    // max, except the color count. For the menu bar we override this
    // feature. This handles a silly edge case.
    int colorCount;
    if (showMore) {
        colorCount = 10;
    } else {
        colorCount = mDataLayer->groupSize(group);
    }
    QColor *colors = mDataLayer->colorGroup(group);

    int k = 0;
    int tempIndex = -1;
    int *arrayIndices = new int[8];
    while (k < 8) {
        tempIndex = (tempIndex + 1) % colorCount;
        arrayIndices[k] = tempIndex;
        k++;
    }

    int count = 16;
    QColor *output = new QColor[count];
    int colorIndex = 0;
    for (int i = 0; i < count - 2; i = i + 2) {
        output[i] = colors[arrayIndices[colorIndex]];
        output[i + 1] = getMiddleColor(colors[arrayIndices[colorIndex]], colors[arrayIndices[colorIndex + 1]]);
        colorIndex++;
    }

    // wrap the last value around
    output[count - 2] = colors[arrayIndices[colorIndex]];
    output[count - 1] = getMiddleColor(colors[arrayIndices[colorIndex]], colors[arrayIndices[0]]);
    int j = 0;
    for (uint i = 0; i < mBufferLength; i = i + 3) {
        mBuffer[i] = output[j].red();
        mBuffer[i + 1] = output[j].green();
        mBuffer[i + 2] = output[j].blue();
        j++;
    }

    bufferToOutput();
}

void IconData::setMultiRandomSolid(EColorGroup group) {
    int colorCount = mDataLayer->groupSize(group);
    QColor *colors = mDataLayer->colorGroup(group);

    int k = 0;
    for (uint i = 0; i < mBufferLength; i = i + 12) {
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
        for (int j = 0; j < 12; j = j + 3) {
            mBuffer[i + j] = randomColor.red();
            mBuffer[i + j + 1] = randomColor.green();
            mBuffer[i + j + 2] = randomColor.blue();
        }
        k++;
    }
    bufferToOutput();
}

void IconData::setMultiRandomIndividual(EColorGroup group) {
    if (group == EColorGroup::eAll) {
        for (uint i = 0; i < mBufferLength; i++) {
            int random = rand() % 256;
            mBuffer[i] = (uchar)random;
        }
    } else {
        int colorCount = mDataLayer->groupSize(group);
        QColor *colors = mDataLayer->colorGroup(group);
        int j = 0;
        for (uint i = 0; i < mBufferLength; i = i + 3) {
            int index;
            if (mRandomIndividual[j] >= colorCount) {
                index = j % 2; // even number use 0, odd numbers use 1
            } else {
                index = mRandomIndividual[j];
            }
            mBuffer[i] = colors[index].red();
            mBuffer[i + 1] = colors[index].green();
            mBuffer[i + 2] = colors[index].blue();
            j++;
        }
    }
    bufferToOutput();
}

void IconData::setMultiBarsSolid(EColorGroup group) {
     if (group == EColorGroup::eAll) {
         for (uint i = 0; i < mBufferLength; i = i + 12) {
             int r = rand() % 256;
             int g = rand() % 256;
             int b = rand() % 256;
             for (int j = 0; j < 12; j = j + 3) {
                 mBuffer[i + j] = (uchar)r;
                 mBuffer[i + j + 1] = (uchar)g;
                 mBuffer[i + j + 2] = (uchar)b;
             }
         }
     } else {
         int colorCount = mDataLayer->groupSize(group);
         QColor *colors = mDataLayer->colorGroup(group);

         int colorIndex = 0;
         for (uint i = 0; i < mBufferLength; i = i + 12) {
             QColor color = colors[colorIndex % colorCount];
             for (int j = 0; j < 6; j = j + 3) {
                 mBuffer[i + j] = color.red();
                 mBuffer[i + j + 1] = color.green();
                 mBuffer[i + j + 2] = color.blue();
             }
             color = colors[(colorIndex + 1) % colorCount];
             for (int j = 6; j < 12; j = j + 3) {
                 mBuffer[i + j] = color.red();
                 mBuffer[i + j + 1] = color.green();
                 mBuffer[i + j + 2] = color.blue();
             }
             colorIndex = (colorIndex + 2) % colorCount;
         }
     }
    bufferToOutput();
}

void IconData::setMultiBarsMoving(EColorGroup group) {
    int colorCount = mDataLayer->groupSize(group);
    QColor *colors = mDataLayer->colorGroup(group);

    int colorIndex = 0;
    QColor color;
    for (uint i = 3; i < mBufferLength; i = i + 12) {
        color = colors[colorIndex % colorCount];
        for (int j = 0; j < 6; j = j + 3) {
            mBuffer[i + j] = color.red();
            mBuffer[i + j + 1] = color.green();
            mBuffer[i + j + 2] = color.blue();
        }
        color = colors[(colorIndex + 1) % colorCount];
        for (int j = 6; j < 12; j = j + 3) {
            if (i + j + 2 < mBufferLength) {
                mBuffer[i + j] = color.red();
                mBuffer[i + j + 1] = color.green();
                mBuffer[i + j + 2] = color.blue();
            }
        }
        colorIndex = (colorIndex + 2) % colorCount;
    }
    color = colors[colorIndex % colorCount];
    mBuffer[0] = color.red();
    mBuffer[1] = color.green();
    mBuffer[2] = color.blue();
    mBuffer[45] = color.red();
    mBuffer[46] = color.green();
    mBuffer[47] = color.blue();
    bufferToOutput();
}

void IconData::addGlimmer() {
    int j = 0;
    for (uint i = 0; i < mBufferLength; i = i + 3) {
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
    bufferToOutput();
}

void IconData::addBlink() {
    bool isOn = false;
    for (uint i = 0; i < mBufferLength; i = i + 3) {
        if (isOn) {
            mBuffer[i] = 0;
            mBuffer[i + 1] = 0;
            mBuffer[i + 2] = 0;
        }
        isOn = !isOn;
    }
    bufferToOutput();
}

void IconData::addWave() {
    float k = 0.0f;
    int j = 3;
    float max = (mBufferLength / 3.0f) - 1.0f;
    bool skip = false;
    for (uint i = 0; i < mBufferLength; i = i + 3) {
        k = j / max;
        if (k <= 0.5f) {
            k = k / 0.5f;
        } else {
            k = 1.0f - (k - 0.5f) / 0.5f;
        }
        mBuffer[i]     = (uint8_t)(mBuffer[i] * k);
        mBuffer[i + 1] = (uint8_t)(mBuffer[i + 1] * k);
        mBuffer[i + 2] = (uint8_t)(mBuffer[i + 2] * k);
        if (!skip) {
            j++;
        }
        skip = !skip;
    }
    bufferToOutput();
}

void IconData::addLinearFade() {
    float k = 0.0f;
    int j = 0;
    float max = (mBufferLength / 3.0f) - 1.0f;
    for (uint i = 0; i < mBufferLength; i = i + 3) {
        k = j / max;
        if (k < 0.5f) {
            k = k / 0.5f;
        } else {
            k = 1.0f - (k - 0.5f) / 0.5f;
        }
        mBuffer[i]     = (uint8_t)(mBuffer[i] * k);
        mBuffer[i + 1] = (uint8_t)(mBuffer[i + 1] * k);
        mBuffer[i + 2] = (uint8_t)(mBuffer[i + 2] * k);
        j++;
    }
    bufferToOutput();
}

void IconData::addSawtoothIn() {
    float k = 0.0f;
    int j = 0;
    float max = (mBufferLength / 3.0f) - 1.0f;
    for (uint i = 0; i < mBufferLength; i = i + 3) {
        k = j / max;
        if (k < 0.5f) {
            k = k / 0.5f;
        } else {
            k = (k - 0.5f) / 0.5f;
        }
        mBuffer[i]     = (uint8_t)(mBuffer[i] * k);
        mBuffer[i + 1] = (uint8_t)(mBuffer[i + 1] * k);
        mBuffer[i + 2] = (uint8_t)(mBuffer[i + 2] * k);
        j++;
    }
    bufferToOutput();
}

void IconData::addSawtoothOut() {
    float k = 0.0f;
    int j = 0;
    float max = (mBufferLength / 3.0f) - 1.0f;
    for (uint i = 0; i < mBufferLength; i = i + 3) {
        k = j / max;
        if (k < 0.5f) {
            k = 1.0f - k / 0.5f;
        } else {
            k = 1.0f - (k - 0.5f) / 0.5f;
        }
        mBuffer[i]     = (uint8_t)(mBuffer[i] * k);
        mBuffer[i + 1] = (uint8_t)(mBuffer[i + 1] * k);
        mBuffer[i + 2] = (uint8_t)(mBuffer[i + 2] * k);
        j++;
    }
    bufferToOutput();
}

void IconData::addSineFade() {
    float k = 0.0f;
    int j = 0;
    float max = (mBufferLength / 3) - 1;
    for (uint i = 0; i < mBufferLength; i = i + 3) {
        k = cbrt((sin(((j / max) * 6.28f) - 1.67f) + 1) / 2.0f);
        mBuffer[i]     = (uint8_t)(mBuffer[i] * k);
        mBuffer[i + 1] = (uint8_t)(mBuffer[i + 1] * k);
        mBuffer[i + 2] = (uint8_t)(mBuffer[i + 2] * k);
        j++;
    }
    bufferToOutput();
}


uint IconData::dataLength() {
    return mDataLength;
}

uint IconData::width() {
    return mWidth;
}

uint IconData::height() {
    return mHeight;
}

QImage IconData::renderAsQImage() {
    return QImage(mData.data(), mWidth, mHeight, QImage::Format_RGB888);
}

QPixmap IconData::renderAsQPixmap() {
    return QPixmap::fromImage(renderAsQImage());
}

QColor IconData::getMiddleColor(QColor first,
                                QColor second) {
    return QColor(abs(first.red() - second.red()) / 2 + std::min(first.red(), second.red()),
                  abs(first.green() - second.green()) / 2 + std::min(first.green(), second.green()),
                  abs(first.blue() - second.blue()) / 2 + std::min(first.blue(), second.blue()));
}
