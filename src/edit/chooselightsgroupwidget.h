#ifndef EDITCHOOSELIGHTSWIDGET_H
#define EDITCHOOSELIGHTSWIDGET_H

#include <QLabel>
#include <QWidget>
#include "edit/editpagechildwidget.h"
#include "menu/standardlightsmenu.h"
#include "menu/statelesslightslistmenu.h"
/*!
 * \copyright
 * Copyright (C) 2015 - 2020.
 * Released under the GNU General Public License.
 *
 *
 * \brief The ChooseLightsWidget class provides a menu for choosing a group of lights for either a
 * group or a room. On the left hand side, it displays all lights sorted into common groups. On the
 * righthand side, it shows all currently selected lights.
 */
class ChooseLightsGroupWidget : public EditPageChildWidget {
    Q_OBJECT
public:
    explicit ChooseLightsGroupWidget(QWidget* parent, CommLayer* comm, GroupData* groups)
        : EditPageChildWidget(parent),
          mChooseLabel{new QLabel("Choose Lights:", this)},
          mSelectedLabel{new QLabel("Selected Lights:", this)},
          mComm{comm},
          mGroups{groups},
          mLightsMenu{new StandardLightsMenu(this, comm, groups)},
          mRoomLights{new StatelessLightsListMenu(this, comm, true)},
          mSelectedLightsMenu{new StatelessLightsListMenu(this, comm, false)},
          mIsRoom{false} {
        mBottomButtons->enableForward(false);
        connect(mRoomLights, SIGNAL(clickedLight(QString)), this, SLOT(lightClicked(QString)));

        connect(mLightsMenu, SIGNAL(clickedLight(QString)), this, SLOT(lightClicked(QString)));
        connect(mLightsMenu,
                SIGNAL(clickedGroupSelectAll(std::uint64_t, bool)),
                this,
                SLOT(selectAllToggled(std::uint64_t, bool)));
    }

    /// programmatically changes the height of rows in scrolling menus
    void changeRowHeight(int height) { mRowHeight = height; }

    /// prefill the lights widget with lights already selected, so that the user can edit that
    /// preexisting selection
    void prefill(const std::vector<QString>& keys) {
        clear();
        mOriginalKeys = keys;
        for (const auto& key : keys) {
            auto light = mComm->lightByID(key);
            mSelectedLightsMenu->addLight(key);
        }
        conditionsMet();
    }

    /// true if any information does not match the original information, false otherwise
    bool hasEdits() override { return !(mOriginalKeys == lightIDs()); }

    /// setup the the page as choosing for groups, showing all available lights
    void setupAsGroup() {
        // set visibility of the proper light menu
        mRoomLights->setVisible(false);
        mLightsMenu->setVisible(true);

        mIsRoom = false;

        resize();
    }

    /// setup the the page as choosing for rooms, showing only lights that exist either in the
    /// current room or no room.
    void setupAsRoom(const std::vector<QString>& roomLights) {
        // start with the room lights
        auto lights = roomLights;
        // add in the orphan lights
        lights.insert(lights.end(), mGroups->orphanLights().begin(), mGroups->orphanLights().end());
        mRoomLights->showGroup(lights);
        mRoomLights->highlightLights(roomLights);

        // set visibility of the proper light menu
        mRoomLights->setVisible(true);
        mLightsMenu->setVisible(false);

        mIsRoom = true;

        resize();
    }

    /// clears all data currently on the page.
    void clear() {
        mOriginalKeys = {};
        // TODO: why does this need to be called for mLightContainer to show lights?
        mLightsMenu->reset();

        auto lights = mSelectedLightsMenu->lightIDs();
        for (const auto& light : lights) {
            mSelectedLightsMenu->removeLight(light);
        }
        mBottomButtons->enableForward(false);
    }

    /// updates the lights based off of app data.
    void updateLights() { mLightsMenu->updateLights(mSelectedLightsMenu->lightIDs()); }

    /// getter for all selected lights.
    const std::vector<QString>& lightIDs() { return mSelectedLightsMenu->lightIDs(); }

protected:
    /*!
     * \brief resizeEvent called whenever the widget resizes so that assets can be updated.
     */
    void resizeEvent(QResizeEvent*) override { resize(); }

private slots:

    /// handles when a light is clicked. it will either add or remove lights from the selected list.
    void lightClicked(QString key) {
        auto lights = mSelectedLightsMenu->lightIDs();
        auto result = std::find(lights.begin(), lights.end(), key);
        if (result != lights.end()) {
            mSelectedLightsMenu->removeLight(key);
        } else {
            mSelectedLightsMenu->addLight(key);
        }
        conditionsMet();
    }


    /// handles when select all is toggled for a group. it will either add or remove all lights from
    /// teh selected list.
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
                    mSelectedLightsMenu->addLight(light.uniqueID());
                }
            } else {
                for (auto light : lights) {
                    mSelectedLightsMenu->removeLight(light.uniqueID());
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
        if (!mSelectedLightsMenu->lightIDs().empty() && allLightsValid) {
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
        if (mIsRoom) {
            mRoomLights->resize(lightRect, mRowHeight);
        } else {
            mLightsMenu->resize(lightRect, mRowHeight);
        }
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

    /// rooms require specific lights to be shown, instead of showing all lights
    StatelessLightsListMenu* mRoomLights;

    /// widget for showing all selected lights
    StatelessLightsListMenu* mSelectedLightsMenu;

    /// original keys when the page was initialized, used to check for changes.
    std::vector<QString> mOriginalKeys;

    /// true if room, false if group
    bool mIsRoom;

    /// height of rows in scroll areas.
    int mRowHeight;
};

#endif // EDITCHOOSELIGHTSWIDGET_H
