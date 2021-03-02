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
          mSpeedLabel{new QLabel("", this)},
          mSlider{new cor::Slider(this)} {
        mSpeedLabel->setAlignment(Qt::AlignVCenter | Qt::AlignHCenter);
        mSlider->setColor(QColor(255, 0, 0));
        mSlider->setRange(2, 200);
        mSlider->setValue(100);
        mStoredValue = 100;
        mSlider->setHeightPercentage(0.8);
        connect(mSlider, SIGNAL(valueChanged(int)), this, SLOT(speedSliderChanged(int)));
    }

    /// set the color for the speed slider
    void setColor(const QColor& color) { mSlider->setColor(color); }

    /// enable or disable the speed slider based off of the state of the RoutineContainer
    void enable(ERoutine routine, int /*param*/) {
        bool blocked = mSlider->blockSignals(true);
        if (routine == ERoutine::singleSolid) {
            setVisible(false);
        } else {
            setVisible(true);
        }
        mSlider->blockSignals(blocked);
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
    void speedSliderChanged(int sliderValue) {
        mStoredValue = sliderValue;
        emit valueChanged(sliderValue);
    }

private:
    /// handle when it resizes
    void resize() {
        auto sliderXSpacing = width() / 15;
        auto sliderYSpacing = height() / 20;

        auto sliderImageSize = width() / 9;
        auto sliderWidth = width() - sliderImageSize * 2 - sliderXSpacing * 3;
        auto xPos = (width() - sliderWidth) / 2;
        auto side = std::min(sliderImageSize, height());

        // first put the slider in the middle
        mSlider->setGeometry(xPos, sliderYSpacing, sliderWidth, height());

        // now put the left image directly left
        mLeftImage->setGeometry(xPos - side, sliderYSpacing, side, side);

        // now put the right image directly right
        mRightImage->setGeometry(xPos + mSlider->width(),
                                 sliderYSpacing,
                                 sliderImageSize,
                                 height());

        addIconTolabel(mLeftImage, ":/images/slow_icon.png", side);
        addIconTolabel(mRightImage, ":/images/fast_icon.png", side);
    }

    /// adds icon to label
    void addIconTolabel(QLabel* label, const QString& path, int side) {
        auto pixmap = QPixmap(path);
        pixmap = pixmap.scaled(side, side, Qt::KeepAspectRatio, Qt::SmoothTransformation);
        label->setPixmap(pixmap);
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
