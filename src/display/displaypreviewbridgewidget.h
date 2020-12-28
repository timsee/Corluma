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
#include "cor/widgets/listitemwidget.h"

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
                                        QWidget* parent);

    /// update the bridge being shown
    void updateBridge(const hue::Bridge& bridge);

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

signals:

    /*!
     * \brief clicked emits the key of the widget whenever it is clicked.
     */
    void clicked(QString);

protected:
    /*!
     * \brief resizeEvent called every time the main window is resized.
     */
    void resizeEvent(QResizeEvent*);

    /*!
     * \brief paintEvent used to draw the background of the widget.
     */
    void paintEvent(QPaintEvent*);

    /*!
     * \brief mouseReleaseEvent picks up when a click (or a tap on mobile) is released.
     */
    virtual void mouseReleaseEvent(QMouseEvent*);

private:
    /// state of bridge
    EBridgeDiscoveryState mState;

    /// handles when the bridge state changes
    void handleBridgeState(EBridgeDiscoveryState state);

    /// sets the title's font size
    void setTitleFontPointSize(int pt);

    /// bridge used to display info
    hue::Bridge mBridge;

    /// pointer to comm data
    CommLayer* mComm;

    /// pointer to selected lights
    cor::LightList* mSelectedLights;

    /// editable field that lets you change the custom name of a bridge.
    QLabel* mNameWidget;

    /// shows the IP address
    QLabel* mIPAddress;

    /// shows the ID of the bridge
    QLabel* mID;

    /// shows the API version of the bridge
    QLabel* mAPI;

    /// spacer used for laying out widgets
    QLabel* mSpacer;

    /// layout for top of the widget
    QHBoxLayout* mTopLayout;

    /// layout of middle of widget
    QHBoxLayout* mMidLayout;

    /// image for displaying a graphic to help with current step of widget.
    QLabel* mImage;

    /// Used for showing when a device is still being discovery.
    QMovie* mMovie;

    /// cachced pixmap of hue bridge
    QPixmap mBridgePixmap;

    /// layout for the top right of the widget
    QVBoxLayout* mTopRightLayout;

    /// reachable count
    std::uint32_t mReachableCount;

    /// selected count
    std::uint32_t mSelectedCount;

    /// true if checked, false otherwise
    bool mIsChecked;
};

} // namespace hue

#endif // BRIDGEINFOWIDGET_H
