#ifndef COR_EDITGROUPPAGE_H
#define COR_EDITGROUPPAGE_H

#include <QWidget>
#include "edit/chooselightsgroupwidget.h"
#include "edit/choosemetadatawidget.h"
#include "edit/editpage.h"
#include "edit/editprogressstate.h"
#include "edit/reviewgroupwidget.h"

namespace cor {
/*!
 * \copyright
 * Copyright (C) 2015 - 2020.
 * Released under the GNU General Public License.
 *
 * \brief The EditGroupPage class is a widget for editing and making new groups.
 *
 * The metadata for the group is only stored in the app if theres no other place to store it. For
 * example, in the case of a Hue Bridge, it can store both groups and rooms, so this page sends
 * messages to bridge. This allows multiple versions of the app to use the same data without
 * explicitly syncing the app's save data.
 */
class EditGroupPage : public cor::EditPage {
    Q_OBJECT
public:
    explicit EditGroupPage(QWidget* parent, CommLayer* comm, AppData* appData)
        : EditPage(parent, comm, false),
          mMetadataWidget{new ChooseMetadataWidget(this, false)},
          mLightsWidget{new ChooseLightsGroupWidget(this, comm, appData)},
          mReviewPage{new ReviewGroupWidget(this, comm, appData)},
          mGroups{appData->groups()} {
        setupWidgets({mMetadataWidget, mLightsWidget, mReviewPage});
    }

    /// fill the page with preexisting data to edit
    void prefillGroup(const cor::Group& group) {
        mMetadataWidget->prefill(group.name(), group.description(), group.type());
        mLightsWidget->prefill(group.lights());
        mReviewPage->editMode(true, group.uniqueID());
    }


    /// clear all data from the page, and reset it to its default state.
    void clearGroup() override {
        mMetadataWidget->clear();
        mLightsWidget->clear();
        mReviewPage->editMode(false, 0u);
        reset();
    }

    /// @copydoc EditPage::pageChanged(std::uint32_t)
    void pageChanged(std::uint32_t i) override {
        switch (i) {
            case 0: {
                break;
            }
            case 1: {
                if (mMetadataWidget->groupType() == EGroupType::group) {
                    mLightsWidget->setupAsGroup();
                } else {
                    if (mReviewPage->isEditMode()) {
                        // get the group from its ID
                        auto group = mGroups->groupFromID(mReviewPage->uniqueID());
                        // setup the choose lights widget so that it knows the lights already in the
                        // room
                        mLightsWidget->setupAsRoom(group.lights());
                    } else {
                        mLightsWidget->setupAsRoom({});
                    }
                }
                mLightsWidget->updateLights();
                break;
            }
            case 2: {
                mReviewPage->displayGroup(mMetadataWidget->name(),
                                          mMetadataWidget->groupType(),
                                          mMetadataWidget->description(),
                                          mLightsWidget->lightIDs());
                break;
            }
            default:
                break;
        }
    }

    /// @copydoc EditPage::changeRowHeight(int)
    void changeRowHeight(int height) override {
        mLightsWidget->changeRowHeight(height);
        mReviewPage->changeRowHeight(height);
    }

private:
    /// widget for choosing the metadata for a group, such as its name and description.
    ChooseMetadataWidget* mMetadataWidget;

    /// widget for choosing the lights in a group.
    ChooseLightsGroupWidget* mLightsWidget;

    /// widget for reviewing the final state of a group.
    ReviewGroupWidget* mReviewPage;

    /// pointer to group data.
    GroupData* mGroups;
};

} // namespace cor

#endif // EDITGROUPPAGE_H
