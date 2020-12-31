/*!
 * \copyright
 * Copyright (C) 2015 - 2020.
 * Released under the GNU General Public License.
 */

#include "cor/widgets/slider.h"
#include "utils/qt.h"

#include <QApplication>
#include <QFontMetrics>
#include <QGraphicsOpacityEffect>
#include <QStyleOption>
#include <QtCore>
#include <QtGui>

namespace cor {

QSize handleSize(int width, int height) {
    // slider handle is only controllable via stylesheets but the values needed for style sheets
    // breaks in some environments (such as high pixel density android screens). to get around this,
    // we always set the handle size programmatically whenever we udpate the stylesheet.
    if (cor::applicationSize().width() > cor::applicationSize().height()) {
        return QSize(int(width / 20.0), int(height * 1.125));
    } else {
        return QSize(int(width / 15.0), int(height * 1.125));
    }
}

Slider::Slider(QWidget* parent) : QSlider(Qt::Horizontal, parent) {
    mHeightScaleFactor = 1.0f;
    mShouldDrawTickLabels = false;
    mShouldSnap = false;
    mUseMinimumPossible = false;
    mMinimumPossible = 0;

    setAutoFillBackground(true);
    setSnapToNearestTick(false);
    connect(this, SIGNAL(valueChanged(int)), this, SLOT(receivedValue(int)));
    connect(this, SIGNAL(sliderReleased()), this, SLOT(releasedSlider()));
    connect(this, SIGNAL(sliderMoved(int)), this, SLOT(movedSlider(int)));

    mType = ESliderType::vanilla;

    adjustStylesheet();
}

void Slider::resize() {
    mHandleSize = handleSize(width(), height());
    switch (mType) {
        case ESliderType::colorLeftBlackRight:
            setColor(mSliderColor);
            break;
        case ESliderType::fullBarGradient:
            setGradient(mSliderColor, mColorGradient);
            break;
        case ESliderType::image:
            setImage(mPath);
            break;
        default:
            break;
    }
    update();
}

void Slider::setColor(const QColor& color) {
    mSliderColor = color;
    mType = ESliderType::colorLeftBlackRight;

    // generate a stylesheet based off of the color with a gradient
    adjustStylesheet();
}

void Slider::setImage(const QString& path) {
    mType = ESliderType::image;
    mPath = path;
    adjustStylesheet();
}

void Slider::setGradient(const QColor& leftColor, const QColor& rightColor) {
    mSliderColor = leftColor;
    mColorGradient = rightColor;
    mType = ESliderType::fullBarGradient;

    // generate a stylesheet based off of the color with a gradient
    adjustStylesheet();
}

void Slider::adjustStylesheet() {
    QString stylesheet;
    auto margin = int((height() * (1.0f - mHeightScaleFactor)) / 2.0f);
    auto handleMargin = int((height() - mHandleSize.height()) / 2.0f);
    switch (mType) {
        case ESliderType::vanilla:
            stylesheet = QString("QSlider::handle:horizontal {"
                                 "width: %1px;"
                                 "height: %2px;"
                                 "}")
                             .arg(QString::number(mHandleSize.width()),
                                  QString::number(mHandleSize.height()));
            break;
        case ESliderType::colorLeftBlackRight: {
            QColor darkColor = QColor(int(mSliderColor.red() / 4),
                                      int(mSliderColor.green() / 4),
                                      int(mSliderColor.blue() / 4));
            stylesheet = QString("QSlider::groove:horizontal{ "
                                 " background: rgb(32, 31, 31);"
                                 "margin-top: %9px;"
                                 "margin-bottom: %9px;"
                                 "margin-left: 0px;"
                                 "margin-right: 0px;"
                                 "border-radius: 2px;"
                                 "}"
                                 "QSlider::sub-page:horizontal{ "
                                 " background:qlineargradient(x1: 0, y1: 0, x2: 0, y2: 1, stop: 0 "
                                 "rgb(%1, %2, %3), stop: 1 rgb(%4, %5, %6));"
                                 " background: qlineargradient(x1: 0, y1: 0.2, x2: 1, y2: 1, stop: "
                                 "0 rgb(%1, %2, %3), stop: 1 rgb(%4, %5, %6));"
                                 "margin-top: %9px;"
                                 "margin-bottom: %9px;"
                                 "}"
                                 "QSlider::handle:horizontal {"
                                 "width: %7px;"
                                 "margin-top: %8px;"
                                 "margin-bottom: %8px;"
                                 "}")
                             .arg(QString::number(darkColor.red()),
                                  QString::number(darkColor.green()),
                                  QString::number(darkColor.blue()),
                                  QString::number(mSliderColor.red()),
                                  QString::number(mSliderColor.green()),
                                  QString::number(mSliderColor.blue()),
                                  QString::number(mHandleSize.width()),
                                  QString::number(handleMargin),
                                  QString::number(margin));
            break;
        }
        case ESliderType::fullBarGradient: {
            stylesheet = QString("QSlider::groove:horizontal{ "
                                 " background: qlineargradient(x1: 0, y1: 0, x2: 1, y2: 0, stop: 0 "
                                 "rgb(%1, %2, %3), stop: 1 rgb(%4, %5, %6));"
                                 "margin-top: %9px;"
                                 "margin-bottom: %9px;"
                                 "margin-left: 0px;"
                                 "margin-right: 0px;"
                                 "border-radius: 2px;"
                                 "}"
                                 "QSlider::handle:horizontal {"
                                 "width: %7px;"
                                 "margin-top: %8px;"
                                 "margin-bottom: %8px;"
                                 "}")
                             .arg(QString::number(mSliderColor.red()),
                                  QString::number(mSliderColor.green()),
                                  QString::number(mSliderColor.blue()),
                                  QString::number(mColorGradient.red()),
                                  QString::number(mColorGradient.green()),
                                  QString::number(mColorGradient.blue()),
                                  QString::number(mHandleSize.width()),
                                  QString::number(handleMargin),
                                  QString::number(margin));
            break;
        }
        case ESliderType::image:
            stylesheet = QString("QSlider::sub-page:horizontal{ "
                                 " background-color:rgba(0,0,0,0);"
                                 "}"
                                 "QSlider::groove:horizontal{ "
                                 " border-image: url(%1) 0 0 0 0 stretch stretch;"
                                 " margin-top: %4px;"
                                 " margin-bottom: %4px;"
                                 " margin-left: 0px;"
                                 " margin-right: 0px;"
                                 " border-radius: 2px;"
                                 "}"
                                 "QSlider::handle:horizontal {"
                                 "width: %2px;"
                                 "margin-top: %3px;"
                                 "margin-bottom: %3px;"
                                 "}")
                             .arg(mPath,
                                  QString::number(mHandleSize.width()),
                                  QString::number(handleMargin),
                                  QString::number(margin));
            break;
    }
    setStyleSheet(stylesheet);
}

void Slider::receivedValue(int value) {
    if (isEnabled()) {
        blockSignals(true);
        setValue(value);
        blockSignals(false);
    }
}

int Slider::snapSliderToNearestTick(QSlider* slider, int pos) {
    if (slider->tickPosition() != QSlider::NoTicks) {
        int numberOfFullTicks = pos / slider->tickInterval();
        int leftTick = slider->minimum() + numberOfFullTicks * slider->tickInterval();
        int rightTick = slider->minimum() + (numberOfFullTicks + 1) * slider->tickInterval();
        if ((leftTick - pos) < (pos - rightTick)) {
            pos = rightTick;
        } else {
            pos = leftTick;
        }
    }
    return pos;
}

void Slider::mousePressEvent(QMouseEvent* event) {
    QStyleOptionSlider opt;
    initStyleOption(&opt);
    QRect sr = style()->subControlRect(QStyle::CC_Slider, &opt, QStyle::SC_SliderHandle, this);

    if (event->button() == Qt::LeftButton && sr.contains(event->pos()) == false) {
        int value = minimum() + ((maximum() - minimum()) * event->pos().x()) / width();

        blockSignals(true);
        setValue(value);
        blockSignals(false);
        emit valueChanged(value);
        event->accept();
    }

    QSlider::mousePressEvent(event);
}

void Slider::resizeEvent(QResizeEvent*) {
    resize();
}

void Slider::paintEvent(QPaintEvent* event) {
    if (tickPosition() != QSlider::NoTicks) {
        QStyleOption opt;
        opt.initFrom(this);
        QPainter painter(this);

        painter.setRenderHint(QPainter::Antialiasing);
        if (mShouldDrawTickLabels) {
            painter.setPen(QColor(255, 255, 255, 80));
        } else if (isEnabled()) {
            painter.setPen(QColor(255, 255, 255, 60));
        } else {
            painter.setPen(QColor(255, 255, 255, 10));
        }

        // draw tick marks
        // do this manually because they are very badly behaved with style sheets
        int interval = tickInterval();
        if (interval == 0) {
            interval = pageStep();
        }

        // get tick count
        QFontMetrics fontMetrics(painter.font());
        int currentStep = minimum();
        int pixelsPerStep = width() / interval;
        bool shouldSubtractOne = (minimum() == 1);
        bool isFirstLabel = true;

        int maximum = this->maximum();
        if (shouldSubtractOne) {
            maximum += 1;
        }
        for (int i = minimum(); i <= maximum; i += interval) {
            int x = i * interval / this->maximum() * pixelsPerStep;
            x = x + contentsMargins().left();

            if (mShouldDrawTickLabels) {
                int y = rect().bottom();
                // handle edge case that makes ugly labels...
                int labelValue = currentStep;
                if (shouldSubtractOne && !isFirstLabel) {
                    labelValue = labelValue - 1;
                }

                // create label and compute its offset
                QString label = QString::number(labelValue);
                int labelOffset = fontMetrics.boundingRect(label).width();

                if (isFirstLabel) {
                    isFirstLabel = false;
                    painter.drawText(x, y, label);
                } else {
                    painter.drawText(x - labelOffset / 2, y, label);
                }
            } else {
                if (tickPosition() == QSlider::TicksBothSides
                    || tickPosition() == QSlider::TicksAbove) {
                    int y = rect().top();
                    painter.drawLine(x, y, x, y * 4 / 5);
                }
                if (tickPosition() == QSlider::TicksBothSides
                    || tickPosition() == QSlider::TicksBelow) {
                    int y = rect().bottom();
                    painter.drawLine(x, y, x, y * 4 / 5);
                }
            }
            currentStep += interval;
        }
    }
    QSlider::paintEvent(event);
}


void Slider::setHeightPercentage(float percent) {
    mHeightScaleFactor = percent;
    adjustStylesheet();
}


void Slider::setSnapToNearestTick(bool shouldSnap) {
    mShouldSnap = shouldSnap;
}


void Slider::releasedSlider() {
    emit valueChanged(value());
}

void Slider::movedSlider(int value) {
    emit valueChanged(value);
}

void Slider::setShouldDrawTickLabels(bool shouldDraw) {
    if (mShouldDrawTickLabels != shouldDraw) {
        mShouldDrawTickLabels = shouldDraw;
        update();
    }
}

void Slider::enable(bool shouldEnable) {
    if (shouldEnable) {
        auto effect = new QGraphicsOpacityEffect(this);
        effect->setOpacity(1.0);
        setGraphicsEffect(effect);
        setEnabled(true);
    } else {
        auto effect = new QGraphicsOpacityEffect(this);
        effect->setOpacity(0.5);
        setGraphicsEffect(effect);
        setEnabled(false);
    }
}

} // namespace cor
