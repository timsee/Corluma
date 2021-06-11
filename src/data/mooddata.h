#ifndef MOODDATA_H
#define MOODDATA_H

#include "cor/dictionary.h"
#include "cor/objects/mood.h"
/*!
 * \copyright
 * Copyright (C) 2015 - 2021.
 * Released under the GNU General Public License.
 */


/*!
 * \brief The MoodData class stores all data related to moods. Moods are a collection of lights in a
 * specific state. Moods can also contain "default states" for groups, where, for all lights not
 * already defined in the mood, the default state will be applied. Moods are useful for setting "all
 * lights to red, except one light in the corner to green" or setting up ambiance for specific
 * movies/games.
 */
class MoodData : public QObject {
    Q_OBJECT

public:
    MoodData();

    /*!
     * \brief moodList getter for all known moods.
     *
     * \return a list of all the moods. Each mood is represented as a pair with its name
     *         and a list of the devices with their associated state.
     */
    const cor::Dictionary<cor::Mood>& moods() const noexcept { return mMoodDict; }

    /// load mood data from json
    void loadFromJson(const QJsonArray& array);

    /// converts a mood ID into a mood, returning an invalid mood if the ID is invalid.
    cor::Mood moodFromID(const QString& ID);

    /// returns a group's name from its ID
    QString nameFromID(const QString& ID);

    /*!
     * \brief saveNewMood save a new mood
     */
    void saveNewMood(const cor::Mood& mood);

    /// remove a move by uniqueID
    QString removeMood(const QString& uniqueID);

    /// clear all moods
    void clear() { mMoodDict = cor::Dictionary<cor::Mood>(); }

    /// convert to json array
    QJsonArray toJsonArray();

    /// remove light from all moods.
    bool removeLightFromMoods(QString);

signals:
    /// signals when a mood is added
    void moodAdded(QString);

    /// signals when a mood is deleted.
    void moodDeleted(QString);

private:
    /*!
     * \brief mMoodDict non-JSON representation of moods. This dictionary is kept so that it is
     * easy to pull all possible moods without having to re-parse the JSON data each time.
     */
    cor::Dictionary<cor::Mood> mMoodDict;
};

#endif // MOODDATA_H
