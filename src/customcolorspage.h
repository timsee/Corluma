

#ifndef CUSTOMARRAYPAGE_H
#define CUSTOMARRAYPAGE_H

#include "lightingpage.h"
#include "icondata.h"
#include "lightsbutton.h"

#include <QWidget>
#include <QDebug>
#include <QToolButton>
#include <QPushButton>
#include <QGridLayout>

namespace Ui {
class CustomColorsPage;
}

/*!
 * \copyright
 * Copyright (C) 2015 - 2016.
 * Released under the GNU General Public License.
 *
 * \brief The CustomColorsPage provides a way to use an array of custom colors
 *        to do multi color routines.
 *
 * The page contains an interface to change the individual array colors and
 * a series of buttons that change the mode.
 *
 * The top interface is split into three parts: a ColorPicker, a color array
 * preview, and a LightsSlider. The ColorPicker chooses the color that is
 * currently selected from the color array preview. The LightsSlider below the
 * preview is used to choose how many colors are active while using an array mode.
 */
class CustomColorsPage : public QWidget, public LightingPage
{
    Q_OBJECT

public:
    /*!
     * \brief Constructor
     */
    explicit CustomColorsPage(QWidget *parent = 0);
    /*!
     * \brief Deconstructor
     */
    ~CustomColorsPage();

    /*!
     * \brief highlightModeButton highlights the button that implements
     *        the lightingMode parameter. If it can't find a button that
     *        implements ELightingRoutine, then all buttons are unhighlighted
     * \param routine the routine that the highlighted button implements.
     */
    void highlightRoutineButton(ELightingRoutine routine);

    /*!
     * \brief setupButtons sets up the routine buttons. Requires the DataLayer
     *        of the application to be set up first.
     */
    void setupButtons();

signals:
    /*!
     * \brief used to signal back to the main page that it should update its top-left icon
     *        with a new color mode
     */
    void updateMainIcons();

public slots:
    /*!
     * \brief modeChanged called whenever a mode button is pressed.
     */
    void modeChanged(int);
    /*!
     * \brief colorsUsedChanged called whenever the colors used slider
     *        (the slider above the mode buttons) changes its value.
     */
    void customColorCountChanged(int);
    /*!
     * \brief colorChanged called whenever the color picker is used.
     */
    void colorChanged(QColor);
    /*!
     * \brief selectArrayColor called whenever a color array icon
     *        is clicked. This sets the that index of the color array
     *        as the color you can change with the color picker.
     */
    void selectArrayColor(int);
    /*!
     * \brief routineButtonClicked whenever a button is clicked, this signal
     *        is called. The first argument is the routine itself, the second
     *        is the color group. For this specific page, the color group will
     *        always be the custom color group, since its the custom color page.
     */
    void routineButtonClicked(int, int);

protected:
    /*!
     * \brief showEvent used to check for edge cases and to sync up the
     *        page to recent state changes before displaying the page
     */
    void showEvent(QShowEvent *);

    /*!
     * \brief resizeEvent used to update the size of the custom color picker slider
     *        and buttons.
     */
    void resizeEvent(QResizeEvent *);

private:
    /*!
     * \brief ui pointer to Qt UI form.
     */
    Ui::CustomColorsPage *ui;

    /*!
     * \brief mArrayColorsButtons buttons for changing the lighting mode.
     */
    std::shared_ptr<std::vector<QPushButton *> > mArrayColorsButtons;

    /*!
     * \brief IconData for the Single Color Routine Icons
     */
    IconData mIconData;

    /*!
     * \brief mGreyIcon used for greying out icons
     */
    IconData mGreyIcon;

    /*!
     * \brief mRoutineButtons pointers to all the routine changes that can
     *        use the custom color array.
     */
    std::shared_ptr<std::vector<LightsButton *> > mRoutineButtons;

    /*!
     * \brief mCurrentColorPickerIndex current index being set by the GUI.
     */
    int mCurrentColorPickerIndex;

    /*!
     * \brief mCustomArraySize size of the custom color array, used to initialize
     *        and access vectors throughout this page.
     */
    int mCustomArraySize;

    /*!
     * \brief updateColorArray called whenever the color array experiences
     *        a change, from the user interacting with the colorPicker
     *        or from the user interacting with the colors used array
     */
    void updateColorArray();

    /*!
     * \brief updateIcons called whenever the colors getting used change
     *        so that the icons can sync up to the new color values.
     */
    void updateIcons();
};

#endif // CUSTOMARRAYPAGE_H
