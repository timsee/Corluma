#ifndef LISTGROUPWIDGET_H
#define LISTGROUPWIDGET_H

#include <QWidget>
#include <QObject>
#include <QWidget>
#include <QLabel>
#include <QLayout>
#include <QPushButton>

#include "icondata.h"
#include "commtype.h"
#include "listcollectionsubwidget.h"

/*!
 * \copyright
 * Copyright (C) 2015 - 2017.
 * Released under the GNU General Public License.
 *
 *
 * \brief The ListMoodWidget class is used to display a pre-saved group of
 *        devices and their settings in a single list widget. It displays the name
 *        of the group, up to 5 device previews as IconData, and the connection state
 *        of the associated devices.
 */
class ListMoodWidget : public ListCollectionSubWidget
{
    Q_OBJECT

public:
    /*!
     * \brief ListMoodWidget constructor
     * \param name name of group
     * \param devices devices in group
     * \param colors Color groups for devices using multi color routines.
     */
    explicit ListMoodWidget(const QString& name,
                             const std::list<SLightDevice>& devices,
                             const std::vector<std::vector<QColor> >& colors,
                             QWidget *parent = 0);

    /*!
    * \brief setChecked set the widget as checked or unchecked. When it is checked it
    *        it will be a light blue color. When it isn't, it will be dark grey.
    * \param checked true to set to checked, false othwerise
    * \return true if successful, false otherwise
    */
    bool setChecked(bool checked);

    /*!
     * \brief checked getter for checked state
     * \return true if checked, false otherwise
     */
    bool checked() { return mIsChecked; }

    /*!
     * \brief moodName getter for name
     * \return name of mood
     */
    const QString& moodName() { return mMoodName; }

signals:

    /*!
     * \brief clicked emited whenever a mouse press is released anywhere on the widget
     */
    void clicked(QString);

    /*!
     * \brief editClicked emitted when edit button is clicked. Emits its key.
     */
    void editClicked(QString);

protected:

    /*!
     * \brief enterEvent picks up when the mouse pointer (or finger on mobile) enters the area of the widget.
     */
    virtual void enterEvent(QEvent *);

    /*!
     * \brief leaveEvent picks up when the mouse pointer (or finger on mobile) leaves the area of the widget.
     */
    virtual void leaveEvent(QEvent *);

    /*!
     * \brief mouseReleaseEvent picks up when a click (or a tap on mobile) is released.
     */
    virtual void mouseReleaseEvent(QMouseEvent *);

    /*!
     * \brief paintEvent paints the background of the widget
     */
    void paintEvent(QPaintEvent *event);

private slots:

    /*!
     * \brief editClicked emitted when edit button is clicked. Emits its key.
     */
    void editButtonClicked(bool) { emit editClicked(mKey);  }

private:

    /*!
     * \brief mIsChecked true if checked, false otherwise
     */
    bool mIsChecked;

    /*!
     * \brief mEditButton button used to edit the collection. Editing can change
     *        the name or the lights contained in the collection.
     */
    QPushButton *mEditButton;

    /// pixmap for icon for the edit button
    QPixmap mEditIcon;

    /*!
     * \brief mName namme of group
     */
    QLabel *mName;

    /*!
     * \brief mMoodName name of mood
     */
    QString mMoodName;

    /*!
     * \brief mDevices list of devices
     */
    std::list<SLightDevice> mDevices;

    /*!
     * \brief mPreviews preview of a device in the group. Uses an icon generated by
     *        the icon data.
     */
    std::vector<QLabel*>  mPreviews;

    /*!
     * \brief mIconData data for the previews of the devices in groups.
     */
    std::vector<IconData> mIconData;

    /*!
     * \brief mLayout layout for widget
     */
    QHBoxLayout *mTopLayout;

    /*!
     * \brief mBottomLayout layout used to display up to five devices from the mood in the widget.
     */
    QHBoxLayout *mBottomLayout;

    /*!
     * \brief mFullLayout combines the various layouts into a single layout that takes up the whole widget.
     */
    QVBoxLayout *mFullLayout;
};

#endif // LISTGROUPWIDGET_H