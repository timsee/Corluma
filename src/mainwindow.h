
#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QPushButton>

#include "lightingpage.h"
#include "icondata.h"
#include "lightsbutton.h"

namespace Ui {
class MainWindow;
}

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
     * \brief settingsPressed called whenever the settings button is pressed.
     */
    void settingsPressed();

    /*!
     * \brief updateMenuBar used to update the menu bar to app state changes.
     */
    void updateMenuBar();

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
     * \brief mPageButtons pointers to all the main buttons, used
     *        to iterate through them quickly.
     */
    std::shared_ptr<std::vector<LightsButton*> > mPageButtons;

    /*!
     * \brief mIconData used to generate the icons in the menu bar.
     */
    IconData mIconData;

    /*!
     * \brief mIsOn true if the LEDS are on, FALSE otherwise.
     * \todo remove isOn flag and use the systems built into the arduino API.
     */
    bool mIsOn;

};

#endif // MAINWINDOW_H
