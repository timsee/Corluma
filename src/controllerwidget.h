#ifndef CONTROLLERPAGE_H
#define CONTROLLERPAGE_H

#include <QWidget>
#include "comm/commlayer.h"
#include "cor/lightlist.h"
#include "cor/objects/page.h"
#include "cor/widgets/topwidget.h"
#include "display/displayarducorcontrollerwidget.h"
#include "display/displayhuebridgewidget.h"
#include "display/displaynanoleafcontrollerwidget.h"

/*!
 * \copyright
 * Copyright (C) 2015 - 2020.
 * Released under the GNU General Public License.
 *
 *
 *
 * \brief The ControllerPage class displays a light controller. A Light Controller is the "brains"
 * behind a light, and can control one or more lights. For instance, a Hue Bridge is a controller.
 * Or, an arduino is a controller.
 *
 */
class ControllerWidget : public QWidget, public cor::Page {
    Q_OBJECT
public:
    explicit ControllerWidget(QWidget* parent, CommLayer* comm, cor::LightList* selectedLights);

    /// getter for nanoleaf widget
    DisplayNanoleafControllerWidget* nanoleafWidget() { return mNanoleafWidget; }

    /// getter for arducor widget
    DisplayArduCorControllerWidget* arduCorWidget() { return mArduCorWidget; }

    /// getter for hue widget
    DisplayHueBridgeWidget* hueWidget() { return mHueBridgeWidget; }

    /// show page
    void showPage(QPoint);

    /// hide page
    void hidePage();

    /// show an arduCor controller
    void showArduCor(const cor::Controller& controller, cor::EArduCorStatus status);

    /// show a nanoleaf controller
    void showNanoleaf(const nano::LeafMetadata& metadata, nano::ELeafDiscoveryState discoveryState);

    /// show a Hue Bridge
    void showHueBridge(const hue::Bridge& bridge);

    /// change row height
    void changeRowHeight(int height);

    /// highlight lights on the controller page.
    void highlightLights();

    /// update the light names for a specific protocol.
    void updateLightNames(EProtocolType type);

    /// returns the lightInfoWidget
    LightInfoListWidget* lightInfoWidget() { return mHueBridgeWidget->lightInfoWidget(); }

    /// handle lights deleted from the commlayer
    void handleDeletedLights(const std::vector<QString>&);

public slots:

    /// handle when a full controller is deleted. this deletes all of its lights.
    void handleDeleteController(QString, EProtocolType);

signals:

    /// back button pressed
    void backButtonPressed();

    /// delete clicked from a controller page
    void deleteLight(QString);

    /// a light is slected from the controller page, true if seleceted, false if deselected.
    void lightClicked(QString);

    /// select lights for a controller by controller key and its protocol
    void selectControllerLights(QString, EProtocolType);

    /// deselect lights for a controller by controller key and its protocol
    void deselectControllerLights(QString, EProtocolType);

protected:
    /*!
     * \brief resizeEvent called every time the main window is resized.
     */
    void resizeEvent(QResizeEvent*);

    /*!
     * \brief paintEvent paint event for rendering. used to overwrite the background
     *        color of the discovery page so that it hides everything behind it.
     */
    void paintEvent(QPaintEvent*);

private slots:
    /// handles when back button is pressed
    void backButtonPressed(bool);

    /// handle when a light is deleted.
    void handleDeleteHueLight(QString);

    /// handle when a nanoleaf is deleted.
    void handleDeleteNanoleaf(QString, QString);

    /// renders and UI that needs explicit updates.
    void renderUI();

private:
    /// pointer to commlayer
    CommLayer* mComm;

    /// list of selected lights
    cor::LightList* mSelectedLights;

    /// timer for rendering.
    QTimer* mRenderTimer;

    /// top widget with settings title and close button
    cor::TopWidget* mTopWidget;

    /// widget for displaying arducor controllers.
    DisplayArduCorControllerWidget* mArduCorWidget;

    /// widget for displaying nanoleaf controllers.
    DisplayNanoleafControllerWidget* mNanoleafWidget;

    /// widget for displaying Hue Bridges.
    DisplayHueBridgeWidget* mHueBridgeWidget;
};

#endif // CONTROLLERPAGE_H
