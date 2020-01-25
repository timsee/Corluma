#ifndef EDITPAGE_H
#define EDITPAGE_H

#include <QListWidget>
#include <QWidget>

#include "comm/commlayer.h"
#include "cor/objects/light.h"
#include "cor/objects/page.h"
#include "cor/widgets/checkbox.h"
#include "cor/widgets/listwidget.h"
#include "editpagetopmenu.h"
#include "listsimplegroupwidget.h"

/*!
 * \copyright
 * Copyright (C) 2015 - 2020.
 * Released under the GNU General Public License.
 *
 *
 *
 * \brief The EditGroupPage class is a page that is used to edit existing groups of lights. It
 * can edit both collections (paths to lights without any colors or states saved) and moods
 * (both a path to a light and its color and states are saved). It allows you to remove
 * and add devices from a group, as well as change its name. You can only add and remove
 * devices from this page, you cannot change the settings of individual devices. A special
 * case of this page can also create new collections and moods.
 */
class EditPage : public QWidget, public cor::Page {
    Q_OBJECT

public:
    /// constructor
    explicit EditPage(QWidget* parent, CommLayer* layer, GroupData* parser);

    /*!
     * \brief updateDevices update the device widgets in the edit page with new values
     *
     * \param groupDevices all devices in the original group
     * \param devices all devices known by the application.
     */
    void updateDevices(const std::vector<cor::Light>& checkedDevices,
                       const std::vector<cor::Light>& devices);

    /// getter for all lights checked on the page.
    std::vector<cor::Light> lights() const { return mSimpleGroupWidget->checkedDevices(); }

    /// resizes widget programmatically
    void resize();

    /// pushes in the widget
    void pushIn();

    /// pushes out the widget
    void pushOut();

    /// resets any changes made in the edit page
    virtual void reset() = 0;

    /// changes the name of the object being edited
    virtual void changeName(const QString&) = 0;

    /*!
     * \brief checkForChanges checks if the name or the selected devices are changed.
     *
     * \return true if the name or selected devices are changed, false otherwise.
     */
    virtual bool checkForChanges() = 0;

    /*!
     * \brief saveChanges uses the state of the UI to save any changes made to the group on this
     * page.
     */
    virtual bool saveChanges() = 0;

signals:

    /*!
     * \brief pressedClose emited when the close button is pressed.
     */
    void pressedClose();

protected:
    /// called when widget is resized
    void resizeEvent(QResizeEvent*);

    /*!
     * \brief paintEvent used to draw the background of the widget.
     */
    void paintEvent(QPaintEvent*);

private slots:

    /*!
     * \brief closePressed called when close button is pressed. Checks if changes were made, asks
     * for user input if needed, and then closes the window.
     */
    void closePressed(bool);

    /*!
     * \brief resetPressed reset button is pressed, this resets the edit page back to the settings
     * it had when it was originally opened.
     */
    void resetPressed(bool);

    /*!
     * \brief deletePressed delete button is pressed, this removes the group from the application's
     * save data.
     */
    virtual void deletePressed(bool) = 0;

    /*!
     * \brief savePressed save button is pressed on edit page.
     */
    void savePressed(bool);


    /*!
     * \brief renderUI renders expensive assets if and only if the assets have had any
     *        change of state.
     */
    void renderUI();

    /*!
     * \brief lineEditChanged called whenever a new character is typed into the QLineEdit. Used to
     * change the name of a group.
     */
    void lineEditChanged(const QString&);

    /// called whenever a device is clicked in the edit widget
    void clickedDevice(const QString&);

protected:
    /*!
     * \brief communication pointer to communication object
     *        for sending comannds to the lights
     */
    CommLayer* mComm;

    /// groups parser
    GroupData* mGroups;

    /*!
     * \brief shouldSetChecked true if should be set checked, false otherwise.
     *
     * \param device the device that is used in the widget that is being analyzed to see if it
     * should be checked.
     * \param groupDevices all known devices in the group
     * \return true if should be set checked, false otherwise.
     */
    bool shouldSetChecked(const cor::Light& device, const std::vector<cor::Light>& groupDevices);

    /// widget used for scroll area.
    ListSimpleGroupWidget* mSimpleGroupWidget;

    /// subwidget holding all the buttons and such in the top of this widget.
    EditPageTopMenu* mTopMenu;
};

#endif // EDITPAGE_H
