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
                              const std::vector<QColor>& colors,
                              bool setHighlightable,
                              QSize size,
                              QWidget *parent = 0);

    /*!
     * \brief updateWidget update the widget with a new state for the device.
     * \param device the new state of the device
     * \param colors all the color groups in the data layer, in case the device uses
     *        the color groups
     */
    void updateWidget(const cor::Light& device,
                      const std::vector<QColor>& colors);

    /*!
     * \brief setHighlightChecked set the widget as checked or unchecked. When it is checked it
     *        it will be a light blue color. When it isn't, it will be dark grey.
     * \param checked true to set to checked, false othwerise
     * \return true if successful, false otherwise
     */
    bool setHighlightChecked(bool checked);

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

protected:

    /*!
     * \brief mouseReleaseEvent picks up when a click (or a tap on mobile) is released.
     */
    virtual void mouseReleaseEvent(QMouseEvent *);

    /*!
     * \brief paintEvent paints the background of the widget
     */
    void paintEvent(QPaintEvent *event);

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
     * \brief createStyleSheet creates a custom style sheet for controlling
     *        how the widget looks
     * \param device a read only version of the device
     * \return a string that represents the style sheet
     */
    QString createStyleSheet(const cor::Light& device);

    /*!
     * \brief createName create the name to display in the Qlabel
     * \param device the device information to generate the name
     * \return A "pretty" version of the name of the light device.
     */
    QString createName(const cor::Light& device);

    /// resize the mIcon and its associated pixmap.
    void resizeIconPixmap();

    /// create the type label, which may be an image or text
    void prepareTypeLabel(ECommType type);

    /*!
     * \brief mDeviceIcon uses mIconData to display an icon
     */
    QLabel *mDeviceIcon;

    /// Shows the status of the device. Shows if it is off, on, how bright it is, or if it is unreachable.
    cor::StatusIcon *mStatusIcon;

    /// label for the type icon, used to show what type of device it is (a hue, or an arduino)
    QLabel *mTypeIcon;

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

    /*!
     * \brief mIsChecked true if checked, false otherwise
     */
    bool mIsChecked;

};


#endif // LIGHTSLISTWIDGET_H
