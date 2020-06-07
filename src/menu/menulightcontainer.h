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
    explicit MenuLightContainer(QWidget* parent, bool allowInteraction, bool displayState)
        : QWidget(parent),
          mLightLayout(cor::EListType::linear),
          mAllowInteraction{allowInteraction},
          mDisplayState{displayState} {
        QScroller::grabGesture(this, QScroller::LeftMouseButtonGesture);
    }

    /// if set to true, unreachable lights can be selected.
    void allowUnreachableLights(bool allowUnreachable) {
        mAllowUnreachableLights = allowUnreachable;
    }

    /// updates the state of the light widgets
    void updateLightWidgets(const std::vector<cor::Light>& lights);

    /// shows the lights provided, with the height of one light widget provided
    void showLights(const std::vector<cor::Light>& lights, int height);

    /// highlights the lights based off of the currently selected lights
    void highlightLights(const std::vector<QString>& selectedLights);

    /// moves the light widgets into place
    void moveLightWidgets(QSize size, QPoint offset);

    /// remove all widgets from the container, reseting it to an empty state
    void clear();

signals:

    /// emits when a light is clicked
    void clickedLight(QString);

private slots:
    /// handles when a light is clicked
    void handleLightClicked(QString light);

private:
    /// stores ListLightWidgets, which show lights
    cor::ListLayout mLightLayout;

    /// true if you can select unreachable lights, false otherwise
    bool mAllowUnreachableLights;

    /// true if interactions are allowed, false if they are disabled.
    bool mAllowInteraction;

    /// true to display state of the lights, false to just display the metadata
    bool mDisplayState;
};

#endif // LIGHTMENUCONTAINER_H
