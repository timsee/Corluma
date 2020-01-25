#ifndef GLOBALSETTINGSWIDGET_H
#define GLOBALSETTINGSWIDGET_H

#include <QCheckBox>
#include <QLabel>
#include <QPushButton>
#include <QWidget>

#include "appsettings.h"
#include "cor/widgets/checkbox.h"
#include "cor/widgets/slider.h"
#include "groupdata.h"
#include "timeoutwidget.h"

/*!
 * \copyright
 * Copyright (C) 2015 - 2020.
 * Released under the GNU General Public License.
 *
 *
 * \brief The GlobalSettingsWidget class is a widget for the settings page
 *        that contains all the advanced settings that don't fit into a button
 *        well. It contains things such as sliders and checkboxes that have a global
 *        effect on the application's state.
 */
class GlobalSettingsWidget : public QWidget {
    Q_OBJECT
public:
    /// constructor
    explicit GlobalSettingsWidget(QWidget* parent, AppSettings* appSettings);

    /*!
     * \brief updateUI updates the colors of various settings in the UI.
     */
    void updateUI();

    /// true if timeout is enabled, false if not
    bool useTimeout() { return mTimeoutWidget->isTimoutEnabled(); }

    /*!
     * \brief checkBoxClicked helper for checking if a checkbox is checked. checkmate!
     *
     * \param type the type of comm type you're checking
     * \param checked true if checked, false otherwise.
     */
    void checkBoxClicked(EProtocolType type, bool checked);

    /// called whenever the widget is shown
    void show();

    /// resizes the contents in the widget
    void resize();

    /// true if timeout should be hidden, false otherwise
    void hideTimeout(bool shouldHide) { mHideTimeout = shouldHide; }

signals:

    /// emitted when a protocol is turned off and on.
    void protocolSettingsUpdate(EProtocolType, bool);

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
     *
     * \param isChecked true if checked, false if not
     */
    void timeoutCheckboxPressed(bool isChecked);

    /// value set for how long it takes the lights to timeout.
    int timeoutValue();

protected:
    /*!
     * \brief paintEvent used to draw the background of the widget.
     */
    void paintEvent(QPaintEvent*);

private:
    //----------------
    // Helper Functions
    //----------------
    /*!
     * \brief checkCheckBoxes checks the necessary check boxes based off of the data layer's
     *        representation of which commtypes are in use.
     */
    void checkCheckBoxes();

    /// value for minimum possible height of widget
    int mMinHeight;

    /// value for space between widgets
    int mSpacerPixels;

    //----------------
    // Labels
    //----------------

    /*!
     * \brief mEnabledConnectionsLabel label for section of widget that contains
     *        the enabled connections
     */
    QLabel* mEnabledConnectionsLabel;

    /// widget for controlling the timeout mode
    TimeoutWidget* mTimeoutWidget;

    //----------------
    // Enabled Connections
    //----------------
    /*!
     * \brief mArduCorButton button to enable/disable yun devices
     */
    QPushButton* mArduCorButton;

    /*!
     * \brief mHueButton button to enable/disable hue devices.
     */
    QPushButton* mHueButton;

    /*!
     * \brief mNanoLeafButton button to enable/disable hue devices.
     */
    QPushButton* mNanoLeafButton;

    //----------------
    // Stored Variables
    //----------------

    /*!
     * \brief mConnectionButtons pointers to all checkboxes so that they can be quickly iterated
     *        through.
     */
    std::vector<QPushButton*> mConnectionButtons;

    /// pointer to the app states that determine if a protocol (such as arducor or nanoleaf) is
    /// currently enabled
    AppSettings* mAppSettings;

    /// true if timeout should be hidden, false otherwise
    bool mHideTimeout;
};

#endif // GLOBALSETTINGSWIDGET_H
