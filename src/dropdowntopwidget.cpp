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
    mPaletteWidget->showInSingleLine(true);

    connect(this, SIGNAL(pressed()), this, SLOT(widgetPressed()));


    mName = new QLabel(this);
    mName->setWordWrap(true);
    mName->setText(name);
    mName->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    mName->setStyleSheet(cor::kTransparentAndBoldStylesheet);
    mName->setAlignment(Qt::AlignVCenter);


    mEditButton = new QPushButton(this);
    mEditButton->setStyleSheet(cor::kTransparentStylesheet);
    mEditButton->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    connect(mEditButton, SIGNAL(clicked(bool)), this, SLOT(editButtonClicked(bool)));
    mEditIcon = QPixmap(":/images/edit_icon.png");
    mEditButton->setIcon(QIcon(mEditIcon));
    mEditButton->setHidden(true);


    mArrowIcon = new QLabel(this);
    mArrowIcon->setPixmap(mClosedPixmap);
    mArrowIcon->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    mArrowIcon->setAlignment(Qt::AlignCenter);
    mArrowIcon->setStyleSheet(cor::kTransparentStylesheet);

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
    mButtonHeight = height() * 0.75;

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

    auto xPos = width();
    mArrowIcon->setGeometry(xPos - mButtonHeight, 0u, mButtonHeight, height());
    xPos -= mArrowIcon->width();

    if (!mHideEdit) {
        mEditButton->setGeometry(xPos - mButtonHeight, 0u, mButtonHeight, height());
        xPos -= mEditButton->width();
    }

    auto spaceWidth = (width() / 20);
    mPaletteWidget->setGeometry(0, 0, xPos, height());

    auto previewHeight = height() / 4;
    mName->setGeometry(spaceWidth, previewHeight, xPos - spaceWidth, height() - previewHeight);
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
