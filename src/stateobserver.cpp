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
                             GroupData* groups,
                             MainWindow* mainWindow,
                             TopMenu* topMenu,
                             QObject* parent)
    : QObject(parent),
      mData{data},
      mComm{comm},
      mGroups{groups},
      mMainWindow{mainWindow},
      mTopMenu{topMenu},
      mMainViewport{mMainWindow->viewport()},
      mColorPage{mMainViewport->colorPage()},
      mPalettePage{mMainViewport->palettePage()},
      mMoodPage{mMainViewport->moodPage()},
      mSpeed{100} {}


void StateObserver::globalBrightnessChanged(std::uint32_t brightness) {
    mIsOn = (brightness > 0);
    // NOTE: in most cases, a computeState() call here would be sufficient. However, if all lights
    // are showing different colors, computeStae would override their colors with either the
    // selection from the color page or palette page. This is bad for moods, or for just dimming the
    // lights a bit. This function will modify the brightness of all selected lights.
    mData->updateBrightness(brightness);

    // UI updates. These shouldn't signal any further changes, but since a UI element has updated
    if (mMainViewport->currentPage() == EPage::colorPage) {
        mColorPage->updateBrightness(brightness);
        mTopMenu->singleColorStateWidget()->updateState(mData->mainColor(),
                                                        mMainWindow->routineWidget()->routine());
    }
    if (mMainViewport->currentPage() == EPage::palettePage) {
        mPalettePage->updateBrightness(brightness);
        mTopMenu->multiColorStateWidget()->updateState(mData->colorScheme());
    }
}

void StateObserver::singleLightBrightnessChanged(std::uint32_t brightness) {
    if (mPalettePage->mode() == EGroupMode::HSV) {
        // update the color scheme color in mData
        auto scheme = mData->colorScheme();
        auto color = scheme[mPalettePage->colorPicker()->selectedLight()];
        color.setHsvF(color.hueF(), color.saturationF(), brightness / 100.0);
        scheme[mPalettePage->colorPicker()->selectedLight()] = color;
        // now that brightness is applied, treat the rest of this update as a standard scheme
        // update.
        updateScheme(scheme, mPalettePage->colorPicker()->selectedLight());
        // update the states of the color picker widget as well, since this state change is coming
        // from another widget
        mPalettePage->colorPicker()->updateColorStates(scheme);
        mPalettePage->updateBrightness(brightness);
    }
}

void StateObserver::ambientColorChanged(std::uint32_t temperature, std::uint32_t brightness) {
    mBrightness = brightness;
    mTemperature = temperature;
    mIsOn = true;
    computeState();
}

void StateObserver::colorChanged(QColor color) {
    mIsOn = true;
    computeState();

    // UI update
    mMainWindow->routineWidget()->singleRoutineColorChanged(color);
}

void StateObserver::routineChanged(ERoutine) {
    mIsOn = true;
    computeState();
}

void StateObserver::isOnChanged(bool isOn) {
    mIsOn = isOn;
    computeState();
}


void StateObserver::routineChanged(ERoutine, EPalette) {
    computeState();
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
    mIsOn = true;
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

void StateObserver::moodChanged(std::uint64_t moodID) {
    const auto& result = mGroups->moods().item(QString::number(moodID).toStdString());
    if (result.second) {
        mData->clearLights();
        const auto& moodDict = mComm->makeMood(result.first);
        mData->addLights(moodDict.items());
        if (!moodDict.items().empty()) {
            mTopMenu->lightCountChanged();
            mMainWindow->leftHandMenu()->deviceCountChanged();
        }
    }
}

void StateObserver::lightCountChanged() {
    mTopMenu->lightCountChanged();
}

void StateObserver::dataInSync(bool inSync) {
    ESyncState state = ESyncState::syncing;
    if (inSync) {
        state = ESyncState::synced;
    }
    if (mMainViewport->currentPage() == EPage::colorPage) {
        mTopMenu->singleColorStateWidget()->updateSyncStatus(state);
    } else if (mMainViewport->currentPage() == EPage::palettePage) {
        mTopMenu->multiColorStateWidget()->updateSyncStatus(state);
    }
}

void StateObserver::computeState() {
    // check the current page
    switch (mMainViewport->currentPage()) {
        case EPage::colorPage: {
            cor::LightState state;
            state.isOn(mIsOn);
            state.routine(mMainWindow->routineWidget()->routine());
            if (mColorPage->pageType() == ESingleColorPickerMode::ambient) {
                state.temperature(mTemperature);
                auto color = cor::colorTemperatureToRGB(mTemperature);
                color.setHsvF(color.hueF(), color.saturationF(), mBrightness / 100.0);
                state.color(color);
            } else {
                state.color(mColorPage->color());
            }
            if (state.routine() != ERoutine::singleSolid) {
                state.param(mMainWindow->routineWidget()->parameter());
                state.speed(mSpeed);
            }
            mData->updateState(state);
            mTopMenu->updateState(state);
            break;
        }
        case EPage::palettePage: {
            cor::LightState state;
            state.isOn(mIsOn);
            state.routine(mMainWindow->routineWidget()->routine());
            state.speed(mSpeed);
            state.param(mMainWindow->routineWidget()->parameter());
            Palette palette(paletteToString(mPalettePage->palette()),
                            mPalettePage->colorScheme(),
                            100u);
            // skip global brightness if HSV and using a custom scheme
            if (mPalettePage->mode() != EGroupMode::HSV
                || mPalettePage->colorPicker()->currentScheme() != EColorSchemeType::custom) {
                palette.brightness(mTopMenu->globalBrightness()->brightness());
            }
            state.palette(palette);
            mData->updateState(state);
            mTopMenu->updateState(state);
            break;
        }
        case EPage::moodPage:
        case EPage::settingsPage:
        case EPage::discoveryPage:
        case EPage::MAX:
            // for all of these cases, theres no universal state for teh page.
            break;
    }
}

} // namespace cor
