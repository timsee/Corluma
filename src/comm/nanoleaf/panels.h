#ifndef PANELS_H
#define PANELS_H

#include <QJsonArray>
#include <QJsonObject>
#include <vector>

#include "cor/range.h"
#include "utils/exception.h"

namespace nano {
/*!
 * \copyright
 * Copyright (C) 2015 - 2019.
 * Released under the GNU General Public License.
 */

/*!
 * \brief The Panel class is a simple class storing data about a panel
 */
class Panel {
public:
    Panel(const QJsonObject& object) {
        if (object["panelId"].isDouble() && object["x"].isDouble() && object["y"].isDouble()
            && object["o"].isDouble()) {
            mID = int(object["panelId"].toDouble());
            mX = int(object["x"].toDouble());
            mY = int(object["y"].toDouble());
            mO = int(object["o"].toDouble());
        } else {
            THROW_EXCEPTION("Invalid Panels JSON");
        }
    }

    /// x coordinate of the centroid of the panel
    int x() const noexcept { return mX; }

    /// y coordinate of the centroid of the panel
    int y() const noexcept { return mY; }

    /// number given to the panel
    int ID() const noexcept { return mID; }

    /// orientation of the panel
    int o() const noexcept { return mO; }

private:
    /// number given to the panel
    int mID;
    /// x coordinate of the centroid of the panel
    int mX;
    /// y coordinate of the centroid of the panel
    int mY;
    /// orientation of the panel
    int mO;
};

/*!
 * \brief The Panels class contains all the information about the
 *        panels used by the NanoLeaf Controller
 */
class Panels {
public:
    /// constructor
    Panels(const QJsonObject& object) : Panels() {
        if (object["layout"].isObject()) {
            QJsonObject layoutObject = object["layout"].toObject();
            if (layoutObject["numPanels"].isDouble() && layoutObject["sideLength"].isDouble()
                && layoutObject["positionData"].isArray()) {
                mCount = int(layoutObject["numPanels"].toDouble());
                mSideLength = int(layoutObject["sideLength"].toDouble());
                QJsonArray array = layoutObject["positionData"].toArray();
                std::vector<nano::Panel> panelInfoVector;
                for (auto value : array) {
                    if (value.isObject()) {
                        QJsonObject object = value.toObject();
                        panelInfoVector.emplace_back(value.toObject());
                    }
                }
                mPositionData = panelInfoVector;
            }
        } else {
            THROW_EXCEPTION("Invalid JSON for Panels");
        }
    }

    /// constructor
    Panels() : mCount{1}, mSideLength{3}, mOrientationValue{0}, mOrientationRange{0, 0} {}

    /// number of panels connected to the controller.
    int count() const noexcept { return mCount; }

    /// the length of a triangle side, in pixels, that is used in calculation of the centroid
    /// location
    int sideLength() const noexcept { return mSideLength; }

    /// current value for the orientation of the panels
    int orientationValue() const noexcept { return mOrientationValue; }

    /// a vector of data about each of the individual panels
    const std::vector<Panel>& positionData() const noexcept { return mPositionData; }

    /// potential range for the orientation value
    const cor::Range<int>& orientationRange() const noexcept { return mOrientationRange; }

private:
    /// number of panels connected to the controller.
    int mCount;

    /// the length of a triangle side, in pixels, that is used in calculation of the centroid
    /// location
    int mSideLength;

    /// a vector of data about each of the individual panels
    std::vector<Panel> mPositionData;

    /// current value for the orientation of the panels
    int mOrientationValue;

    /// potential range for the orientation value
    cor::Range<int> mOrientationRange;
};

} // namespace nano

#endif // PANELS_H
