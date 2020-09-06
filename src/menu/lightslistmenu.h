#ifndef LIGHTSLISTMENU_H
#define LIGHTSLISTMENU_H

#include <QScrollArea>
#include <QWidget>
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
class LightsListMenu : public QWidget {
    Q_OBJECT
public:
    explicit LightsListMenu(QWidget* parent, bool allowInteraction);

    /// resizes programmatically
    void resize(const QRect& rect, int buttonHeight);

    /// updates the lights with their current states
    void updateLights();

    /// add a light to display
    void addLight(const cor::Light&);

    /// remove a light from display.
    void removeLight(const cor::Light&);

    /// shows a group of lights.
    void showLights(const std::vector<cor::Light>&);

    /// highlight the lights listed, removing highlights from all others.
    void highlightLights(const std::vector<QString>& lights) {
        mLightContainer->highlightLights(lights);
    }

    /// clears all data from the widget, so it is no longer showing any lights.
    void clear();

    /// getter for selected lights.
    std::vector<cor::Light> lights() const noexcept { return mLights; }

signals:

    /// emits when a light is clicked.
    void clickedLight(cor::Light);

private slots:
    /// handles when a light is clicked, emits the full cor::Light
    void lightClicked(QString light) {
        for (auto storedLight : mLights) {
            if (storedLight.uniqueID() == light) {
                emit clickedLight(storedLight);
            }
        }
    }

private:
    /// scroll area for showing the MenuLightContainer
    QScrollArea* mScrollArea;

    /// shows the light widgets
    MenuLightContainer* mLightContainer;

    /// stores the height of each row in the scroll area.
    int mRowHeight;

    /// stores the unique IDs of all lights being displayed.
    std::vector<cor::Light> mLights;
};


#endif // LIGHTSLISTMENU_H
