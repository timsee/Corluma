/*!
 * \copyright
 * Copyright (C) 2015 - 2020.
 * Released under the GNU General Public License.
 */

#include "moodpage.h"

#include <QDebug>
#include <QInputDialog>
#include <QScroller>

#include "listmoodpreviewwidget.h"
#include "listmoodwidget.h"
#include "utils/qt.h"

MoodPage::MoodPage(QWidget* parent, GroupData* groups, CommLayer* comm)
    : QWidget(parent),
      mGroups(groups),
      mComm{comm},
      mMoodMenu{new StandardMoodsMenu(this, comm, groups)},
      mPlaceholderWidget{new ListPlaceholderWidget(
          this,
          "There are no moods saved in the app. Click the + button to make your first mood.")},
      mGreyOut{new GreyOutOverlay(false, parentWidget())},
      mCurrentMood{0} {
    connect(mMoodMenu, SIGNAL(moodClicked(std::uint64_t)), this, SLOT(moodSelected(std::uint64_t)));

    mMoodDetailedWidget = new MoodDetailedWidget(parent, mGroups, mComm);
    connect(mMoodDetailedWidget, SIGNAL(pressedClose()), this, SLOT(detailedClosePressed()));

    mMoodDetailedWidget->setGeometry(0, -1 * height(), width(), height());
    mMoodDetailedWidget->setVisible(false);

    connect(mGreyOut, SIGNAL(clicked()), this, SLOT(greyoutClicked()));

    connect(mGroups, SIGNAL(newMoodAdded(QString)), this, SLOT(newMoodAdded(QString)));
          
    mMoodMenu->setVisible(false);
    mPlaceholderWidget->setVisible(false);
}

void MoodPage::newMoodAdded(const QString&) {
    clearWidgets();
    mMoodMenu->updateData();
    checkForMissingMoods();
}

void MoodPage::updateMoods() {
    clearWidgets();
    mMoodMenu->updateData();
    checkForMissingMoods();
}


void MoodPage::selectedMood(const QString&, std::uint64_t moodKey) {
    mCurrentMood = moodKey;
    moodSelected(moodKey);
}

void MoodPage::resize() {
    auto mainWidgetHeight = height() * 0.97;
    auto xSpacer = width() * 0.02;
    auto mainRect = QRect(xSpacer, 0, width() - xSpacer * 2, mainWidgetHeight);
    if (mGroups->moods().empty()) {
        mPlaceholderWidget->setGeometry(mainRect);
    } else {
        mMoodMenu->widgetHeight(this->height() / 8);
        mMoodMenu->setGeometry(mainRect);
    }
    if (mMoodDetailedWidget->isOpen()) {
        mMoodDetailedWidget->resize();
    }
    mGreyOut->resize();
}

void MoodPage::resizeEvent(QResizeEvent*) {
    resize();
}

void MoodPage::show(std::uint64_t currentMood) {
    checkForMissingMoods();
    mMoodMenu->setVisible(true);
    mCurrentMood = currentMood;
    mMoodMenu->updateData();
    resize();
}

void MoodPage::hide() {
    mMoodMenu->setVisible(false);
}


void MoodPage::clearWidgets() {
    mCurrentMood = 0;
}

void MoodPage::greyoutClicked() {
    if (mMoodDetailedWidget->isOpen()) {
        detailedClosePressed();
    }
}

void MoodPage::detailedMoodDisplay(std::uint64_t key) {
    mGreyOut->greyOut(true);

    const auto& moodResult = mGroups->moods().item(QString::number(key).toStdString());
    if (moodResult.second) {
        mMoodDetailedWidget->update(moodResult.first);
    }

    mMoodDetailedWidget->pushIn();
}

void MoodPage::moodSelected(std::uint64_t key) {
    detailedMoodDisplay(key);
}

void MoodPage::detailedClosePressed() {
    mGreyOut->greyOut(false);
    mMoodDetailedWidget->pushOut();
}

void MoodPage::checkForMissingMoods() {
    if (mGroups->moods().empty()) {
        mPlaceholderWidget->setVisible(true);
        mMoodMenu->setVisible(false);
    } else {
        mPlaceholderWidget->setVisible(false);
        mMoodMenu->setVisible(true);
    }
}
