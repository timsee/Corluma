#ifndef BRIDGEINFOWIDGET_H
#define BRIDGEINFOWIDGET_H

#include <QGridLayout>
#include <QLabel>
#include <QMovie>
#include <QPushButton>
#include <QWidget>

#include "comm/commlayer.h"
#include "comm/hue/bridge.h"
#include "comm/hue/huemetadata.h"
#include "cor/lightlist.h"
#include "cor/widgets/checkbox.h"
#include "cor/widgets/expandingtextscrollarea.h"
#include "cor/widgets/listitemwidget.h"
#include "menu/lightslistmenu.h"

namespace hue {

/*!
 * \copyright
 * Copyright (C) 2015 - 2020.
 * Released under the GNU General Public License.
 *
 * \brief The BridgeInfoWidget class is a simple widget that shows the metadata of a bridge and
 *        provides a button to discover new hues for the bridge.
 */
class DisplayPreviewBridgeWidget : public cor::ListItemWidget {
    Q_OBJECT
public:
    /// constructor
    explicit DisplayPreviewBridgeWidget(const hue::Bridge& bridge,
                                        const QString& key,
                                        CommLayer* comm,
                                        cor::LightList* selectedLights,
                                        int rowHeight,
                                        QWidget* parent);

    /// update the bridge being shown
    void updateBridge(const hue::Bridge& bridge);

    /// getter of bridge
    hue::Bridge bridge() const { return mBridge; }

    /// change the row height of menus
    void changeRowHeight(int rowHeight) { mRowHeight = rowHeight; }

    /*!
     * \brief setChecked true to highlight widget and treat as checked, false to keep in regular
     *        state.
     * \param checked true to highlight widget and treat as checked, false to keep in regular
     *        state.
     */
    void setChecked(bool checked);

    /// true if checked, false otherwise.
    bool checked() { return mIsChecked; }

    /// resize programmatically
    void resize();

    /// getter for bridge that is being displayed
    const hue::Bridge& bridge() { return mBridge; }

    /// highlight bridge based on the number of lights.
    void highlightLights();

    /// add an individual light to the preview
    void addLight(const cor::Light& light);

    /// remove lights by keys from the light list.
    void removeLights(const std::vector<cor::LightID>& keys);

signals:

    /*!
     * \brief clicked emits the key of the widget whenever it is clicked.
     */
    void bridgeClicked(QString);

    /// emits when a light should be selected
    void selectLight(cor::LightID);

    /// emits when a light should be deselected
    void deselectLight(cor::LightID);

    /// emits when the checkbox is clicked which selects all of the lights for the bridge.
    void selectAllClicked(QString, EProtocolType);

    /// emits when the checkbox is clicked which deselects all of the lights for the bridge.
    void deselectAllClicked(QString, EProtocolType);

    /// emits when a bridge is signaled for deletion, signals the bridge id and its protocol type.
    void deleteBridge(QString, EProtocolType);

protected:
    /*!
     * \brief resizeEvent called every time the main window is resized.
     */
    void resizeEvent(QResizeEvent*);

private slots:

    /// handle when the checkbox is clicked
    void checkBoxClicked(ECheckboxState state);

    /// handle when a light is clicked
    void clickedLight(cor::Light light);

    /// handles when the manage button is pressed.
    void manageButtonPressed(bool);

private:
    /// state of bridge
    EBridgeDiscoveryState mState;

    /// handles when the bridge state changes
    void handleBridgeState(EBridgeDiscoveryState state);

    /// handle the state of the button, which is typically for management, but if the bridge is not
    /// fully discovered, it instead is a delete button.
    void handleButtonState();

    /// handle checkbox state
    void handleCheckboxState() {
        bool anyLightSelected = false;
        for (auto light : mBridge.lightIDs()) {
            if (mSelectedLights->doesLightExist(light)) {
                anyLightSelected = true;
                break;
            }
        }
        if (anyLightSelected) {
            mCheckBox->checkboxState(ECheckboxState::checked);
        } else {
            mCheckBox->checkboxState(ECheckboxState::unchecked);
        }
    }

    /// update the metadata of the widget.
    void updateMetadata(const hue::Bridge& bridge);

    /// bridge used to display info
    hue::Bridge mBridge;

    /// pointer to comm data
    CommLayer* mComm;

    /// pointer to selected lights
    cor::LightList* mSelectedLights;

    /// editable field that lets you change the custom name of a bridge.
    QLabel* mNameWidget;

    /// displays the lights that are part of this group and their current states.
    LightsListMenu* mLights;

    /// image for displaying a graphic to help with current step of widget.
    QLabel* mImage;

    /// Used for showing when a device is still being discovery.
    QMovie* mMovie;

    /// checkbox for selecting/deselecting the lights in the bridge.
    cor::CheckBox* mCheckBox;

    /// widget for metadata
    QLabel* mMetadata;

    /// button to open the full widget.
    QPushButton* mManageButton;

    /// cachced pixmap of hue bridge
    QPixmap mBridgePixmap;

    /// reachable count
    std::uint32_t mReachableCount;

    /// selected count
    std::uint32_t mSelectedCount;

    /// true if checked, false otherwise
    bool mIsChecked;

    /// height of a row.
    int mRowHeight;
};

} // namespace hue

#endif // BRIDGEINFOWIDGET_H
