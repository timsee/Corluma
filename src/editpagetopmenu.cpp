/*!
 * \copyright
 * Copyright (C) 2015 - 2020.
 * Released under the GNU General Public License.
 */

#include "editpagetopmenu.h"


EditPageTopMenu::EditPageTopMenu(QWidget* parent) : QWidget(parent) {
    mCloseButton = new QPushButton(this);
    mCloseButton->setText("X");
    mCloseButton->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

    mResetButton = new QPushButton(this);
    mResetButton->setText("Reset");
    mResetButton->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

    mDeleteButton = new QPushButton(this);
    mDeleteButton->setText("Delete");
    mDeleteButton->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

    mSaveButton = new QPushButton(this);
    mSaveButton->setText("Save");
    mSaveButton->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

    mNameEdit = new QLineEdit(this);
    mNameEdit->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

    mHelpLabel = new QLabel(this);

    mLayout = new QGridLayout(this);
    mLayout->setContentsMargins(4, 4, 4, 4);
    mLayout->setSpacing(2);

    mLayout->addWidget(mHelpLabel, 0, 0, 1, 9);
    mLayout->addWidget(mCloseButton, 0, 10, 1, 2);

    mLayout->addWidget(mNameEdit, 1, 0, 1, 8);
    mLayout->addWidget(mSaveButton, 1, 10, 1, 2);

    mLayout->addWidget(mResetButton, 2, 8, 1, 2);
    mLayout->addWidget(mDeleteButton, 2, 10, 1, 2);
}
