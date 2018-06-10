#ifndef LISTCOLLECTIONWIDGET_H
#define LISTCOLLECTIONWIDGET_H

#include <QWidget>
#include <QGridLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <vector>
#include <QScrollArea>

#include "comm/commlayer.h"
#include "listcollectionsubwidget.h"

/// type of list
enum class EListType {
    grid,
    linear,
    linear2X
};


/// contents of the widget
enum class EWidgetContents {
    moods,
    groups,
    devices
};

/*!
 * \copyright
 * Copyright (C) 2015 - 2018.
 * Released under the GNU General Public License.
 *
 *
 * \brief The ListCollectionWidget class is parent class for widgets shown on the lights page. When displaying either
 *        devices or moods, they are grouped by collections. Collections are a series of devices that can be controlled together.
 *        In the case of the devices page, each collection shows all its devices. On the moods page, each collection shows all the
 *        moods that use the devices in the collection.
 */
class ListCollectionWidget : public QWidget
{
    Q_OBJECT
public:
    /*!
     * \brief Constructor
     */
    explicit ListCollectionWidget(QWidget *parent = 0);

    /// destructor
    virtual ~ListCollectionWidget(){}

    /*!
     * \brief setup called from constructors, sets up objects that are used in all
     *        collection widgets
     * \param name name of collection
     * \param key unique key for collection
     * \param type type of list, determines how the list displays
     * \param hideEdit true for groups that can't be edited such as available and not reachable,
     *        false otherwise
     */
    void setup(const QString& name,
               const QString& key,
               EListType type,
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

    /// resizes the widget
    void resize();

    /// vector of all sub widgets in the list.
    const std::vector<ListCollectionSubWidget*>& widgets() { return mWidgets; }

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
     * \brief widgetContents contents of the widget, this is a hack and should be removed.
     * \TODO remove this
     * \return type of contents.
     */
    virtual EWidgetContents widgetContents() = 0;

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

    /*!
     * \brief rowCountChanged emits when the row count changes so that the parent holding this listcollectionwidget
     *        can know if it needs to move its collection of listcollectionwidgets around. Emits the new row count
     */
    void rowCountChanged(int);

protected:

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
     * \brief insertWidget insert ListCollectionSubWidget into the QGridLayout used for collection widgets
     * \param widget widget to be inserted, if it doesn't already exist. Will reorganize widgets if needed.
     */
    void insertWidget(ListCollectionSubWidget* widget);

    /*!
     * \brief removeWidget remove ListCollectionSubWidget from the QGridLayout used for collection widgets.
     *        all widgets to the right of the removed widget get moved back one cell.
     * \param widget widget to be removed, if it exists.
     */
    void removeWidget(ListCollectionSubWidget* widget);

    /*!
     * \brief widgetPosition gives the widget position based off of the given widget. Position is not *actual* position,
     *        but where it falls in the group overall. For example, top left widget is (0,0). Next widget is (0,1). First
     *        widget in second row is (1,0)
     * \param widget widget to look for position for.
     * \return Position of widget as a point.
     */
    QPoint widgetPosition(QWidget *widget);

    /*!
     * \brief mWidgets list of all device widgets displayed in this widget.
     */
    std::vector<ListCollectionSubWidget*> mWidgets;

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

    /// widget used for background of grid
    QWidget *mWidget;

    /// size of overall background of grid.
    QSize mWidgetSize;

    /*!
     * \brief mLayout layout that displays all of the sub widgets.
     */
    QVBoxLayout *mLayout;

    /*!
     * \brief mMinimumHeight minimum size allowed for collection.
     */
    int mMinimumHeight;

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

    /// type of list
    EListType mType;

public slots:

    /*!
    * \brief editButtonClicked called when edit button is pressed, sends out an edit signal
    */
   void editButtonClicked(bool) { emit editClicked(mKey);  }

private:

   /// stored count of how many rows there are. Used to detect changes
   uint32_t mRowCount;

   /// moves widgets into their proper location on a grid.
   void moveWidgets();
};

#endif // LISTCOLLECTIONWIDGET_H
