
#ifndef SETTINGSPAGE_H
#define SETTINGSPAGE_H

#include <QWidget>
#include <QListWidgetItem>

#include "lightingpage.h"
#include "lightsslider.h"
#include "lightcheckbox.h"
#include "commlayer.h"

namespace Ui {
class SettingsPage;
}

/*!
 * \copyright
 * Copyright (C) 2015 - 2017.
 * Released under the GNU General Public License.
 *
 *
 * \brief The SettingsPage provides a way to configure the
 * application settings.
 *
 * It provides the ability to change the speed the LEDs update
 * and the amount of minutes it takes for the LEDs to timeout.
 *
 * It also provides a way to switch between different communication types.
 * It currently support Serial, HTTP, UDP, and Hue lights. This interface
 * automatically populates serial and Hue connections, but allows users to
 * add and remove connections for UDP and HTTP.
 *
 */
class SettingsPage : public QWidget, public LightingPage
{
    Q_OBJECT

public:
    /*!
     * \brief Constructor
     */
    explicit SettingsPage(QWidget *parent = 0);

    /*!
     * \brief Deconstructor
     */
    ~SettingsPage();

    /*!
     * \brief setupUI sets up initial UI elements.
     */
    void setupUI();

    /*!
     * \brief connectCommLayer connec the commlayer to this page.
     * \param layer a pointer to the commlayer object.
     */
    void connectCommLayer(CommLayer *layer) { mComm = layer; }

    /*!
     * \brief updateUI updates the colors of various settings in the UI.
     */
    void updateUI();

signals:
    /*!
     * \brief Used to signal back to the main page that it should update its top-left icon
     *        with new RGB values
     */
    void updateMainIcons();

    /*!
     * \brief settingsPageIsStandard signaled when top menu is pressed. True if settings page should
     *        show standard settings, false if showing hue-specific settings.
     */
    void settingsPageIsStandard(bool);

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
     * \brief httpCheckboxClicked checks and unchecks the HTTP checkbox.
     */
    void httpCheckboxClicked(bool);

    /*!
     * \brief udpCheckboxClicked checks and unchecks the UDP checkbox.
     */
    void udpCheckboxClicked(bool);

    /*!
     * \brief serialCheckboxClicked checks and unchecks the serial checkbox.
     */
    void serialCheckboxClicked(bool);

    /*!
     * \brief deviceCountReachedZero disable and greys out assets that require a device to be
     *        connected to use.
     */
    void deviceCountReachedZero();

private slots:
    /*!
     * \brief renderUI renders expensive assets if and only if the assets have had any
     *        change of state.
     */
    void renderUI();

protected:
    /*!
     * \brief showEvent called before the this page is shown. Used to sync up
     *        any changes that may have happened on other pages.
     */
    void showEvent(QShowEvent *);

    /*!
     * \brief hideEvent called whenever the page is being hidden.
     */
    void hideEvent(QHideEvent *);

    /*!
     * \brief resizeEvent called whenever the widget resizes so that assets can be updated.
     */
    void resizeEvent(QResizeEvent *event);

private:

    /*!
     * \brief checkBoxClicked helper for checking if a checkbox is checked. checkmate!
     * \param type the type of comm type you're checking
     * \param checked true if checked, false otherwise.
     */
    void checkBoxClicked(ECommType type, bool checked);

    /*!
     * \brief checkCheckBoxes checks the necessary check boxes based off of the data layer's
     *        representation of which commtypes are in use.
     */
    void checkCheckBoxes();

    /*!
     * \brief ui pointer to Qt UI form.
     */
    Ui::SettingsPage *ui;

    /*!
     * \brief mCheckBoxes pointers to all checkboxes so that they can be quickly iterated
     *        through.
     */
    std::vector<LightCheckBox*> mCheckBoxes;

    /*!
     * \brief communication pointer to communication object
     *        for sending comannds to the lights
     */
    CommLayer *mComm;

    /*!
     * \brief mSliderSpeedValue storage for the current slider value, which
     *        differs from the actual slider speed saved in the data layer.
     */
    int mSliderSpeedValue;
};

#endif // SETTINGSPAGE_H
