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

signals:

    /// signals when one or more lights should be selected
    void selectLights(std::vector<QString>);

    /// signals when one or more lights should be deselected
    void deselectLights(std::vector<QString>);

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

    /// handles when a light is selected
    void selectLight(QString);

    /// handles when a light is deselected
    void deselectLight(QString);

    /// handles when all lights for a controller should be selected
    void selectAllControllerLights(QString, EProtocolType);

    /// handles when all lights of a controller should be deselected
    void deselectAllControllerLights(QString, EProtocolType);

    /// handles when a discovery widget signals to delete a controller.
    void deleteControllerFromDiscovery(QString, EProtocolType);

    /// handles when a controller name changes from a controller widget, updates the DiscoveryWidget
    void handleControllerNameChanged(QString key, QString name);

private:
    /// programmatically resize
    void resize();

    /// uses a controller name and type to get all the lights associated with the controller, second
    /// flag of pair is whether or not the lookup was successful.
    std::pair<std::vector<cor::Light>, bool> getLightsFromController(const QString& controllerName,
                                                                     EProtocolType type);

    /// pointer to comm data
    CommLayer* mComm;

    /// pointer to selected lights, to add or remove selections.
    cor::LightList* mSelectedLights;

    /// pointer to controller widget, which shows information about a light controller (IE, a
    /// Philips Bridge or a NanoLeaf Controller).
    ControllerWidget* mControllerWidget;

    /// pointer to the DiscoveryWidget, which allows for discovering new lights and viewing
    /// currently discovered lights.
    DiscoveryWidget* mDiscoveryWidget;
};

#endif // LIGHTSPAGE_H
