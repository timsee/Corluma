
#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QPushButton>

#include "lightingpage.h"
#include "icondata.h"
#include "lightsbutton.h"
#include "floatinglayout.h"
#include "commtypesettings.h"
#include "commlayer.h"
#include "datasync.h"

namespace Ui {
class MainWindow;
}

/*!
 * \brief The EPage enum The main pages of the application, as they are ordered
 *        in their QStackedWidget.
 */
enum class EPage {
    eSinglePage,
    eHueSinglePage,
    eCustomArrayPage,
    ePresetPage,
    eSettingsPage,
    eHueSettingsPage,
    eConnectionPage
};

/*!
 * \copyright
 * Copyright (C) 2015 - 2016.
 * Released under the GNU General Public License.
 *
 * \brief The MainWindow class is the only window in this application.
 * It contains a QStackedWidget which shows 4 pages: SingleColorPage, CustomColorsPage,
 * PresetColorsPage, and the SettingsPage.
 *
 * On the top of the MainWindow, there is an on/off button in the far left. This button
 * will reflect the current mode when on, and will be black when off. There is
 * also a slider to control the overall brightness of the LEDs. 
 *
 */
class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    /*!
     * \brief Constructor
     */
    explicit MainWindow(QWidget *parent = 0);

    /*!
     * \brief Deconstructor
     */
    ~MainWindow();

public slots:
    /*!
     * \brief toggleOnOff Connected to the button in the top left of the GUI at all times.
     *        Toggles between running the current routine at current settings, and off.
     */
    void toggleOnOff();

    /*!
     * \brief brightnessChanged Connected to the the slider at the top, this takeas a value between 0-100
     *        and sends that value to the lights to control how bright they are.
     */
    void brightnessChanged(int);

    /*!
     * \brief pageChanged Connected to the main menu buttons. Each button sends a different int,
     *        which opens a different page on the QStackedWidget.
     */
    void pageChanged(int);

    /*!
     * \brief connectionButtonPressed called whenever the connection button is pressed.
     */
    void connectionButtonPressed();

    /*!
     * \brief settingsPressed called whenever the settings button is pressed.
     */
    void settingsButtonPressed();

    /*!
     * \brief updateMenuBar used to update the menu bar to app state changes.
     */
    void updateMenuBar();

    /*!
     * \brief updateSingleColor updates the icon for the single color page and the menu bar and on/off button
     *         based off of the color emitted.
     */
    void updateSingleColor(QColor);

    /*!
     * \brief updatePresetColorGroup updates the icon for the preset color page and the menu bar and on/off button
     *        based off of the two integers provided. The first integer should be cast to a ELightingRoutine and the
     *        second should be cast to a EColorGroup.
     */
    void updatePresetColorGroup(int, int);

    /*!
     * \brief floatingLayoutButtonPressed handles whenever the floating layout has a button pressed. Parses the string
     *        that the layout emits, then updates the UI accordingly.
     */
    void floatingLayoutButtonPressed(QString);

    /*!
     * \brief deviceCountChangedOnConnectionPage handles the case when the device count changes.
     */
    void deviceCountChangedOnConnectionPage();

    /*!
     * \brief deviceCountChangedOnConnectionPage handles the case when the device count reaches zero. This gets signaled
     *        from the data layer whereas deviceCOuntChangedOnConnectionPage gets signaled from the connection page.
     */
    void deviceCountReachedZero();

    /*!
     * \brief defaultSettingsPageChanged handles whenever the default settings page changes from standard to hue or vice versa.
     * \param newPage true if should use the standard settings page, false if using the hue page.
     */
    void defaultSettingsPageChanged(bool newPage);

protected:
    /*!
     * \brief paintEvent called whenever there is a paint update. This is used
     *        to draw the dark grey background, since using the stylesheet
     *        for backgrounds makes some lesser features of GUI elements break.
     *
     * \param event event triggered that requires a repaint
     */
    void paintEvent(QPaintEvent *event);

    /*!
     * \brief resizeEvent called whenever the window resizes. This is used to override
     *        the resizing of the icons of the menu bar.
     */
    virtual void resizeEvent(QResizeEvent *);

    /*!
     * \brief changeEvent called whenever an event occurs. Currently used to off communication
     *        threads when the application is put to sleep.
     * \param event the event that has ocurred.
     */
    void changeEvent(QEvent *event);

private:

    /*!
     * \brief ui pointer to Qt UI form.
     */
    Ui::MainWindow *ui;

    /*!
     * \brief communication pointer to communication object
     *        for sending comannds to the lights
     */
    CommLayer *mComm;

    /*!
     * \brief data layer that maintains and tracks the states of the lights
     *        and the saved data of the GUI
     */
    DataLayer *mData;

    /*!
     * \brief mDataSync Data Sync is a thread that gets turned on whenever the DataLayer's devices don't
     *        match the commLayer's representation of the devices. It continually sends packets through
     *        the comm layer until it receives acknowledgements that the commlayer and the datalayer
     *        match exactly.
     */
    DataSync *mDataSync;

    /*!
     * \brief mFloatingLayout a floating layout that provides an array of buttons on certain pages.
     *        It floats near the top right of the page and is managed automatically through resizeEvents
     *        as it does not correspond to the rest of the applications layout.
     */
    FloatingLayout *mFloatingLayout;

    /*!
     * \brief mIconData used to generate the icons in the menu bar.
     */
    IconData mIconData;

    /*!
     * \brief mShouldGreyOutIcons cahced satte of whether any device is selected. If none
     *        are selected, icons that require a device are all greyed out.
     */
    bool mShouldGreyOutIcons;

    /*!
     * \brief mLastPageIsMultiColor true if the second page is currently multiple colors,
     *        false if its a single color page.
     */
    bool mLastPageIsMultiColor;

    /*!
     * \brief mUseStandardSettings true if the standard settings page should be used, false otherwise.
     */
    bool mUseStandardSettings;
};

#endif // MAINWINDOW_H
