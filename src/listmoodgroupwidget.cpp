/*!
 * \copyright
 * Copyright (C) 2015 - 2018.
 * Released under the GNU General Public License.
 */

#include "listmoodgroupwidget.h"
#include "cor/utils.h"

ListMoodGroupWidget::ListMoodGroupWidget(const QString& name,
                                         std::list<cor::LightGroup> moods,
                                         QString key,
                                         bool hideEdit,
                                         QWidget *parent) : cor::ListItemWidget(key, parent), mListLayout(cor::EListType::linear2X) {

    mWidget = new QWidget(this);

    mLayout = new QVBoxLayout(this);
    mLayout->setContentsMargins(0, 0, this->size().width() / 40, 0);
    mLayout->setSpacing(0);

    setLayout(mLayout);

    mMoods = moods;
    mDropdownTopWidget = new DropdownTopWidget(name, hideEdit, this);
    mDropdownTopWidget->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Minimum);

    updateMoods(moods, false);

    mLayout->addWidget(mDropdownTopWidget);
    mLayout->addWidget(mWidget);
}


void ListMoodGroupWidget::updateMoods(const std::list<cor::LightGroup>& moods,
                                      bool removeIfNotFound) {
    std::vector<bool> foundWidgets(mListLayout.widgets().size(), false);
    for (const auto& mood : moods) {
        bool foundMood = false;
        uint32_t x = 0;
        for (const auto& existingWidget : mListLayout.widgets()) {
            if (mood.name.compare(existingWidget->key()) == 0) {
                foundMood = true;
//                qDebug() << x << "  vs " << mListLayout.widgets().size();
//                qDebug() << mood.name << " vs " << existingWidget->key();
                foundWidgets[x] = true;
            }
            ++x;
        }

        if (!foundMood) {
            ListMoodWidget *widget = new ListMoodWidget(mood, mWidget);
            connect(widget, SIGNAL(clicked(QString)), this, SLOT(handleClicked(QString)));
            connect(widget, SIGNAL(editClicked(QString)), this, SLOT(clickedEdit(QString)));
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
}

void ListMoodGroupWidget::setCheckedMoods(std::list<QString> checkedMoods) {
    for (auto&& existingWidget : mListLayout.widgets()) {
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

void ListMoodGroupWidget::resizeEvent(QResizeEvent *) {
    mWidget->setFixedWidth(this->width());
    QPoint offset(0, 0);
    QSize size = mListLayout.widgetSize(QSize(this->width(), mDropdownTopWidget->height()));
    for (uint32_t i = 0; i < mListLayout.widgets().size(); ++i) {
        QPoint position = mListLayout.widgetPosition(mListLayout.widgets()[i]);
        mListLayout.widgets()[i]->setFixedSize(size);
        mListLayout.widgets()[i]->setGeometry(offset.x() + position.x() * size.width(),
                                 offset.y() + position.y() * size.height(),
                                 size.width(),
                                 size.height());
      //qDebug() << "this is the widget position of " << i << position << "and geometry"  << mWidgets[i]->geometry();
    }
}

void ListMoodGroupWidget::mouseReleaseEvent(QMouseEvent *) {
    setShowButtons(!mDropdownTopWidget->showButtons());
}

void ListMoodGroupWidget::removeMood(QString mood) {
    // check if mood exists
    bool foundMood = false;
    ListMoodWidget *moodWidget = nullptr;
    for (auto&& widget : mListLayout.widgets()) {
        ListMoodWidget *existingWidget = qobject_cast<ListMoodWidget*>(widget);
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
