/*!
 * \copyright
 * Copyright (C) 2015 - 2020.
 * Released under the GNU General Public License.
 */

#include "stateobserver.h"

#include "topmenu.h"

namespace cor {

StateObserver::StateObserver(cor::LightList* data,
                             CommLayer* comm,
                             AppData* appData,
                             AppSettings* appSettings,
                             MainWindow* mainWindow,
                             LightsPage* lightsPage,
                             TopMenu* topMenu,
                             QObject* parent)
    : QObject(parent),
      mData{data},
      mComm{comm},
      mAppData{appData},
      mAppSettings{appSettings},
      mMainWindow{mainWindow},
      mLightsPage{lightsPage},
      mTopMenu{topMenu},
      mMainViewport{mMainWindow->viewport()},
      mColorPage{mMainViewport->colorPage()},
      mPalettePage{mMainViewport->palettePage()},
      mMoodPage{mMainViewport->moodPage()},
      mSpeed{100},
      mTimeObserver{new TimeObserver(this)} {}


void StateObserver::globalBrightnessChanged(std::uint32_t brightness) {
    mData->isOn((brightness > 0));
    // NOTE: in most cases, a computeState() call here would be sufficient. However, if all lights
    // are showing different colors, computeStae would override their colors with either the
    // selection from the color page or palette page. This is bad for moods, or for just dimming the
    // lights a bit. This function will modify the brightness of all selected lights.
    mData->updateBrightness(brightness);
    updateTime();

    // UI updates. These shouldn't signal any further changes, but since a UI element has updated
    if (mMainViewport->currentPage() == EPage::colorPage) {
        mColorPage->updateBrightness(brightness);
        if (mData->supportsRoutines()) {
            mTopMenu->singleColorStateWidget()->updateState(
                mData->mainColor(),
                mMainWindow->singleRoutines()->state().routine());
        } else {
            mTopMenu->singleColorStateWidget()->updateState(mData->mainColor(),
                                                            ERoutine::singleSolid);
        }
    } else if (mMainViewport->currentPage() == EPage::palettePage) {
        //   mPalettePage->updateBrightness(brightness);
        mTopMenu->multiColorStateWidget()->updateState(mData->multiColorScheme());
    }
}

void StateObserver::ambientColorChanged(std::uint32_t temperature, std::uint32_t brightness) {
    mBrightness = brightness;
    mTemperature = temperature;
    mData->isOn(true);

    computeState();
    updateTime();
}

void StateObserver::colorChanged(QColor color) {
    computeState();
    updateTime();
    mData->isOn(true);

    // grab the values from the routine container
    mSpeed = mMainWindow->singleRoutines()->state().speed();
    mRoutineParameter = mMainWindow->singleRoutines()->state().param();

    // UI update
    mMainWindow->singleRoutines()->changeColor(color);
}

void StateObserver::routineChanged(ERoutine, int speed, int param) {
    mData->isOn(true);

    mSpeed = speed;
    mRoutineParameter = param;

    computeState();
    updateTime();
}

void StateObserver::isOnChanged(bool isOn) {
    mData->isOn(isOn);
    computeState();
    updateTime();
}


void StateObserver::paletteChanged(cor::Palette) {
    mData->isOn(true);

    computeState();
    updateTime();
}

void StateObserver::timeoutChanged(bool enabled, std::uint32_t value) {
    bool shouldUpdate = false;
    if (enabled != mAppSettings->timeoutEnabled()) {
        shouldUpdate = true;
        mAppSettings->enableTimeout(enabled);
    }

    if (value != std::uint32_t(mAppSettings->timeout())) {
        shouldUpdate = true;
        mAppSettings->updateTimeout(value);
    }

    if (shouldUpdate) {
        mMainWindow->leftHandMenu()->updateTimeoutButton(enabled, value);
    }
    updateTime();
}

void StateObserver::multiColorSelectionChange(std::uint32_t, const QColor& color) {
    // changing the selection doesnt change the light states, but it does change what displays
    // in the single light brightness widget.
    if (mMainViewport->currentPage() == EPage::palettePage) {
        //        if (mColorPage->colorPicker()->currentScheme() == EColorSchemeType::custom) {
        //            mTopMenu->singleLightBrightness()->updateColor(color);
        //        }
    }
}

void StateObserver::updateScheme(const std::vector<QColor>& colors) {
    mData->isOn(true);
    updateTime();

    mData->updateColorScheme(colors);
    mTopMenu->updateScheme(colors);
}

void StateObserver::colorSchemeTypeChanged(EColorSchemeType) {
    mTopMenu->handleBrightnessSliders();
}

void StateObserver::speedChanged(std::uint32_t speed) {
    mData->updateSpeed(speed);
}

void StateObserver::protocolSettingsChanged(EProtocolType type, bool enabled) {
    if (enabled) {
        mComm->startup(type);
    } else {
        mComm->shutdown(type);
        mData->removeLightOfType(type);
    }
}

void StateObserver::moodChanged(cor::Mood mood) {
    const auto& result = mAppData->moods()->moods().item(mood.uniqueID().toStdString());
    if (result.second) {
        mData->clearLights();
        const auto& moodDict = mComm->makeMood(result.first);
        mData->addMood(moodDict.items());
        if (!moodDict.items().empty()) {
            mTopMenu->lightCountChanged();
        }
    }
}

void StateObserver::lightCountChanged() {
    mTopMenu->lightCountChanged();
    mMainViewport->timeoutPage()->updateLights();
    mLightsPage->highlightLights();
    auto protocolType = mData->mostFeaturedProtocolType();

    if (mColorPage->isOpen()) {
        if (mData->empty() || !mData->supportsRoutines()) {
            mMainWindow->pushOutSingleRoutinePage();
            mTopMenu->closeRoutinesPage();
        } else {
            auto routineAndParam = mData->routineAndParam();
            mMainWindow->singleRoutines()->highlightRoutine(routineAndParam.first,
                                                            routineAndParam.second);
            mMainWindow->singleRoutines()->changeColor(mData->mainColor());
            mMainWindow->singleRoutines()->changeProtocol(protocolType);
            /// main color used on palette page for speed slider
            mMainWindow->multiRoutines()->changeColor(mData->mainColor());
            mMainWindow->multiRoutines()->changeProtocol(protocolType);
        }
    }

    if (mPalettePage->isOpen()) {
        if (mData->empty() || !mData->supportsRoutines()) {
            mMainWindow->pushOutMultiRoutinePage();
            //            mTopMenu->highlightButton("Preset");
        } else {
            auto routineAndParam = mData->routineAndParam();
            mMainWindow->multiRoutines()->highlightRoutine(routineAndParam.first,
                                                           routineAndParam.second);
            mMainWindow->multiRoutines()->changeColorScheme(mData->multiColorScheme());
            mMainWindow->multiRoutines()->changeProtocol(protocolType);
        }
    }
}

void StateObserver::dataInSync(bool inSync) {
    ESyncState state = ESyncState::syncing;
    if (inSync) {
        state = ESyncState::synced;
    }
    if (mMainViewport->currentPage() == EPage::colorPage) {
        mTopMenu->singleColorStateWidget()->updateSyncStatus(state);
    } else if (mMainViewport->currentPage() == EPage::palettePage) {
        mPalettePage->detailedWidget()->updateSyncStatus(state);
    } else if (mMainViewport->currentPage() == EPage::moodPage) {
        mMoodPage->moodDetailedWidget()->updateSyncStatus(state);
    }
}

void StateObserver::computeState() {
    // check the current page
    switch (mMainViewport->currentPage()) {
        case EPage::colorPage: {
            cor::LightState state;
            state.isOn(mData->isOn());
            if (mData->supportsRoutines()) {
                state.routine(mMainWindow->singleRoutines()->state().routine());
            } else {
                state.routine(ERoutine::singleSolid);
            }

            if (mColorPage->pageType() == EColorPickerMode::ambient) {
                state.temperature(mTemperature);
                auto color = cor::colorTemperatureToRGB(mTemperature);
                color.setHsvF(color.hueF(), color.saturationF(), mBrightness / 100.0);
                state.color(color);
            } else {
                state.color(mColorPage->color());
            }
            if (state.routine() != ERoutine::singleSolid) {
                state.param(mRoutineParameter);
                state.speed(mSpeed);
            }
            mMainWindow->singleRoutines()->highlightRoutine(state.routine(), state.param());
            mData->updateState(state);
            mTopMenu->updateState(state);
            break;
        }
        case EPage::palettePage: {
            cor::LightState state;
            state.isOn(mData->isOn());
            state.speed(mSpeed);
            state.routine(mMainWindow->multiRoutines()->state().routine());
            state.param(mMainWindow->multiRoutines()->state().param());
            auto palette = mPalettePage->palette();
            state.paletteBrightness(mTopMenu->globalBrightness()->brightness());
            state.palette(palette);

            mMainWindow->multiRoutines()->highlightRoutine(state.routine(), state.param());
            /// main color used on palette page for speed slider
            mMainWindow->multiRoutines()->changeColor(state.palette().averageColor());
            mData->updateState(state);
            mTopMenu->updateState(state);
            break;
        }
        case EPage::moodPage:
        case EPage::settingsPage:
        case EPage::lightsPage:
        case EPage::timeoutPage:
        case EPage::MAX:
            // for all of these cases, theres no universal state for the page.
            break;
    }
} // namespace cor

void StateObserver::updateTime() {
    mTimeObserver->updateTime();
}

void StateObserver::lightNameChange(const cor::LightID& uniqueID, const QString&) {
    // qDebug() << " TODO: light name changed in StateObserver: " << key << " to " << name;
    // get light by uniqueID
    auto light = mComm->lightByID(uniqueID);
    if (light.isValid()) {
        mLightsPage->updateLightNames(light.protocol());
    }
}

void StateObserver::lightsAdded(std::vector<cor::LightID> keys) {
    qDebug() << "INFO: lights added " << cor::lightIDVectorToStringVector(keys);
    mAppData->addLightsToGroups(keys);
    mMainWindow->leftHandMenu()->updateLights();
    if (!mMainWindow->anyDiscovered()) {
        mMainWindow->anyDiscovered(true);
    }
}

void StateObserver::lightsDeleted(std::vector<cor::LightID> keys) {
    qDebug() << "INFO: lights deleted " << cor::lightIDVectorToStringVector(keys);

    mData->removeByIDs(keys);
    mAppData->lightsDeleted(keys);
    mLightsPage->handleDeletedLights(keys);
    mMainWindow->leftHandMenu()->clearWidgets();
    mMainWindow->leftHandMenu()->updateLights();
    lightCountChanged();
}

void StateObserver::lightCountChangedFromLightsPage(std::vector<cor::LightID>) {
    mMainWindow->leftHandMenu()->updateLights();
    lightCountChanged();
}

void StateObserver::connectionStateChanged(EProtocolType type, EConnectionState newState) {
    mTopMenu->updateLightsButton(type, newState);
    if (newState == EConnectionState::discovered && !mMainWindow->anyDiscovered()) {
        mMainWindow->anyDiscovered(true);
    }
}
} // namespace cor
