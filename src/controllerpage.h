#ifndef CONTROLLERPAGE_H
#define CONTROLLERPAGE_H

#include <QWidget>
#include "comm/commlayer.h"
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
class ControllerPage : public QWidget, public cor::Page {
    Q_OBJECT
public:
    explicit ControllerPage(QWidget* parent, CommLayer* comm);

    /// show page
    void showPage(QPoint);

    /// hide page
    void hidePage();

    /// show an arduCor controller
    void showArduCor(const cor::Controller& controller);

    /// show a nanoleaf controller
    void showNanoleaf(const nano::LeafMetadata& metadata);

    /// show a Hue Bridge
    void showHueBridge(const hue::Bridge& bridge);

    /// change row height
    void changeRowHeight(int height);

signals:

    /// back button pressed
    void backButtonPressed();

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

private:
    /// pointer to commlayer
    CommLayer* mComm;

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
