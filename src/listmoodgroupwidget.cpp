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

    updateMoods(moods, colors);
}


void ListMoodGroupWidget::updateMoods(std::list<std::pair<QString, std::list<SLightDevice> > > moods,
                                      const std::vector<std::vector<QColor> >& colors) {
    for (auto&& mood : moods) {
        bool foundDevice = false;
        for (auto&& existingWidget : mWidgets) {
            if (mood.first.compare(existingWidget->key()) == 0) {
                foundDevice = true;
                //TODO update
            }
        }

        if (!foundDevice) {
            ListMoodWidget *widget = new ListMoodWidget(mood.first, mood.second,
                                                        colors);
            connect(widget, SIGNAL(clicked(QString)), this, SLOT(handleClicked(QString)));
            connect(widget, SIGNAL(editClicked(QString)), this, SLOT(clickedEdit(QString)));
            insertWidgetIntoGrid(widget);
        }
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
        this->setFixedHeight(preferredSize().height());
    } else {
        mHiddenStateIcon->setPixmap(mClosedPixmap);
        mName->setFixedHeight(mMinimumSize);
        this->setFixedHeight(mMinimumSize);
    }
    emit buttonsShown(mKey, mShowButtons);
}

void ListMoodGroupWidget::setCheckedMoods(std::list<QString> checkedMoods) {
    for (auto&& existingWidget : mWidgets) {
        ListMoodWidget *widget = qobject_cast<ListMoodWidget*>(existingWidget);
        Q_ASSERT(widget);


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
    int height = mMinimumSize;
    if (mShowButtons && mWidgets.size() > 0) {
        int widgetHeight = std::max(mName->height(), (int)(height * 1.2f));
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

void ListMoodGroupWidget::removeMood(QString mood) {
    // check if mood exists
    bool foundMood = false;
    ListMoodWidget *moodWidget;
    for (auto&& widget : mWidgets) {
        ListMoodWidget *existingWidget = qobject_cast<ListMoodWidget*>(widget);
        Q_ASSERT(existingWidget);

        if (mood.compare(existingWidget->key()) == 0) {
            foundMood = true;
            moodWidget = existingWidget;
        }
    }

    if (foundMood) {
        removeWidgetFromGrid(moodWidget);
    }
}
