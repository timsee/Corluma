#ifndef LIGHTMENUCONTAINER_H
#define LIGHTMENUCONTAINER_H

#include <QScroller>
#include <QWidget>
#include "cor/listlayout.h"
#include "listlightwidget.h"

/*!
 * \copyright
 * Copyright (C) 2015 - 2020.
 * Released under the GNU General Public License.
 *
 *
 * \brief The MenuLightContainer class is a container widget that displays mulitple
 * ListLightWidgets. This widget controls its own height so that it can properly display enough
 * lights. The widget signals the unique ID of a light when a light is clicked. Lights can be
 * highlighted when selected.
 */
class MenuLightContainer : public QWidget {
    Q_OBJECT
public:
    /// constructor
    explicit MenuLightContainer(QWidget* parent, bool allowInteraction, const QString& name)
        : QWidget(parent),
          mLightLayout(cor::EListType::oneColumn),
          mAllowInteraction{allowInteraction},
          mDisplayState{true},
          mRowHeight{10},
          mName{name} {
        QScroller::grabGesture(this, QScroller::LeftMouseButtonGesture);
    }

    /// change the height of each light widget.
    void changeRowHeight(int rowHeight) { mRowHeight = rowHeight; }

    /// set whether the state should be displayed by the container or not.
    void displayState(bool displayState) { mDisplayState = displayState; }

    /// if set to true, unreachable lights can be selected.
    void allowUnreachableLights(bool allowUnreachable) {
        mAllowUnreachableLights = allowUnreachable;
    }

    /// add lights to the widget, updating if the the lights already exist, and adding if they don't
    void addLights(const std::vector<cor::Light>& lights);

    /// remove a light from the container.
    void removeLight(cor::LightID lightID);

    /// updates the state of the light widgets, but will not add new lights if they don't exist
    void updateLights(const std::vector<cor::Light>& lights);

    /// highlights the lights based off of the currently selected lights
    void highlightLights(const std::vector<cor::LightID>& selectedLights);

    /// moves the light widgets into place
    void moveLightWidgets(QSize size, QPoint offset);

    /// remove all widgets from the container, reseting it to an empty state
    void clear();

    /// getter for currently highlighted lights.
    std::vector<cor::LightID> highlightedLights();

    /// true to show timeouts, false to hide them.
    void showTimeouts(bool shouldShowTimeouts);

    /// update the timeout values.
    void updateTimeouts(const std::vector<std::pair<cor::LightID, std::uint32_t>> keyTimeoutPairs);

signals:

    /// emits when a light is clicked
    void clickedLight(cor::LightID);

private slots:
    /// handles when a light is clicked
    void handleLightClicked(cor::LightID light);

private:
    /// stores ListLightWidgets, which show lights
    cor::ListLayout mLightLayout;

    /// true if you can select unreachable lights, false otherwise
    bool mAllowUnreachableLights;

    /// true if interactions are allowed, false if they are disabled.
    bool mAllowInteraction;

    /// true to display state of the lights, false to just display the metadata
    bool mDisplayState;

    /// the height of a button.
    int mRowHeight;

    /// the name of the widget.
    QString mName;
};

#endif // LIGHTMENUCONTAINER_H
