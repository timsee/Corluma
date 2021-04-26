#ifndef LISTGROUPGROUPWIDGET_H
#define LISTGROUPGROUPWIDGET_H

#include <QGridLayout>
#include <QLabel>
#include <QObject>
#include <QPushButton>
#include <QWidget>

#include "cor/objects/mood.h"
#include "cor/widgets/groupbutton.h"
#include "cor/widgets/listitemwidget.h"
#include "listmoodpreviewwidget.h"
#include "menu/menumoodcontainer.h"

/*!
 * \copyright
 * Copyright (C) 2015 - 2020.
 * Released under the GNU General Public License.
 *
 * \brief The ListMoodGroupWidget class is a subclass of a ListCollectionWidget. It displays a
 * collection of moods. It contains a widget at the top that displays all the information about the
 * collection and gives the ability to edit it. The rest of the collection is a series of buttons
 * where each one represents a mood that contains the same devices as the collection.
 */
class ListMoodWidget : public cor::ListItemWidget {
    Q_OBJECT
public:
    /*!
     * \brief ListMoodGroupWidget constructor
     *
     * \param name name of collection
     * \param moods moods to display for collection
     * \param key key for collection
     */
    explicit ListMoodWidget(const QString& name,
                            const std::vector<cor::Mood>& moods,
                            const QString& key,
                            QWidget* parent);

    /*!
     * \brief updateMoods update moods based off of the mood list and the vector of preset colors.
     *
     * \param moods list of moods
     */
    void updateMoods(const std::vector<cor::Mood>& moods);

    /*!
     * \brief moods getter for the mood data of this collection group
     *
     * \return all the mood data for this collection group
     */
    const std::vector<cor::Mood>& moods() { return mMoodContainer->moods(); }

    /*!
     * \brief setShowButtons shows and hides all buttons on the widget
     *
     * \param show true to show, false otherwise.
     */
    void setShowButtons(bool show);

    /// resizes programmatically
    void resize();

signals:

    /// called when a mood is selected
    void moodSelected(QString, std::uint64_t);

    /*!
     * \brief buttonsShown emitted when the buttons are shown or hidden. emits the key and a boolean
     *        representing whether the buttons are shown.
     */
    void buttonsShown(QString, bool);

protected:
    /// resizes the widgets
    void resizeEvent(QResizeEvent*);

    /*!
     * \brief mouseReleaseEvent picks up when a click (or a tap on mobile) is released.
     */
    virtual void mouseReleaseEvent(QMouseEvent*);

private slots:
    /// called when a mood is clicked
    void selectMood(std::uint64_t key) { emit moodSelected(mKey, key); }

private:
    /// widget for showing/hiding
    cor::GroupButton* mGroupButton;

    /// displays the moods in the widget
    MenuMoodContainer* mMoodContainer;
};

#endif // LISTGROUPGROUPWIDGET_H
