#ifndef TIMEOUTBUTTON_H
#define TIMEOUTBUTTON_H

#include <QTimer>
#include <QWidget>
#include "comm/commlayer.h"
#include "cor/lightlist.h"
#include "lefthandbutton.h"
#include "timeobserver.h"
#include "utils/cormath.h"

/*!
 * \brief The TimeoutButton class inherits a LeftHandButton and is intended for displaying the
 * Timeout state on the LeftHandMenu. It will show different text based off of if timeouts are or
 * not, and will display the timeout if it is enabled.
 */
class TimeoutButton : public LeftHandButton {
    Q_OBJECT
public:
    explicit TimeoutButton(const QString& text,
                           EPage page,
                           const QString& iconResource,
                           CommLayer* comm,
                           cor::LightList* data,
                           LeftHandMenu* menu)
        : LeftHandButton(text, page, iconResource, menu),
          mRenderTimer{new QTimer(this)},
          mComm{comm},
          mData{data} {
        connect(mRenderTimer, SIGNAL(timeout()), this, SLOT(renderUI()));
        mRenderTimer->start(333);
    };

    /// update the metadata about timeouts
    void update(bool enabled, std::uint32_t timeoutValue) {
        mTimeoutValue = timeoutValue;
        mTimeoutEnabled = enabled;
        renderUI();
    }

private slots:
    /// renders the timeout values
    void renderUI() {
        if (mTimeoutEnabled) {
            auto lights = mData->lights();
            if (!isAnyLightOn(lights)) {
                mTitle->setText("Timeout: " + QString::number(mTimeoutValue));
            } else {
                auto timeouts = mComm->secondsUntilTimeout(cor::lightVectorToIDs(lights));
                auto modeTimeout = std::round(cor::mode(timeouts) / 60.0);
                mTitle->setText("Timeout: " + QString::number(modeTimeout));
            }
        } else {
            mTitle->setText("Timeout Disabled");
        }
    }

private:
    /// returns true if any light is on, false if not.
    bool isAnyLightOn(const std::vector<cor::Light> lights) {
        auto commLights = mComm->lightsByIDs(cor::lightVectorToIDs(lights));
        for (const auto& light : commLights) {
            if (light.state().isOn()) {
                return true;
            }
        }
        return false;
    }

    /// timer that renders the UI
    QTimer* mRenderTimer;

    /// pointer to comm data
    CommLayer* mComm;

    /// pointer to selected lights
    cor::LightList* mData;

    /// value for how many minutes until timeout
    std::uint32_t mTimeoutValue;

    /// true if timeout is enabled, false otherwise
    bool mTimeoutEnabled;
};

#endif // TIMEOUTBUTTON_H
