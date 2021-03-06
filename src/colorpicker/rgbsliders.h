#ifndef RGBSLIDERS_H
#define RGBSLIDERS_H

#include <QLineEdit>
#include <QWidget>

#include "cor/widgets/slider.h"

/*!
 * \copyright
 * Copyright (C) 2015 - 2020.
 * Released under the GNU General Public License.
 *
 *
 * \brief The RGBSliders class is a set of 3 sliders, one representing red values, one representing
 * green values, and one representing blue values.These sliders are used by the ColorPicker during
 * its standard layout to give a second option and represnetation of the RGB color being chosen by
 * the ColorPicker.
 */
class RGBSliders : public QWidget {
    Q_OBJECT
public:
    /// constructor
    explicit RGBSliders(QWidget* parent);

    /*!
     * \brief changeColor programmatically change the values of the sliders. Does not emit a signal
     * with its new value \param color new color for the sliders.
     */
    void changeColor(const QColor& color);

    /// enables and disables the RGB sliders
    void enable(bool enable);

    /// getter for current color
    QColor color();

    /// getter for overall "brightness" of the color.
    std::uint32_t brightness();

signals:

    /*!
     * \brief colorChanged emitted whenever a slider changes its values. Emits the full color
     * representation of all 3 sliders \param color the color representation of all three sliders
     */
    void colorChanged(QColor color);

protected:
    /// called when the widget resizes, handles sizing the labels and sliders
    void resizeEvent(QResizeEvent*);

private slots:

    /*!
     * \brief redSliderChanged called whenever the red slider changes its value.
     */
    void redSliderChanged(int);

    /*!
     * \brief greenSliderChanged called whenever the green slider changes its value.
     */
    void greenSliderChanged(int);

    /*!
     * \brief blueSliderChanged called whenever the blue slider changes its value.
     */
    void blueSliderChanged(int);

    /// handles when the line edit changes for red.
    void lineRedEditChanged(QString);

    /// handles when the line edit changes for green.
    void lineGreenEditChanged(QString);

    /// handles when the line edit changes for blue.
    void lineBlueEditChanged(QString);

    /*!
     * \brief releasedSlider uses the QSlider inside of the LightsSlider to pick up
     *        when the slider is released. This always sets the color of the color picker.
     *        This system is used to prevent an edge case with throttling with a timer.
     *        Without it, its possible to change the UI without updating the lights if you are
     *        quick enough.
     */
    void releasedSlider();

private:
    /// handle the text input for a line edit, makes sure the value is between 0 and 255.
    int handleTextInputString(QLineEdit* lineEdit, QString input);

    /*!
     * \brief mRLabel puts that little "R" in front of the slider.
     */
    QLabel* mRLabel;
    /*!
     * \brief mGLabel puts that little "G" in front of the slider.
     */
    QLabel* mGLabel;
    /*!
     * \brief mBLabel puts that little "B" in front of the slider.
     */
    QLabel* mBLabel;

    /// line edit for adding specific values for red.
    QLineEdit* mRLineEdit;

    /// line edit for adding specific values for green.
    QLineEdit* mGLineEdit;

    /// line edit for adding specific values for blue.
    QLineEdit* mBLineEdit;

    /*!
     * \brief mRedSlider top slider. Used for choosing amount of red in color.
     */
    cor::Slider* mRedSlider;

    /*!
     * \brief mGreenSlider middle slider. Used for choosing amount of green in color.
     */
    cor::Slider* mGreenSlider;

    /*!
     * \brief mBlueSlider bottom slider. Used for choosing amount of blue in color.
     */
    cor::Slider* mBlueSlider;
};

#endif // RGBSLIDERS_H
