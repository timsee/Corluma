/*!
 * \copyright
 * Copyright (C) 2015 - 2019.
 * Released under the GNU General Public License.
 */

#include "cor/button.h"

#include <QDebug>
#include <QGraphicsOpacityEffect>

namespace cor {

Button::Button(QWidget* parent, const QJsonObject& routineObject)
    : QPushButton(parent), mRoutineObject(routineObject) {
    mShouldResizeIcon = true;

    this->setCheckable(true);
    connect(this, SIGNAL(clicked(bool)), this, SLOT(handleButton()));

    mIconData = IconData(4, 4);

    mIconData.setRoutine(routineObject);
    resizeIcon();
}

void Button::resizeEvent(QResizeEvent*) {
    if (mShouldResizeIcon) {
        resizeIcon();
    }
}

void Button::resizeIcon() {
    int size = std::min(this->size().height(), this->size().width());
    mIconSize = QSize(int(size * 0.6f), int(size * 0.6f));
    QPixmap pixmap = mIconData.renderAsQPixmap();
    pixmap = pixmap.scaled(
        mIconSize.width(), mIconSize.height(), Qt::KeepAspectRatio, Qt::FastTransformation);
    this->setIcon(QIcon(pixmap));
    this->setIconSize(mIconSize);
}


void Button::updateRoutine(const QJsonObject& routineObject) {
    mIconData.setRoutine(routineObject);
    QPixmap pixmap = mIconData.renderAsQPixmap();
    pixmap = pixmap.scaled(
        mIconSize.width(), mIconSize.height(), Qt::KeepAspectRatio, Qt::FastTransformation);
    this->setIcon(QIcon(pixmap));
}

void Button::handleButton() {
    emit buttonClicked(mRoutineObject);
}

} // namespace cor
