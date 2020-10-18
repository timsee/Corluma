#ifndef DISPLAYMOODMETADATA_H
#define DISPLAYMOODMETADATA_H
/*!
 * \copyright
 * Copyright (C) 2015 - 2020.
 * Released under the GNU General Public License.
 */

#include <QWidget>
#include <sstream>
#include "comm/commlayer.h"
#include "cor/widgets/expandingtextscrollarea.h"
#include "data/groupdata.h"

/*!
 * \brief The DisplayGroupMetadata class compares a mood to all other mood, and generates
 * metadata such as its parent and whether or not it duplicates anything from other moods.
 */
class DisplayMoodMetadata : public cor::ExpandingTextScrollArea {
    Q_OBJECT
public:
    explicit DisplayMoodMetadata(QWidget* parent, CommLayer* comm, GroupData* groups);

    /// true if errors that would prevent the mood from being valid exist, false if the mood can
    /// be or is a valid mood. Errors include identical moods being detected, or moods that share
    /// the same name.
    bool errorsExist() const noexcept { return mErrorsExist; }

    /// update the widget to display metadata about a new mood.
    void update(const cor::Mood& group, bool moodExistsAlready);

    /// reset to showing no value
    void reset();

private:
    /// true if errors that would prevent the mood from being valid exist, false if the mood can
    /// be or is a valid mood. Errors include identical moods being detected, or moods that share
    /// the same name.
    bool mErrorsExist;

    /// pointer to comm data
    CommLayer* mComm;

    /// pointer to group data
    GroupData* mGroups;
};

#endif // DISPLAYMOODMETADATA_H
