/*!
 * \copyright
 * Copyright (C) 2015 - 2017.
 * Released under the GNU General Public License.
 */

#include "listmoodgroupwidget.h"

ListMoodGroupWidget::ListMoodGroupWidget(const QString& name,
                                           std::list<std::pair<QString, std::list<SLightDevice> > > moods,
                                           const std::vector<std::vector<QColor> >& colors,
                                           QString key,
                                           int listHeight,
                                           bool hideEdit)
{
    setup(name, key, listHeight, hideEdit);

    mTopLayout = new QHBoxLayout();
    mTopLayout->addWidget(mName);
    mTopLayout->addWidget(mEditButton);
    mTopLayout->addWidget(mHiddenStateIcon);

    mTopLayout->setStretch(0, 14);
    mTopLayout->setStretch(1, 2);
    mTopLayout->setStretch(2, 2);

    mGridLayout = new QGridLayout(this);
    mGridLayout->setContentsMargins(0,0, this->size().width() / 40,0);
    mGridLayout->setVerticalSpacing(0);
    mGridLayout->setHorizontalSpacing(0);
    mGridLayout->addLayout(mTopLayout, 0, 0, 1, 2);

    mMoods = moods;

    mIndex = 0;
    for (auto&& mood : moods) {
        ListMoodWidget *widget = new ListMoodWidget(mood.first, mood.second,
                                                      colors,
                                                      listHeight);

        connect(widget, SIGNAL(clicked(QString)), this, SLOT(handleClicked(QString)));
        connect(widget, SIGNAL(editClicked(QString)), this, SLOT(clickedEdit(QString)));

        int column = mIndex / 2 + 1;
        int row = mIndex % 2;
        //qDebug() << "CR" << column << row;
        mGridLayout->addWidget(widget, column, row);
        widget->setVisible(mShowButtons);
        mIndex++;
        mWidgets.push_back(widget);
    }
}

void ListMoodGroupWidget::setShowButtons(bool show) {
    mShowButtons = show;
    for (auto&& device : mWidgets) {
        if (mShowButtons) {
            device->setVisible(true);
        } else {
            device->setVisible(false);
        }
    }

    if (mShowButtons) {
        mHiddenStateIcon->setPixmap(mOpenedPixmap);
    } else {
        mHiddenStateIcon->setPixmap(mClosedPixmap);
    }
    emit buttonsShown(mKey, mShowButtons);
}

void ListMoodGroupWidget::setCheckedMoods(std::list<QString> checkedMoods) {
    for (auto&& widget : mWidgets) {
        bool checkedNameFound = false;
        for (auto&& checkedName : checkedMoods) {
            if (widget->moodName().compare(checkedName) == 0) {
                checkedNameFound = true;
                widget->setChecked(true);
            }
        }
        if (!checkedNameFound) widget->setChecked(false);
    }
}

QSize ListMoodGroupWidget::preferredSize() {
    int height = mName->height();
    if (mShowButtons && mWidgets.size() > 0) {
        int widgetHeight = std::max(mName->height(), mListHeight / 6);
        height = (mWidgets.size() / 2 * widgetHeight) + (mWidgets.size() % 2 * widgetHeight) + height;
    }
    return QSize(this->width(), height);
}

void ListMoodGroupWidget::enterEvent(QEvent *) {
    if (!mHideEdit) {
        mEditButton->setHidden(false);
    }
}

void ListMoodGroupWidget::leaveEvent(QEvent *) {
    if (!mHideEdit) {
        mEditButton->setHidden(true);
    }
}


void ListMoodGroupWidget::mouseReleaseEvent(QMouseEvent *) {
    setShowButtons(!mShowButtons);
}

