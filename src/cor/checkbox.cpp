/*!
 * \copyright
 * Copyright (C) 2015 - 2018.
 * Released under the GNU General Public License.
 */

#include <QDebug>

#include "checkbox.h"

namespace cor
{

CheckBox::CheckBox(QWidget *parent, QString title) : QWidget(parent) {
    mIsChecked = false;
    mSpacer = 5;

    const QString transparentStyleSheet = "background-color: rgba(0,0,0,0);";

    mTitle = new QLabel(title, this);
    mTitle->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    mTitle->setStyleSheet(transparentStyleSheet);
    mTitle->setAlignment(Qt::AlignBottom);
    QRect r = mTitle->fontMetrics().boundingRect(mTitle->text());
    mTitle->setFixedWidth(r.width());
    mTitle->setMinimumHeight(r.height() * 1.75f);

    mCheckBox = new QPushButton(this);
    mCheckBox->setCheckable(true);
    connect(mCheckBox, SIGNAL(clicked(bool)), this, SLOT(buttonPressed(bool)));
    mCheckBox->setStyleSheet("QPushButton:checked{ background-color:rgb(61, 142, 201); } QPushButton{ border:5px solid #AAAAAA; }");
    mCheckBox->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);

    this->setMinimumHeight(mTitle->height());
    mCheckBox->setMinimumHeight(mTitle->height());
    mCheckBox->setMinimumWidth(mTitle->height());
}

void CheckBox::setTitle(QString title) {
    mTitle->setText(title);
    QRect r = mTitle->fontMetrics().boundingRect(mTitle->text());
    mTitle->setFixedWidth(r.width());
    mTitle->setMinimumHeight(r.height() * 1.75f);

    this->setMinimumHeight(mTitle->height());
    mCheckBox->setMinimumHeight(mTitle->height());
    mCheckBox->setMinimumWidth(mTitle->height());
}

void CheckBox::downsizeTextWidthToFit(int maxWidth) {
    QLabel label(mTitle->text());
    int systemFontWidth  = label.fontMetrics().boundingRect(label.text()).width();
    int fontPtSize       = label.font().pointSize();
    int nonTitleSize     = mCheckBox->width() + mSpacer * 3;
    int computedSize     = systemFontWidth;
    maxWidth             = maxWidth - nonTitleSize;
    if (maxWidth > computedSize) {
        // just use the systems font instead of scaling up
        QFont font = mTitle->font();
        font.setPointSize(fontPtSize);
        mTitle->setFont(font);
        QRect r = mTitle->fontMetrics().boundingRect(mTitle->text());
        mTitle->setFixedWidth(r.width());
    } else {
        while((maxWidth < computedSize) && (fontPtSize > 2)) {
            fontPtSize--;
            QFont font = mTitle->font();
            font.setPointSize(fontPtSize);
            mTitle->setFont(font);
            QRect r = mTitle->fontMetrics().boundingRect(mTitle->text());
            mTitle->setFixedWidth(r.width());
            computedSize = mTitle->width();
        }
    }
    adjustSize();
}

void CheckBox::setChecked(bool shouldCheck) {
    mCheckBox->setChecked(shouldCheck);
    mIsChecked = shouldCheck;
}

void CheckBox::buttonPressed(bool pressed) {
    Q_UNUSED(pressed);

    mIsChecked = !mIsChecked;
    emit boxChecked(mIsChecked);
}

void CheckBox::resizeEvent(QResizeEvent *event) {
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

}
