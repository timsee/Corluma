/*!
 * \copyright
 * Copyright (C) 2015 - 2020.
 * Released under the GNU General Public License.
 */

#include "listmoodwidget.h"

#include "utils/qt.h"

ListMoodWidget::ListMoodWidget(const QString& name,
                               const std::vector<cor::Mood>& moods,
                               const QString& key,
                               QWidget* parent)
    : cor::ListItemWidget(key, parent),
      mMoodContainer{new MenuMoodContainer(this)} {
    mDropdownTopWidget = new DropdownTopWidget(name, name, cor::EWidgetType::full, true, this);
    mDropdownTopWidget->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);

    connect(mMoodContainer,
            SIGNAL(moodSelected(std::uint64_t)),
            this,
            SLOT(selectMood(std::uint64_t)));
    mMoodContainer->showMoods(moods, mDropdownTopWidget->height() * 2);
    resize();
}


void ListMoodWidget::updateMoods(const std::vector<cor::Mood>& moods) {
    mMoodContainer->setVisible(true);
    mMoodContainer->showMoods(moods, mDropdownTopWidget->height() * 2);
}

void ListMoodWidget::setShowButtons(bool show) {
    mDropdownTopWidget->showButtons(show);
    mMoodContainer->setVisible(show);

    resize();
    emit buttonsShown(mKey, mDropdownTopWidget->showButtons());
}

void ListMoodWidget::resize() {
    if (mDropdownTopWidget->showButtons()) {
        setFixedHeight(mMoodContainer->height() + mDropdownTopWidget->height());
    } else {
        setFixedHeight(mDropdownTopWidget->height());
    }
    auto yPos = 0;
    mDropdownTopWidget->setGeometry(0, yPos, width(), mDropdownTopWidget->height());
    yPos += mDropdownTopWidget->height();

    if (mMoodContainer->isVisible()) {
        mMoodContainer->setGeometry(0, yPos, width(), height() - mDropdownTopWidget->height());
        mMoodContainer->resize();
    }
}

void ListMoodWidget::resizeEvent(QResizeEvent*) {
    resize();
}

void ListMoodWidget::mouseReleaseEvent(QMouseEvent* event) {
    if (cor::leftHandMenuMoving()) {
        event->ignore();
        return;
    }
    if (cor::isMouseEventTouchUpInside(event, this, true)) {
        setShowButtons(!mDropdownTopWidget->showButtons());
    }
    event->ignore();
}
