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
                mColorPage->routines()->state().routine());
        } else {
            mTopMenu->singleColorStateWidget()->updateState(mData->mainColor(),
                                                            ERoutine::singleSolid);
        }
    } else if (mMainViewport->currentPage() == EPage::palettePage) {
        mPalettePage->updateBrightness(brightness);
        mTopMenu->multiColorStateWidget()->updateState(mData->multiColorScheme());
    }
}

void StateObserver::singleLightBrightnessChanged(std::uint32_t brightness) {
    if (mPalettePage->mode() == EGroupMode::wheel) {
        // update the color scheme color in mData
        auto scheme = mData->multiColorScheme();
        auto color = scheme[mPalettePage->colorPicker()->selectedLight()];
        color.setHsvF(color.hueF(), color.saturationF(), brightness / 100.0);
        scheme[mPalettePage->colorPicker()->selectedLight()] = color;
        // now that brightness is applied, treat the rest of this update as a standard scheme
        // update.
        updateScheme(scheme, mPalettePage->colorPicker()->selectedLight());
        // update the states of the color picker widget as well, since this state change is coming
        // from another widget
        mPalettePage->updateBrightness(brightness);
        updateTime();
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
    mSpeed = mColorPage->routines()->state().speed();
    mRoutineParameter = mColorPage->routines()->state().param();

    // UI update
    mColorPage->routines()->changeColor(color);
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
        if (mPalettePage->colorPicker()->currentScheme() == EColorSchemeType::custom) {
            mTopMenu->singleLightBrightness()->updateColor(color);
        }
    }
}

void StateObserver::updateScheme(const std::vector<QColor>& colors, std::uint32_t index) {
    mData->isOn(true);
    updateTime();

    mData->updateColorScheme(colors);
    mTopMenu->updateScheme(colors, index);
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

void StateObserver::moodChanged(QString moodID) {
    const auto& result = mAppData->moods()->moods().item(moodID.toStdString());
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
            mColorPage->showRoutines(false);
            mTopMenu->closeRoutinesPage();
        } else {
            auto routineAndParam = mData->routineAndParam();
            mColorPage->routines()->highlightRoutine(routineAndParam.first, routineAndParam.second);
            mColorPage->routines()->changeColor(mData->mainColor());
            mColorPage->routines()->changeProtocol(protocolType);
            /// main color used on palette page for speed slider
            mPalettePage->routines()->changeColor(mData->mainColor());
            mPalettePage->routines()->changeProtocol(protocolType);
        }
    }

    if (mPalettePage->isOpen()) {
        if (mData->empty() || !mData->supportsRoutines()) {
            mPalettePage->setMode(EGroupMode::presets);
            mTopMenu->highlightButton("Preset");
        } else {
            auto routineAndParam = mData->routineAndParam();
            mPalettePage->routines()->highlightRoutine(routineAndParam.first,
                                                       routineAndParam.second);
            mPalettePage->routines()->changeColorScheme(mData->multiColorScheme());
            mPalettePage->routines()->changeProtocol(protocolType);
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
                state.routine(mColorPage->routines()->state().routine());
            } else {
                state.routine(ERoutine::singleSolid);
            }

            if (mColorPage->pageType() == ESingleColorPickerMode::ambient) {
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
            mColorPage->routines()->highlightRoutine(state.routine(), state.param());
            mData->updateState(state);
            mTopMenu->updateState(state);
            break;
        }
        case EPage::palettePage: {
            cor::LightState state;
            state.isOn(mData->isOn());
            state.speed(mSpeed);
            state.routine(mPalettePage->routines()->state().routine());

            if (mPalettePage->mode() == EGroupMode::wheel) {
                state.param(mPalettePage->routines()->state().param());
                auto palette = mPalettePage->palette();
                if (mPalettePage->colorPicker()->currentScheme() != EColorSchemeType::custom) {
                    state.paletteBrightness(mTopMenu->globalBrightness()->brightness());
                }
                state.palette(palette);
            } else if (mPalettePage->mode() == EGroupMode::presets
                       || mPalettePage->mode() == EGroupMode::routines) {
                state.param(mPalettePage->routines()->state().param());
                auto palette = mPalettePage->palette();
                state.paletteBrightness(mTopMenu->globalBrightness()->brightness());
                state.palette(palette);
            }
            mPalettePage->routines()->highlightRoutine(state.routine(), state.param());
            /// main color used on palette page for speed slider
            mPalettePage->routines()->changeColor(state.palette().averageColor());
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
}

void StateObserver::updateTime() {
    mTimeObserver->updateTime();
}

void StateObserver::lightNameChange(const QString& uniqueID, const QString&) {
    // qDebug() << " TODO: light name changed in StateObserver: " << key << " to " << name;
    // get light by uniqueID
    auto light = mComm->lightByID(uniqueID);
    if (light.isValid()) {
        mLightsPage->updateLightNames(light.protocol());
    }
}

void StateObserver::lightsAdded(std::vector<QString> keys) {
    qDebug() << "INFO: lights added " << keys;
    mAppData->addLightsToGroups(keys);
    mMainWindow->leftHandMenu()->updateLights();
    if (!mMainWindow->anyDiscovered()) {
        mMainWindow->anyDiscovered(true);
    }
}

void StateObserver::lightsDeleted(std::vector<QString> keys) {
    qDebug() << "INFO: lights deleted " << keys;

    mData->removeByIDs(keys);
    mAppData->lightsDeleted(keys);
    mLightsPage->handleDeletedLights(keys);
    mMainWindow->leftHandMenu()->clearWidgets();
    mMainWindow->leftHandMenu()->updateLights();
    lightCountChanged();
}

void StateObserver::lightCountChangedFromLightsPage(std::vector<QString>) {
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
