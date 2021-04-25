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
    : QWidget(parent) {
    setStyleSheet(cor::kTransparentStylesheet);

    mName = new QLabel("<b>" + group.name() + "</b>", this);
    auto font = mName->font();
    auto fontSize = int(font.pointSize() * 1.33);
    font.setPointSize(fontSize);
    mName->setFont(font);
    mName->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    mName->setAttribute(Qt::WA_TransparentForMouseEvents, true);

    mIndex = new QLabel("<b>Index:</b> " + QString::number(index), this);
    mIndex->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    mIndex->setAttribute(Qt::WA_TransparentForMouseEvents, true);

    mGroupDescription = new QLabel(generateDescription(group.lights().size(), false), this);
    mGroupDescription->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    mGroupDescription->setAttribute(Qt::WA_TransparentForMouseEvents, true);

    mLayout = new QVBoxLayout(this);
    mLayout->addWidget(mName, 1);
    mLayout->addWidget(mIndex, 1);
    mLayout->addWidget(mGroupDescription, 1);
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
    returnString += " lights.</i>";
    return returnString;
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
