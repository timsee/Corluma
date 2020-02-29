/*!
 * \copyright
 * Copyright (C) 2015 - 2020.
 * Released under the GNU General Public License.
 */

#include "editablefieldwidget.h"

#include <QMessageBox>

#include "utils/reachability.h"
#include "utils/qt.h"

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
    mEditableField->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    mEditableField->setStyleSheet("border-style: outset; border-width: 1px; border-color: white; "
                                  "background-color:rgb(30,30,30);");
    mEditableField->setAttribute(Qt::WA_TransparentForMouseEvents, false);
    connect(mEditableField, SIGNAL(textEdited(QString)), this, SLOT(lineEditChanged(QString)));

    mRightButton = new QPushButton(this);
    mRightButton->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    mRightButton->setAttribute(Qt::WA_TransparentForMouseEvents, false);
    mRightButton->setStyleSheet("border: none;");
    connect(mRightButton, SIGNAL(clicked(bool)), this, SLOT(rightButtonClicked(bool)));

    mLeftButton = new QPushButton(this);
    mLeftButton->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    mLeftButton->setAttribute(Qt::WA_TransparentForMouseEvents, false);
    mLeftButton->setStyleSheet("border: none;");
    mLeftButton->setVisible(false);
    connect(mLeftButton, SIGNAL(clicked(bool)), this, SLOT(leftButtonClicked(bool)));

    mEditIcon = QPixmap(":/images/editIcon.png");
    mCheckIcon = QPixmap(":/images/checkmark.png");

    mRightButton->setIcon(QIcon(mEditIcon));
    mLeftButton->setIcon(QIcon(QPixmap(":/images/closeX.png")));

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
    setStyleSheet(stylesheet);

    QFont font(this->font().toString(), pt);
    QFontMetrics fm(font);
    setMinimumHeight(fm.height());
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
        mEditableField->setVisible(true);
        mText->setVisible(false);
        mLeftButton->setVisible(true);
        mRightButton->setIcon(mCheckIcon);
    } else {
        // turn off editing and signal it
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

void EditableFieldWidget::resizeEvent(QResizeEvent*) {
    resize();
}

void EditableFieldWidget::resize() {
    auto h = height();
    auto buttonSize = h * 0.8;
    auto w = width();
    mRightButton->setGeometry(w - buttonSize, buttonSize * 0.1, buttonSize, buttonSize);
    mLeftButton->setGeometry(w - 2 * buttonSize, buttonSize * 0.1, buttonSize, buttonSize);
    if (mIsEditing) {
        cor::resizeIcon(mRightButton, ":/images/checkmark.png");
    } else {
        cor::resizeIcon(mRightButton, ":/images/editIcon.png");
    }
    cor::resizeIcon(mLeftButton, ":/images/closeX.png");

    mText->setGeometry(0, 0, w - 2 * buttonSize, h);
    mEditableField->setGeometry(mText->geometry());
}
