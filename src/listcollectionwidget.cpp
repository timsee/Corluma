/*!
 * \copyright
 * Copyright (C) 2015 - 2018.
 * Released under the GNU General Public License.
 */

#include <QPainter>
#include <QStyleOption>

#include "listcollectionwidget.h"
#include "listcollectionsubwidget.h"
#include <algorithm>

ListCollectionWidget::ListCollectionWidget(QWidget *parent) : QWidget(parent) {}

void ListCollectionWidget::setup(const QString& name,
                                 const QString& key,
                                 EListType type,
                                 bool hideEdit) {
    mHideEdit = hideEdit;
    mShowButtons = false;

    mType = type;

    mIconRatio = 0.5f;
    mRowCount = 0;

    // setup main label
    mName = new QLabel(this);
    mName->setWordWrap(true);
    mName->setText(name);
    mName->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    mName->setStyleSheet("margin-left: 5px; font: bold;");
    if (mType == EListType::eGrid) {
        mMinimumHeight = mName->height();
    } else if (mType == EListType::eLinear || mType == EListType::eLinear2X) {
        mMinimumHeight = mName->height();
    }

    mEditButton = new QPushButton(this);
    mEditButton->setStyleSheet("border: none;");
    mEditButton->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Minimum);
    connect(mEditButton, SIGNAL(clicked(bool)), this, SLOT(editButtonClicked(bool)));
    mEditIcon = QPixmap(":/images/editIcon.png");
    mEditButton->setIcon(QIcon(mEditIcon));
    mEditButton->setFixedSize(mMinimumHeight * mIconRatio, mMinimumHeight * mIconRatio);
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
    mHiddenStateIcon->setFixedSize(mMinimumHeight, mMinimumHeight);
    mHiddenStateIcon->setAlignment(Qt::AlignCenter);

    if (mType == EListType::eGrid) {
        mWidgetSize = QSize(this->width() / 2, mMinimumHeight);
    } else if (mType == EListType::eLinear || mType == EListType::eLinear2X) {
        mWidgetSize = QSize(this->width(), mMinimumHeight);
    }

    mName->setFixedHeight(mMinimumHeight);

    mWidget = new QWidget(this);
    mWidget->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Minimum);

    mKey = key;

    mLayout = new QVBoxLayout(this);
    mLayout->setContentsMargins(0, 0, this->size().width() / 40, 0);
    mLayout->setSpacing(0);

    setLayout(mLayout);
}

void ListCollectionWidget::setListHeight(int newHeight) {
    if (mType == EListType::eGrid) {
        mMinimumHeight = newHeight / 8;
    } else if (mType == EListType::eLinear || mType == EListType::eLinear2X) {
        mMinimumHeight = newHeight / 8;
    }

    mClosedPixmap = QPixmap(":/images/closedArrow.png");
    mClosedPixmap = mClosedPixmap.scaled(mMinimumHeight, mMinimumHeight,
                                         Qt::KeepAspectRatio,
                                         Qt::SmoothTransformation);

    mOpenedPixmap = QPixmap(":/images/openedArrow.png");
    mOpenedPixmap = mOpenedPixmap.scaled(mMinimumHeight, mMinimumHeight,
                                         Qt::KeepAspectRatio,
                                         Qt::SmoothTransformation);
    mHiddenStateIcon->setFixedSize(mMinimumHeight, mMinimumHeight);
    mName->setFixedHeight(mMinimumHeight);
    resizeRightHandIcon(mEditIcon, mEditButton);
    resize();
}

void ListCollectionWidget::resizeRightHandIcon(QPixmap pixmap, QPushButton *button) {
    button->setIconSize(QSize(mMinimumHeight * mIconRatio, mMinimumHeight * mIconRatio));
    pixmap = pixmap.scaled(mMinimumHeight * mIconRatio, mMinimumHeight * mIconRatio,
                                         Qt::KeepAspectRatio,
                                         Qt::SmoothTransformation);
    button->setIcon(QIcon(pixmap));
    button->setFixedSize(mMinimumHeight, mMinimumHeight);
}



void ListCollectionWidget::insertWidget(ListCollectionSubWidget* widget) {
    // insert into sorted set
    mWidgets.push_back(widget);
    widget->setParent(mWidget);
    std::sort(mWidgets.begin(), mWidgets.end(), subWidgetCompare());

    moveWidgets();
}

void ListCollectionWidget::removeWidget(ListCollectionSubWidget* widget) {
    auto findResult = std::find(mWidgets.begin(), mWidgets.end(), widget);
    if (findResult == mWidgets.end()) {
        qDebug() << "WARNING: Could not find widget in set" << __func__;
        return;
    }
    (*findResult)->setVisible(false);
    // store index of inserted element
    mWidgets.erase(findResult);
    mWidgets.shrink_to_fit();
    moveWidgets();
}

QPoint ListCollectionWidget::widgetPosition(QWidget *widget) {
    auto findResult = std::find(mWidgets.begin(), mWidgets.end(), widget);
    if (findResult == mWidgets.end()) {
        qDebug() << "WARNING: Could not find widget in set" << __func__;
        return QPoint(-1, -1);
    }
    // store index of inserted element
    int index = std::distance(mWidgets.begin(), findResult);
    if (mType == EListType::eGrid) {
        int x = index % 2;     // 2 rows per column
        int y = index / 2;     // new column every other index
        return QPoint(x, y);
    } else if (mType == EListType::eLinear || mType == EListType::eLinear2X) {
        int x = 0;     // 1 row per column
        int y = index; // new column every index
        return QPoint(x, y);
    }
    return QPoint(-1, -1);
}


void ListCollectionWidget::moveWidgets() {
    uint32_t tempRowCount = 0;
    for (uint32_t i = 0; i < mWidgets.size(); ++i) {
        QPoint position = widgetPosition(mWidgets[i]);
        mWidgets[i]->setGeometry(position.x() * mWidgetSize.width(),
                                 position.y() * mWidgetSize.height(),
                                 mWidgetSize.width(),
                                 mWidgetSize.height());
        mWidgets[i]->setVisible(mShowButtons);
       // qDebug() << "this is the widget position of " << i << position << "and geometry"  << mWidgets[i]->geometry();
        if (((uint32_t)position.y() + 1) > tempRowCount) {
            tempRowCount = (position.y() + 1);
        }
    }
    if (mRowCount != tempRowCount) {
      //  mWidget->setMinimumHeight(tempRowCount * mWidgetSize.height());
        mRowCount = tempRowCount;
        emit rowCountChanged((int)mRowCount);
    }
}


void ListCollectionWidget::resize() {
   // qDebug() << "parent size" << this->parentWidget()->size();
    this->setFixedSize(preferredSize());
    // pad the width a bit in case its an odd sized width so it takes up the whole region.
    if (mType == EListType::eGrid) {
        mWidgetSize = QSize(this->width() / 2, mMinimumHeight);
    } else if (mType == EListType::eLinear) {
        mWidgetSize = QSize(this->width(), mMinimumHeight);
    } else if (mType == EListType::eLinear2X) {
        mWidgetSize = QSize(this->width(), mMinimumHeight * 2);
    }
   // qDebug() << "widget size" << mWidgetSize << "total size" << this->size();
    for (uint32_t i = 0; i < mWidgets.size(); ++i) {
        mWidgets[i]->setMaximumSize(mWidgetSize);
    }
    moveWidgets();
}
