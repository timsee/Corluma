#ifndef GLOBALSETTINGSWIDGET_H
#define GLOBALSETTINGSWIDGET_H

#include <QWidget>
#include <QPushButton>
#include <QLabel>
#include <QCheckBox>

#include "corlumaslider.h"
#include "corlumacheckbox.h"
#include "comm/commlayer.h"
#include "datalayer.h"
/*!
 * \copyright
 * Copyright (C) 2015 - 2017.
 * Released under the GNU General Public License.
 *
 *
 * \brief The GlobalSettingsWidget class is a widget for the settings page
 *        that contains all the advanced settings that don't fit into a button
 *        well. It contains things such as sliders and checkboxes that have a global
 *        effect on the application's state.
 */
class GlobalSettingsWidget : public QWidget
{
    Q_OBJECT
public:
    /// constructor
    explicit GlobalSettingsWidget(QWidget *parent = 0);

    /*!
     * \brief updateUI updates the colors of various settings in the UI.
     */
    void updateUI();

    /*!
     * \brief connectBackendLayers connec the commlayer to this page.
     * \param layer a pointer to the commlayer object.
     */
    void connectBackendLayers(CommLayer *commLayer, DataLayer *dataLayer) {
        mComm = commLayer;
        mData = dataLayer;
    }

    /*!
     * \brief checkBoxClicked helper for checking if a checkbox is checked. checkmate!
     * \param type the type of comm type you're checking
     * \param checked true if checked, false otherwise.
     */
    void checkBoxClicked(ECommType type, bool checked);

    /// called whenever the widget is shown
    void show();

    /// resizes the contents in the widget
    void resize();

public slots:

    /*!
     * \brief speedChanged signaled whenever the slider that controls
     *        the LEDs speed changes its value.
     */
    void speedChanged(int);

    /*!
     * \brief timeoutChanged signaled whenever the slider that controls
     *        the LEDs idle time out changes its value.
     */
    void timeoutChanged(int);

    /*!
     * \brief hueCheckboxClicked checks and unchecks the hue checkbox.
     */
    void hueCheckboxClicked(bool);


    /*!
     * \brief yunCheckboxClicked checks and unchecks the Yun checkbox.
     */
    void yunCheckboxClicked(bool);

    /*!
     * \brief serialCheckboxClicked checks and unchecks the serial checkbox.
     */
    void serialCheckboxClicked(bool);

    /*!
     * \brief advanceModeButtonPressed called when advance mode button is pressed
     * \param isChecked true if checked, false if not
     */
    void advanceModeButtonPressed(bool isChecked);

    /*!
     * \brief timeoutButtonPressed called when timeout button is pressed
     * \param isChecked true if checked, false if not
     */
    void timeoutButtonPressed(bool isChecked);

    /// value set for how long it takes the lights to timeout.
    int timeoutValue();

    /// value set for how quick color routines run.
    int speedValue();

protected:
    /*!
     * \brief resizeEvent called whenever the widget resizes so that assets can be updated.
     */
    void resizeEvent(QResizeEvent *event);

    /*!
     * \brief paintEvent used to draw the background of the widget.
     */
    void paintEvent(QPaintEvent *);

private:

    //----------------
    // Helper Functions
    //----------------
    /*!
     * \brief checkCheckBoxes checks the necessary check boxes based off of the data layer's
     *        representation of which commtypes are in use.
     */
    void checkCheckBoxes();

    /*!
     * \brief showTimeout shows/hides the timeout slider
     * \param showTimeout true to show the timeout slider, false to hide it
     */
    void showTimeout(bool showTimeout);

    /*!
     * \brief showAdvanceMode shows/hides the advance mode settings
     * \param showAdvanceMode true to show the advance mode slider and enabled connections
     *        list, false to hide them.
     */
    void showAdvanceMode(bool showAdvanceMode);

    /// value for minimum possible height of widget
    int mMinHeight;

    /// value for space between widgets
    int mSpacerPixels;

    /// value for minimum possible width of slider
    int mSliderMinWidth;

    //----------------
    // Labels
    //----------------

    /*!
     * \brief mEnabledConnectionsLabel label for section of widget that contains
     *        the enabled connections
     */
    QLabel *mEnabledConnectionsLabel;

    //----------------
    // Checkboxes
    //----------------

    /*!
     * \brief mAdvanceModeCheckBox checkbox for turning advanced mode on and off
     */
    CorlumaCheckBox *mAdvanceModeCheckBox;

    /*!
     * \brief mTimeoutCheckBox checkbox for turning timeouts on and off
     */
    CorlumaCheckBox *mTimeoutCheckBox;

    //----------------
    // Sliders
    //----------------

    /*!
     * \brief mTimeoutLabel label for timeout slider, says "Timeout."
     */
    QLabel *mTimeoutLabel;

    /*!
     * \brief mTimeoutSlider slider for determining how short/long timeouts
     *        will be.
     */
    CorlumaSlider *mTimeoutSlider;

    /*!
     * \brief mSpeedLabel label for speed slider, says "speed."
     */
    QLabel *mSpeedLabel;

    /*!
     * \brief mSpeedSlider slider for determing how fast routines will be
     *        on RGB-LED-Routines powered devices.
     */
    CorlumaSlider *mSpeedSlider;

    //----------------
    // Enabled Connections
    //----------------
    /*!
     * \brief mYunButton button to enable/disable yun devices
     */
    QPushButton *mYunButton;

    /*!
     * \brief mHueButton button to enable/disable hue devices.
     */
    QPushButton *mHueButton;

    /*!
     * \brief mSerialButton button to enable/disable serial devices
     */
    QPushButton *mSerialButton;

    //----------------
    // Stored Variables
    //----------------

    /*!
     * \brief mConnectionButtons pointers to all checkboxes so that they can be quickly iterated
     *        through.
     */
    std::vector<QPushButton*> mConnectionButtons;

    /*!
     * \brief communication pointer to communication object
     *        for sending comannds to the lights
     */
    CommLayer *mComm;

    /// pointer to data layer
    DataLayer *mData;

    /*!
     * \brief mSettings pointer to QSettings, used to store and access data in persistent app memory.
     */
    QSettings *mSettings;
};

#endif // GLOBALSETTINGSWIDGET_H
