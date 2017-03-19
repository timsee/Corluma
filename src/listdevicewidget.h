#ifndef LIGHTSLISTWIDGET_H
#define LIGHTSLISTWIDGET_H

#include <QObject>
#include <QWidget>
#include <QLabel>
#include <QLayout>

#include "icondata.h"
#include "commtype.h"
#include "listcollectionsubwidget.h"

/*!
 * \copyright
 * Copyright (C) 2015 - 2017.
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
     * \param name name for device on list
     * \param parent parent widget
     */
    explicit ListDeviceWidget(const SLightDevice& device,
                              const QString& name,
                              const std::vector<QColor>& colors,
                              QWidget *parent = 0);

    /*!
     * \brief updateWidget update the widget with a new state for the device.
     * \param device the new state of the device
     * \param colors all the color groups in the data layer, in case the device uses
     *        the color groups
     */
    void updateWidget(const SLightDevice& device,
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
    const SLightDevice&  device()  { return mDevice; }

signals:
    /*!
     * \brief clicked emited whenever a mouse press is released anywhere on the widget
     */
    void clicked(QString);

protected:

    /*!
     * \brief paintEvent paints the background of the widget
     */
    void paintEvent(QPaintEvent *event);

    /*!
     * \brief mouseReleaseEvent picks up when a click (or a tap on mobile) is released.
     */
    virtual void mouseReleaseEvent(QMouseEvent *);

private:
    /*!
     * \brief structToIdentifierString converts a SLightDevice struct to a string in the format
     *        of comma delimited values with only the values needed to identiy if as unique.
     * \param dataStruct the struct to convert to a string
     * \return a comma delimited string that represents all values in the SLightDevice.
     */
    QString structToIdentifierString(const SLightDevice& device);

    /// Called by constructors
    void init(const SLightDevice& device, const QString& name);

    /// adds capitalization and stuff like that to a hue name.
    QString convertUglyHueNameToPrettyName(QString name);

    /*!
     * \brief createStyleSheet creates a custom style sheet for controlling
     *        how the widget looks
     * \param device a read only version of the device
     * \return a string that represents the style sheet
     */
    QString createStyleSheet(const SLightDevice& device);

    /*!
     * \brief mStatusIcon uses mIconData to display an icon
     */
    QLabel *mStatusIcon;

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
    QHBoxLayout *mLayout;

    /*!
     * \brief mDevice stores the SLightDevice used by the widget.
     */
    SLightDevice mDevice;

    /*!
     * \brief mIsChecked true if checked, false otherwise
     */
    bool mIsChecked;

};


#endif // LIGHTSLISTWIDGET_H
