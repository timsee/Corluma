/*!
 * \copyright
 * Copyright (C) 2015 - 2017.
 * Released under the GNU General Public License.
 */

#include "corlumacheckbox.h"

CorlumaCheckBox::CorlumaCheckBox(QString title, QWidget *parent) : QWidget(parent) {
    mIsChecked = false;
    mSpacer = 5;

    const QString transparentStyleSheet = "background-color: rgba(0,0,0,0)";

    mTitle = new QLabel(title, this);
    mTitle->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    mTitle->setStyleSheet(transparentStyleSheet);
    mTitle->setAlignment(Qt::AlignBottom);
    QRect r = mTitle->fontMetrics().boundingRect(mTitle->text());
    mTitle->setFixedWidth(r.width());
    mTitle->setFixedHeight(r.height());

    mCheckBox = new QPushButton(this);
    mCheckBox->setCheckable(true);
    connect(mCheckBox, SIGNAL(clicked(bool)), this, SLOT(buttonPressed(bool)));
    mCheckBox->setStyleSheet("QPushButton:checked{ background-color:rgb(61, 142, 201); }");
    mCheckBox->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);

    this->setMinimumHeight(mTitle->height() * 1.5f);
}

void CorlumaCheckBox::setChecked(bool shouldCheck) {
    mCheckBox->setChecked(shouldCheck);
    mIsChecked = shouldCheck;
}

void CorlumaCheckBox::buttonPressed(bool pressed) {
    Q_UNUSED(pressed);

    mIsChecked = !mIsChecked;
    emit boxChecked(mIsChecked);
}

void CorlumaCheckBox::resizeEvent(QResizeEvent *event) {
    Q_UNUSED(event);

    mTitle->setGeometry(mSpacer,
                        mSpacer,
                        mTitle->width(),
                        mTitle->height());

    mCheckBox->setGeometry(mTitle->width() + 2 * mSpacer,
                           mTitle->geometry().y(),
                           mTitle->height() + mSpacer,
                           mTitle->height() + mSpacer);

    adjustSize();
}

