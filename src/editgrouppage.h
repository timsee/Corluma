#ifndef EDITCOLLECTIONPAGE_H
#define EDITCOLLECTIONPAGE_H

#include <QWidget>
#include <QListWidget>

#include "cor/light.h"
#include "listdevicewidget.h"
#include "lightingpage.h"
#include "comm/commlayer.h"
#include "groupsparser.h"
#include "cor/checkbox.h"


namespace Ui {
class EditCollectionPage;
}

/*!
 * \copyright
 * Copyright (C) 2015 - 2018.
 * Released under the GNU General Public License.
 *
 *
 *
 * \brief The EditGroupPage class is a page that is used to edit existing groups of lights. It
 *        can edit both collections (paths to lights without any colors or states saved) and moods
 *        (both a path to a light and its color and states are saved). It allows you to remove
 *        and add devices from a group, as well as change its name. You can only add and remove
 *        devices from this page, you cannot change the settings of individual devices. A special case
 *        of this page can also create new collections and moods.
 */
class EditGroupPage : public QWidget, public LightingPage
{
    Q_OBJECT

public:
    /// constructor
    explicit EditGroupPage(QWidget *parent = 0);

    /// deconstructor
    ~EditGroupPage();

    /*!
     * \brief showGroup open the edit page with the given data
     * \param key key for the group that you are displaying
     * \param groupDevices all devicess contained in the group
     * \param devices all devicess known by the commlayer
     * \param isMood true if a mood, false if a collection.
     * \param isRoom true if a room, false if a lightgroup.
     */
    void showGroup(QString key, std::list<cor::Light> groupDevices, std::list<cor::Light> devices, bool isMood, bool isRoom);

    /*!
     * \brief updateDevices update the device widgets in the edit page with new values
     * \param groupDevices all devices in the original group
     * \param devices all devices known by the application.
     */
    void updateDevices(std::list<cor::Light> groupDevices, std::list<cor::Light> devices);

    /*!
     * \brief resize the widget. Call this to explicitly resize this widget based off of the size of the
     *        parent. If the parent has not resized since this was last called, this does nothing.
     * \brief resizeFullWidget if true, resizes full widget including its geometry. If false, the geometry is not
     *        resized, just the contents.
     */
    void resize(bool resizeFullWidget = true);

    /*!
     * \brief connectCommLayer connec the commlayer to this page.
     * \param layer a pointer to the commlayer object.
     */
    void connectCommLayer(CommLayer *layer) { mComm = layer; }

    /// connects the GroupsParser object to this UI widget.
    void connectGroupsParser(GroupsParser *parser) { mGroups = parser; }

signals:

    /*!
     * \brief pressedClose emited when the close button is pressed.
     */
    void pressedClose();

protected:

    /*!
     * \brief showEvent called before the this page is shown. Used to sync up
     *        any changes that may have happened on other pages.
     */
    void showEvent(QShowEvent *);

    /*!
     * \brief hideEvent called as the page is hidden. This happens when a new page
     *        is displayed.
     */
    void hideEvent(QHideEvent *);

    /*!
     * \brief paintEvent used to draw the background of the widget.
     */
    void paintEvent(QPaintEvent *);

private slots:

    /// updates when the isRoom checkbox is checked
    void isRoomChecked(bool);

    /*!
     * \brief closePressed called when close button is pressed. Checks if changes were made, asks for user
     *        input if needed, and then closes the window.
     */
    void closePressed(bool);

    /*!
     * \brief resetPressed reset button is pressed, this resets the edit page back to the settings it had
     *        when it was originally opened.
     */
    void resetPressed(bool);

    /*!
     * \brief deletePressed delete button is pressed, this removes the group from the application's save data.
     */
    void deletePressed(bool);

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
     * \brief listDeviceWidgetClicked called when the device widget has any device clicked.
     */
    void listDeviceWidgetClicked(QString);

    /*!
     * \brief lineEditChanged called whenever a new character is typed into the QLineEdit. Used to change the
     *        name of a group.
     */
    void lineEditChanged(const QString&);

private:

    /*!
     * \brief ui pointer to Qt UI form.
     */
    Ui::EditCollectionPage *ui;

    /*!
     * \brief shouldSetChecked true if should be set checked, false otherwise.
     * \param device the device that is used in the widget that is being analyzed to see if it should be checked.
     * \param groupDevices all known devices in the group
     * \return true if should be set checked, false otherwise.
     */
    bool shouldSetChecked(const cor::Light& device, const std::list<cor::Light>& groupDevices);

    /*!
     * \brief checkForChanges checks if the name or the selected devices are changed.
     * \return true if the name or selected devices are changed, false otherwise.
     */
    bool checkForChanges();

    /*!
     * \brief saveChanges uses the state of the UI to save any changes made to the group on this page.
     */
    void saveChanges();

    /// connects the GroupsParser object to this UI widget.
    GroupsParser *mGroups;

    /*!
     * \brief mIsMood true if mood, false if collection
     */
    bool mIsMood;

    /*!
     * \brief mOriginalDevices The original state of the group.
     */
    std::list<cor::Light> mOriginalDevices;

    /*!
     * \brief createCollection creates a collection based off of the state of the UI
     * \return list of devices in the collection
     */
    std::list<cor::Light> createCollection();

    /*!
     * \brief createMood creates a mood based off of the state of the UI
     * \return list of devices in the mood
     */
    std::list<cor::Light> createMood();

    /// original name of group we're editing.
    QString mOriginalName;

    /// stores whether the room flag is set or not
    bool mIsRoomCurrent;

    /// stores whether the room flag was originally set or not.
    bool mIsRoomOriginal;

    /// new name for group, saved by QLineEdit
    QString mNewName;

    /*!
     * \brief communication pointer to communication object
     *        for sending comannds to the lights
     */
    CommLayer *mComm;

    /// widget used for scroll area.
    QWidget *mScrollAreaWidget;

    /// vertical layout for widget
    QVBoxLayout *mLayout;

    /*!
     * \brief mWidgets widgets used to display all connected devices.
     */
    std::vector<ListDeviceWidget *> mWidgets;
};

#endif // EDITCOLLECTIONPAGE_H
