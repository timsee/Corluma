#ifndef CHOOSELIGHTSMOODWIDGET_H
#define CHOOSELIGHTSMOODWIDGET_H

#include <QLabel>
#include <QWidget>
#include "edit/editpagechildwidget.h"
#include "menu/choosestatewidget.h"
#include "menu/lightslistmenu.h"
#include "menu/standardlightsmenu.h"
#include "utils/qt.h"

/// state of the ChooseLightsMoodWidget.
enum class EChooseLightsMoodState { disabled, addLight, changeLightState, removeLight };

/*!
 * \copyright
 * Copyright (C) 2015 - 2020.
 * Released under the GNU General Public License.
 *
 *
 * \brief The ChooseLightsMoodWidget class is a widget for choosing lights to use in a mood. The
 * current state of the light is added to the mood.
 */
class ChooseMoodLightStatesWidget : public EditPageChildWidget {
    Q_OBJECT
public:
    explicit ChooseMoodLightStatesWidget(QWidget* parent,
                                         CommLayer* comm,
                                         GroupData* groups,
                                         PaletteData* palettes)
        : EditPageChildWidget(parent),
          mChooseLabel{new QLabel("Choose Lights:", this)},
          mSelectedLabel{new QLabel("Selected Lights:", this)},
          mComm{comm},
          mLightsMenu{new StandardLightsMenu(this, comm, groups, "ChooseLightsMoodMenu")},
          mMoodLights{new LightsListMenu(this, true)},
          mLeftButton{new QPushButton(this)},
          mRightButton{new QPushButton(this)},
          mLightWidget{new ListLightWidget({}, false, EListLightWidgetType::standard, this)},
          mStateWidget{new ChooseStateWidget(this, palettes)},
          mRowHeight{10} {
        mBottomButtons->enableForward(false);

        mLightsMenu->singleLightsMode(true);
        mMoodLights->singleLightMode(true);
        mStateWidget->enable(true);

        connect(mMoodLights,
                SIGNAL(clickedLight(cor::Light)),
                this,
                SLOT(moodLightClicked(cor::Light)));

        connect(mLightsMenu, SIGNAL(clickedLight(QString)), this, SLOT(lightClicked(QString)));
        connect(mLightsMenu, SIGNAL(unselectLight(QString)), this, SLOT(unselectLight(QString)));

        connect(mStateWidget,
                SIGNAL(stateChanged(cor::LightState)),
                this,
                SLOT(stateUpdated(cor::LightState)));

        connect(mLeftButton, SIGNAL(pressed()), this, SLOT(leftButtonPressed()));
        connect(mRightButton, SIGNAL(pressed()), this, SLOT(rightButtonPressed()));

        // start with state hidden, since nothing is selected at start
        hideState();
    }

    /// programmatically changes the height of rows in scrolling menus
    void changeRowHeight(int height) { mRowHeight = height; }

    /// clears all data currently on the page.
    void clear() {
        mOriginalLights = {};
        // TODO: why does this need to be called for mLightContainer to show lights?
        mLightsMenu->reset();
        mLightsMenu->ignoreLights({});
        mLightsMenu->updateMenu();

        auto lights = mMoodLights->lights();
        for (const auto& light : lights) {
            mMoodLights->removeLight(light);
        }
        mBottomButtons->enableForward(false);
        hideState();
        conditionsMet();
    }

    /// prefill selected lights and their states.
    void prefill(const std::vector<cor::Light>& lights) {
        clear();
        mOriginalLights = lights;
        for (const auto& light : lights) {
            mMoodLights->addLight(light);
        }
        mLightsMenu->ignoreLights(cor::lightVectorToIDs(mMoodLights->lights()));
        conditionsMet();
    }

    /// true if any information does not match the original information, false otherwise
    bool hasEdits() override { return !(mOriginalLights == lights()); }

    /// getter for all selected lights.
    std::vector<cor::Light> lights() { return mMoodLights->lights(); }

protected:
    /*!
     * \brief resizeEvent called whenever the widget resizes so that assets can be updated.
     */
    void resizeEvent(QResizeEvent*) override { resize(); }

private slots:

