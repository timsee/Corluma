#ifndef LIGHTSTIMEOUTMENU_H
#define LIGHTSTIMEOUTMENU_H

#include "comm/commlayer.h"
#include "lightslistmenu.h"

/*!
 * \copyright
 * Copyright (C) 2015 - 2020.
 * Released under the GNU General Public License.
 *
 *
 * \brief The LightsTimeoutMenu class is a LightsListMenu that displays timeouts instead of just the
 * light and its state.
 */
class LightsTimeoutMenu : public LightsListMenu {
    Q_OBJECT
public:
    explicit LightsTimeoutMenu(QWidget* parent, bool allowInteraction, CommLayer* comm)
        : LightsListMenu(parent, allowInteraction),
          mComm{comm} {}

    /// update the timeouts on the widgets.
    void updateTimeouts() {
        auto secondsUntilTimeoutVector = mComm->secondsUntilTimeout(cor::lightVectorToIDs(mLights));
        std::vector<std::pair<cor::LightID, std::uint32_t>> keyTimeoutPairs;
        keyTimeoutPairs.reserve(mLights.size());
        for (const auto& light : mLights) {
            keyTimeoutPairs.emplace_back(
                std::make_pair(light.uniqueID(), mComm->secondsUntilTimeout(light.uniqueID())));
        }
        mLightContainer->showTimeouts(true);
        mLightContainer->updateTimeouts(keyTimeoutPairs);
    }

private:
    /// pointer to comm layer
    CommLayer* mComm;
};

#endif // LIGHTSTIMEOUTMENU_H
