/*!
 * \copyright
 * Copyright (C) 2015 - 2018.
 * Released under the GNU General Public License.
 */

#include <QMessageBox>
#include <QDebug>

#include "searchwidget.h"

SearchWidget::SearchWidget(QString defaultLineEditValue, QWidget *parent, int maxSearchingCount, const QString errorMaxSearchString) : QWidget(parent) {
    //----------
    // Top layout
    //----------

    mLineEdit = new QLineEdit(this);
    mLineEdit->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    mLineEdit->setText(defaultLineEditValue);
    mLineEdit->setAlignment(Qt::AlignBottom);
    mLineEdit->setStyleSheet("font-size:20pt;");


    mPlusButton = new QPushButton(this);
    connect(mPlusButton, SIGNAL(clicked(bool)), this, SLOT(plusButtonClicked()));
    mPlusButton->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    mPlusButton->setText("+");
    mPlusButton->setStyleSheet("font-size:30pt;");


    mMinusButton = new QPushButton(this);
    connect(mMinusButton, SIGNAL(clicked(bool)), this, SLOT(minusButtonClicked()));
    mMinusButton->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    mMinusButton->setText("-");
    mMinusButton->setStyleSheet("font-size:30pt;");


    mInputLayout = new QHBoxLayout;
    mInputLayout->addWidget(mLineEdit, 10);
    mInputLayout->addWidget(mPlusButton, 2);
    mInputLayout->addWidget(mMinusButton, 2);

    //----------
    // UI assets
    //----------

    mConnectedLabel = new QLabel(this);
    mConnectedLabel->setText("Connected:");
    mConnectedLabel->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    mConnectedLabel->setAlignment(Qt::AlignLeft | Qt::AlignBottom);

    mConnectedListWidget = new QListWidget(this);
    mConnectedListWidget->setStyleSheet("color: silver;");
    mConnectedListWidget->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    connect(mConnectedListWidget, SIGNAL(itemClicked(QListWidgetItem*)), this, SLOT(connectedListClicked(QListWidgetItem*)));

    mDiscoveringLabel = new QLabel(this);
    mDiscoveringLabel->setText("Searching:");
    mDiscoveringLabel->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    mDiscoveringLabel->setAlignment(Qt::AlignLeft | Qt::AlignBottom);

    mDiscoveringListWidget = new QListWidget(this);
    mDiscoveringListWidget->setStyleSheet("color: silver;");
    mDiscoveringListWidget->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    connect(mDiscoveringListWidget, SIGNAL(itemClicked(QListWidgetItem*)), this, SLOT(discoveringListClicked(QListWidgetItem*)));

    //----------
    // Main Layout
    //----------

    mLayout = new QVBoxLayout;
    mLayout->addLayout(mInputLayout, 4);
    mLayout->addWidget(mDiscoveringLabel, 2);
    mLayout->addWidget(mDiscoveringListWidget, 10);
    mLayout->addWidget(mConnectedLabel, 2);
    mLayout->addWidget(mConnectedListWidget, 10);
    setLayout(mLayout);

    mForceUpperCase = false;
    mCheckSize = false;
    mMinSizeCheck = -1;
    mMaxSizeCheck = -1;
    mMaxSearchingCount = maxSearchingCount;
    mMaxSearchError = errorMaxSearchString;
    mSizeCheckError = "";
}


// ----------------------------
// Plus/Minus/Line Edit
// ----------------------------


void SearchWidget::plusButtonClicked() {
    //-----------
    // Checks
    //-----------
    // check count
    if (mMaxSearchingCount != -1
            && (mDiscoveringListWidget->count() >= mMaxSearchingCount)) {
        qDebug() << mDiscoveringListWidget->count()  << " is greater than" << mMaxSearchingCount;
        // give error
        QMessageBox reply;
        reply.setText(mMaxSearchError);
        reply.exec();
        return;
    }
    // check size
    if (mCheckSize) {
        int textSize = mLineEdit->text().size();
        if (!(textSize >= mMinSizeCheck
                && textSize <= mMaxSizeCheck)) {
            // give error
            QMessageBox reply;
            reply.setText(mSizeCheckError);
            reply.exec();
            return;
        }
    }
    //-----------
    // Preprocess, if checks passed
    //-----------
    // adjust string if needed
    if (mForceUpperCase) {
        mLineEdit->setText(mLineEdit->text().toUpper());
    }
    //-----------
    // Update
    //-----------
    emit plusClicked();
    // add to searching widget
    addToSearchList(mLineEdit->text());
}

