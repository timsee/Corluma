#ifndef EDITGROUPPAGE_H
#define EDITGROUPPAGE_H

#include "editpage.h"

/*!
 * \copyright
 * Copyright (C) 2015 - 2019.
 * Released under the GNU General Public License.
 *
 * \brief The EditGroupPage class is a widget for editing and making new groups.
 */
class EditGroupPage : public EditPage {
    Q_OBJECT
public:
    /// constructor
    explicit EditGroupPage(QWidget* parent, CommLayer* layer, GroupData* parser)
        : EditPage(parent, layer, parser) {
        connect(mTopMenu->roomCheckBox(),
                SIGNAL(boxChecked(bool)),
                this,
                SLOT(isRoomChecked(bool)));
    }

    /*!
     * \brief showGroup show the editing page with the selected group
     * \param group group to show
     * \param groupLights full data on the lights in the group
     * \param lights all known lights
     */
    void showGroup(const cor::Group& group,
                   const std::vector<cor::Light>& groupLights,
                   const std::vector<cor::Light>& lights,
                   bool isRoom);

    /// @copydoc EditPage::reset()
    void reset() override { mSimpleGroupWidget->setCheckedDevices(mOriginalLights); }

    /// @copydoc EditPage::changeName(const QString&)
    void changeName(const QString& name) override { mNewName = name; }

    /// @copydoc EditPage::checkForChanges()
    bool checkForChanges() override;

    /// @copydoc EditPage::saveChanges()
    bool saveChanges() override;

private slots:
    /// updates when the isRoom checkbox is checked
    void isRoomChecked(bool);

    /// delete button is pressed
    void deletePressed(bool) override;

private:
    /// original group state
    cor::Group mOriginalGroup;

    /// name of the new group
    QString mNewName;

    /// vector of lights for the new group
    std::vector<cor::Light> mNewLights;

    /// stores if originally using a room
    bool mIsRoomOriginal;

    /// stores if currently creating a room
    bool mIsRoom;

    /// store the state of the original lights
    std::vector<cor::Light> mOriginalLights;
};

#endif // EDITGROUPPAGE_H
