#ifndef DISCOVERYNANOLEAFWIDGET_H
#define DISCOVERYNANOLEAFWIDGET_H

#include <QHBoxLayout>

#include "comm/commnanoleaf.h"
#include "cor/lightlist.h"
#include "cor/widgets/listwidget.h"
#include "discoverytypewidget.h"
#include "editablefieldwidget.h"
#include "searchwidget.h"

/*!
 * \copyright
 * Copyright (C) 2015 - 2020.
 * Released under the GNU General Public License.
 *
 *
 * \brief The DiscoveryNanoLeafWidget class helps in discovering a NanoLeaf. It provides
 *        instructions to connect to the NanoLeaf and also provides the ability to manually
 *        enter an IP address, if discovery is failing.
 */
class DiscoveryNanoLeafWidget : public DiscoveryTypeWidget {
    Q_OBJECT

public:
    /*!
     * \brief DiscoveryNanoLeafWidget constructor
     *
     * \param parent
     */
    explicit DiscoveryNanoLeafWidget(QWidget* parent,
                                     CommLayer* comm,
                                     cor::LightList* selectedLights,
                                     ControllerWidget* controllerPage);

    /// See DiscoveryWidget.h
    void handleDiscovery(bool isActive) override;

    /// check if IP exists
    void checkIfIPExists(const QString& IP) override;

    /// prompt for an IP widget
    QString IPWidgetPrompt() override { return "Add an IP Address for the Nanoleaf:"; }

    /// default value for the IP widget.
    QString IPWidgetDefaultValue() override { return "192.168.0.100"; }

    /// delete a light from the discovery page.
    void deleteLight(const QString&) override;

    /// handles how to higlight lights.
    void highlightLights() override;

protected:
    /// called when the widget resizes
    virtual void resizeEvent(QResizeEvent*) override;

private slots:

    /// handles when the greyout is clicked
    void greyOutClicked() override;

    /// handles when a nanoleaf is clicked
    void nanoleafClicked(QString);

private:
    /// handle how to display or update a nanoleaf
    void handleNanoleaf(const nano::LeafMetadata& leafMetadata, nano::ELeafDiscoveryState status);

    /// handle when nanoleafs are duplicated (such as when one was in discovery but was now fully
    /// discovered)
    void removeDuplicatedNanoleafs();

    /// programmatically resize
    void resize();

    /// list of selected lights.
    cor::LightList* mSelectedLights;

    /// widget for displaying a scrollable list of other widgets
    cor::ListWidget* mListWidget;

    /// buffer for last IP address used
    QString mLastIP;

    /// label to prompt the user through the application.
    QLabel* mLabel;
};

#endif // DISCOVERYNANOLEAFWIDGET_H
