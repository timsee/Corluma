#ifndef LIGHTSLISTMENU_H
#define LIGHTSLISTMENU_H

#include <QScrollArea>
#include <QWidget>
#include "comm/commlayer.h"
#include "menu/menulightcontainer.h"

/*!
 * \copyright
 * Copyright (C) 2015 - 2020.
 * Released under the GNU General Public License.
 *
 *
 * \brief The LightsListMenu class is a simple menu that displays a list of lights. It does not
 * allow interaction with this list, and is used for things like displaying the lights in a group,
 * or displaying the set of selected lights.
 */
class LightsListMenu : public QWidget {
    Q_OBJECT
public:
    explicit LightsListMenu(QWidget* parent, CommLayer* comm);

    /// resizes programmatically
    void resize(const QRect& rect, int buttonHeight);

    /// updates the lights with their current states
    void updateLights();

    /// add a light to display
    void addLight(const QString&);

    /// remove a light from display.
    void removeLight(const QString&);

    /// shows a group of lights.
    void showGroup(const std::vector<QString>&);

    /// clears all data from the widget, so it is no longer showing any lights.
    void clear();

    /// getter for the lights that are displayed
    const std::vector<QString>& lights() const noexcept { return mLights; }

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
    std::vector<QString> mLights;
};

#endif // LIGHTSLISTMENU_H