    /// handles when a light is clicked. it will either add or remove lights from the selected list.
    void lightClicked(cor::Light light) {
        bool shouldRemove = (light.uniqueID() == mLightWidget->light().uniqueID());
        auto lights = mMoodLights->lights();
        auto lightResult = cor::findLightInVectorByID(lights, light.uniqueID());
        // deselect whatever is selected on the selected lights menu
        mMoodLights->highlightLights({});
        if (shouldRemove) {
            hideState();
            mLightsMenu->clearSelection();
        } else {
            handleState(EChooseLightsMoodState::addLight);
            showState(light);
        }
    }

    /// handles when a light is clicked from the lights menu.
    void lightClicked(QString key) { lightClicked(mComm->lightByID(key)); }

    /// handles when the left top button is pressed. what this button does depends on the state of
    /// the widget.
    void leftButtonPressed() {
        if (mState == EChooseLightsMoodState::addLight
            || mState == EChooseLightsMoodState::changeLightState) {
            auto light = mLightWidget->light();
            mLightWidget->updateWidget(light);
            mMoodLights->addLight(light);
            mLightsMenu->clearSelection();
            mMoodLights->highlightLights({light.uniqueID()});
            mLightsMenu->ignoreLights(cor::lightVectorToIDs(mMoodLights->lights()));
            handleState(EChooseLightsMoodState::removeLight);
        } else if (mState == EChooseLightsMoodState::removeLight) {
            mMoodLights->removeLight(mLightWidget->light());
            mLightsMenu->ignoreLights(cor::lightVectorToIDs(mMoodLights->lights()));
            mLightsMenu->highlightLight(mLightWidget->light().uniqueID());
            handleState(EChooseLightsMoodState::addLight);
        }
        conditionsMet();
    }

    /// handles when the right top button is pressed. What this button does depends on the state of
    /// the widget.
    void rightButtonPressed() {
        if (mState == EChooseLightsMoodState::changeLightState) {
            // cancel button pressed, if the selected light is currently in the mood, switch back to
            // "remove" state. If the selected light hasn't been added yet, switch back to the "add"
            // state.
            if (mMoodLights->highlightedLights().size() == 1) {
                handleState(EChooseLightsMoodState::removeLight);
            } else {
                handleState(EChooseLightsMoodState::addLight);
            }
        } else if (mState == EChooseLightsMoodState::addLight
                   || mState == EChooseLightsMoodState::removeLight) {
            handleState(EChooseLightsMoodState::changeLightState);
        }
    }


    // a mood light is clicked, this either deselects the light, or selects the light for
    // editing/removing
    void moodLightClicked(cor::Light light) {
        bool shouldRemove = (light.uniqueID() == mLightWidget->light().uniqueID());
        if (shouldRemove) {
            hideState();
            mMoodLights->highlightLights({});
        } else {
            handleState(EChooseLightsMoodState::removeLight);
            showState(light);
            mLightsMenu->clearSelection();
        }
    }

    /// the standard lights menu deselected a light
    void unselectLight(QString key) {
        bool shouldRemove = (key == mLightWidget->light().uniqueID());
        handleState(EChooseLightsMoodState::disabled);
        if (shouldRemove) {
            hideState();
        }
    }

