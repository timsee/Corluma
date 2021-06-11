#ifndef STATEMANAGER_H
#define STATEMANAGER_H

#include "colorpage.h"
#include "cor/lightlist.h"
#include "mainwindow.h"
#include "palettepage.h"
#include "timeobserver.h"

#include <QObject>

class TopMenu;

namespace cor {

/*!
 * \copyright
 * Copyright (C) 2015 - 2020.
 * Released under the GNU General Public License.
 */


/*!
 * \brief The StateObserver class listens to state changes in any of the main widgets of the
 * application, and computes the intended state for the selected lights. It then triggers any
 * updates that are required for the app. This includes both UI updates and packets that need to be
 * sent to light hardware. The observer is set up from the MainWindow and acts as a sorta arbitrator
 * for all user actions.
 *
 * This class exists to simplify app design. Before its existence, if the user wanted to select a
 * color from the ColorPage, they also needed to query the RoutineButtonsWidget to check what
 * routine to use, and in some cases, the GlobalBrightnessWidget to check how bright the lights
 * should be. This class allows widgets to have less relationships with each other, and instead,
 * theres one class that pays attention to everyone and resolves the complex relationships.
 */
class StateObserver : public QObject {
    Q_OBJECT
public:
    /// constructor
    explicit StateObserver(cor::LightList* data,
                           CommLayer* comm,
                           AppData* appData,
                           AppSettings* appSettings,
                           MainWindow* mainWindow,
                           LightsPage* lightsPage,
                           TopMenu* topMenu,
                           QObject* parent);

public slots:

    /// brightness changed from GlobalBrightnessWidget
    void globalBrightnessChanged(std::uint32_t);

    /// on/off changed from GlobalBrightnessWidget
    void isOnChanged(bool);

    /// brightness changed from SingleLightBrightnessWidget
    void singleLightBrightnessChanged(std::uint32_t);

    /// ambient color changed from color page
    void ambientColorChanged(std::uint32_t, std::uint32_t);

    /// color page changed from color page
    void colorChanged(QColor);

    /// routine changed from routine widget
    void routineChanged(ERoutine, int, int);

    /// palette changed from a palette widget
    void paletteChanged(cor::Palette);

    /// speed changed from a speed widget
    void speedChanged(std::uint32_t);

    /// timeout changed.
    void timeoutChanged(bool, std::uint32_t);

    /// connection state has changed for a specific protocol.
    void connectionStateChanged(EProtocolType type, EConnectionState newState);

    /// getter for the time observer
    TimeObserver* timeObserver() { return mTimeObserver; }

    /*!
     * \brief updateScheme update the color scheme chosen by the app
     * \param colors the colors to use in the new color scheme
     */
    void updateScheme(const std::vector<QColor>& colors, std::uint32_t);

    /// the selection for the multi color picker changed
    void multiColorSelectionChange(std::uint32_t index, const QColor& color);

    /// color scheme changed
    void colorSchemeTypeChanged(EColorSchemeType scheme);

    /// mood changed from moodpage
    void moodChanged(QString mood);

    /// protocolsettings page changed from GlobalSettingsWidget
    void protocolSettingsChanged(EProtocolType, bool);

    /// set if data is in sync or not
    void dataInSync(bool);

    /// light count changed
    void lightCountChanged();

    /*!
     * \brief lightNameChange change the name of a widget given the key and the new name.
     *
     * \param key the key for the light
     * \param name the new name for the light.
     */
    void lightNameChange(const QString& key, const QString& name);

    /// handles when a light is added, updates the UI
    void lightsAdded(std::vector<QString>);

    /// handles when a light is deleted, updates the UI
    void lightsDeleted(std::vector<QString>);

    /// light count changed from lights page.
    void lightCountChangedFromLightsPage(std::vector<QString>);

private:
    /// computes the state based off of the various states of the widget
    void computeState();

    /// update the time that the last update occurred
    void updateTime();

    /// desired state of the lights. Although this can be seen from many parts of the app, the
    /// StateObserver is the only class that uses the setters for cor::LightList
    cor::LightList* mData;

    /// comm layer
    CommLayer* mComm;

    /// app group data
    AppData* mAppData;

    /// app settings
    AppSettings* mAppSettings;

    /// main window of the app
    MainWindow* mMainWindow;

    /// page for managing light connections.
    LightsPage* mLightsPage;

    /// top menu of the app
    TopMenu* mTopMenu;

    /// main viewport of the app
    MainViewport* mMainViewport;

    /// color page
    ColorPage* mColorPage;

    /// palette page
    PalettePage* mPalettePage;

    /// mood page
    MoodPage* mMoodPage;

    /// stored brightness during ambient colors
    std::uint32_t mBrightness;

    /// stored temperature during ambient colors
    std::uint32_t mTemperature;

    /// speed for routines.
    std::uint32_t mSpeed;

    /// parameter for routines. This has a different impact per routine and is not required for all
    /// routines. For instance, for glimmer routines its value between 1 and 100 denoting how
    /// frequently lights should be dimmed to produce a "glimmer". For multiBars, it is the size of
    /// the bars.
    std::uint32_t mRoutineParameter;

    /// observes the last time a state is changed
    TimeObserver* mTimeObserver;
};

} // namespace cor

#endif // STATEMANAGER_H
