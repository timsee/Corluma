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
    mGroupButton = new cor::GroupButton(name, this);
    mGroupButton->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);

    connect(mMoodContainer,
            SIGNAL(moodSelected(std::uint64_t)),
            this,
            SLOT(selectMood(std::uint64_t)));
    mMoodContainer->showMoods(moods, mGroupButton->height() * 2);
    resize();
}


void ListMoodWidget::updateMoods(const std::vector<cor::Mood>& moods) {
    mMoodContainer->setVisible(true);
    mMoodContainer->showMoods(moods, mGroupButton->height() * 2);
}

void ListMoodWidget::setShowButtons(bool show) {
    if (show) {
        mGroupButton->changeArrowState(cor::EArrowState::open);
    } else {
        mGroupButton->changeArrowState(cor::EArrowState::closed);
    }
    mMoodContainer->setVisible(show);

    resize();
    emit buttonsShown(mKey, mGroupButton->isArrowOpen());
}

void ListMoodWidget::resize() {
    if (mGroupButton->isArrowOpen()) {
        setFixedHeight(mMoodContainer->height() + mGroupButton->height());
    } else {
        setFixedHeight(mGroupButton->height());
    }
    auto yPos = 0;
    mGroupButton->setGeometry(0, yPos, width(), mGroupButton->height());
    yPos += mGroupButton->height();

    if (mMoodContainer->isVisible()) {
        mMoodContainer->setGeometry(0, yPos, width(), height() - mGroupButton->height());
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
        setShowButtons(!mGroupButton->isArrowOpen());
    }
    event->ignore();
}
