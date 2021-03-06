#ifndef COR_EDITMOODPAGE_H
#define COR_EDITMOODPAGE_H

#include <QWidget>
#include "edit/choosemetadatawidget.h"
#include "edit/choosemoodgroupstateswidget.h"
#include "edit/choosemoodlightstateswidget.h"
#include "edit/editpage.h"
#include "edit/editprogressstate.h"
#include "edit/reviewmoodwidget.h"

namespace cor {
/*!
 * \copyright
 * Copyright (C) 2015 - 2020.
 * Released under the GNU General Public License.
 *
 * \brief The EditMoodPage class is a widget for editing and making new moods.
 *
 * All data for moods is stored app-side and is not synced between instances of the app.
 */
class EditMoodPage : public cor::EditPage {
    Q_OBJECT
public:
    explicit EditMoodPage(QWidget* parent,
                          CommLayer* comm,
                          AppData* appData,
                          PaletteData* palettes,
                          cor::LightList* data)
        : EditPage(parent, comm, true),
          mComm{comm},
          mData{data},
          mMetadataWidget{new ChooseMetadataWidget(this, true)},
          mLightsStateWidget{new ChooseMoodLightStatesWidget(this, comm, appData, palettes)},
          mGroupStatesWidget{new ChooseMoodGroupStatesWidget(this, appData->groups(), palettes)},
          mReviewPage{new ReviewMoodWidget(this, comm, appData)} {
        setupWidgets({mMetadataWidget, mLightsStateWidget, mGroupStatesWidget, mReviewPage});

        connect(mPreviewButton, SIGNAL(clicked(bool)), this, SLOT(previewPressed(bool)));
    }

    /// fill the page with preexisting data to edit
    void prefillMood(const cor::Mood& mood) {
        mMetadataWidget->prefill(mood.name(), mood.description(), cor::EGroupType::mood);

        // apply light metadata for when it is getting displayed. A mood isn't going to store
        // whether a light is a cube or a lighstrip, but the comm layer queries that info and stores
        // it.
        auto lights = mood.lights();
        for (auto&& light : lights) {
            light = mComm->addLightMetaData(light);
            // since we are displaying a mood, mark the light as reachable even when it isn't.
            light.isReachable(true);
        }

        mLightsStateWidget->prefill(lights);
        mGroupStatesWidget->prefill(mood.defaults());
        mReviewPage->editMode(true, mood.uniqueID());
    }

    /// clear all data from the page, and reset it to its default state.
    void clearGroup() override {
        mMetadataWidget->clear();
        mLightsStateWidget->clear();
        mGroupStatesWidget->clear();
        mReviewPage->editMode(false, cor::UUID::makeNew());
        reset();
    }

    /// @copydoc EditPage::pageChanged(std::uint32_t)
    void pageChanged(std::uint32_t i) override {
        switch (i) {
            case 0: {
                break;
            }
            case 1: {
                break;
            }
            case 2: {
                mGroupStatesWidget->addGroupsToLeftMenu();
                break;
            }
            case 3: {
                mReviewPage->displayMood(mMetadataWidget->name(),
                                         mMetadataWidget->description(),
                                         mLightsStateWidget->lights(),
                                         mGroupStatesWidget->defaults());
                break;
            }
            default:
                break;
        }
    }

    /// @copydoc EditPage::changeRowHeight(int)
    void changeRowHeight(int height) override {
        mLightsStateWidget->changeRowHeight(height);
        mGroupStatesWidget->changeRowHeight(height);
        mReviewPage->changeRowHeight(height);
    }

private slots:

    /// handle when preview is pressed, creates the mood and syncs it.
    void previewPressed(bool) {
        auto mood = mReviewPage->mood();
        mood.lights(mLightsStateWidget->lights());
        mood.defaults(mGroupStatesWidget->defaults());
        const auto& moodDict = mComm->makeMood(mood);
        mData->clearLights();
        mData->addLights(moodDict.items());
    }

private:
    /// pointer to comm data.
    CommLayer* mComm;

    /// data layer
    cor::LightList* mData;

    /// widget for choosing the metadata for a group, such as its name and description.
    ChooseMetadataWidget* mMetadataWidget;

    /// widget for choosing the lights in a mood
    ChooseMoodLightStatesWidget* mLightsStateWidget;

    /// widget for choosing default states of groups in a mood
    ChooseMoodGroupStatesWidget* mGroupStatesWidget;

    /// widget for reviewing the final state of a group.
    ReviewMoodWidget* mReviewPage;
};

} // namespace cor

#endif // COR_EDITMOODPAGE_H
