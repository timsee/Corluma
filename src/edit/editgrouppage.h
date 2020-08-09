#ifndef COR_EDITGROUPPAGE_H
#define COR_EDITGROUPPAGE_H

#include <QWidget>
#include "edit/chooselightswidget.h"
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
 * \brief The EditGroupPage class is a widget for editing and making new groups. It can make only
 * groups and cannot make rooms.
 *
 * The metadata for the group is only stored in the app if theres no other place to store it. For
 * example, in the case of a Hue Bridge, it can store both groups and rooms, so this page sends
 * messages to bridge. This allows multiple versions of the app to use the same data without
 * explicitly syncing the app's save data.
 */
class EditGroupPage : public cor::EditPage {
    Q_OBJECT
public:
    explicit EditGroupPage(QWidget* parent, CommLayer* comm, GroupData* groups)
        : EditPage(parent, comm, groups),
          mMetadataWidget{new ChooseMetadataWidget(this)},
          mLightsWidget{new ChooseLightsWidget(this, comm, groups)},
          mReviewPage{new ReviewGroupWidget(this, comm, groups)} {
        setupWidgets({mMetadataWidget, mLightsWidget, mReviewPage});
    }

    /// fill the page with preexisting data to edit
    void prefillGroup(const cor::Group& group) {
        mMetadataWidget->prefill(group.name(), group.description(), group.type());
        mLightsWidget->prefill(group.lights());
        mReviewPage->editMode(true, group.uniqueID());
    }

    /// clear all data from the page, and reset it to its default state.
    void clearGroup() {
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
                mLightsWidget->updateLights();
                break;
            }
            case 2: {
                mReviewPage->displayGroup(mMetadataWidget->name(),
                                          mMetadataWidget->groupType(),
                                          mMetadataWidget->description(),
                                          mLightsWidget->lights());
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
    /// true if room, false otherwise
    bool mIsRoom;

    /// widget for choosing the metadata for a group, such as its name and description.
    ChooseMetadataWidget* mMetadataWidget;

    /// widget for choosing the lights in a group
    ChooseLightsWidget* mLightsWidget;

    /// widget for reviewing the final state of a group.
    ReviewGroupWidget* mReviewPage;
};

} // namespace cor

#endif // EDITGROUPPAGE_H
