#ifndef LIGHTSLISTWIDGET_H
#define LIGHTSLISTWIDGET_H

#include <QObject>
#include <QWidget>
#include <QLabel>
#include <QLayout>

#include "icondata.h"
#include "comm/commtype.h"
#include "listcollectionsubwidget.h"
#include "cor/statusicon.h"
#include "cor/switch.h"

/*!
 * \copyright
 * Copyright (C) 2015 - 2018.
 * Released under the GNU General Public License.
 *
 *
 * \brief The ListDeviceWidget class This widget is used on the SettingsPage as a replacement
 *        for a QListWidget on the connectionList. It shows more information than a standard
 *        QListWidget, by giving an icon that represents the lights states, the name of of the
 *        controller, and the index of the light.
 */
class ListDeviceWidget : public ListCollectionSubWidget
{
    Q_OBJECT
public:

    /*!
     * \brief ListDeviceWidget Constructor for multi color routines.
     * \param device device for the widget
     * \param setHighlightable if true, the widget highlights itself, if false,
     *        it does not highlight.
     * \param size desired size of widget
     * \param parent parent widget
     */
    explicit ListDeviceWidget(const cor::Light& device,
                              bool setHighlightable,
                              QSize size,
                              QWidget *parent);

    /*!
     * \brief updateWidget update the widget with a new state for the device.
     * \param device the new state of the device
     * \param colors all the color groups in the data layer, in case the device uses
     *        the color groups
     */
    void updateWidget(const cor::Light& device);

    /*!
     * \brief setHighlightChecked set the widget as checked or unchecked. When it is checked it
     *        it will be a light blue color. When it isn't, it will be dark grey.
     * \param checked true to set to checked, false othwerise
     * \return true if successful, false otherwise
     */
    bool setHighlightChecked(bool checked);

    /// hides if set to true, shows if set to false.
    void hideOnOffSwitch(bool shouldHide);

    /*!
     * \brief checked getter for checked state
     * \return true if checked, false otherwise
     */
    bool checked() { return mIsChecked; }

    /*!
     * \brief device getter for device
     * \return device displayed by widget
     */
    const cor::Light&  device()  { return mDevice; }

signals:
    /*!
     * \brief clicked emited whenever a mouse press is released anywhere on the widget
     */
    void clicked(QString);

    /// emits the key and the state of the on/off switch when its clicked
    void switchToggled(QString, bool);

protected:

    /*!
     * \brief mouseReleaseEvent picks up when a click (or a tap on mobile) is released.
     */
    virtual void mouseReleaseEvent(QMouseEvent *);

    /*!
     * \brief paintEvent paints the background of the widget
     */
    void paintEvent(QPaintEvent *event);

private slots:

    /// handles when the switch state changes
    void changedSwitchState(bool);

    /// cool down timer used to override drawing on the device widget until the cooldown is complete.
    void coolDownClick();

private:
    /*!
     * \brief structToIdentifierString converts a cor::Light struct to a string in the format
     *        of comma delimited values with only the values needed to identiy if as unique.
     * \param dataStruct the struct to convert to a string
     * \return a comma delimited string that represents all values in the cor::Light.
     */
    QString structToIdentifierString(const cor::Light& device);

    /// Called by constructors
    void init(const cor::Light& device);

    /// adds capitalization and stuff like that to a hue name.
    QString convertUglyHueNameToPrettyName(QString name);

    /*!
     * \brief createName create the name to display in the Qlabel
     * \param device the device information to generate the name
     * \return A "pretty" version of the name of the light device.
     */
    QString createName(const cor::Light& device);

    /// update the type icon to reflect the hardware type.
    void updateTypeIcon(ELightHardwareType type);

    /// displays the type of light, such as a lightbulb or a light cube.
    QLabel *mTypeIcon;

    /// pixmap for the type.
    QPixmap mTypePixmap;

    /// pixmap used by main icon
    QPixmap mIconPixmap;

    /// true if should highlight, false otherwise
    bool mShouldHighlight;

    /*!
     * \brief mIconData creates a QPixmap that represents the current light states
     *        which gets displayed on the mStatusIcon.
     */
    IconData mIconData;

    /*!
     * \brief mController name of the controller
     */
    QLabel *mController;

    /*!
     * \brief mLayout layout of widget
     */
    QGridLayout *mLayout;

    /*!
     * \brief mDevice stores the cor::Light used by the widget.
     */
    cor::Light mDevice;

    /// switch for turning on and off a device
    cor::Switch *mOnOffSwitch;

    /// timer for tracking when the last click happened
    QTimer *mCooldownTimer;

    /// true to hide switch, false to show it
    bool mHideSwitch;

    /// used by cooldown timer, if true state updates are blocked.
    bool mBlockStateUpdates;

    /*!
     * \brief mIsChecked true if checked, false otherwise
     */
    bool mIsChecked;

};


#endif // LIGHTSLISTWIDGET_H
