/*!
 * \copyright
 * Copyright (C) 2015 - 2019.
 * Released under the GNU General Public License.
 */

#include "listmoodgroupwidget.h"
#include "utils/qt.h"

ListMoodGroupWidget::ListMoodGroupWidget(const QString& name,
                                         std::list<cor::Mood> moods,
                                         QString key,
                                         bool hideEdit,
                                         QWidget *parent) : cor::ListItemWidget(key, parent), mListLayout(cor::EListType::grid) {

    mWidget = new QWidget(this);

    mLayout = new QVBoxLayout(this);
    mLayout->setContentsMargins(0, 0, this->size().width() / 40, 0);
    mLayout->setSpacing(0);

    setLayout(mLayout);

    mMoods = moods;
    mDropdownTopWidget = new DropdownTopWidget(name, cor::EWidgetType::full, hideEdit, this);
    mDropdownTopWidget->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);

    updateMoods(moods, false);

    mLayout->addWidget(mDropdownTopWidget);
    mLayout->addWidget(mWidget);
}


void ListMoodGroupWidget::updateMoods(const std::list<cor::Mood>& moods,
                                      bool removeIfNotFound) {
    std::vector<bool> foundWidgets(moods.size(), false);
    for (const auto& mood : moods) {
        bool foundMood = false;
        uint32_t x = 0;
        for (const auto& existingWidget : mListLayout.widgets()) {
            if (mood.name() == existingWidget->key()) {
                foundMood = true;
                foundWidgets[x] = true;
            }
            ++x;
        }

        if (!foundMood) {
            ListMoodPreviewWidget *widget = new ListMoodPreviewWidget(mood, mWidget);
            connect(widget, SIGNAL(moodSelected(std::uint64_t)), this, SLOT(selectMood(std::uint64_t)));
            mListLayout.insertWidget(widget);
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
                mListLayout.removeWidget(mListLayout.widgets()[x]);
            }
            ++x;
        }
    }
    resizeInteralWidgets();
}

void ListMoodGroupWidget::setShowButtons(bool show) {
    mDropdownTopWidget->showButtons(show);
    for (auto&& device : mListLayout.widgets()) {
        if (mDropdownTopWidget->showButtons()) {
            device->setVisible(true);
        } else {
            device->setVisible(false);
        }
    }

    resizeInteralWidgets();
    emit buttonsShown(mKey, mDropdownTopWidget->showButtons());
}

void ListMoodGroupWidget::resizeInteralWidgets() {
    if (mDropdownTopWidget->showButtons()) {
        this->setFixedHeight(mListLayout.overallSize().height() + mDropdownTopWidget->height());
    } else {
        this->setFixedHeight(mDropdownTopWidget->height());
    }
    mWidget->setFixedWidth(this->width());

    QPoint offset(0, 0);

    QSize size = mListLayout.widgetSize(QSize(this->width(), int(mDropdownTopWidget->height() * 2)));
    for (uint32_t i = 0; i < mListLayout.widgets().size(); ++i) {
        ListMoodPreviewWidget *widget = qobject_cast<ListMoodPreviewWidget*>(mListLayout.widgets()[i]);
        Q_ASSERT(widget);
        QPoint position = mListLayout.widgetPosition(mListLayout.widgets()[i]);
        mListLayout.widgets()[i]->setGeometry(offset.x() + position.x() * size.width(),
                                              offset.y() + position.y() * size.height(),
                                              size.width(),
                                              size.height());
      //qDebug() << "this is the widget position of " << i << position << "and geometry"  << mListLayout.widgets()[i]->geometry();
    }
}

void ListMoodGroupWidget::setCheckedMoods(std::list<QString> checkedMoods) {
    for (auto&& existingWidget : mListLayout.widgets()) {
        ListMoodPreviewWidget *widget = qobject_cast<ListMoodPreviewWidget*>(existingWidget);
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

void ListMoodGroupWidget::resizeEvent(QResizeEvent *) {
    resizeInteralWidgets();
}

void ListMoodGroupWidget::mouseReleaseEvent(QMouseEvent *) {
    setShowButtons(!mDropdownTopWidget->showButtons());
}


void ListMoodGroupWidget::selectMood(std::uint64_t key) {
    for (const auto& existingWidget : mListLayout.widgets()) {
        ListMoodPreviewWidget *widget = qobject_cast<ListMoodPreviewWidget*>(existingWidget);
        Q_ASSERT(widget);
        if (std::uint64_t(widget->key().toInt()) == key) {
            widget->setSelected(true);
        } else {
            widget->setSelected(false);
        }
    }
    resizeInteralWidgets();
    emit moodSelected(mKey, key);
}

void ListMoodGroupWidget::removeMood(QString mood) {
    // check if mood exists
    bool foundMood = false;
    ListMoodPreviewWidget *moodWidget = nullptr;
    for (auto&& widget : mListLayout.widgets()) {
        ListMoodPreviewWidget *existingWidget = qobject_cast<ListMoodPreviewWidget*>(widget);
        Q_ASSERT(existingWidget);

        if (mood.compare(existingWidget->key()) == 0) {
            foundMood = true;
            moodWidget = existingWidget;
        }
    }

    if (foundMood && moodWidget != nullptr) {
        mListLayout.removeWidget(moodWidget);
    }
}

void ListMoodGroupWidget::closeLights() {
    mDropdownTopWidget->showButtons(false);
    for (const auto& device : mListLayout.widgets()) {
        device->setVisible(false);
    }
    resizeInteralWidgets();
}
