#ifndef GLOBALSETTINGSWIDGET_H
#define GLOBALSETTINGSWIDGET_H

#include <QWidget>
#include <QPushButton>
#include <QLabel>
#include <QCheckBox>

#include "cor/slider.h"
#include "cor/checkbox.h"
#include "comm/commlayer.h"
#include "datalayer.h"
/*!
 * \copyright
 * Copyright (C) 2015 - 2018.
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
    explicit GlobalSettingsWidget(QWidget *parent, CommLayer *comm, DataLayer *data, ProtocolSettings *protocols);

    /*!
     * \brief updateUI updates the colors of various settings in the UI.
     */
    void updateUI();

    /*!
     * \brief checkBoxClicked helper for checking if a checkbox is checked. checkmate!
     * \param type the type of comm type you're checking
     * \param checked true if checked, false otherwise.
     */
    void checkBoxClicked(EProtocolType type, bool checked);

    /// called whenever the widget is shown
    void show();

    /// resizes the contents in the widget
    void resize();

public slots:

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
     * \brief arduCorButtonClicked checks and unchecks the ArduCor checkbox.
     */
    void arduCorButtonClicked(bool);

    /*!
     * \brief nanoLeafButtonClicked checks and unchecks the NanoLeaf checkbox.
     */
    void nanoLeafButtonClicked(bool);

    /*!
     * \brief timeoutButtonPressed called when timeout button is pressed
     * \param isChecked true if checked, false if not
     */
    void timeoutButtonPressed(bool isChecked);

    /// value set for how long it takes the lights to timeout.
    int timeoutValue();

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
     * \brief mTimeoutCheckBox checkbox for turning timeouts on and off
     */
    cor::CheckBox *mTimeoutCheckBox;

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
    cor::Slider *mTimeoutSlider;

    //----------------
    // Enabled Connections
    //----------------
    /*!
     * \brief mArduCorButton button to enable/disable yun devices
     */
    QPushButton *mArduCorButton;

    /*!
     * \brief mHueButton button to enable/disable hue devices.
     */
    QPushButton *mHueButton;

    /*!
     * \brief mNanoLeafButton button to enable/disable hue devices.
     */
    QPushButton *mNanoLeafButton;

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

    /// pointer to the app states that determine if a protocol (such as arducor or nanoleaf) is currently enabled
    ProtocolSettings *mProtocolSettings;
};

#endif // GLOBALSETTINGSWIDGET_H
