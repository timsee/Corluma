#ifndef LOADINGSCREEN_H
#define LOADINGSCREEN_H

#include <QLabel>
#include <QQuickWidget>
#include <QSplashScreen>
#include <QWidget>
#include "appsettings.h"
#include "comm/commlayer.h"

/*!
 * \copyright
 * Copyright (C) 2015 - 2021.
 * Released under the GNU General Public License.
 */

/*!
 * \brief The LoadingScreen class is a screen that loads over the MainWindow while the MainWindow
 * discovers lights and loads UI elements.
 */
class LoadingScreen : public QWidget {
    Q_OBJECT
public:
    explicit LoadingScreen(CommLayer* comm, AppSettings* appSettings, QWidget* parent);

    /// true if loading has completed, false if loading is still necessary
    bool isReady() const noexcept { return mIsReady; }

    /// cancel the timer of the page that updates the UI.
    void cancelTimer() { mUpdateTimer->stop(); }

    /// method for generating the initial splash screen for the app. This is different from the
    /// loading screen. A splash screen is always static, and loads before the GUI  has initialized.
    /// The LoadingScreen loads after the GUI has initialized, but before the rest of the app has
    /// loaded.
    static QPixmap splashScreenPixmap(const QSize& screenSize);

signals:
    /// signals when the loading screen has completed loading and is ready to close.
    void readyToClose();

private slots:
    /// runs periodically, checks the "searching for lights" conditions have passed and verifies we
    /// have been on the loading screen for the proper amount of time.
    void handleUpdate();

protected:
    /// handles whenever the app resizes.
    void resizeEvent(QResizeEvent*);

private:
    /// checks each protocol and determines if the conditions to exit the loading screen have been
    /// met.
    bool verifyIfEnoughLightsConnected();

    /// marks the loading screen as "done loading."
    void markAsReady() {
        mIsReady = true;
        emit readyToClose();
    }

    /// pointer to comm layer
    CommLayer* mComm;

    /// pointer to app settings.
    AppSettings* mAppSettings;

    /// animation displayed on the loading screen.
    QQuickWidget* mAnimation;

    /// Text displayed at the bottom of the loading screen.
    QLabel* mText;

    /// timer for handling updates
    QTimer* mUpdateTimer;

    /// counter for how long we've been on the loading screen.
    std::uint32_t mCounter;

    /// true if ready, false if still loading.
    bool mIsReady;
};

#endif // LOADINGSCREEN_H
