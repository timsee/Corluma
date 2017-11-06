/*!
 * \copyright
 * Copyright (C) 2015 - 2017.
 * Released under the GNU General Public License.
 */

#include "editablefieldwidget.h"
#include "corlumautils.h"

EditableFieldWidget::EditableFieldWidget(const QString& text, QWidget *parent) : QWidget(parent) {
    mStoredText = text;

    mText = new QLabel(text, this);
    mText->setAttribute(Qt::WA_TransparentForMouseEvents, true);
    mText->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

    mEditableField = new QLineEdit(text, this);
    mEditableField->setFont(mText->font());
    mEditableField->setVisible(false);
    mEditableField->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    mEditableField->setStyleSheet("border-style: outset; border-width: 1px; border-color: white; background-color:rgb(30,30,30);");
    mEditableField->setAttribute(Qt::WA_TransparentForMouseEvents, false);
    connect(mEditableField, SIGNAL(textEdited(QString)), this, SLOT(lineEditChanged(QString)));

    mRightButton = new QPushButton(this);
    mRightButton->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    mRightButton->setAttribute(Qt::WA_TransparentForMouseEvents, false);
    mRightButton->setStyleSheet("border: none;");
    connect(mRightButton, SIGNAL(clicked(bool)), this, SLOT(rightButtonClicked(bool)));

    mLeftButton = new QPushButton(this);
    mLeftButton->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    mLeftButton->setAttribute(Qt::WA_TransparentForMouseEvents, false);
    mLeftButton->setStyleSheet("border: none;");
    mLeftButton->setVisible(false);
    connect(mLeftButton, SIGNAL(clicked(bool)), this, SLOT(leftButtonClicked(bool)));

    mEditIcon = QPixmap(":/images/editIcon.png");
    mCheckIcon = QPixmap(":/images/checkmark.png");

    mRightButton->setIcon(QIcon(mEditIcon));
    mLeftButton->setIcon(QIcon(QPixmap(":/images/closeX.png")));

    mLayout = new QHBoxLayout(this);
    mLayout->addWidget(mText, 16);
    mLayout->addWidget(mLeftButton, 2);
    mLayout->addWidget(mRightButton, 2);
    mLayout->setContentsMargins(0,0,0,0);
    mLayout->setSpacing(0);

    mIsEditing = false;
}

void EditableFieldWidget::setFontPointSize(int pt) {
    QString stylesheet = "font-size:" + QString::number(pt)+ "pt;";
    this->setStyleSheet(stylesheet);

    QFont font(this->font().toString(), pt);
    QFontMetrics fm(font);
    this->setMinimumHeight(fm.height());
}

void EditableFieldWidget::setText(const QString& text) {
    mText->setText(text);
    mEditableField->setText(text);
    mStoredText = text;
}


void EditableFieldWidget::lineEditChanged(QString newText) {
    mStoredText = newText;

}

void EditableFieldWidget::rightButtonClicked(bool) {
    if (mIsEditing) {
        setInEditMode(false);
        if (mStoredText.compare(mText->text()) != 0) {
            setText(mStoredText);
            emit updatedField(mStoredText);
        }
    } else {
        setInEditMode(true);
    }
}


void EditableFieldWidget::leftButtonClicked(bool) {
    setInEditMode(false);
}

void EditableFieldWidget::setInEditMode(bool inEditMode) {
    mIsEditing = inEditMode;
    if (inEditMode) {
        // turn on editing
        mLayout->replaceWidget(mText, mEditableField);
        mEditableField->setVisible(true);
        mText->setVisible(false);
        mLeftButton->setVisible(true);
        mRightButton->setIcon(mCheckIcon);
    } else {
        // turn off editing and signal it
        mLayout->replaceWidget(mEditableField, mText);
        mEditableField->setVisible(false);
        mText->setVisible(true);
        mLeftButton->setVisible(false);
        mRightButton->setIcon(mEditIcon);
    }
}
