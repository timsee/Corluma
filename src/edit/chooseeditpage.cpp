/*!
 * \copyright
 * Copyright (C) 2015 - 2020.
 * Released under the GNU General Public License.
 */
#include "chooseeditpage.h"
#include <QStyleOption>
#include <QtCore>
#include <QtGui>
#include "utils/qt.h"

ChooseEditPage::ChooseEditPage(QWidget* parent)
    : QWidget(parent),
      mCloseButton{new QPushButton(this)},
      mAddButton{new QPushButton("Add", this)},
      mEditButton{new QPushButton("Edit", this)},
      mDeleteButton{new QPushButton("Delete", this)} {
    connect(mCloseButton, SIGNAL(clicked(bool)), this, SLOT(closePressed(bool)));

    connect(mAddButton, SIGNAL(clicked(bool)), this, SLOT(addPressed(bool)));
    connect(mEditButton, SIGNAL(clicked(bool)), this, SLOT(editPressed(bool)));
    connect(mDeleteButton, SIGNAL(clicked(bool)), this, SLOT(deletePressed(bool)));
}

void ChooseEditPage::pushIn(const QPoint& startPoint, const QPoint& endPoint) {
    setVisible(true);
    raise();
    isOpen(true);
    moveWidget(this, startPoint, endPoint);
}

void ChooseEditPage::pushOut(const QPoint& endPoint) {
    moveWidget(this, pos(), endPoint);
    isOpen(false);
}


void ChooseEditPage::resizeCloseButton() {
    QPixmap pixmap(":images/closeX.png");
    int closeSize = int(mTopHeight * 0.8);
    int finalSize = int(mTopHeight * 0.5);
    int spacer = (mTopHeight - finalSize) / 4;
    mCloseButton->setIconSize(QSize(finalSize, finalSize));
    mCloseButton->setIcon(
        QIcon(pixmap.scaled(finalSize, finalSize, Qt::KeepAspectRatio, Qt::SmoothTransformation)));
    mCloseButton->setGeometry(spacer, spacer, closeSize, closeSize);
}

void ChooseEditPage::resizeEvent(QResizeEvent*) {
#ifdef MOBILE_BUILD
    mTopHeight = cor::applicationSize().height() * 0.075;
#else
    mTopHeight = int(cor::applicationSize().height() * 0.1);
#endif
    resizeCloseButton();
    int yPos = mTopHeight;
    int buttonHeight = this->height() / 4;
    mAddButton->setGeometry(0, yPos, this->width(), buttonHeight);
    yPos += mAddButton->height();

    mEditButton->setGeometry(0, yPos, this->width(), buttonHeight);
    yPos += mEditButton->height();

    mDeleteButton->setGeometry(0, yPos, this->width(), buttonHeight);
    yPos += mDeleteButton->height();
}

void ChooseEditPage::paintEvent(QPaintEvent*) {
    QStyleOption opt;
    opt.init(this);
    QPainter painter(this);

    painter.setRenderHint(QPainter::Antialiasing);
    painter.fillRect(rect(), QBrush(QColor(48, 47, 47)));
}

void ChooseEditPage::closePressed(bool) {
    emit pressedClose();
}

void ChooseEditPage::addPressed(bool) {
    emit modeSelected(EChosenEditMode::add);
}

void ChooseEditPage::editPressed(bool) {
    emit modeSelected(EChosenEditMode::edit);
}

void ChooseEditPage::deletePressed(bool) {
    emit modeSelected(EChosenEditMode::remove);
}
