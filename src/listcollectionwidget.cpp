/*!
 * \copyright
 * Copyright (C) 2015 - 2017.
 * Released under the GNU General Public License.
 */

#include "listcollectionwidget.h"


void ListCollectionWidget::setup(const QString& name,
                                 const QString& key,
                                 int listHeight,
                                 bool hideEdit) {
    mHideEdit = hideEdit;
    mShowButtons = false;
    mListHeight = listHeight;

#ifdef MOBILE_BUILD
    mIconRatio = 1.0f;
#else
    mIconRatio = 0.9f;
#endif

    this->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

    QString backgroundStyleSheet = "background:rgba(0, 0, 0, 0%); font: bold;";
    this->setStyleSheet(backgroundStyleSheet);

    // setup main label
    mName = new QLabel(this);
    mName->setWordWrap(true);
    mName->setText(name);
    mName->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);

    mMinimumSize = std::max(mName->height(), mListHeight / 8);

    mEditButton = new QPushButton(this);
    mEditButton->setStyleSheet("border: none;");
    mEditButton->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    connect(mEditButton, SIGNAL(clicked(bool)), this, SLOT(editButtonClicked(bool)));
    mEditIcon = QPixmap(":/images/editIcon.png");
    mEditButton->setIcon(QIcon(mEditIcon));
    mEditButton->setFixedSize(mMinimumSize * mIconRatio, mMinimumSize * mIconRatio);
    mEditButton->setHidden(true);

    mClosedPixmap = QPixmap(":/images/closedArrow.png");
    mClosedPixmap = mClosedPixmap.scaled(mMinimumSize, mMinimumSize,
                                         Qt::KeepAspectRatio,
                                         Qt::SmoothTransformation);


    mOpenedPixmap = QPixmap(":/images/openedArrow.png");
    mOpenedPixmap = mOpenedPixmap.scaled(mMinimumSize, mMinimumSize,
                                         Qt::KeepAspectRatio,
                                         Qt::SmoothTransformation);

    mHiddenStateIcon = new QLabel(this);
    mHiddenStateIcon->setPixmap(mClosedPixmap);
    mHiddenStateIcon->setFixedSize(mMinimumSize, mMinimumSize);
    mHiddenStateIcon->setAlignment(Qt::AlignCenter);

    this->setMinimumHeight(mMinimumSize);
    mName->setMinimumHeight(mMinimumSize);
    mName->setMaximumHeight(mMinimumSize);

    mKey = key;

    mIndex = 0;
}

void ListCollectionWidget::setListHeight(int newHeight) {
    mListHeight = newHeight;
    mMinimumSize = std::max(mEditButton->height(), mListHeight / 8);

    mClosedPixmap = mClosedPixmap.scaled(mMinimumSize, mMinimumSize,
                                         Qt::KeepAspectRatio,
                                         Qt::SmoothTransformation);


    mOpenedPixmap = mOpenedPixmap.scaled(mMinimumSize, mMinimumSize,
                                         Qt::KeepAspectRatio,
                                         Qt::SmoothTransformation);
    mHiddenStateIcon->setFixedSize(mMinimumSize, mMinimumSize);

    resizeRightHandIcon(mEditIcon, mEditButton);

}

void ListCollectionWidget::resizeRightHandIcon(QPixmap pixmap, QPushButton *button) {
   // button->setIconSize(QSize(mMinimumSize * mIconRatio, mMinimumSize * mIconRatio));
    pixmap = pixmap.scaled(mMinimumSize * mIconRatio, mMinimumSize * mIconRatio,
                                         Qt::KeepAspectRatio,
                                         Qt::SmoothTransformation);
    button->setIcon(QIcon(pixmap));
    button->setFixedSize(mMinimumSize, mMinimumSize);
}
