#ifndef SINGLELIGHTBRIGHTNESSWIDGET_H
#define SINGLELIGHTBRIGHTNESSWIDGET_H

#include "cor/widgets/slider.h"

#include <QWidget>

/*!
 * \copyright
 * Copyright (C) 2015 - 2020.
 * Released under the GNU General Public License.
 *
 *
 * \brief The SingleLightBrightnessWidget controls the brightness of a single light. This replaces
 * the global brightness widget on widgets where only one light is selected at time. It shows the
 * light's current color on the left, and then provides a slider for changing hwo bright it is.
 */
class SingleLightBrightnessWidget : public QWidget {
    Q_OBJECT

public:
    /// constructor
    explicit SingleLightBrightnessWidget(const QSize& size, QWidget* parent);

    /// update state of widget
    void updateColor(const QColor& color);

    /// updates the brightness of the widget
    void updateBrightness(std::uint32_t brightness);

    /// getter for brightness
    int brightness() { return mBrightnessSlider->value(); }

    /// true if in, false otherwise
    bool isIn() { return mIsIn; }

    /// push in the widget
    void pushIn(const QPoint&);

    /// push out the widget
    void pushOut(const QPoint&);

signals:
    /// the new value of the brightness slider
    void brightnessChanged(std::uint32_t newValue);

private slots:
    /*!
     * \brief brightnessSliderChanged Connected to the the slider at the top, this takeas a value
     * between 0-100 and sends that value to the lights to control how bright they are.
     */
    void brightnessSliderChanged(int);

protected:
    /*!
     * \brief paintEvent paints the background of the widget
     */
    void paintEvent(QPaintEvent* event);

    /// handles when the widget resizes.
    void resizeEvent(QResizeEvent*);

private:
    /// true if in, false if out
    bool mIsIn;

    /// size used for determining widget size
    QSize mSize;

    /*!
     * \brief mBrightnessSlider slider for adjusting the brightness of all selected devices.
     */
    cor::Slider* mBrightnessSlider;
};

#endif // SINGLELIGHTBRIGHTNESSWIDGET_H
