#ifndef EDITCHOOSELIGHTSWIDGET_H
#define EDITCHOOSELIGHTSWIDGET_H

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
 * \brief The ChooseLightsWidget class provides a menu for choosing a group of lights. On the left
 * hand side, it displays all lights sorted into common groups. On the righthand side, it shows all
 * currently selected lights.
 */
class ChooseLightsWidget : public EditPageChildWidget {
    Q_OBJECT
public:
    explicit ChooseLightsWidget(QWidget* parent, CommLayer* comm, GroupData* groups)
        : EditPageChildWidget(parent),
          mChooseLabel{new QLabel("Choose Lights:", this)},
          mSelectedLabel{new QLabel("Selected Lights:", this)},
          mLights{new cor::LightList(this)},
          mComm{comm},
          mGroups{groups},
          mLightsMenu{new StandardLightsMenu(this, comm, mLights, groups)},
          mSelectedLightsMenu{new LightsListMenu(this, comm)} {
        mBottomButtons->enableForward(false);

        connect(mLightsMenu, SIGNAL(clickedLight(QString)), this, SLOT(lightClicked(QString)));
        connect(mLightsMenu,
                SIGNAL(clickedGroupSelectAll(std::uint64_t, bool)),
                this,
                SLOT(selectAllToggled(std::uint64_t, bool)));
    }

    /// programmatically changes the height of rows in scrolling menus
    void changeRowHeight(int height) { mRowHeight = height; }

    /// clears all data currently on the page.
    void clear() { mBottomButtons->enableForward(false); }

    /// updates the lights based off of app data.
    void updateLights() { mLightsMenu->updateLights(); }

    /// getter for all selected lights.
    const std::vector<QString>& lights() { return mSelectedLightsMenu->lights(); }

protected:
    /*!
     * \brief resizeEvent called whenever the widget resizes so that assets can be updated.
     */
    void resizeEvent(QResizeEvent*) {
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

private slots:

    /// handles when a light is clicked. it will either add or remove lights from the selected list.
    void lightClicked(QString key) {
        auto light = mComm->lightByID(key);
        auto state = light.state();
        if (light.isReachable()) {
            if (mLights->doesLightExist(light)) {
                mLights->removeLight(light);
                mSelectedLightsMenu->removeLight(key);
            } else {
                mLights->addLight(light);
                mSelectedLightsMenu->addLight(key);
            }
            conditionsMet();
        }
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
                    if (light.isReachable()) {
                        mSelectedLightsMenu->addLight(light.uniqueID());
                    }
                }
                mLights->addLights(lights);
            } else {
                for (auto light : lights) {
                    mSelectedLightsMenu->removeLight(light.uniqueID());
                }
                mLights->removeLights(lights);
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
        if (mLights->lights().size() > 1) {
            mBottomButtons->enableForward(true);
            emit stateChanged(mIndex, EEditProgressState::completed);
            return true;
        } else {
            mBottomButtons->enableForward(false);
            emit stateChanged(mIndex, EEditProgressState::incomplete);
            return false;
        }
    }

    /// label for left hand column
    QLabel* mChooseLabel;

    /// label for right hand column
    QLabel* mSelectedLabel;

    /// list of currently selected lights
    cor::LightList* mLights;

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

#endif // EDITCHOOSELIGHTSWIDGET_H
