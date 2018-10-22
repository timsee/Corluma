#ifndef LISTGROUPGROUPWIDGET_H
#define LISTGROUPGROUPWIDGET_H

#include <QObject>
#include <QWidget>
#include <QLabel>
#include <QGridLayout>
#include <QHBoxLayout>
#include <QPushButton>

#include <set>

#include "cor/listitemwidget.h"
#include "cor/listlayout.h"
#include "listmoodwidget.h"
#include "dropdowntopwidget.h"


/*!
 * \copyright
 * Copyright (C) 2015 - 2018.
 * Released under the GNU General Public License.
 *
 * \brief The ListMoodGroupWidget class is a subclass of a ListCollectionWidget. It displays a collection of moods.
 *        It contains a widget at the top that displays all the information about the collection and gives the ability
 *        to edit it. The rest of the collection is a series of buttons where each one represents a mood that contains the
 *        same devices as the collection.
 */
class ListMoodGroupWidget : public cor::ListItemWidget
{
    Q_OBJECT
public:

    /*!
     * \brief ListMoodGroupWidget constructor
     * \param name name of collection
     * \param moods moods to display for collection
     * \param key key for collection
     * \param hideEdit true if edit button should be hidden, such as in special case collections.
     *        An example of a special case is "Not Reachable" moods, which contains moods you can't
     *        connect to.
     */
    explicit ListMoodGroupWidget(const QString& name,
                                  std::list<cor::LightGroup> moods,
                                  QString key,
                                  bool hideEdit,
                                  QWidget *parent);

    /*!
     * \brief updateMoods update moods based off of the mood list and the vector of preset colors.
     * \param moods list of moods
     * \param bool removeIfNotFound if a widget already exists but this flag is set to true and it doesn't exist
     *        in the mood list provided, the widget gets removed from the list.
     */
    void updateMoods(const std::list<cor::LightGroup>& moods,
                     bool removeIfNotFound);

    /*!
     * \brief setCheckedMoods takes a list of moods as input and compares it against the
     *        widgets displayed. If a mood is in both, it is set as checked. If it is only in the widgets,
     *        it is set as unchecked.
     * \param checkedMoods
     */
    void setCheckedMoods(std::list<QString> checkedMoods);

    /*!
     * \brief removeMood remove a mood from the ListCollectionWidget
     * \param mood name of mood to remove.
     */
    void removeMood(QString mood);

    /*!
     * \brief moods getter for the mood data of this collection group
     * \return all the mood data for this collection group
     */
    const std::list<cor::LightGroup>& moods() { return mMoods; }

    /*!
     * \brief setShowButtons shows and hides all buttons on the widget
     * \param show true to show, false otherwise.
     */
    void setShowButtons(bool show);

    /// closes all the displayed lights for this widget
    void closeLights();

signals:

    /*!
     * \brief moodClicked emitted when a mood is clicked in a collection. Gives the key of the collection
     *        and the key of the mood.
     */
    void moodClicked(QString, QString);

    /*!
     * \brief editClicked the edit button of a mood is clicked. Emits both the collection
     *        key and the mood's key.
     *
     */
    void editClicked(QString, QString);


    /*!
     * \brief buttonsShown emitted when the buttons are shown or hidden. emits the key and a boolean
     *        representing whether the buttons are shown.
     */
    void buttonsShown(QString, bool);

protected:

    /// resizes the widgets
    void resizeEvent(QResizeEvent *);

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

    cor::ListLayout mListLayout;

    /// widget used for background of grid
    QWidget *mWidget;

    /*!
     * \brief mLayout layout that displays all of the sub widgets.
     */
    QVBoxLayout *mLayout;

    /// type of list
    cor::EListType mType;

    /// widget for showing/hiding and selecting/deselecting
    DropdownTopWidget *mDropdownTopWidget;

    /// resize the widgets displayed in the group
    void resizeInteralWidgets();

    /*!
     * \brief mMoods the data that represents the mood widgets that are displayed
     *        by this widget.
     */
    std::list<cor::LightGroup> mMoods;

};

#endif // LISTGROUPGROUPWIDGET_H
