#ifndef LIGHTSPAGE_H
#define LIGHTSPAGE_H

#include <QWidget>
#include "controllerwidget.h"
#include "cor/objects/page.h"
#include "discoverywidget.h"


/*!
 * \copyright
 * Copyright (C) 2015 - 2020.
 * Released under the GNU General Public License.
 */
/*!
 * \brief The LightsPage class is page that contains two widgets. The DiscoveryWidget allows the
 * user to select and discover new lights. The ControllerWidget displays a light controller, which
 * is the device that controls one or more lights (IE, a Philips Bridge is a controller, as well as
 * a Nanoleaf's Controller.). The LightsPage allows users to select, view info, add, and delete
 * lights.
 */
class LightsPage : public QWidget, public cor::Page {
    Q_OBJECT
public:
    explicit LightsPage(QWidget* parent,
                        CommLayer* comm,
                        cor::LightList* lights,
                        AppSettings* appSettings);

    /// getter for controller widget
    ControllerWidget* controllerWidget() { return mControllerWidget; }

    /// getter for discovery widget
    DiscoveryWidget* discoveryWidget() { return mDiscoveryWidget; }

    /// show widgets
    void showWidgets();

    /// hide widgets
    void hideWidgets();

protected:
    /*!
     * \brief resizeEvent called every time the main window is resized.
     */
    void resizeEvent(QResizeEvent*);

private slots:
    /// handles when the discovery widget asks to display the controller widget
    void shouldShowControllerWidget();

    /// handles when the controller widget asks to hide itself.
    void hideControllerWidget();

private:
    /// programmatically resize
    void resize();

    /// pointer to controller widget, which shows information about a light controller (IE, a
    /// Philips Bridge or a NanoLeaf Controller).
    ControllerWidget* mControllerWidget;

    /// pointer to the DiscoveryWidget, which allows for discovering new lights and viewing
    /// currently discovered lights.
    DiscoveryWidget* mDiscoveryWidget;
};

#endif // LIGHTSPAGE_H
