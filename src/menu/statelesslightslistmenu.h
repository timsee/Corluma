#ifndef STATELESSLIGHTSLISTMENU_H
#define STATELESSLIGHTSLISTMENU_H

#include <QScrollArea>
#include <QWidget>
#include "comm/commlayer.h"
#include "menu/menulightcontainer.h"

/*!
 * \copyright
 * Copyright (C) 2015 - 2020.
 * Released under the GNU General Public License.
 *
 * \brief The StatelessLightsListMenu class is a simple menu that displays a list of lights. By
 * default it displays the state of the lights, but that can be toggled by calling
 * displayState(bool)
 */
class StatelessLightsListMenu : public QWidget {
    Q_OBJECT
public:
    explicit StatelessLightsListMenu(QWidget* parent, CommLayer* comm, bool allowInteraction);

    /// resizes programmatically
    void resize(const QRect& rect, int buttonHeight);

    /// updates the lights with their current states
    void updateLights();

    /// add a light to display
    void addLight(const cor::LightID&);

    /// remove a light from display.
    void removeLight(const cor::LightID&);

    /// shows a group of lights.
    void addLights(const std::vector<cor::LightID>&);

    /// highlight all the lights provided, removing the highlight from lights not in the list.
    void highlightLights(const std::vector<cor::LightID>& lights) {
        mLightContainer->highlightLights(lights);
    }

    /// clears all data from the widget, so it is no longer showing any lights.
    void clear();

    /// getter for the lights that are displayed
    const std::vector<cor::LightID>& lightIDs() const noexcept { return mLights; }

signals:

    /// emits when a light is clicked
    void clickedLight(cor::LightID);

private slots:
    /// handles when a light is clicked
    void lightClicked(cor::LightID light) { emit clickedLight(light); }

private:
    /// pointer to comm layer
    CommLayer* mComm;

    /// scroll area for showing the MenuLightContainer
    QScrollArea* mScrollArea;

    /// shows the light widgets
    MenuLightContainer* mLightContainer;

    /// stores the height of each row in the scroll area.
    int mRowHeight;

    /// stores the unique IDs of all lights being displayed.
    std::vector<cor::LightID> mLights;
};

#endif // STATELESSLIGHTSLISTMENU_H
