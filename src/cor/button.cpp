/*!
 * \copyright
 * Copyright (C) 2015 - 2018.
 * Released under the GNU General Public License.
 */

#include "cor/button.h"

#include <QDebug>
#include <QGraphicsOpacityEffect>

namespace cor
{

Button::Button(QJsonObject routineObject,
               const std::vector<QColor>& group,
               QWidget *parent) : QPushButton(parent) {
    mRoutineObject = routineObject;

    mShouldResizeIcon = true;

    this->setCheckable(true);
    connect(this, SIGNAL(clicked(bool)), this, SLOT(handleButton()));

    mIconData = IconData(4, 4);

    mIconData.setRoutine(routineObject, group);
    resizeIcon();
}

void Button::resizeEvent(QResizeEvent *event) {
    Q_UNUSED(event);
    if (mShouldResizeIcon) {
        resizeIcon();
   }
}

void Button::resizeIcon() {
    int size = std::min(this->size().height(), this->size().width());
    mIconSize = QSize(size * 0.6f, size * 0.6f);
    QPixmap pixmap = mIconData.renderAsQPixmap();
    pixmap = pixmap.scaled(mIconSize.width(),
                           mIconSize.height(),
                           Qt::KeepAspectRatio,
                           Qt::FastTransformation);
    this->setIcon(QIcon(pixmap));
    this->setIconSize(mIconSize);
}


void Button::updateRoutine(const QJsonObject& routineObject, const std::vector<QColor>& colors) {
    mIconData.setRoutine(routineObject, colors);
    QPixmap pixmap = mIconData.renderAsQPixmap();
    pixmap = pixmap.scaled(mIconSize.width(),
                           mIconSize.height(),
                           Qt::KeepAspectRatio,
                           Qt::FastTransformation);
    this->setIcon(QIcon(pixmap));
    this->setIconSize(mIconSize);
}

void Button::handleButton() {
    emit buttonClicked(mRoutineObject);
}

}
