
#ifndef PresetColorsPage_H
#define PresetColorsPage_H

#include "lightingpage.h"
#include "cor/button.h"
#include "presetgroupwidget.h"
#include "cor/listwidget.h"
#include "listmoodgroupwidget.h"

#include <QWidget>
#include <QToolButton>
#include <QGridLayout>
#include <QLabel>
#include <QPushButton>
#include <QScrollArea>

/// mode of the page
enum class EGroupMode {
    eArduinoPresets,
    eHuePresets
};

/*!
 * \copyright
 * Copyright (C) 2015 - 2016.
 * Released under the GNU General Public License.
 *
 * \brief The GroupPage provides a way to use the Color Presets
 *        from themed groups of colors to do Multi Color Routines.
 *
 * It contains a grid of buttons that map color presets to lighting
 * modes. The list expands vertically into a QScrollArea.
 *
 */
class GroupPage : public QWidget, public LightingPage
{
    Q_OBJECT

public:
    /*!
     * \brief Constructor
     */
    explicit GroupPage(QWidget *parent = 0);

    /*!
     * \brief Deconstructor
     */
    ~GroupPage();

    /*!
     * \brief highlightRoutineButton highlights the button that implements
     *        the routine parameter. If it can't find a button that
     *        implements this lighting routine, then all buttons are unhighlighted
     * \param routine the lighting routine the highlighted button implements.
     * \param palette the color group that the highlighted button implements.
     */
    void highlightRoutineButton(ERoutine routine, EPalette palette);

    /*!
     * \brief setupButtons sets up the routine buttons. Requires the DataLayer
     *        of the application to be set up first.
     */
    void setupButtons();

    /// called whenever the group page is shown
    void show();

    /// getter for current mode of page
    EGroupMode mode() { return mMode; }

    /// programmatically set the mode of the page
    void setMode(EGroupMode mode);

    /*!
     * show the preset greset group widgets, but show the version
     * with less features designed for selecting hue colors.
     */
    void showPresetHueGroups();

    /*!
     * \brief connectCommLayer connec the commlayer to this page.
     * \param layer a pointer to the commlayer object.
     */
    void connectCommLayer(CommLayer *layer);

    /// called to programmatically resize the widget
    void resize();

signals:

    /*!
     * \brief changedDeviceCount signaled to UI assets whenever a click on the page results in changing
     *        the number of devices connected.
     */
    void changedDeviceCount();

    /*!
     * \brief used to signal back to the main page that it should update its
     *        top-left icon with a new color mode
     */
    void updateMainIcons();

    /*!
     * \brief presetPaletteChanged emits data to the MainWindow about the changes
     *        to the preset color page.
     */
    void presetPaletteChanged(EPalette);

public slots:

    /*!
     * \brief multiButtonClicked every button setup as a presetButton will signal
     *        this slot whenever they are clicked.
     */
    void multiButtonClicked(QJsonObject);

private slots:
    /*!
     * \brief renderUI renders expensive assets if and only if the assets have had any
     *        change of state.
     */
    void renderUI();

    /*!
     * \brief speedChanged signaled whenever the slider that controls
     *        the LEDs speed changes its value.
     */
    void speedChanged(int);

protected:
    /*!
     * \brief called whenever the page is shown on screen.
     */
    void showEvent(QShowEvent *);

    /*!
     * \brief hideEvent called whenever the page is changed and this page is hidden
     *        from the screen.
     */
    void hideEvent(QHideEvent *);

    /*!
     * \brief resizeEvent called every time the main window is resized.
     */
    void resizeEvent(QResizeEvent *);

private:

    /*!
     * \brief mPresetWidgets vector of all preset widgets getting displayed in the
     *        scroll area.
     */
    std::vector<PresetGroupWidget *> mPresetArduinoWidgets;

    /*!
     * \brief mPresetHueWidgets vector of all preset hue widgets.
     */
    std::vector<PresetGroupWidget *> mPresetHueWidgets;

    /// main layout of grouppage
    QVBoxLayout *mLayout;

    /*!
     * \brief mSpeedSlider slider for determing how fast routines will be
     */
    cor::Slider *mSpeedSlider;

    /// scroll area for preset groups
    QScrollArea *mScrollAreaArduino;

    /// scroll area for preset hue groups
    QScrollArea *mScrollAreaHue;

    /// widget used as main widget of QScrollArea when any arduinos are connected.
    QWidget *mScrollWidgetArduino;

    /// widget used as main widget of QScrollArea when only hues are connected.
    QWidget *mScrollWidgetHue;

    /*!
     * \brief mPresetLayout layout of all arduino preset widgets.
     */
    QVBoxLayout *mPresetArduinoLayout;

    /*!
     * \brief mPresetHueLayout layout of all hue preset widgets.
     */
    QGridLayout *mPresetHueLayout;

    /// mode
    EGroupMode mMode;

    /// stored state for current speed
    int mSpeed;

};

#endif // PresetColorsPage_H
