#ifndef MAINVIEWPORT_H
#define MAINVIEWPORT_H

#include <QWidget>

#include "appsettings.h"
#include "colorpage.h"
#include "comm/commlayer.h"
#include "cor/lightlist.h"
#include "data/groupdata.h"
#include "moodpage.h"
#include "palettepage.h"

class MainWindow;

/*!
 * \copyright
 * Copyright (C) 2015 - 2020.
 * Released under the GNU General Public License.
 *
 * \brief The MainViewport class is the main viewport on the mainwindow, and is used
 * to display the standard pages of the app, such as the color page and the mood page
 */
class MainViewport : public QWidget {
    Q_OBJECT
public:
    /// constructor
    explicit MainViewport(MainWindow* parent,
                          CommLayer* comm,
                          cor::LightList* data,
                          GroupData* groups,
                          AppSettings* settings);

    /*!
     * \brief pageChanged change the QStackedWidget to the page specified
     */
    void pageChanged(EPage page, bool skipTransition = false);

    /// resize to given geometry
    void resize(const QRect& geometry);

    /// getter for page
    EPage currentPage() { return mPageIndex; }

    /// getter for the color page
    ColorPage* colorPage() { return mColorPage; }

    /// getter for pallette page
    PalettePage* palettePage() { return mPalettePage; }

    /// getter for mood page
    MoodPage* moodPage() { return mMoodPage; }

    /// loads the mood page programmatically
    void loadMoodPage();

private:
    /*!
     * \brief showMainPage transitions either the color page, group page, or lights page onto the
     * the main screen. The widgets come in from the left or the right, depending on the
     * particular widget
     *
     * \param page the widget to put on the screen.
     */
    void showMainPage(EPage page, bool skipTransition);

    /*!
     * \brief hideMainPage transitions either the color page, group page, or lights page off of the
     * main screen. The widgets go to the left or the right depending the particular widget
     *
     * \param page the page to push off of the screen
     */
    void hideMainPage(EPage page);

    /// getter for the cor::Page given the enum
    cor::Page* mainPage(EPage page);

    /// getter for the QWidget given the enum
    QWidget* mainWidget(EPage page);

    /// page for choosing colors of the LEDs
    ColorPage* mColorPage;

    /// page for choosing group of colors for the LEDs
    PalettePage* mPalettePage;

    /// page for choosing moods
    MoodPage* mMoodPage;

    /// groups parser
    GroupData* mGroups;

    /*!
     * \brief communication pointer to communication object
     * for sending comannds to the lights
     */
    CommLayer* mComm;

    /*!
     * \brief data layer that maintains and tracks the states of the lights
     * and the saved data of the GUI
     */
    cor::LightList* mData;

    /// pointer to main window
    MainWindow* mMainWindow;

    /*!
     * \brief mAppSettings maintains which comnmtypes are currently enabled.
     */
    AppSettings* mAppSettings;

    /*!
     * \brief mPageIndex index of current page.
     */
    EPage mPageIndex;
};

#endif // MAINVIEWPORT_H