void SearchWidget::minusButtonClicked() {
    emit minusClicked();
    removeKey(mLineEdit->text());
}


// ----------------------------
// Discovery Lists
// ----------------------------


void SearchWidget::connectedListClicked(QListWidgetItem *item) {
    mLineEdit->setText(item->text());
    for (int i = 0; i < mConnectedListWidget->count(); ++i) {
       mConnectedListWidget->item(i)->setSelected(false);
    }
    for (int i = 0; i < mDiscoveringListWidget->count(); ++i) {
        mDiscoveringListWidget->item(i)->setSelected(false);
    }
    item->setSelected(true);
}


void SearchWidget::discoveringListClicked(QListWidgetItem *item) {
    mLineEdit->setText(item->text());
    for (int i = 0; i < mConnectedListWidget->count(); ++i) {
       mConnectedListWidget->item(i)->setSelected(false);
    }
    for (int i = 0; i < mDiscoveringListWidget->count(); ++i) {
        mDiscoveringListWidget->item(i)->setSelected(false);
    }
    item->setSelected(true);
}

bool SearchWidget::addToSearchList(QString name) {
    // check if item is already in the table, if not, add it
    bool found = false;
    for (int i = 0; i < mDiscoveringListWidget->count(); ++i) {
        QListWidgetItem *item = mDiscoveringListWidget->item(i);
        if (item->text().compare(name) == 0) {
            found = true;
        }
    }

    if (!found) {
        mDiscoveringListWidget->addItem(name);
        return true;
    }
    return false;
}

bool SearchWidget::addToConnectedList(QString name) {
    // look for it in discovering list
    for (int i = 0; i < mDiscoveringListWidget->count(); ++i) {
        QListWidgetItem *item = mDiscoveringListWidget->item(i);
        if (item->text().compare(name) == 0) {
            mDiscoveringListWidget->takeItem(i);
        }
    }

    bool foundInConnectedList = false;
    for (int i = 0; i < mConnectedListWidget->count(); ++i) {
        QListWidgetItem *item = mConnectedListWidget->item(i);
        if (item->text().compare(name) == 0) {
            foundInConnectedList = true;
        }
    }
    if (!foundInConnectedList) {
        mConnectedListWidget->addItem(name);
        return true;
    }

    return false;
}


void SearchWidget::removeKey(QString key) {
    for (int i = 0; i < mDiscoveringListWidget->count(); ++i) {
        QListWidgetItem *item = mDiscoveringListWidget->item(i);
        if (item->text().compare(key) == 0) {
            mDiscoveringListWidget->takeItem(i);
        }
    }

    for (int i = 0; i < mConnectedListWidget->count(); ++i) {
        QListWidgetItem *item = mConnectedListWidget->item(i);
        if (item->text().compare(key) == 0) {
            mConnectedListWidget->takeItem(i);
        }
    }
}

std::list<QString> SearchWidget::searchingFor() {
    std::list<QString> searchingForList;
    for (int i = 0; i < mDiscoveringListWidget->count(); ++i) {
        QListWidgetItem *item = mDiscoveringListWidget->item(i);
        searchingForList.push_back(item->text());
    }
    return searchingForList;
}

// ----------------------------
// Size Checks
// ----------------------------

void SearchWidget::enableSizeChecks(int min, int max, QString error) {
    mMinSizeCheck = min;
    mMaxSizeCheck = max;
    mSizeCheckError = error;
    mCheckSize = true;
}

void SearchWidget::disableSizeChecks() {
    mCheckSize = false;
}
