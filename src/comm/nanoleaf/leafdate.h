#ifndef LEAFDATE_H
#define LEAFDATE_H

#include <time.h>
#include <QDebug>
#include <QJsonObject>

namespace nano {

/*!
 * \copyright
 * Copyright (C) 2015 - 2020.
 * Released under the GNU General Public License.
 */


/*!
 * \brief The LeafDate class converts between the nanoleaf representation of a date and an internal
 * format that stores the date in std::tm. This is a utility class for simplifying working with
 * nanoleaf's date format.
 */
class LeafDate {
public:
    LeafDate() {
        m_date.tm_hour = 0;
        m_date.tm_mday = 0;
        m_date.tm_min = 0;
        m_date.tm_mon = 0;
        m_date.tm_year = 3000;
    }

    /// creates a LeafDate at the current time.
    static LeafDate currentTime() {
        auto t = std::time(nullptr);
        auto now = std::localtime(&t);
        now->tm_sec = 0;
        return LeafDate(*now);
    }

    LeafDate(std::tm time) : m_date(time) {}

    LeafDate(const QJsonObject& object) {
        // use localtime to fill in non-defined objects
        auto t = std::time(nullptr);
        auto now = std::localtime(&t);
        now->tm_sec = 0;
        m_date = *now;
        m_date.tm_mday = int(object["day"].toDouble());
        m_date.tm_hour = int(object["hour"].toDouble());
        m_date.tm_min = int(object["minute"].toDouble());
        m_date.tm_mon = int(object["month"].toDouble()) - 1.0;
        auto year = int(object["year"].toDouble());
        if (year > 1900) {
            m_date.tm_year = year - 1900;
        }
    }

    /// helper to print out all helper values, for debugging
    static void printAllTimeValues(tm tm) {
        qDebug() << " tm_gmtoff: " << tm.tm_gmtoff << " tm_hour: " << tm.tm_hour
                 << " isdst: " << tm.tm_isdst << " mday: " << tm.tm_mday << " min: " << tm.tm_min
                 << " mon: " << tm.tm_mon << " sec: " << tm.tm_sec << " wday: " << tm.tm_wday
                 << " yday: " << tm.tm_yday << " year: " << tm.tm_year << " zone: " << tm.tm_zone;
    }

    /// checks if JSON represents a LeafDate
    static bool isValidJson(const QJsonObject& object) {
        return (object["day"].isDouble() && object["hour"].isDouble() && object["minute"].isDouble()
                && object["month"].isDouble() && object["time_zone"].isDouble()
                && object["year"].isDouble());
    }

    /// converts the object to a valid JSON representation
    QJsonObject toJson() const {
        QJsonObject object;
        object["day"] = m_date.tm_mday;
        object["hour"] = m_date.tm_hour;
        object["minute"] = m_date.tm_min;
        if (isEmpty()) {
            object["time_zone"] = 0;
            object["month"] = 0;
        } else {
            object["time_zone"] = 0; //-14400;
            object["month"] = m_date.tm_mon;
        }
        auto year = m_date.tm_year;
        if (year < 1900) {
            year = year + 1900;
        }
        object["year"] = year;
        return object;
    }

    /// prints the date in a human readable format
    QString toString() const {
        char buffer[80];
        std::strftime(buffer, sizeof(buffer), "%m-%d-%Y %H:%M", &m_date);
        return QString(buffer);
    }

    /// returns the date
    const std::tm& date() const noexcept { return m_date; }

    /// true if empty date, false otherwise
    bool isEmpty() const {
        return (m_date.tm_hour == 0) && (m_date.tm_min == 0)
               && ((m_date.tm_year == 2999) || (m_date.tm_year == 3000));
    }

private:
    /// stores the date in a standard format
    std::tm m_date;
};


} // namespace nano

#endif // LEAFDATE_H
