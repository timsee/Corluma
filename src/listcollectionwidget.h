#ifndef LISTCOLLECTIONWIDGET_H
#define LISTCOLLECTIONWIDGET_H

#include <QWidget>
#include <QGridLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <set>

#include "datalayer.h"
#include "comm/commlayer.h"
#include "listcollectionsubwidget.h"

/*!
 * \copyright
 * Copyright (C) 2015 - 2017.
 * Released under the GNU General Public License.
 *
 *
 * \brief The ListCollectionWidget class is parent class for widgets shown on the connection page. When displaying either
 *        devices or moods, they are grouped by collections. Collections are a series of devices that can be controlled together.
 *        In the case of the devices page, each collection shows all its devices. On the moods page, each collection shows all the
 *        moods that use the devices in the collection.
 */
class ListCollectionWidget : public QWidget
{
    Q_OBJECT
public:

    /// deconstructor
    virtual ~ListCollectionWidget(){}

    /*!
     * \brief setup called from constructors, sets up objects that are used in all
     *        collection widgets
     * \param name name of collection
     * \param key unique key for collection
     * \param listHeight maximum height for collection
     * \param hideEdit true for groups that can't be edited such as available and not reachable,
     *        false otherwise
     */
    void setup(const QString& name,
               const QString& key,
               int listHeight,
               bool hideEdit);

    /*!
     * \brief key getter for key
     * \return key of collection
     */
    QString key() { return mKey; }

    /*!
     * \brief showButtons getter that checks if buttons are showing
     * \return  true if buttons are showing, false otherwise.
     */
    bool showButtons() { return mShowButtons; }

    /*!
     * \brief setListHeight set the height of the list when displayed
     * \param newHeight new height for the list.
     */
    void setListHeight(int newHeight);

    /*!
     * \brief preferredSize all collection widgets must implement a preferred size. this is the size
     *        the widget wants to be. It may not end up this size but its a baseline if theres no other
     *        widgets pushing against it.
     * \return a QSize representing its ideal size.
     */
    virtual QSize preferredSize() = 0;

    /*!
     * \brief setShowButtons shows and hides all buttons on the widget
     * \param show true to show, false otherwise.
     */
    virtual void setShowButtons(bool show) = 0;

    /*!
     * \brief isMoodWidget true if ListMoodGroupWidget, false if ListDevicesGroupWidget.
     * \TODO: remove the need for this...
     * \return true if ListMoodGroupWidget, false if ListDevicesGroupWidget.
     */
    virtual bool isMoodWidget() = 0;

signals:

    /*!
     * \brief editClicked emitted when edit button is clicked. Emits its key.
     */
    void editClicked(QString);

    /*!
     * \brief buttonsShown emitted when the buttons are shown or hidden. emits the key and a boolean
     *        representing whether the buttons are shown.
     */
    void buttonsShown(QString, bool);

protected:

    /*!
     * \brief enterEvent picks up when the mouse pointer (or finger on mobile) enters the area of the widget.
     */
    virtual void enterEvent(QEvent *) = 0;

    /*!
     * \brief leaveEvent picks up when the mouse pointer (or finger on mobile) leaves the area of the widget.
     */
    virtual void leaveEvent(QEvent *) = 0;

    /*!
     * \brief mouseReleaseEvent picks up when a click (or a tap on mobile) is released.
     */
    virtual void mouseReleaseEvent(QMouseEvent *) = 0;

    /*!
     * \brief resizeRightHandIcon helper to resize any icon on the right hand side of the widget except the
     *        mHiddenStateIcon.
     * \param pixmap pixmap for the button
     * \param button pointer to widget for the button
     */
    void resizeRightHandIcon(QPixmap pixmap, QPushButton *button);

    /*!
     * \brief insertWidgetIntoGrid insert ListCollectionSubWidget into the QGridLayout used for collection widgets
     * \param widget widget to be inserted, if it doesn't already exist. Will reorganize widgets if needed.
     */
    void insertWidgetIntoGrid(ListCollectionSubWidget* widget);

    /*!
     * \brief removeWidgetFromGrid remove ListCollectionSubWidget from the QGridLayout used for collection widgets.
     *        all widgets to the right of the removed widget get moved back one cell.
     * \param widget widget to be removed, if it exists.
     */
    void removeWidgetFromGrid(ListCollectionSubWidget* widget);

    /*!
     * \brief mWidgets list of all device widgets displayed in this widget.
     */
    std::set<ListCollectionSubWidget*, subWidgetCompare> mWidgets;

    /*!
     * \brief mName label for name of collection
     */
    QLabel *mName;

    /*!
     * \brief mHiddenStateIcon an arrow in the top right of the widget. If its pointing right, no
     *        buttons are shown. If its pointing down, all buttons are shown.
     */
    QLabel *mHiddenStateIcon;

    /*!
     * \brief mEditButton button used to edit the collection. Editing can change
     *        the name or the lights contained in the collection.
     */
    QPushButton *mEditButton;

    /*!
     * \brief mKey unique key for collection
     */
    QString mKey;

    /*!
     * \brief mTopLayout layout for the top of the widget
     */
    QHBoxLayout *mTopLayout;

    /// pixmap for icon for the edit button
    QPixmap mEditIcon;

    /// pixmap for icon that conveys no buttons being shown
    QPixmap mClosedPixmap;

    /// pixmap for icon that conveys all buttons being shown
    QPixmap mOpenedPixmap;

    /*!
     * \brief mGridLayout layout that displays all of the sub widgets.
     */
    QGridLayout *mGridLayout;

    /*!
     * \brief mMinimumSize minimum size allowed for collection.
     */
    int mMinimumSize;

    /*!
     * \brief mShowButtons true if buttons are showing, false otherwise.
     */
    bool mShowButtons;

    /*!
     * \brief mIconRatio
     */
    float mIconRatio;

    /*!
     * \brief mHideEdit true if the edit button should be hidden, false otherwise.
     */
    bool mHideEdit;

public slots:


    /*!
    * \brief editButtonClicked called when edit button is pressed, sends out an edit signal
    */
   void editButtonClicked(bool) { emit editClicked(mKey);  }

};

#endif // LISTCOLLECTIONWIDGET_H
