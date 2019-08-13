/*!
 * \copyright
 * Copyright (C) 2015 - 2019.
 * Released under the GNU General Public License.
 */

#include "editablefieldwidget.h"

#include <QMessageBox>

#include "utils/reachability.h"

EditableFieldWidget::EditableFieldWidget(const QString& text,
                                         QWidget* parent,
                                         int maxFieldSize,
                                         const QString& maxFieldError)
    : QWidget(parent) {
    mStoredText = text;

    mText = new QLabel(text, this);
    mText->setAttribute(Qt::WA_TransparentForMouseEvents, true);
    mText->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

    mEditableField = new QLineEdit(text, this);
    mEditableField->setFont(mText->font());
    mEditableField->setVisible(false);
    mEditableField->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    mEditableField->setStyleSheet("border-style: outset; border-width: 1px; border-color: white; "
                                  "background-color:rgb(30,30,30);");
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
    mLayout->setContentsMargins(0, 0, 0, 0);
    mLayout->setSpacing(0);

    mMaxFieldSize = maxFieldSize;
    mMaxFieldError = maxFieldError;
    mIsEditing = false;
    mEnableEditing = true;
    mRequireIP = false;
}

void EditableFieldWidget::setFontPointSize(int pt) {
    if (pt <= 0) {
        pt = 1;
    }
    QString stylesheet = "font-size:" + QString::number(pt) + "pt;";
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
    mStoredText = std::move(newText);
}

void EditableFieldWidget::rightButtonClicked(bool) {
    if (mIsEditing) {
        if (mRequireIP) {
            // check for IP address
            if (!cor::checkIfValidIP(mStoredText)) {
                QMessageBox reply;
                reply.setText(mStoredText + "is not a valid IP address.");
                reply.exec();
                return;
            }
        }
        if (mStoredText != mText->text()) {
            if (mMaxFieldSize != -1 && mStoredText.size() > mMaxFieldSize) {
                QMessageBox reply;
                reply.setText(mMaxFieldError);
                reply.exec();
                return;
            }
            // passed checks
            setText(mStoredText);
            emit updatedField(mStoredText);
            setInEditMode(false);

        } else {
            setInEditMode(false);
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

void EditableFieldWidget::enableEditing(bool enableEditing) {
    if (enableEditing != mEnableEditing) {
        mEnableEditing = enableEditing;
        if (mEnableEditing) {
            mRightButton->setVisible(true);
        } else {
            setInEditMode(false);
            mLeftButton->setVisible(false);
            mRightButton->setVisible(false);
        }
    }
}

void EditableFieldWidget::requireIPAddress(bool requireIP) {
    mRequireIP = requireIP;
}
