#ifndef CHOOSELIGHTSMOODWIDGET_H
#define CHOOSELIGHTSMOODWIDGET_H

#include <QLabel>
#include <QWidget>
#include "edit/editpagechildwidget.h"
#include "menu/lightslistmenu.h"
#include "menu/standardlightsmenu.h"
/*!
 * \copyright
 * Copyright (C) 2015 - 2020.
 * Released under the GNU General Public License.
 *
 *
 * \brief The ChooseLightsMoodWidget class is a widget for choosing lights to use in a mood. The
 * current state of the light is added to the mood.
 */
class ChooseLightsMoodWidget : public EditPageChildWidget {
    Q_OBJECT
public:
    explicit ChooseLightsMoodWidget(QWidget* parent, CommLayer* comm, GroupData* groups)
        : EditPageChildWidget(parent),
          mChooseLabel{new QLabel("Choose Lights:", this)},
          mSelectedLabel{new QLabel("Selected Lights:", this)},
          mComm{comm},
          mGroups{groups},
          mLightsMenu{new StandardLightsMenu(this, comm, groups)},
          mSelectedLightsMenu{new LightsListMenu(this, comm)} {
        mBottomButtons->enableForward(false);

        connect(mLightsMenu, SIGNAL(clickedLight(QString)), this, SLOT(lightClicked(QString)));
        connect(mLightsMenu,
                SIGNAL(clickedGroupSelectAll(std::uint64_t, bool)),
                this,
                SLOT(selectAllToggled(std::uint64_t, bool)));

        // set visibility of the proper light menu
        mLightsMenu->setVisible(true);

        resize();
    }

    /// programmatically changes the height of rows in scrolling menus
    void changeRowHeight(int height) { mRowHeight = height; }

    /// clears all data currently on the page.
    void clear() {
        // TODO: why does this need to be called for mLightContainer to show lights?
        mLightsMenu->reset();

        auto lights = mSelectedLightsMenu->lights();
        for (const auto& light : lights) {
            mSelectedLightsMenu->removeLight(light);
        }
        mBottomButtons->enableForward(false);
    }

    /// prefill selected lights and their states.
    void prefill(const std::vector<cor::Light>& lights) {
        clear();
        for (const auto& light : lights) {
            mSelectedLightsMenu->addLight(light);
        }
        conditionsMet();
    }

    /// updates the lights based off of app data.
    void updateLights() {
        mLightsMenu->updateLights(cor::lightVectorToIDs(mSelectedLightsMenu->lights()));
    }

    /// getter for all selected lights.
    std::vector<cor::Light> lights() { return mSelectedLightsMenu->lights(); }

protected:
    /*!
     * \brief resizeEvent called whenever the widget resizes so that assets can be updated.
     */
    void resizeEvent(QResizeEvent*) { resize(); }

private slots:

    /// handles when a light is clicked. it will either add or remove lights from the selected list.
    void lightClicked(cor::Light light) {
        auto lights = mSelectedLightsMenu->lights();
        auto lightResult = cor::findLightInVectorByID(lights, light);
        if (lightResult.isValid()) {
            mSelectedLightsMenu->removeLight(light);
        } else {
            mSelectedLightsMenu->addLight(light);
        }
        conditionsMet();
    }

    /// handles when a light is clicked from the lights menu.
    void lightClicked(QString key) { lightClicked(mComm->lightByID(key)); }

    /// handles when select all is toggled for a group. it will either add or remove all lights from
    /// the selected list.
    void selectAllToggled(std::uint64_t ID, bool shouldSelect) {
        // convert the group ID to a group
        auto groupResult = mGroups->groupDict().item(QString::number(ID).toStdString());
        bool groupFound = groupResult.second;
        cor::Group group = groupResult.first;
        if (groupFound) {
            auto lightIDs = group.lights();
            auto lights = mComm->lightsByIDs(group.lights());
            // if the group selected is found, either select or deselect it
            if (shouldSelect) {
                for (auto light : lights) {
                    mSelectedLightsMenu->addLight(light);
                }
            } else {
                for (auto light : lights) {
                    mSelectedLightsMenu->removeLight(light);
                }
            }
            updateLights();
            conditionsMet();
        } else {
            qDebug() << " group not found " << ID;
        }
    }

private:
    /// true if all fields have met the conditions needed to proceed, false if any widget has an
    /// invalid input.
    bool conditionsMet() {
        bool allLightsValid = true;
        for (auto light : lights()) {
            if (!light.isValid()) {
                allLightsValid = false;
            }
        }


        if (mSelectedLightsMenu->lights().size() > 0 && allLightsValid) {
            mBottomButtons->enableForward(true);
            emit stateChanged(mIndex, EEditProgressState::completed);
            return true;
        } else {
            mBottomButtons->enableForward(false);
            emit stateChanged(mIndex, EEditProgressState::incomplete);
            return false;
        }
    }

    /// resize programmatically
    void resize() {
        int yPos = 0;
        int buttonHeight = this->height() / 10;

        mChooseLabel->setGeometry(0, yPos, int(this->width() / 2 * 0.95), buttonHeight);

        mSelectedLabel->setGeometry(int(this->width() / 2 * 1.05),
                                    yPos,
                                    this->width() / 2,
                                    buttonHeight);
        yPos += mChooseLabel->height();

        QRect lightRect(0, yPos, int((this->width() / 2) * 0.95), 7 * buttonHeight);

        mLightsMenu->resize(lightRect, mRowHeight);
        QRect selectedLightsRect(int(this->width() / 2 * 1.05),
                                 lightRect.y(),
                                 lightRect.width(),
                                 lightRect.height());
        mSelectedLightsMenu->resize(selectedLightsRect, mRowHeight);
        yPos += mLightsMenu->height();

        mBottomButtons->setGeometry(0, this->height() - buttonHeight, this->width(), buttonHeight);
    }

    /// label for left hand column
    QLabel* mChooseLabel;

    /// label for right hand column
    QLabel* mSelectedLabel;

    /// pointer to comm data
    CommLayer* mComm;

    /// pointer to group data
    GroupData* mGroups;

    /// widget for showing all available lights.
    StandardLightsMenu* mLightsMenu;

    /// widget for showing all selected lights
    LightsListMenu* mSelectedLightsMenu;

    /// height of rows in scroll areas.
    int mRowHeight;
};


#endif // CHOOSELIGHTSMOODWIDGET_H
