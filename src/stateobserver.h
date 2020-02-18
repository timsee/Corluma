#ifndef STATEMANAGER_H
#define STATEMANAGER_H

#include "colorpage.h"
#include "cor/lightlist.h"
#include "mainwindow.h"
#include "palettepage.h"

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
 * application, and computes the intended state for the selected lights.
 */
class StateObserver : public QObject {
    Q_OBJECT
public:
    /// constructor
    explicit StateObserver(cor::LightList* data,
                           CommLayer* comm,
                           GroupData* groups,
                           MainWindow* mainWindow,
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
    void routineChanged(ERoutine);

    /// routine changed from a palette widget
    void routineChanged(ERoutine, EPalette);

    /// speed changed from a speed widget
    void speedChanged(std::uint32_t);

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
    void moodChanged(std::uint64_t mood);

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

    /// delete a light by unique ID. This deletes the light from memory.
    void deleteLight(const QString& uniqueID);

private:
    /// computes the state based off of the various states of the widget
    void computeState();

    /// desired state of the lights. Although this can be seen from many parts of the app, the
    /// StateObserver is the only class that uses the setters for cor::LightList
    cor::LightList* mData;

    /// comm layer
    CommLayer* mComm;

    /// app group data
    GroupData* mGroups;

    /// main window of the app
    MainWindow* mMainWindow;

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

    /// stored isOn flag
    bool mIsOn;
};

} // namespace cor

#endif // STATEMANAGER_H
