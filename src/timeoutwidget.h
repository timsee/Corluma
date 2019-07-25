#ifndef TIMEOUTWIDGET_H
#define TIMEOUTWIDGET_H

#include <QCheckBox>
#include <QLabel>

#include "cor/widgets/checkbox.h"
#include "cor/widgets/slider.h"

/// handles state of timeout widget
enum class ETimeoutMode { minutes, hours };

/*!
 * \brief The TimeoutWidget class provides a widget that controls whether or Timeout Mode is on or
 * off. If it is on, _X_ minutes after the last packet is sent the lights turn off. If its off,
 * lights stay on indefinitely. This widget also provides an interface to choose how many minutes a
 * light should stay on.
 */
class TimeoutWidget : public QWidget {
    Q_OBJECT
public:
    /// constructor
    explicit TimeoutWidget(QWidget* parent,
                           int defaultTimeoutValue,
                           bool timeoutEnabled,
                           int spacerPixels);

    /// true to show, false to hide
    void show(bool shouldShow);

    /// resizes the contents in the widget
    int resize(int widgetHeight);

    /// getter for timeout value
    int timeoutValue();

    /// true if timeout is enabled, false otherwise
    bool isTimoutEnabled() { return mCheckBox->checked(); }

    /// changes the text of the timeout label
    void changeTimeoutLabel(const QString& title);

private slots:
    /// handles when minutes button is clicked
    void minutesClicked(bool);

    /// handles when the hours button is clicked
    void hoursClicked(bool);

private:
    /// changes the timeout mode
    void handleMode(ETimeoutMode newMode);

    /// stores the current mode
    ETimeoutMode mMode;

    /// spacer used for UI assets
    int mSpacerPixels;

    /// height of widget
    int mLastWidgetHeight;

    //----------------
    // Checkboxes
    //----------------

    /*!
     * \brief mCheckBox checkbox for turning timeouts on and off
     */
    cor::CheckBox* mCheckBox;

    //----------------
    // Sliders
    //----------------

    /*!
     * \brief mSlider slider for determining how short/long timeouts
     *        will be.
     */
    cor::Slider* mSlider;

    /// button for setting the widget into minutes mode
    QPushButton* mMinutesButton;

    /// button for setting the widget into hours mode
    QPushButton* mHoursButton;
};

#endif // TIMEOUTWIDGET_H
