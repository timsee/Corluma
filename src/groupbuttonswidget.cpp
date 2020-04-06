#include "groupbuttonswidget.h"

#include <QDebug>

GroupButtonsWidget::GroupButtonsWidget(QWidget* parent, cor::EWidgetType type)
    : QWidget(parent),
      mType{type} {
    if (mType == cor::EWidgetType::condensed) {
        mGroupCount = 1;
    } else {
        mGroupCount = 3;
    }
}

void GroupButtonsWidget::showGroups(std::vector<QString> groups) {
    // remove any existing groups
    for (auto widget : mButtons) {
        delete widget;
    }
    mButtons.clear();

    addGroup("All");
    for (const auto& group : groups) {
        addGroup(group);
    }
}


void GroupButtonsWidget::addGroup(const QString& group) {
    auto groupButton = new cor::GroupButton(this, group);
    connect(groupButton, SIGNAL(groupButtonPressed(QString)), this, SLOT(buttonPressed(QString)));
    connect(groupButton,
            SIGNAL(groupSelectAllToggled(QString, bool)),
            this,
            SLOT(buttonToggled(QString, bool)));
    mButtons.push_back(groupButton);
    groupButton->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);

    std::sort(mButtons.begin(), mButtons.end(), [](cor::GroupButton* a, cor::GroupButton* b) {
        return (a->key() < b->key());
    });
}


void GroupButtonsWidget::updateCheckedLights(const QString& key,
                                             std::uint32_t checkedLightCount,
                                             std::uint32_t reachableLightCount) {
    for (const auto& widget : mButtons) {
        if (widget->key() == key) {
            widget->handleSelectAllButton(checkedLightCount, reachableLightCount);
        }
    }
}

void GroupButtonsWidget::buttonPressed(const QString& key) {
    mCurrentKey = key;
    for (const auto& widget : mButtons) {
        widget->setSelectAll(widget->key() == key);
    }
    emit groupButtonPressed(key);
}

void GroupButtonsWidget::resizeEvent(QResizeEvent*) {
    resize();
}

void GroupButtonsWidget::resize() {
    // if no group is selected, display all groups
    int buttonHeight = int(float(height()) / mButtons.size());
    for (std::size_t i = 0; i < mButtons.size(); ++i) {
        int column = i % mGroupCount;
        int row = i / mGroupCount;
        int yPos = row * buttonHeight;
        mButtons[i]->setGeometry(int(column * (width() / mGroupCount)),
                                 yPos,
                                 width() / mGroupCount,
                                 buttonHeight);
        mButtons[i]->setVisible(true);
    }
}

int GroupButtonsWidget::expectedHeight(int topWidgetHeight) {
    if ((mButtons.size() % mGroupCount) == 0) {
        return int(mButtons.size() / mGroupCount) * topWidgetHeight;
    }
    return int(mButtons.size() / mGroupCount) * topWidgetHeight + topWidgetHeight;
}

int GroupButtonsWidget::groupEndPointY(int topWidgetHeight, const QString& key) {
    int height = topWidgetHeight;
    int i = 1;
    for (const auto& widget : mButtons) {
        if (widget->key() == key) {
            return height * i;
        }
        ++i;
    }
    return height * i;
}