    /// state is updated from the state widget
    void stateUpdated(cor::LightState state) {
        auto light = mLightWidget->light();
        light.state(state);
        mLightWidget->updateWidget(light);
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

        if (mMoodLights->lights().size() > 0 && allLightsValid) {
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

        mLightWidget->setGeometry(0, yPos, this->width() / 2, buttonHeight);
        mLeftButton->setGeometry(mLightWidget->width(), yPos, this->width() / 4, buttonHeight);
        mRightButton->setGeometry(mLightWidget->width() + mLeftButton->width(),
                                  yPos,
                                  this->width() / 4,
                                  buttonHeight);

        yPos += mLightWidget->height();
        if (mLightWidget->light().isValid()) {
            mLightWidget->setVisible(true);
        } else {
            mLightWidget->setVisible(false);
        }
        if (mState == EChooseLightsMoodState::addLight
            || mState == EChooseLightsMoodState::removeLight
            || mState == EChooseLightsMoodState::disabled) {
            QRect topRect(0, yPos, this->width() / 2, buttonHeight);

            mChooseLabel->setGeometry(0, yPos, int(this->width() / 2 * 0.95), buttonHeight);

            mSelectedLabel->setGeometry(int(this->width() / 2 * 1.05),
                                        yPos,
                                        this->width() / 2,
                                        buttonHeight);
            yPos += mChooseLabel->height();


            // handle widget sizes
            QRect lightRect(0, yPos, int((this->width() / 2) * 0.95), 7 * buttonHeight);
            mLightsMenu->resize(lightRect, mRowHeight);

            QRect selectedLightsRect(int(this->width() / 2 * 1.05),
                                     lightRect.y(),
                                     lightRect.width(),
                                     7 * buttonHeight);
            mMoodLights->resize(selectedLightsRect, mRowHeight);
            yPos += mLightsMenu->height();
        } else if (mState == EChooseLightsMoodState::changeLightState) {
            mStateWidget->setGeometry(0, yPos, this->width(), 8 * buttonHeight);
            yPos += mLightsMenu->height();
        }

        mBottomButtons->setGeometry(0, this->height() - buttonHeight, this->width(), buttonHeight);
    }

    /// handle the state of the widget
    void handleState(EChooseLightsMoodState state) {
        mState = state;
        mLeftButton->setEnabled(state != EChooseLightsMoodState::disabled);
        mRightButton->setEnabled(state != EChooseLightsMoodState::disabled);

        // handle whats visible
        if (mState == EChooseLightsMoodState::addLight
            || mState == EChooseLightsMoodState::removeLight) {
            mChooseLabel->setVisible(true);
            mSelectedLabel->setVisible(true);
            mLightsMenu->setVisible(true);
            mMoodLights->setVisible(true);
            mStateWidget->setVisible(false);

            mLeftButton->setVisible(true);
            mRightButton->setVisible(true);
        } else if (mState == EChooseLightsMoodState::changeLightState) {
            mChooseLabel->setVisible(false);
            mSelectedLabel->setVisible(false);
            mLightsMenu->setVisible(false);
            mMoodLights->setVisible(false);
            mStateWidget->setVisible(true);

            mLeftButton->setVisible(true);
            mRightButton->setVisible(true);
        } else if (mState == EChooseLightsMoodState::disabled) {
            mChooseLabel->setVisible(true);
            mSelectedLabel->setVisible(true);
            mLightsMenu->setVisible(true);
            mMoodLights->setVisible(true);
            mStateWidget->setVisible(false);

            mLeftButton->setVisible(false);
            mRightButton->setVisible(false);
        }

        // handle button text
        if (mState == EChooseLightsMoodState::addLight) {
            mLeftButton->setText("Add");
            mRightButton->setText("Edit");
        } else if (mState == EChooseLightsMoodState::removeLight) {
            mLeftButton->setText("Remove");
            mRightButton->setText("Edit");
        } else if (mState == EChooseLightsMoodState::changeLightState) {
            mLeftButton->setText("Add");
            mRightButton->setText("Cancel");
            resize();
        }
    }

    /// called when the state should be hidden
    void hideState() {
        cor::Light light;
        mLightWidget->updateWidget(light);
        mLightWidget->setVisible(false);
        handleState(EChooseLightsMoodState::disabled);
    }

    /// called whnen the state should be shown
    void showState(cor::Light light) {
        light.isReachable(true);
        mStateWidget->updateState(light.state(), light.protocol());
        mLightWidget->updateWidget(light);
        mLightWidget->setVisible(true);
    }


    /// label for left hand column
    QLabel* mChooseLabel;

    /// label for right hand column
    QLabel* mSelectedLabel;

    /// pointer to comm data
    CommLayer* mComm;

    /// widget for showing all available lights.
    StandardLightsMenu* mLightsMenu;

    /// widget for showing all selected lights
    LightsListMenu* mMoodLights;

    /// left button at the top of the widget
    QPushButton* mLeftButton;

    /// right button at the top of the widget
    QPushButton* mRightButton;

    /// top widget that shows the selected light
    ListLightWidget* mLightWidget;

    /// widget for choosing a state of a light
    ChooseStateWidget* mStateWidget;

    /// state of the widget
    EChooseLightsMoodState mState;

    /// stores the original lights, to check for changes.
    std::vector<cor::Light> mOriginalLights;

    /// height of rows in scroll areas.
    int mRowHeight;
};


#endif // CHOOSELIGHTSMOODWIDGET_H
