#ifndef SPEEDSLIDER_H
#define SPEEDSLIDER_H

#include <QPainter>
#include <QStyleOption>
#include <QWidget>
#include "cor/protocols.h"
#include "cor/widgets/slider.h"

/*!
 * \copyright
 * Copyright (C) 2015 - 2021.
 * Released under the GNU General Public License.
 *
 *
 * \brief The SpeedSlider class contains a slider that determines the speed of routines.
 */
class SpeedSlider : public QWidget {
    Q_OBJECT
public:
    explicit SpeedSlider(QWidget* parent)
        : QWidget(parent),
          mLeftImage{new QLabel(this)},
          mRightImage{new QLabel(this)},
          mSpeedLabel{new QLabel("Speed:", this)},
          mSlider{new cor::Slider(this)} {
        mSpeedLabel->setAlignment(Qt::AlignVCenter | Qt::AlignHCenter);
        mSlider->setColor(QColor(255, 0, 0));
        mSlider->setRange(2, 200);
        mSlider->setValue(100);
        mSlider->setHeightPercentage(0.8);
        connect(mSlider, SIGNAL(valueChanged(int)), this, SLOT(speedSliderChanged(int)));
    }

    /// set the color for the speed slider
    void setColor(const QColor& color) { mSlider->setColor(color); }

    /// enable or disable the speed slider based off of the state of the RoutineContainer
    void enable(ERoutine routine, int /*param*/) {
        if (routine == ERoutine::singleSolid) {
            mStoredValue = mSlider->value();
            mSlider->setValue(0);
            mSlider->setEnabled(false);
        } else {
            mSlider->setValue(mStoredValue);
            mSlider->setEnabled(true);
        }
    }

    /// update the speed slider
    void updateSpeed(int speed) {
        qDebug() << "TODO: update speed from " << mSlider->value() << " to " << speed;
    }

    /// getter for value
    int value() {
        if (mSlider->isEnabled()) {
            return mSlider->value();
        } else {
            return mStoredValue;
        }
    }

signals:

    /// signal when the slider changes.
    void valueChanged(int);

protected:
    /*!
     * \brief resizeEvent called every time the main window is resized.
     */
    void resizeEvent(QResizeEvent*) override { resize(); }

    void paintEvent(QPaintEvent*) override {
        QStyleOption opt;
        opt.initFrom(this);
        QPainter painter(this);

        painter.setRenderHint(QPainter::Antialiasing);
        painter.fillRect(rect(), QBrush(QColor(48, 47, 47)));
    }

private slots:

    /// handles when the time slder changes.
    void speedSliderChanged(int sliderValue) { emit valueChanged(sliderValue); }

private:
    /// handle when it resizes
    void resize() {
        auto sliderXSpacing = width() / 15;
        auto sliderYSpacing = height() / 20;

        auto sliderImageSize = width() / 9;
        auto sliderWidth = width() - sliderImageSize * 2 - sliderXSpacing * 3;
        auto xPos = sliderXSpacing;

        //    mSpeedLabel->setGeometry(sliderXSpacing,
        //                             0,
        //                             width() - sliderXSpacing * 2,
        //                             mSpeedLabel->height());
        mLeftImage->setGeometry(xPos, sliderYSpacing, sliderImageSize, height());
        xPos += mLeftImage->width() + sliderXSpacing;
        mSlider->setGeometry(xPos, sliderYSpacing, sliderWidth, height());
        xPos += mSlider->width() + sliderXSpacing;
        mRightImage->setGeometry(xPos, sliderYSpacing, sliderImageSize, height());
    }

    /// image to the left of the slider
    QLabel* mLeftImage;

    /// image to the right of the slider
    QLabel* mRightImage;

    /// label for the sldier
    QLabel* mSpeedLabel;

    /// slider to change the speed.
    cor::Slider* mSlider;

    /// stored value for the slider.
    std::uint32_t mStoredValue;
};

#endif // SPEEDSLIDER_H
