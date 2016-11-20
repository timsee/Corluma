#ifndef LIGHTSBUTTON_H
#define LIGHTSBUTTON_H

#include <QWidget>
#include <QLayout>
#include <QPushButton>
#include <QLabel>
#include "datalayer.h"
#include "icondata.h"
#include "lightingprotocols.h"

/*!
 * \copyright
 * Copyright (C) 2015 - 2016.
 * Released under the GNU General Public License.
 *
 * \brief The LightsButton class provides all of the buttons used within the application.
 *        All buttons have an icon, while some have labels or extra logic attached.
 *
 * There are currently three different ways you can set up a button. A standard button
 * emits a EColorGroup and a ELightingRoutine and doesn't have a label. A labeled button
 * emits a EColorGroup and a ELightingRoutine, and it also has a label at the bottom of
 * the button. A menu button emits a page number, and is used by the main menu.
 *
 */
class LightsButton : public QWidget
{
    Q_OBJECT
public:
    /*!
     * \brief Constructor
     */
    explicit LightsButton(QWidget *parent = 0);

    /*!
     * \brief setupAsMultiButton sets up the button for use with the PresetArrayPage. Assigns
     *        it a mode, a preset, and an icon. Whenever the button is clicked it will emit
     *        presetClicked and will send its mode and preset in that signal.
     * \param routine the ELightingRoutine that emits when this button is pushed.
     * \param colorGroup The EColorGroup that emits when this button is pushed.
     * \param dataLayer used to create the icon for the button
     */
    void setupAsStandardButton(ELightingRoutine routine,
                               EColorGroup colorGroup,
                               DataLayer *dataLayer,
                               QString label = QString(""));

    /*!
     * \brief setupAsMenuButton Used by the mainWindow for the top menu. Buttons send out
     *        their page number whenever they are clicked.
     * \param pageNumber the number that this menu button is getting set up to signal whenever
     *        its clicked.
     * \param dataLayer pointer to the data layer, which is used for creating the icon.
     */
    void setupAsMenuButton(int pageNumber, DataLayer *dataLayer);

    /*!
     * \brief updateIcon update the icon of the of the lightsbutton.
     */
    void updateIcon();

    /*!
     * \brief updateIconSingleColorRoutine update the button's icon based off of a lighting
     *        routine and color. This function requires single color routines to work properly.
     * \param lightingRoutine lighting routine to use for the icon. must be single color routine.
     * \param color color to use for the icon.
     */
    void updateIconSingleColorRoutine(ELightingRoutine lightingRoutine, QColor color);

    /*!
     * \brief updateIconPresetColorRoutine update the button's icon based off of a lighting routine
     *        and color group. This function requires multi color routiens tow ork properly.
     * \param lightingRoutine lighting routine to use for the icon. must be multi color routine.
     * \param colorGroup the color group to use for the icon.
     */
    void updateIconPresetColorRoutine(ELightingRoutine lightingRoutine, EColorGroup colorGroup);

    /*!
     * \brief lightingRoutine the ELightingRoutine assigned to the button by setupAsMultiButton.
     * \return the button's lighting routine
     */
    ELightingRoutine lightingRoutine();

    /*!
     * \brief colorGroup the EColorGroup assigned to the button by setupAsMultiButton.
     * \return the button's color group
     */
    EColorGroup colorGroup();

    /*!
     * \brief button The QPushButton that this QWidget wraps.
     */
    QPushButton *button;

    /*!
     * \brief buttonLabel label used when setup as a label button
     */
    QLabel *buttonLabel;

signals:
    /*!
     * \brief buttonClicked sent only when setupAsPresetButton has been called.
     */
    void buttonClicked(int, int);

    /*!
     * \brief menuButtonClicked sent out if and only if the button is set up
     *        as a menu button
     */
    void menuButtonClicked(int);

private slots:
    /*!
     * \brief handleButton listens for a click on the button.
     */
    void handleButton();

protected:
    /*!
    * \brief resizeEvent used to resize the icon on the QPushButton.
    */
   virtual void resizeEvent(QResizeEvent *);

private:
    /*!
     * \brief mLayout layout of a lights button
     */
    QVBoxLayout *mLayout;

    /*!
     * \brief mIconData icon data used by the button's
     *        icon.
     */
    IconData mIconData;

    /*!
     * \brief mDataLayer pointer to the data layer of the application
     */
    DataLayer *mDataLayer;

    /*!
     * the string representation of the text of the QLabel of the button
     * If the button doesn't have any label, this is set as a blank string.
     */
    QString mLabel;

    /*!
     * \brief mSetupHasBeenCalled prevents illegal calls before its been set up
     *        as a specific button
     */
    bool mSetupHasBeenCalled;

    /*!
     * \brief mIsMenuButton true if setupAsMenuButton is called, false otherwise.
     */
    bool mIsMenuButton;

    /*!
     * \brief mPageNumber set as the page number that gets sent out if the button is
     *        setup as a menuButton.
     */
    int mPageNumber;

    /*!
     * \brief mLightingRoutine stored lighting routine. can only be
     *        used if the button is set up as a presetButton.
     *        If it is a preset button, this will be emitted
     *        as a signal whenever the button is clicked.
     */
    ELightingRoutine mLightingRoutine;
    /*!
     * \brief mColorGroup stored color group. can only be
     *        used if the button is set up as a presetButton.
     *        If it is a preset button, this will be emitted
     *        as a signal whenever the button is clicked.
     */
    EColorGroup mColorGroup;
};

#endif // LIGHTSBUTTON_H
