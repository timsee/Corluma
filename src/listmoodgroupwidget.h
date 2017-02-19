#ifndef LISTGROUPGROUPWIDGET_H
#define LISTGROUPGROUPWIDGET_H

#include <QObject>
#include <QWidget>
#include <QLabel>
#include <QGridLayout>
#include <QHBoxLayout>
#include <QPushButton>

#include "listcollectionwidget.h"
#include "listmoodwidget.h"
#include "datalayer.h"
#include "commlayer.h"

/*!
 * \copyright
 * Copyright (C) 2015 - 2017.
 * Released under the GNU General Public License.
 *
 * \brief The ListMoodGroupWidget class is a subclass of a ListCollectionWidget. It displays a collection of moods.
 *        It contains a widget at the top that displays all the information about the collection and gives the ability
 *        to edit it. The rest of the collection is a series of buttons where each one represents a mood that contains the
 *        same devices as the collection.
 */
class ListMoodGroupWidget : public ListCollectionWidget
{
    Q_OBJECT
public:

    /*!
     * \brief ListMoodGroupWidget constructor
     * \param name name of collection
     * \param moods moods to display for collection
     * \param colors all color groups from the data layer. used for displaying icons
     *        for collections.
     * \param key key for collection
     * \param listHeight given height for the widget
     * \param hideEdit true if edit button should be hidden, such as in special case collections.
     *        An example of a special case is "Not Reachable" moods, which contains moods you can't
     *        connect to.
     */
    explicit ListMoodGroupWidget(const QString& name,
                                  std::list<std::pair<QString, std::list<SLightDevice> > > moods,
                                  const std::vector<std::vector<QColor> >& colors,
                                  QString key,
                                  int listHeight,
                                  bool hideEdit = false);

    /*!
     * \brief setCheckedMoods takes a list of moods as input and compares it against the
     *        widgets displayed. If a mood is in both, it is set as checked. If it is only in the widgets,
     *        it is set as unchecked.
     * \param checkedMoods
     */
    void setCheckedMoods(std::list<QString> checkedMoods);

    /*!
     * \brief moods getter for the mood data of this collection group
     * \return all the mood data for this collection group
     */
    const std::list<std::pair<QString, std::list<SLightDevice> > >& moods() { return mMoods; }

    /*!
     * \brief preferredSize all collection widgets must implement a preferred size. this is the size
     *        the widget wants to be. It may not end up this size but its a baseline if theres no other
     *        widgets pushing against it.
     * \return a QSize representing its ideal size.
     */
    QSize preferredSize();

    /*!
     * \brief setShowButtons shows and hides all buttons on the widget
     * \param show true to show, false otherwise.
     */
    void setShowButtons(bool show);

signals:

    /*!
     * \brief moodClicked
     */
    void moodClicked(QString, QString);

    /*!
     * \brief editClicked the edit button of a mood is clicked. Emits both the collection
     *        key and the mood's key.
     *
     */
    void editClicked(QString, QString);

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

private slots:

    /*!
     * \brief handleClicked the widget was clicked on one of the moods, so emit both the collection key
     *        and the mood key as a mood click event
     * \param key the mood's key.
     */
    void handleClicked(QString key) { emit moodClicked(mKey, key); }

    /*!
     * \brief clickedEdit the edit button was clicked on one of the moods, so emit both the collection key
     *        and the mood key as an edit clicked event.
     * \param key the mood's key.
     */
    void clickedEdit(QString key) { emit editClicked(mKey, key); }

private:


    /*!
     * \brief mWidgets list of mood widgets that are displayed by this widget.
     */
    std::list<ListMoodWidget*> mWidgets;

    /*!
     * \brief mMoods the data that represents the mood widgets that are displayed
     *        by this widget.
     */
    std::list<std::pair<QString, std::list<SLightDevice> > > mMoods;

    /*!
     * \brief mGridLayout grid to display the mood widgets
     */
    QGridLayout *mGridLayout;

};

#endif // LISTGROUPGROUPWIDGET_H
