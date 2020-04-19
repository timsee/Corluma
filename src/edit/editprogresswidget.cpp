/*!
 * \copyright
 * Copyright (C) 2015 - 2020.
 * Released under the GNU General Public License.
 */
#include "editprogresswidget.h"
#include <QInputDialog>
#include <QMessageBox>
#include <QStyleOption>
#include <QtCore>
#include <QtGui>

#include "utils/qt.h"

EditProgressWidget::EditProgressWidget(QWidget* parent, std::uint32_t numberOfPages)
    : QWidget(parent),
      mNumberOfPages{numberOfPages},
      mCurrentPage{0u},
      mPageState(mNumberOfPages, EEditProgressState::incomplete) {}


void EditProgressWidget::changeToPage(std::uint32_t pageNumber) {
    if (mCurrentPage != pageNumber) {
        mCurrentPage = pageNumber;
        update();
    }
}


void EditProgressWidget::updateState(std::uint32_t index, EEditProgressState state) {
    if (index < mPageState.size()) {
        mPageState[index] = state;
    } else {
        qDebug() << "sent incorrect index, expected an index less than " << mPageState.size()
                 << " but got " << index;
    }
}

void EditProgressWidget::mouseReleaseEvent(QMouseEvent* event) {
    if (cor::isMouseEventTouchUpInside(event, this, true)) {
        auto buttonRegionWidth = std::uint32_t(width()) / mNumberOfPages;
        auto buttonPage = std::uint32_t(event->x()) / buttonRegionWidth;
        if (mPageState[buttonPage] != EEditProgressState::locked) {
            emit changePage(buttonPage);
        } else {
            qDebug() << " button locked";
        }
    }
    event->ignore();
}

namespace {

QColor stateToColor(EEditProgressState state) {
    switch (state) {
        case EEditProgressState::locked:
            return QColor(200, 200, 200);
        case EEditProgressState::incomplete:
            return QColor(127, 127, 0);
        case EEditProgressState::completed:
            return QColor(0, 127, 0);
    }
    return QColor(0, 0, 0);
}

void paintBackgroundLine(QPainter& painter,
                         const QPoint& startPoint,
                         const QPoint& endPoint,
                         int width) {
    QPen pen(QColor(127, 127, 127), width);
    painter.setPen(pen);
    painter.drawLine(startPoint, endPoint);
}

void paintCircle(QPainter& painter,
                 const QRect& position,
                 EEditProgressState state,
                 bool isActive,
                 bool paintLine) {
    // generate circle rect
    auto circleDiameter = position.height() * 3 / 5;
    auto circleSpacer = (position.height() - circleDiameter) / 2;
    QRect circleRect(position.x() + circleSpacer / 2, circleSpacer, circleDiameter, circleDiameter);
    QBrush circleBrush(stateToColor(state));
    if (paintLine) {
        //        auto lastStartX = position.x() - position.width() + circleSpacer / 2 +
        //        circleDiameter; qDebug() << "last start x is " << lastStartX; qDebug() << "
        //        current start x is " << circleRect.x(); auto distance = position.x() +
        //        circleDiameter - lastStartX; qDebug() << " distance is " << distance; QPoint
        //        startPoint(distance, position.height() / 2); QPoint endPoint(circleRect.x(),
        //        position.height() / 2); QPen pen(stateToColor(state), position.height() / 10);
        //        painter.setPen(pen);
        //        painter.drawLine(startPoint, endPoint);
    }
    if (isActive) {
        painter.setPen(QPen(QColor(255, 255, 255), position.height() / 15));
    } else {
        painter.setPen(Qt::NoPen);
    }
    painter.setBrush(circleBrush);
    painter.drawEllipse(circleRect);
}


} // namespace

void EditProgressWidget::paintEvent(QPaintEvent*) {
    QStyleOption opt;
    opt.init(this);
    QPainter painter(this);

    painter.setRenderHint(QPainter::Antialiasing);
    painter.fillRect(rect(), QBrush(QColor(48, 47, 47)));

    // get regions for each circle
    auto widgetSize = this->geometry().size();
    auto circleRegionWidth = std::uint32_t(widgetSize.width()) / mNumberOfPages;

    paintBackgroundLine(painter,
                        QPoint(circleRegionWidth / 2, this->height() / 2),
                        QPoint(this->width() - (circleRegionWidth / 2), this->height() / 2),
                        this->height() / 15);

    // get initial widget's region
    QRect initalRect(0, 0, int(circleRegionWidth), widgetSize.height());
    paintCircle(painter, initalRect, mPageState[0u], mCurrentPage == 0u, false);

    // now draw the rest of the circles
    for (auto i = 1u; i < mNumberOfPages; ++i) {
        QRect circleRect(int(circleRegionWidth * i),
                         0,
                         int(circleRegionWidth),
                         widgetSize.height());
        paintCircle(painter, circleRect, mPageState[i], mCurrentPage == i, true);
    }
}
