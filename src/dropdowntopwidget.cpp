/*!
 * \copyright
 * Copyright (C) 2015 - 2020.
 * Released under the GNU General Public License.
 */

#include "dropdowntopwidget.h"

#include <QDebug>
#include <QGraphicsEffect>
#include <QGraphicsScene>
#include <QMouseEvent>
#include <QPainter>
#include <QStyleOption>

#include "utils/qt.h"

DropdownTopWidget::DropdownTopWidget(const QString& key,
                                     const QString& name,
                                     cor::EWidgetType type,
                                     bool hideEdit,
                                     QWidget* parent)
    : QWidget(parent),
      mPaletteWidget{new cor::PaletteWidget(this)},
      mKey{key},
      mShowStates{false} {
    mType = type;
    mShowButtons = false;
    mHideEdit = hideEdit;

    mPaletteWidget->skipOffLightStates(true);
    mPaletteWidget->shouldForceSquares(true);

    connect(this, SIGNAL(pressed()), this, SLOT(widgetPressed()));

    mName = new QLabel(this);
    mName->setWordWrap(true);
    mName->setText(name);
    mName->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    mName->setStyleSheet("font: bold; background-color: rgba(0,0,0,0);");
    mName->setAlignment(Qt::AlignVCenter);


    mEditButton = new QPushButton(this);
    mEditButton->setStyleSheet("border: none; background-color: rgba(0,0,0,0);");
    mEditButton->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    connect(mEditButton, SIGNAL(clicked(bool)), this, SLOT(editButtonClicked(bool)));
    mEditIcon = QPixmap(":/images/edit_icon.png");
    mEditButton->setIcon(QIcon(mEditIcon));
    mEditButton->setHidden(true);


    mArrowIcon = new QLabel(this);
    mArrowIcon->setPixmap(mClosedPixmap);
    mArrowIcon->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    mArrowIcon->setAlignment(Qt::AlignCenter);
    mArrowIcon->setStyleSheet("background-color: rgba(0,0,0,0);");

    resize();
    showButtons(false);
}

void DropdownTopWidget::mouseReleaseEvent(QMouseEvent* event) {
    if (cor::isMouseEventTouchUpInside(event, this, true)) {
        emit pressed();
    }
    event->ignore();
}

void DropdownTopWidget::resizeEvent(QResizeEvent*) {
    resize();
}

void DropdownTopWidget::resize() {
    auto originalIconSide = mButtonHeight;
    mButtonHeight = height();

    if (mButtonHeight != originalIconSide) {
        mClosedPixmap = QPixmap(":/images/closedArrow.png");
        mClosedPixmap = mClosedPixmap.scaled(mButtonHeight,
                                             mButtonHeight,
                                             Qt::KeepAspectRatio,
                                             Qt::SmoothTransformation);


        mOpenedPixmap = QPixmap(":/images/openedArrow.png");
        mOpenedPixmap = mOpenedPixmap.scaled(mButtonHeight,
                                             mButtonHeight,
                                             Qt::KeepAspectRatio,
                                             Qt::SmoothTransformation);
        showButtons(mShowButtons);
    }

    auto xPos = width() - mButtonHeight;
    mArrowIcon->setGeometry(xPos, 0u, mButtonHeight, mButtonHeight);
    xPos -= mArrowIcon->width();

    if (!mHideEdit) {
        mEditButton->setGeometry(xPos, 0u, mButtonHeight, mButtonHeight);
        xPos -= mEditButton->width();
    }

    if (mShowStates) {
        mPaletteWidget->setGeometry(xPos, 0u, mButtonHeight, mButtonHeight);
        xPos -= mPaletteWidget->width();
    }

    auto rightSpacer = width() * 0.03;
    auto nameWidth = xPos - rightSpacer;
    mName->setGeometry(rightSpacer, 0, nameWidth, mButtonHeight);
}

void DropdownTopWidget::showButtons(bool showButtons) {
    mShowButtons = showButtons;
    if (mShowButtons) {
        mArrowIcon->setPixmap(mOpenedPixmap);
    } else {
        mArrowIcon->setPixmap(mClosedPixmap);
    }
}


void DropdownTopWidget::showStates(bool showStates) {
    mShowStates = showStates;
    resize();
}
