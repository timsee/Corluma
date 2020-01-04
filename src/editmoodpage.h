#ifndef EDITMOODPAGE_H
#define EDITMOODPAGE_H

#include "editpage.h"

/*!
 * \copyright
 * Copyright (C) 2015 - 2019.
 * Released under the GNU General Public License.
 *
 * \brief The EditMoodPage class is a widget for editing and making new moods.
 */
class EditMoodPage : public EditPage {
    Q_OBJECT
public:
    /// constructor
    explicit EditMoodPage(QWidget* parent, CommLayer* layer, GroupData* parser)
        : EditPage(parent, layer, parser) {}

    /*!
     * \brief showMood show the edit page with the provided mood
     * \param mood the mood to show
     * \param lightsall known lights
     */
    void showMood(const cor::Mood& mood, const std::vector<cor::Light>& lights);

    /// @copydoc EditPage::reset()
    void reset() override { mSimpleGroupWidget->setCheckedDevices(mOriginalMood.lights()); }

    /// @copydoc EditPage::changeName(const QString&)
    void changeName(const QString& name) override { mNewMood.name(name); }

    /// @copydoc EditPage::checkForChanges()
    bool checkForChanges() override;

    /// @copydoc EditPage::saveChanges()
    bool saveChanges() override;

private slots:

    /// delete button is pressed
    void deletePressed(bool) override;

private:
    /// original state of the mood
    cor::Mood mOriginalMood;

    /// all changes made by the edit page
    cor::Mood mNewMood;
};

#endif // EDITMOODPAGE_H
