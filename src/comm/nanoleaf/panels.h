#ifndef PANELS_H
#define PANELS_H

#include <QDebug>
#include <QJsonArray>
#include <QJsonObject>
#include <vector>

#include "cor/range.h"
#include "utils/exception.h"

namespace nano {
/*!
 * \copyright
 * Copyright (C) 2015 - 2020.
 * Released under the GNU General Public License.
 */

enum class EShapeType {
    triangle = 0,
    rhythm = 1,
    square = 2,
    controlSquareMaster = 3,
    controlSquarePassive = 4,
    heaxagonShapes = 7,
    triangleShapes = 8,
    miniTriangleShapes = 9,
    controllerShapes = 12
};

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
            mShape = EShapeType(object["shapeType"].toDouble());
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

    /// shape for the panel
    EShapeType shape() const noexcept { return mShape; }

    /// getter for side length, inferred by shapeType.
    int sideLength() {
        switch (mShape) {
            case EShapeType::triangle:
                return 150;
            case EShapeType::square:
            case EShapeType::controlSquareMaster:
            case EShapeType::controlSquarePassive:
                return 100;
            case EShapeType::heaxagonShapes:
            case EShapeType::miniTriangleShapes:
                return 67;
            case EShapeType::triangleShapes:
                return 134;
            case EShapeType::controllerShapes:
            case EShapeType::rhythm:
            default:
                return 0u;
        }
    }

private:
    /// number given to the panel
    int mID;
    /// x coordinate of the centroid of the panel
    int mX;
    /// y coordinate of the centroid of the panel
    int mY;
    /// orientation of the panel
    int mO;
    /// type of shape
    EShapeType mShape;
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
                QJsonArray array = layoutObject["positionData"].toArray();
                std::vector<nano::Panel> panelInfoVector;
                for (auto value : array) {
                    if (value.isObject()) {
                        QJsonObject object = value.toObject();
                        panelInfoVector.emplace_back(value.toObject());
                    }
                }
                mPositionLayout = panelInfoVector;
            }
        } else {
            THROW_EXCEPTION("Invalid JSON for Panels");
        }
    }

    /// constructor
    Panels() : mCount{1}, mOrientationValue{0}, mOrientationRange{0, 0} {}

    /// number of panels connected to the controller.
    int count() const noexcept { return mCount; }

    /// current value for the orientation of the panels
    int orientationValue() const noexcept { return mOrientationValue; }

    /// a vector of data about each of the individual panels
    const std::vector<Panel>& positionLayout() const noexcept { return mPositionLayout; }

    /// potential range for the orientation value
    const cor::Range<int>& orientationRange() const noexcept { return mOrientationRange; }

private:
    /// number of panels connected to the controller.
    int mCount;

    /// a vector of data about each of the individual panels
    std::vector<Panel> mPositionLayout;

    /// current value for the orientation of the panels
    int mOrientationValue;

    /// potential range for the orientation value
    cor::Range<int> mOrientationRange;
};

} // namespace nano

#endif // PANELS_H
