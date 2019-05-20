#ifndef MOODDETAILSWIDGET_H
#define MOODDETAILSWIDGET_H

#include <QWidget>

#include <QLabel>
#include <QVBoxLayout>

#include "cor/mood.h"
#include "groupdata.h"
#include "listsimplegroupwidget.h"

/*!
 * \copyright
 * Copyright (C) 2015 - 2019.
 * Released under the GNU General Public License.
 */


/*!
 * \brief The MoodDetailsWidget class is a simple widget used to display details about a mood, such
 * as its additional info and its default states
 */
class MoodDetailsWidget : public QWidget {
    Q_OBJECT
public:
    /// contructor
    explicit MoodDetailsWidget(GroupData* groups, QWidget* parent);

    /// displays a mood
    void display(const cor::Mood& mood, const QSize& size);

    /// resizes the widget programmatically
    void resize(const QSize& size);

private:
    /// data for all the groups
    GroupData* mGroups;

    /// text for more info widget
    QLabel* mMoreInfoText;

    /// title for room default section
    QLabel* mRoomDefaultsTitle;

    /// widget displaying room defaults
    ListSimpleGroupWidget* mRoomDefaults;

    /// title for group default section
    QLabel* mGroupDefaultsTitle;

    /// widget displaying group defautls
    ListSimpleGroupWidget* mGroupDefaults;
};

#endif // MOODDETAILSWIDGET_H
