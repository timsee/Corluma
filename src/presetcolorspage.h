
#ifndef PresetColorsPage_H
#define PresetColorsPage_H

#include "lightingpage.h"
#include "lightsbutton.h"

#include <QWidget>
#include <QToolButton>
#include <QGridLayout>
#include <QLabel>
#include <QPushButton>

namespace Ui {
class PresetColorsPage;
}

/*!
 * \copyright
 * Copyright (C) 2015 - 2016.
 * Released under the GNU General Public License.
 *
 * \brief The PresetColorsPage provides a way to use the Color Presets
 *        from themed groups of colors to do Multi Color Routines.
 *
 * It contains a grid of buttons that map color presets to lighting
 * modes. The list expands horizontally into a QScrollArea.
 *
 */
class PresetColorsPage : public QWidget, public LightingPage
{
    Q_OBJECT

public:
    /*!
     * \brief Constructor
     */
    explicit PresetColorsPage(QWidget *parent = 0);
    /*!
     * \brief Deconstructor
     */
    ~PresetColorsPage();
    /*!
     * \brief highlightRoutineButton highlights the button that implements
     *        the routine parameter. If it can't find a button that
     *        implements this lighting routine, then all buttons are unhighlighted
     * \param routine the lighting routine the highlighted button implements.
     * \param colorGroup the color group that the highlighted button implements.
     */
    void highlightRoutineButton(ELightingRoutine routine, EColorGroup colorGroup);
    /*!
     * \brief setupButtons sets up the routine buttons. Requires the DataLayer
     *        of the application to be set up first.
     */
    void setupButtons();

signals:
    /*!
     * \brief used to signal back to the main page that it should update its
     *        top-left icon with a new color mode
     */
    void updateMainIcons();


public slots:
    /*!
     * \brief multiButtonClicked every button setup as a presetButton will signal
     *        this slot whenever they are clicked.
     * \param routine the stored ELightingRoutine of the button cast to an int.
     * \param colorGroup the stored EColorGroup of the button cast to an int.
     */
    void multiButtonClicked(int routine, int colorGroup);

protected:
    /*!
     * \brief called whenever the page is shown on screen.
     */
    void showEvent(QShowEvent *);

private:
    /*!
     * \brief ui pointer to Qt UI form.
     */
    Ui::PresetColorsPage *ui;

    /*!
     * \brief mButtonLayout layout used to put the buttons in the QScrollArea as a grid.
     */
    QGridLayout *mButtonLayout;
    /*!
     * \brief mPresetButtons Buttons used in the grid in the scroll area, each one signals
     *        a EColorPreset and a ELightingMode.
     */
    std::vector<LightsButton *> mPresetButtons;
    /*!
     * \brief mPresetLabels labels for the top of the columns of the preset buttons.
     */
    std::vector<QLabel *> mPresetLabels;

    /*!
     * \brief mButtonCount number of buttons used, equals the number of presets
     *        multiplied by the number of lighting routines that the presets can use.
     */
    int mButtonCount;

    /*!
     * \brief mPageButtons pointers to all the main buttons, used
     *        to iterate through them quickly.
     */
    std::shared_ptr<std::vector<QToolButton *> > mPageButtons;

};

#endif // PresetColorsPage_H
