/*!
 * \copyright
 * Copyright (C) 2015 - 2020.
 * Released under the GNU General Public License.
 */

#include "huegroupwidget.h"
#include "cor/stylesheets.h"

#include <QGraphicsOpacityEffect>
#include <QStyleOption>
#include <QtCore>
#include <QtGui>

namespace hue {

HueGroupWidget::HueGroupWidget(QWidget* parent, std::uint32_t index, const cor::Group& group)
    : QWidget(parent),
      mName{new QLabel("<b>" + group.name() + "</b>", this)},
      mGroupIcon{new QLabel(this)},
      mIndex{new QLabel("<b>Index:</b> " + QString::number(index), this)},
      mGroupDescription{new QLabel(
          generateDescription(group.lights().size(), group.type() == cor::EGroupType::room),
          this)},
      mLastSize{0, 0},
      mGroup{group} {
    setStyleSheet(cor::kTransparentStylesheet);

    auto font = mName->font();
    auto fontSize = int(font.pointSize() * 1.33);
    font.setPointSize(fontSize);
    mName->setFont(font);
    mName->setAttribute(Qt::WA_TransparentForMouseEvents, true);

    mIndex->setAttribute(Qt::WA_TransparentForMouseEvents, true);

    mGroupDescription->setAttribute(Qt::WA_TransparentForMouseEvents, true);
}

QString HueGroupWidget::generateDescription(std::size_t lightCount, bool isRoom) {
    QString returnString;
    if (isRoom) {
        returnString += "<i>Room";
    } else {
        returnString += "<i>Group";
    }
    returnString += ", with ";
    returnString += QString::number(lightCount);
    if (lightCount == 1) {
        returnString += " light.</i>";
    } else {
        returnString += " lights.</i>";
    }
    return returnString;
}


void HueGroupWidget::resize() {
    auto yPos = 0u;
    auto rowHeight = height() / 3;
    auto xSpacer = width() * 0.03;
    auto buttonWidth = width() - xSpacer;

    auto iconWidth = rowHeight;
    auto xPos = xSpacer;
    mGroupIcon->setGeometry(xPos, yPos, iconWidth, rowHeight);
    xPos += mGroupIcon->width();
    mName->setGeometry(xPos, yPos, buttonWidth - iconWidth, rowHeight);
    yPos += rowHeight;

    mIndex->setGeometry(xSpacer, yPos, buttonWidth, rowHeight);
    yPos += rowHeight;

    mGroupDescription->setGeometry(xSpacer, yPos, buttonWidth, rowHeight);
    yPos += rowHeight;

    if (QSize(iconWidth * 0.9, iconWidth * 0.9) != mLastSize) {
        renderIcon();
    }
}

void HueGroupWidget::renderIcon() {
    mLastSize = QSize(mGroupIcon->width() * 0.9, mGroupIcon->width() * 0.9);
    QPixmap pixmap;
    if (mGroup.type() == cor::EGroupType::room) {
        pixmap = QPixmap(":/images/room_icon.png");
    } else {
        pixmap = QPixmap(":/images/groups_icon.png");
    }
    pixmap = pixmap.scaled(mLastSize.width(),
                           mLastSize.height(),
                           Qt::KeepAspectRatio,
                           Qt::SmoothTransformation);
    mGroupIcon->setPixmap(pixmap);
}


void HueGroupWidget::paintEvent(QPaintEvent*) {
    QStyleOption opt;
    opt.initFrom(this);
    QPainter painter(this);

    painter.setRenderHint(QPainter::Antialiasing);
    painter.fillRect(rect(), QBrush(QColor(32, 31, 31)));

    // draw line at bottom of widget
    auto lineOffset = 3;
    QRect area(x(), y(), width(), height());
    QPainter linePainter(this);
    linePainter.setRenderHint(QPainter::Antialiasing);
    linePainter.setBrush(QBrush(QColor(255, 255, 255)));
    QLine spacerLine(QPoint(area.x(), area.height() - lineOffset),
                     QPoint(area.width(), area.height() - lineOffset));
    linePainter.drawLine(spacerLine);
}


} // namespace hue
