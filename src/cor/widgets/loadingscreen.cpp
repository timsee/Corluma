#include "loadingscreen.h"
#include <QTimer>
#include "comm/commarducor.h"
#include "comm/commhue.h"
#include "comm/commnanoleaf.h"

#define MIN_TIME_TO_SHOW_LOADING_SCREEN 5
#define MAX_TIME_TO_SHOW_LOADING_SCREEN 15

LoadingScreen::LoadingScreen(CommLayer* comm, AppSettings* appSettings, QWidget* parent)
    : QWidget(parent),
      mComm{comm},
      mAppSettings{appSettings},
      mAnimation{new QQuickWidget(QUrl::fromLocalFile(":loading.qml"), this)},
      mText{new QLabel("Looking for Lights...", this)},
      mUpdateTimer{new QTimer(this)},
      mCounter{0u},
      mIsReady{false} {
    mText->setAlignment(Qt::AlignHCenter | Qt::AlignTop);
    mAnimation->setResizeMode(QQuickWidget::SizeRootObjectToView);
    mAnimation->setAttribute(Qt::WA_AlwaysStackOnTop);
    mAnimation->setClearColor(QColor(48, 47, 47));

    mText->setStyleSheet("font-size:16pt;");
    connect(mUpdateTimer, SIGNAL(timeout()), this, SLOT(handleUpdate()));
    mUpdateTimer->start(1000);
}

void LoadingScreen::resizeEvent(QResizeEvent*) {
    auto yPos = 0;
    mAnimation->setGeometry(0, yPos, width(), height() * 0.8);
    yPos += mAnimation->height();

    mText->setGeometry(0, yPos, width(), height() * 0.2);
}

void LoadingScreen::handleUpdate() {
    auto textUpdate = mCounter % 4;
    switch (textUpdate) {
        case 0:
            mText->setText("Looking for Lights.   ");
            break;
        case 1:
            mText->setText("Looking for Lights..  ");
            break;
        case 2:
            mText->setText("Looking for Lights..  ");
            break;
        default:
            mText->setText("Looking for Lights....");
            break;
    }


    if (mCounter < MIN_TIME_TO_SHOW_LOADING_SCREEN) {
        // don't exit yet, give time to pre-load lights that are quick to connect
    } else if (mCounter > MAX_TIME_TO_SHOW_LOADING_SCREEN) {
        // force out of the loading screen if its taking longer than MAX_TIME_TO_SHOW_LOADING_SCREEN
        // seconds.
        markAsReady();
    } else {
        // standard case, check if expected lights have connected for each protocol type.
        if (verifyIfEnoughLightsConnected()) {
            markAsReady();
        }
    }
    mCounter++;
}


bool LoadingScreen::verifyIfEnoughLightsConnected() {
    bool hueIsReady = false;
    if (mAppSettings->enabled(EProtocolType::hue)) {
        hueIsReady = mComm->hue()->discovery()->notFoundBridges().empty();
    } else {
        hueIsReady = true;
    }

    bool arduCorIsReady = false;
    if (mAppSettings->enabled(EProtocolType::arduCor)) {
        arduCorIsReady = mComm->arducor()->discovery()->undiscoveredControllers().empty();
    } else {
        arduCorIsReady = true;
    }

    bool nanoleafIsReady = false;
    if (mAppSettings->enabled(EProtocolType::nanoleaf)) {
        nanoleafIsReady = !mComm->nanoleaf()->discovery()->foundLights().empty();
    } else {
        nanoleafIsReady = true;
    }

    //    qDebug() << " hue ready to start: " << hueIsReady
    //             << " nanoleaf ready to start: " << nanoleafIsReady << " arducor ready to start "
    //             << arduCorIsReady;

    return hueIsReady && arduCorIsReady && nanoleafIsReady;
}
