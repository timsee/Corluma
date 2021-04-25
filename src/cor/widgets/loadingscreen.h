#ifndef LOADINGSCREEN_H
#define LOADINGSCREEN_H

#include <QLabel>
#include <QWidget>
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
    explicit LoadingScreen(QWidget* parent);

    /// true if loading has completed, false if loading is still necessary
    bool isReady() const noexcept { return mIsReady; }

signals:
    /// signals when the loading screen has completed loading and is ready to close.
    void readyToClose();

private slots:
    /// handles when the startup timer finishes.
    void startupTimerFinished() {
        mIsReady = true;
        emit readyToClose();
    }

protected:
    /// handles whenever the app resizes.
    void resizeEvent(QResizeEvent*);

private:
    /// Image displayed on the loading screen.
    QLabel* mImage;

    /// Text displayed at the bottom of the loading screen.
    QLabel* mText;

    /// true if ready, false if still loading.
    bool mIsReady;
};

#endif // LOADINGSCREEN_H
