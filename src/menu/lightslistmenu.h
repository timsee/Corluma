#ifndef LIGHTSLISTMENU_H
#define LIGHTSLISTMENU_H

#include <QScrollArea>
#include <QWidget>
#include "cor/widgets/widgetoutlinebox.h"
#include "menu/menulightcontainer.h"

/*!
 * \copyright
 * Copyright (C) 2015 - 2020.
 * Released under the GNU General Public License.
 *
 * \brief The LightsListMenu class is a simple menu that displays a list of lights. By
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

    /// remove lights by key.
    void removeLights(const std::vector<QString>&);

    /// shows a group of lights.
    void addLights(const std::vector<cor::Light>&);

    /// highlight the lights listed, removing highlights from all others.
    void highlightLights(const std::vector<QString>& lights) {
        mLightContainer->highlightLights(lights);
    }

    /// vector of highlighted lights
    std::vector<QString> highlightedLights() { return mLightContainer->highlightedLights(); }

    /// clears all data from the widget, so it is no longer showing any lights.
    void clear();

    /// getter for selected lights.
    std::vector<cor::Light> lights() const noexcept { return mLights; }

    /// setter for whether or not this widget is in SingleLightMode. In SingleLightMode, only one
    /// light can be highlighted at a time, and the parents and subgroups do not highlight at all.
    void singleLightMode(bool singleLightMode) { mSingleLightMode = singleLightMode; }

signals:

    /// emits when a light is clicked.
    void clickedLight(cor::Light);

private slots:
    /// handles when a light is clicked, emits the full cor::Light
    void lightClicked(QString light) {
        // highlight only the light that was clicked
        if (mSingleLightMode) {
            mLightContainer->highlightLights({light});
        }
        // emit the light that was clicked
        for (const auto& storedLight : mLights) {
            if (storedLight.uniqueID() == light) {
                emit clickedLight(storedLight);
            }
        }
    }

private:
    /// scroll area for showing the MenuLightContainer
    QScrollArea* mScrollArea;

protected:
    /// shows the light widgets
    MenuLightContainer* mLightContainer;

    /// applies an outline to the widget.
    cor::WidgetOutlineBox* mOutlineBox;

    /// stores the unique IDs of all lights being displayed.
    std::vector<cor::Light> mLights;

    /*!
     * \brief paintEvent paints the background of the widget
     */
    void paintEvent(QPaintEvent* event);

private:
    /// stores the height of each row in the scroll area.
    int mRowHeight;

    /// true if in single light mode, false if multiple lights can be picked.
    bool mSingleLightMode;
};


#endif // LIGHTSLISTMENU_H
