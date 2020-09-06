#include "menumoodcontainer.h"
#include "listmoodpreviewwidget.h"
#include "utils/qt.h"

MenuMoodContainer::MenuMoodContainer(QWidget* parent)
    : QWidget(parent),
      mListLayout(cor::EListType::grid) {}


void MenuMoodContainer::showMoods(const std::vector<cor::Mood>& moods, int height) {
    clear();
    mHeight = height;
    std::vector<bool> foundWidgets(moods.size(), false);
    // sort the moods by name
    auto sortedMoods = moods;
    auto lambda = [](const cor::Mood& a, const cor::Mood& b) -> bool {
        return a.name() < b.name();
    };
    std::sort(sortedMoods.begin(), sortedMoods.end(), lambda);
    for (const auto& mood : sortedMoods) {
        auto widget = new ListMoodPreviewWidget(mood, this);
        connect(widget, SIGNAL(moodSelected(std::uint64_t)), this, SLOT(selectMood(std::uint64_t)));
        mListLayout.insertWidget(widget);
    }

    resize();
}

void MenuMoodContainer::resize() {
    QPoint offset(0, 0);
    QSize size = mListLayout.widgetSize(QSize(width(), mHeight));

    int yPos = 0;
    for (std::size_t i = 0u; i < mListLayout.widgets().size(); ++i) {
        auto widget = qobject_cast<ListMoodPreviewWidget*>(mListLayout.widgets()[i]);
        widget->setVisible(true);
        Q_ASSERT(widget);
        QPoint position = mListLayout.widgetPosition(mListLayout.widgets()[i]);
        mListLayout.widgets()[i]->setGeometry(offset.x() + position.x() * size.width(),
                                              offset.y() + position.y() * size.height(),
                                              size.width(),
                                              size.height());
        yPos = mListLayout.widgets()[i]->geometry().y() + mListLayout.widgets()[i]->height();
    }
    setFixedHeight(yPos);
}

void MenuMoodContainer::clear() {
    mListLayout.clear();
}

void MenuMoodContainer::selectMood(std::uint64_t key) {
    for (const auto& existingWidget : mListLayout.widgets()) {
        auto widget = qobject_cast<ListMoodPreviewWidget*>(existingWidget);
        Q_ASSERT(widget);
        if (std::uint64_t(widget->key().toInt()) == key) {
            widget->setSelected(true);
        } else {
            widget->setSelected(false);
        }
    }
    emit moodSelected(key);
}
