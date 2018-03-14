/*!
 * \copyright
 * Copyright (C) 2015 - 2018.
 * Released under the GNU General Public License.
 */

#include "listmoodgroupwidget.h"

ListMoodGroupWidget::ListMoodGroupWidget(const QString& name,
                                         std::list<cor::LightGroup> moods,
                                         const std::vector<std::vector<QColor> >& colors,
                                         QString key,
                                         bool hideEdit,
                                         QWidget *parent) {
    this->setParent(parent);
    this->setMaximumSize(parent->size());
    setup(name, key, EListType::eLinear2X, hideEdit);

    mTopLayout = new QHBoxLayout();
    mTopLayout->addWidget(mName);
    mTopLayout->addWidget(mHiddenStateIcon);

    mEditButton->setVisible(false);

    mTopLayout->setStretch(0, 16);
    mTopLayout->setStretch(1, 2);

    mMoods = moods;

    updateMoods(moods, colors);

    mLayout->addLayout(mTopLayout);
    mLayout->addWidget(mWidget);
}


void ListMoodGroupWidget::updateMoods(std::list<cor::LightGroup> moods,
                                      const std::vector<std::vector<QColor> >& colors,
                                      bool removeIfNotFound) {
    std::vector<bool> foundWidgets(mWidgets.size(), false);
    for (auto&& mood : moods) {
        bool foundMood = false;
        uint32_t x = 0;
        for (auto&& existingWidget : mWidgets) {
            if (mood.name.compare(existingWidget->key()) == 0) {
                foundMood = true;
                //TODO update
                foundWidgets[x] = true;
                ++x;
            }
        }

        if (!foundMood) {
            ListMoodWidget *widget = new ListMoodWidget(mood, colors);
            connect(widget, SIGNAL(clicked(QString)), this, SLOT(handleClicked(QString)));
            connect(widget, SIGNAL(editClicked(QString)), this, SLOT(clickedEdit(QString)));
            insertWidget(widget);
        }
    }

    //----------------
    // Remove widgets that are not found
    //----------------
    if (removeIfNotFound) {
        uint32_t x = 0;
        for (auto widgetFound : foundWidgets) {
            if (!widgetFound) {
                // remove this widget
                removeWidget(mWidgets[x]);
            }
            ++x;
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

    resizeInteralWidgets();
    emit buttonsShown(mKey, mShowButtons);
}

void ListMoodGroupWidget::resizeInteralWidgets() {
    if (mShowButtons) {
        mHiddenStateIcon->setPixmap(mOpenedPixmap);
        this->setFixedHeight(preferredSize().height());
    } else {
        mHiddenStateIcon->setPixmap(mClosedPixmap);
        mName->setFixedHeight(mMinimumHeight);
        this->setFixedHeight(mMinimumHeight);
    }
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
    int height = mMinimumHeight;
    if (mShowButtons && mWidgets.size() > 0) {
        int widgetHeight = std::max(mName->height(), mMinimumHeight * 2);
        height = mWidgets.size() * widgetHeight + mMinimumHeight;
    }
    return QSize(this->parentWidget()->width(), height);
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
        removeWidget(moodWidget);
    }
}
