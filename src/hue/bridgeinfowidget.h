#ifndef BRIDGEINFOWIDGET_H
#define BRIDGEINFOWIDGET_H

#include <QWidget>
#include <QGridLayout>
#include <QLabel>
#include <QPushButton>

#include "bridge.h"
#include "huelight.h"
#include "editablefieldwidget.h"

namespace hue
{

/*!
 * \copyright
 * Copyright (C) 2015 - 2018.
 * Released under the GNU General Public License.
 *
 * \brief The BridgeInfoWidget class is a simple widget that shows the metadata of a bridge and
 *        provides a button to discover new hues for the bridge.
 */
class BridgeInfoWidget : public QWidget
{
    Q_OBJECT
public:

    /// constructor
    explicit BridgeInfoWidget(const hue::Bridge& bridge, QWidget *parent);

    /// set height of info widget
    void setHeight(int);

    /*!
     * \brief setChecked true to highlight widget and treat as checked, false to keep in regular
     *        state.
     * \param checked true to highlight widget and treat as checked, false to keep in regular
     *        state.
     */
    void setChecked(bool checked);

    /// true if checked, false otherwise.
    bool checked() { return mIsChecked; }

    /// getter for the key of the widget.
    const QString& key() { return mBridge.id; }

signals:

    /// signals the bridge's key when the discover hues button is pressed
    void discoverHuesPressed(QString);

    /// groups button is pressed. signals the bridge ID
    void groupsPressed(QString);

    /// schedules button is pressed. signals the bridge ID
    void schedulesPressed(QString);

    /// name changed is pressed. signals the bridge ID and the new name
    void nameChanged(QString, QString);

    /*!
     * \brief clicked emits the key of the widget whenever it is clicked.
     */
    void clicked(QString);

protected:
    /*!
     * \brief resizeEvent called every time the main window is resized.
     */
    void resizeEvent(QResizeEvent *);

    /*!
     * \brief paintEvent used to draw the background of the widget.
     */
    void paintEvent(QPaintEvent *);

    /*!
     * \brief mouseReleaseEvent picks up when a click (or a tap on mobile) is released.
     */
    virtual void mouseReleaseEvent(QMouseEvent *);

private slots:

    /// handles when the discover hues button is pressed
    void pressedDiscoverHues();

    /// handles when the schedules button is pressed
    void schedulesListPressed();

    /// handles when the groups button is pressed
    void groupsListPressed();

    /// handles when the name is changed from the EditableFieldWidget
    void changedName(QString);

private:

    /// sets the title's font size
    void setTitleFontPointSize(int pt);

    /// bridge used to display info
    hue::Bridge mBridge;

    /// label for the name widget
    QLabel *mNameLabel;

    /// editable field that lets you change the custom name of a bridge.
    EditableFieldWidget *mNameWidget;

    /// shows the IP address
    QLabel *mIPAddress;

    /// shows the ID of the bridge
    QLabel *mID;

    /// shows the API version of the bridge
    QLabel *mAPI;

    /// spacer used for laying out widgets
    QLabel *mSpacer;

    /// layout of top of widget
    QHBoxLayout *mTopLayout;

    /// image for displaying a graphic to help with current step of widget.
    QLabel *mImage;

    /// cachced pixmap of hue bridge
    QPixmap mBridgePixmap;

    /// layout for the top right of the widget
    QVBoxLayout *mTopRightLayout;

    /// layout for the name widgets
    QHBoxLayout *mNameLayout;

    /// layout for the buttons
    QHBoxLayout *mButtonsLayout;

    /// button that allows the user to discover new hue lights.
    QPushButton *mDiscoverHueButton;

    /// button used for viewing the schedules of a bridge
    QPushButton *mSchedulesButton;

    /// button used for viewing the groups of a bridge.
    QPushButton *mGroupsButton;

    /// main layout
    QVBoxLayout *mLayout;

    /// true if checked, false otherwise
    bool mIsChecked;

    /// used for setting the height;
    int mHeight;
};

}

#endif // BRIDGEINFOWIDGET_H
