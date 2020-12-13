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
      mKey(key) {
    mType = type;
    mShowButtons = false;
    mHideEdit = hideEdit;
    mLayout = new QHBoxLayout;
    setLayout(mLayout);

    connect(this, SIGNAL(pressed()), this, SLOT(widgetPressed()));

    mName = new QLabel(this);
    mName->setWordWrap(true);
    mName->setText(name);
    mName->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    mName->setStyleSheet("font: bold; background-color: rgba(0,0,0,0);");
    mName->setAlignment(Qt::AlignVCenter);

    if (mType == cor::EWidgetType::condensed) {
        mMinimumHeight = cor::applicationSize().height() / 15;
        mIconRatio = 0.25f;
    } else {
        mMinimumHeight = cor::applicationSize().height() / 10;
        mIconRatio = 0.5f;
    }
    mName->setFixedHeight(mMinimumHeight);

    mEditButton = new QPushButton(this);
    mEditButton->setStyleSheet("border: none; background-color: rgba(0,0,0,0);");
    mEditButton->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    connect(mEditButton, SIGNAL(clicked(bool)), this, SLOT(editButtonClicked(bool)));
    mEditIcon = QPixmap(":/images/editIcon.png");
    mEditButton->setIcon(QIcon(mEditIcon));
    mEditButton->setFixedSize(int(mMinimumHeight * mIconRatio), int(mMinimumHeight * mIconRatio));
    mEditButton->setHidden(true);
    mEditButton->setFixedHeight(mMinimumHeight);

    mClosedPixmap = QPixmap(":/images/closedArrow.png");
    mClosedPixmap = mClosedPixmap.scaled(mMinimumHeight,
                                         mMinimumHeight,
                                         Qt::KeepAspectRatio,
                                         Qt::SmoothTransformation);


    mOpenedPixmap = QPixmap(":/images/openedArrow.png");
    mOpenedPixmap = mOpenedPixmap.scaled(mMinimumHeight,
                                         mMinimumHeight,
                                         Qt::KeepAspectRatio,
                                         Qt::SmoothTransformation);

    mHiddenStateIcon = new QLabel(this);
    mHiddenStateIcon->setPixmap(mClosedPixmap);
    mHiddenStateIcon->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    mHiddenStateIcon->setAlignment(Qt::AlignCenter);
    mHiddenStateIcon->setFixedHeight(mMinimumHeight);
    mHiddenStateIcon->setStyleSheet("background-color: rgba(0,0,0,0);");

    mLayout->addWidget(mName);
    mLayout->addWidget(mEditButton);
    mLayout->addWidget(mHiddenStateIcon);

    mLayout->setContentsMargins(10, 0, 0, 0);
    mLayout->setSpacing(0);
    mLayout->setStretch(0, 12);
    mLayout->setStretch(2, 2);
    mLayout->setStretch(3, 2);

    // TODO: figure out a more elegant solution for this, too much relies on fixed height
    setFixedHeight(mMinimumHeight);
}

void DropdownTopWidget::mouseReleaseEvent(QMouseEvent* event) {
    if (cor::isMouseEventTouchUpInside(event, this, true)) {
        emit pressed();
    }
    event->ignore();
}


void DropdownTopWidget::showButtons(bool showButtons) {
    mShowButtons = showButtons;
    if (mShowButtons) {
        mHiddenStateIcon->setPixmap(mOpenedPixmap);
    } else {
        mHiddenStateIcon->setPixmap(mClosedPixmap);
    }
}
