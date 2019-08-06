/*!
 * \copyright
 * Copyright (C) 2015 - 2019.
 * Released under the GNU General Public License.
 */

#include "dropdowntopwidget.h"

#include <QDebug>
#include <QMouseEvent>

#include "utils/qt.h"

DropdownTopWidget::DropdownTopWidget(const QString& key,
                                     cor::EWidgetType type,
                                     bool hideEdit,
                                     QWidget* parent)
    : QWidget(parent),
      mKey(key) {
    mType = type;
    mShowButtons = false;
    mHideEdit = hideEdit;

    mName = new QLabel(this);
    mName->setWordWrap(true);
    mName->setText(key);
    mName->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    mName->setStyleSheet("font: bold;");
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
    mEditButton->setStyleSheet("border: none;");
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

    mLayout = new QHBoxLayout;
    mLayout->addWidget(mName);
    mLayout->addWidget(mEditButton);
    mLayout->addWidget(mHiddenStateIcon);
    setLayout(mLayout);

    mLayout->setContentsMargins(10, 0, 0, 0);
    mLayout->setSpacing(0);
    mLayout->setStretch(0, 12);
    mLayout->setStretch(2, 2);
    mLayout->setStretch(3, 2);

    this->setFixedHeight(mMinimumHeight);
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
