/*!
 * \copyright
 * Copyright (C) 2015 - 2020.
 * Released under the GNU General Public License.
 */
#include "parentgroupwidget.h"

#include <QGraphicsEffect>
#include <QGraphicsScene>
#include <QMouseEvent>
#include <QPainter>
#include <QPainterPath>
#include <QStyleOption>

namespace {

QColor computeHighlightColor(std::uint32_t checkedCount, std::uint32_t reachableCount) {
    QColor pureBlue(61, 142, 201);
    QColor pureBlack(33, 32, 32);
    QColor difference(pureBlue.red() - pureBlack.red(),
                      pureBlue.green() - pureBlack.green(),
                      pureBlue.blue() - pureBlack.blue());

    if (reachableCount == 0u || checkedCount == 0u) {
        return pureBlack;
    } else {
        double amountOfBlue = double(checkedCount) / double(reachableCount);
        return {int(amountOfBlue * difference.red() + pureBlack.red()),
                int(amountOfBlue * difference.green() + pureBlack.green()),
                int(amountOfBlue * difference.blue() + pureBlack.blue())};
    }
}

} // namespace


void ParentGroupWidget::updateCheckedLights(std::uint32_t checkedLightCount,
                                            std::uint32_t reachableLightCount) {
    mCheckedCount = checkedLightCount;
    mReachableCount = reachableLightCount;
    update();
}

void ParentGroupWidget::paintEvent(QPaintEvent*) {
    QStyleOption opt;
    opt.initFrom(this);
    QPainter painter(this);
    QPen pen(Qt::white, 5);
    painter.setPen(pen);

    QPainterPath path;
    path.addRect(rect());

    painter.fillPath(path, QBrush(computeHighlightColor(mCheckedCount, mReachableCount)));
}
