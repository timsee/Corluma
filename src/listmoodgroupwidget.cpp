/*!
 * \copyright
 * Copyright (C) 2015 - 2020.
 * Released under the GNU General Public License.
 */

#include "listmoodgroupwidget.h"

#include "utils/qt.h"

ListMoodGroupWidget::ListMoodGroupWidget(const QString& name,
                                         const std::vector<cor::Mood>& moods,
                                         const QString& key,
                                         bool hideEdit,
                                         QWidget* parent)
    : cor::ListItemWidget(key, parent),
      mListLayout(cor::EListType::grid) {
    mWidget = new QWidget(this);
    mWidget->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);

    mMoods = moods;
    mDropdownTopWidget = new DropdownTopWidget(name, cor::EWidgetType::full, hideEdit, this);
    mDropdownTopWidget->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);

    updateMoods(moods, false);
    resize();
}


void ListMoodGroupWidget::updateMoods(const std::vector<cor::Mood>& moods, bool removeIfNotFound) {
    std::vector<bool> foundWidgets(moods.size(), false);
    // sort the moods by name
    auto sortedMoods = moods;
    auto lambda = [](const cor::Mood& a, const cor::Mood& b) -> bool {
        return a.name() < b.name();
    };
    std::sort(sortedMoods.begin(), sortedMoods.end(), lambda);
    for (const auto& mood : sortedMoods) {
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
            auto widget = new ListMoodPreviewWidget(mood, mWidget);
            connect(widget,
                    SIGNAL(moodSelected(std::uint64_t)),
                    this,
                    SLOT(selectMood(std::uint64_t)));
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
        setFixedHeight(mListLayout.overallSize().height() + mDropdownTopWidget->height());
    } else {
        setFixedHeight(mDropdownTopWidget->height());
    }
    auto yPos = 0;
    mDropdownTopWidget->setGeometry(0, yPos, width(), mDropdownTopWidget->height());
    yPos += mDropdownTopWidget->height();

    mWidget->setGeometry(0, yPos, width(), height() - mDropdownTopWidget->height());

    QPoint offset(0, 0);

    QSize size = mListLayout.widgetSize(QSize(width(), int(mDropdownTopWidget->height() * 2)));
    for (std::size_t i = 0u; i < mListLayout.widgets().size(); ++i) {
        auto widget = qobject_cast<ListMoodPreviewWidget*>(mListLayout.widgets()[i]);
        Q_ASSERT(widget);
        QPoint position = mListLayout.widgetPosition(mListLayout.widgets()[i]);
        mListLayout.widgets()[i]->setGeometry(offset.x() + position.x() * size.width(),
                                              offset.y() + position.y() * size.height(),
                                              size.width(),
                                              size.height());
        // qDebug() << "this is the widget position of " << i << position << "and geometry"  <<
        // mListLayout.widgets()[i]->geometry();
    }
}

void ListMoodGroupWidget::setCheckedMoods(const std::vector<QString>& checkedMoods) {
    for (auto&& existingWidget : mListLayout.widgets()) {
        auto widget = qobject_cast<ListMoodPreviewWidget*>(existingWidget);
        Q_ASSERT(widget);

        bool checkedNameFound = false;
        for (const auto& checkedName : checkedMoods) {
            if (widget->moodName() == checkedName) {
                checkedNameFound = true;
                widget->setChecked(true);
            }
        }
        if (!checkedNameFound) {
            widget->setChecked(false);
        }
    }
}

void ListMoodGroupWidget::resize() {
    resizeInteralWidgets();
}

void ListMoodGroupWidget::resizeEvent(QResizeEvent*) {
    resize();
}

void ListMoodGroupWidget::mouseReleaseEvent(QMouseEvent*) {
    setShowButtons(!mDropdownTopWidget->showButtons());
}


void ListMoodGroupWidget::selectMood(std::uint64_t key) {
    for (const auto& existingWidget : mListLayout.widgets()) {
        auto widget = qobject_cast<ListMoodPreviewWidget*>(existingWidget);
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

void ListMoodGroupWidget::removeMood(const QString& mood) {
    // check if mood exists
    bool foundMood = false;
    ListMoodPreviewWidget* moodWidget = nullptr;
    for (const auto& widget : mListLayout.widgets()) {
        auto existingWidget = qobject_cast<ListMoodPreviewWidget*>(widget);
        Q_ASSERT(existingWidget);

        if (mood == existingWidget->key()) {
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
