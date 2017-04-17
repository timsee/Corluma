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

    mIconRatio = 0.5f;

    this->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

    QString backgroundStyleSheet = "border: none; background:rgba(0, 0, 0, 0%);";
#ifdef MOBILE_BUILD
    backgroundStyleSheet += "font: 14pt;";
#else
    backgroundStyleSheet += "font: bold;";
#endif
    this->setStyleSheet(backgroundStyleSheet);

    // setup main label
    mName = new QLabel(this);
    mName->setWordWrap(true);
    mName->setText(name);
    mName->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    mName->setStyleSheet("margin-left: 5px;");
    mMinimumSize = std::max(mName->height(), listHeight / 8);

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
}

void ListCollectionWidget::setListHeight(int newHeight) {
    mMinimumSize = newHeight / 8;
    mClosedPixmap = QPixmap(":/images/closedArrow.png");
    mClosedPixmap = mClosedPixmap.scaled(mMinimumSize, mMinimumSize,
                                         Qt::KeepAspectRatio,
                                         Qt::SmoothTransformation);

    mOpenedPixmap = QPixmap(":/images/openedArrow.png");
    mOpenedPixmap = mOpenedPixmap.scaled(mMinimumSize, mMinimumSize,
                                         Qt::KeepAspectRatio,
                                         Qt::SmoothTransformation);
    mHiddenStateIcon->setFixedSize(mMinimumSize, mMinimumSize);
    mName->setFixedHeight(mMinimumSize);
    resizeRightHandIcon(mEditIcon, mEditButton);
}

void ListCollectionWidget::resizeRightHandIcon(QPixmap pixmap, QPushButton *button) {
    button->setIconSize(QSize(mMinimumSize * mIconRatio, mMinimumSize * mIconRatio));
    pixmap = pixmap.scaled(mMinimumSize * mIconRatio, mMinimumSize * mIconRatio,
                                         Qt::KeepAspectRatio,
                                         Qt::SmoothTransformation);
    button->setIcon(QIcon(pixmap));
    button->setFixedSize(mMinimumSize, mMinimumSize);
}



void ListCollectionWidget::insertWidgetIntoGrid(ListCollectionSubWidget* widget) {
    // insert into sorted set
    auto result = mWidgets.insert(widget);
    if (!result.second) {
        qDebug() << "Warning: insertion failed in " << __func__;
    }
    // store index of inserted element
    int index = std::distance(mWidgets.begin(), result.first);
    // store max index
    int maxIndex =  mWidgets.size() - 1;
    // if inserting at end, easy case, just insert at end
    if (index == maxIndex) {
        int row = maxIndex / 2 + 1;
        int column = maxIndex % 2;
        mGridLayout->addWidget(widget, row, column);
        widget->setVisible(mShowButtons);
    } else {
        // if not inserting at end, push all elements after insertion back
        int tempIndex = maxIndex;
        while (tempIndex > index) {
            // get previous element
            int prev = tempIndex - 1;
            int row = prev / 2 + 1;
            int column = prev % 2;
            QWidget *tempWidget = mGridLayout->itemAtPosition(row, column)->widget();
            // remove from location
            mGridLayout->removeWidget(tempWidget);
            // add to new location instead
            row = tempIndex / 2 + 1;
            column = tempIndex % 2;
            mGridLayout->addWidget(tempWidget, row, column);
            tempIndex--;
        }
        // place final element in the now open spot.
        int row = tempIndex / 2 + 1;
        int column = tempIndex % 2;
        mGridLayout->addWidget(widget, row, column);
        widget->setVisible(mShowButtons);
    }
}

void ListCollectionWidget::removeWidgetFromGrid(ListCollectionSubWidget* widget) {
    auto findResult = std::find(mWidgets.begin(), mWidgets.end(), widget);
    if (findResult == mWidgets.end()) {
        qDebug() << "WARNING: Could not find widget in set" << __func__;
        return;
    }
    // store index of inserted element
    int index = std::distance(mWidgets.begin(), findResult);
    // store max index
    int maxIndex =  mWidgets.size() - 1;


    // if remove at end, easy case, just remove at end
    if (index == maxIndex) {
        int row = index / 2 + 1;
        int column = index % 2;
        QWidget *widgetToDelete = mGridLayout->itemAtPosition(row, column)->widget();
        // remove from internal data.
        mWidgets.erase(findResult);
        delete widgetToDelete;

    } else {
        // remove first one and delete it
        bool isFirst = true;

        // if not inserting at end, shift all elements one to the left
        int tempIndex = index;
        while (tempIndex < maxIndex) {
            // get next element
            int next = tempIndex + 1;
            int row = next / 2 + 1;
            int column = next % 2;
            QWidget *tempWidget = mGridLayout->itemAtPosition(row, column)->widget();
            // get current location instead
            row = tempIndex / 2 + 1;
            column = tempIndex % 2;
            // remove old item from GUI
            QLayoutItem *layoutItem = mGridLayout->itemAtPosition(row, column);
            if (isFirst) {
                isFirst = false;
                QWidget *widgetToDelete = layoutItem->widget();
                // remove from internal data.
                mWidgets.erase(findResult);
                delete widgetToDelete;
            } else {
                mGridLayout->removeItem(layoutItem);
            }

            mGridLayout->addWidget(tempWidget, row, column);
            tempIndex++;
        }
    }
}
