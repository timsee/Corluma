/*!
 * \copyright
 * Copyright (C) 2015 - 2019.
 * Released under the GNU General Public License.
 */

#include "dropdowntopwidget.h"
#include "cor/utils.h"
#include <QDebug>

DropdownTopWidget::DropdownTopWidget(const QString& key, bool hideEdit, QWidget *parent) : QWidget(parent), mKey(key)
{
    mShowButtons = false;
    mHideEdit = hideEdit;

    mName = new QLabel(this);
    mName->setWordWrap(true);
    mName->setText(key);
    mName->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    mName->setStyleSheet("margin-left: 5px; font: bold;");

    mMinimumHeight = std::max(mName->height() * 2, cor::applicationSize().height() / 10);
    this->setFixedHeight(mMinimumHeight);
    mIconRatio = 0.5f;

    mEditButton = new QPushButton(this);
    mEditButton->setStyleSheet("border: none;");
    mEditButton->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    connect(mEditButton, SIGNAL(clicked(bool)), this, SLOT(editButtonClicked(bool)));
    mEditIcon = QPixmap(":/images/editIcon.png");
    mEditButton->setIcon(QIcon(mEditIcon));
    mEditButton->setFixedSize(int(mMinimumHeight * mIconRatio),
                              int(mMinimumHeight * mIconRatio));
    mEditButton->setHidden(true);

    mClosedPixmap = QPixmap(":/images/closedArrow.png");
    mClosedPixmap = mClosedPixmap.scaled(mMinimumHeight, mMinimumHeight,
                                         Qt::KeepAspectRatio,
                                         Qt::SmoothTransformation);


    mOpenedPixmap = QPixmap(":/images/openedArrow.png");
    mOpenedPixmap = mOpenedPixmap.scaled(mMinimumHeight, mMinimumHeight,
                                         Qt::KeepAspectRatio,
                                         Qt::SmoothTransformation);

    mHiddenStateIcon = new QLabel(this);
    mHiddenStateIcon->setPixmap(mClosedPixmap);
    mHiddenStateIcon->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    mHiddenStateIcon->setAlignment(Qt::AlignCenter);

    mName->setFixedHeight(mMinimumHeight);

    mLayout = new QHBoxLayout;
    mLayout->addWidget(mName);
    mLayout->addWidget(mEditButton);
    mLayout->addWidget(mHiddenStateIcon);
    setLayout(mLayout);

    mLayout->setStretch(0, 12);
    mLayout->setStretch(2, 2);
    mLayout->setStretch(3, 2);
}


void DropdownTopWidget::resizeRightHandIcon(QPixmap pixmap, QPushButton *button) {
    button->setIconSize(iconSize());
    pixmap = pixmap.scaled(iconSize().width(),
                           iconSize().height(),
                           Qt::KeepAspectRatio,
                           Qt::SmoothTransformation);
    button->setIcon(QIcon(pixmap));
    button->setFixedSize(iconSize());
}

void DropdownTopWidget::showButtons(bool showButtons) {
    mShowButtons = showButtons;
    if (mShowButtons) {
        mHiddenStateIcon->setPixmap(mOpenedPixmap);
    } else {
        mHiddenStateIcon->setPixmap(mClosedPixmap);
    }
}

