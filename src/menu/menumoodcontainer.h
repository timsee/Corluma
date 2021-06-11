#ifndef MENUMOODCONTAINER_H
#define MENUMOODCONTAINER_H

#include <QWidget>
#include <vector>
#include "cor/listlayout.h"
#include "cor/objects/mood.h"


/*!
 * \copyright
 * Copyright (C) 2015 - 2020.
 * Released under the GNU General Public License.
 *
 *
 * \brief The MenuMoodContainer class is a widget that displays all the moods requested for it. it
 * will grow automatically based off of the provided height to show all possible widgets.
 */
class MenuMoodContainer : public QWidget {
    Q_OBJECT
public:
    explicit MenuMoodContainer(QWidget* parent);

    /// shows the moods in the vector, with the height provided used for each mood widget.
    void showMoods(const std::vector<cor::Mood>& moods, int moodHeight);

    /*!
     * \brief setCheckedMoods takes a list of moods as input and compares it against the
     * widgets displayed. If a mood is in both, it is set as checked. If it is only in the
     * widgets, it is set as unchecked.
     *
     * \param checkedMoods
     */
    void setCheckedMoods(const std::vector<QString>& checkedMoods);

    /*!
     * \brief moods getter for the mood data of this collection group
     *
     * \return all the mood data for this collection group
     */
    const std::vector<cor::Mood>& moods() { return mMoods; }

    /// remove all widgets from the container, reseting it to an empty state
    void clear();

    /// resize programmatically
    void resize();

signals:
    /// emits when a signal is selected.
    void moodSelected(QString key);

private slots:

    /// called when a mood is clicked
    void selectMood(QString key);


private:
    /// layout for widget
    cor::ListLayout mListLayout;

    /*!
     * \brief mMoods the data that represents the mood widgets that are displayed
     * by this widget.
     */
    std::vector<cor::Mood> mMoods;

    /// height for any mood widget.
    int mHeight;
};

#endif // MENUMOODCONTAINER_H
