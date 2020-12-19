#ifndef DISCOVERYARDUCORWIDGET_H
#define DISCOVERYARDUCORWIDGET_H

#include <QPushButton>

#include "comm/commarducor.h"
#include "cor/widgets/listwidget.h"
#include "discovery/discoverytypewidget.h"
#include "display/displaypreviewarducorwidget.h"

/*!
 * \copyright
 * Copyright (C) 2015 - 2020.
 * Released under the GNU General Public License.
 *
 *
 * \brief The DiscoveryArduCorWidget class is a discovery widget for helping with
 *        both Yun UDP and HTTP communication streams. It allows the user to enter
 *        IP addresses manually, and then shows which IP addresses are currently running
 *        discovery and which have passed a discovery check.
 */
class DiscoveryArduCorWidget : public DiscoveryTypeWidget {
    Q_OBJECT

public:
    /*!
     * \brief DiscoveryArduCorWidget constructor
     *
     * \param parent
     */
    explicit DiscoveryArduCorWidget(QWidget* parent,
                                    CommLayer* comm,
                                    cor::LightList* selectedLights,
                                    ControllerWidget* controllerPage);

    /// See DiscoveryWidget.h
    void handleDiscovery(bool isActive) override;

    /// check if an IP already exists for a light
    void checkIfIPExists(const QString& IP) override;

    /// prompt for the IP widget
    QString IPWidgetPrompt() override { return "Add an IP Address for the ArduCor:"; }

    /// default value for the IP widget
    QString IPWidgetDefaultValue() override { return "192.168.0.101"; }

    /// delete a light from the discovery page.
    void deleteLight(const QString&) override;

    /// handles how to higlight lights.
    void highlightLights() override;

private slots:

    /// handles when the greyout is clicked
    void greyOutClicked() override;

    /// handles when a controller is clicked.
    void controllerClicked(QString);

protected:
    /// called when the widget resizes
    virtual void resizeEvent(QResizeEvent*) override;

private:
    /*!
     * \brief doesYunControllerExistAlready checks both UDP and HTTP yun device lists to see if it
     * has any knowledge of the given controller.
     *
     * \param controller name of controller
     * \return true if yun controller exists on discovered or undiscovered list, false otherwise
     */
    bool doesYunControllerExistAlready(const QString& controller);

    /// programmatically resized
    void resize();

    /// handle how to display or update a controller
    void handleController(const cor::Controller& controller, cor::EArduCorStatus status);

    /// list of selected lights.
    cor::LightList* mSelectedLights;

    /// widget for displaying a scrollable list of other widgets
    cor::ListWidget* mListWidget;

    /// buffer for last IP address used
    QString mLastIP;

    /// top label that explains the widget
    QLabel* mTopLabel;
};

#endif // DISCOVERYARDUCORWIDGET_H
