
#ifndef PresetColorsPage_H
#define PresetColorsPage_H

#include "lightingpage.h"
#include "lightsbutton.h"
#include "presetgroupwidget.h"

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

    /*!
     * \brief presetColorGroupChanged emits data to the MainWindow about the changes
     *        to the preset color page. The first int is a ELightingRoutine cast to an int,
     *        the second is a EColorGroup cast to an int.
     */
    void presetColorGroupChanged(int, int);


public slots:
    /*!
     * \brief multiButtonClicked every button setup as a presetButton will signal
     *        this slot whenever they are clicked.
     * \param routine the stored ELightingRoutine of the button cast to an int.
     * \param colorGroup the stored EColorGroup of the button cast to an int.
     */
    void multiButtonClicked(int routine, int colorGroup);

private slots:
    /*!
     * \brief renderUI renders expensive assets if and only if the assets have had any
     *        change of state.
     */
    void renderUI();

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

private:
    /*!
     * \brief ui pointer to Qt UI form.
     */
    Ui::PresetColorsPage *ui;

    /*!
     * \brief mPresetWidgets vector of all preset widgets getting displayed in the
     *        scroll area.
     */
    std::vector<PresetGroupWidget *> mPresetWidgets;

    /*!
     * \brief mPresetLayout layout of all preset widgets.
     */
    QVBoxLayout *mPresetLayout;
};

#endif // PresetColorsPage_H
