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

MoodPage::MoodPage(QWidget* parent, AppData* appData, CommLayer* comm)
    : QWidget(parent),
      mMoodData{appData->moods()},
      mComm{comm},
      mMoodMenu{new StandardMoodsMenu(this, comm, appData)},
      mPlaceholderWidget{new ListPlaceholderWidget(
          this,
          "There are no moods saved in the app. Click the + button to make your first mood.")},
      mGreyOut{new GreyOutOverlay(false, parentWidget())},
      mCurrentMood{} {
    connect(mMoodMenu, SIGNAL(moodClicked(QString)), this, SLOT(moodSelected(QString)));

    mMoodDetailedWidget = new MoodDetailedWidget(parent, appData, mComm);
    connect(mMoodDetailedWidget, SIGNAL(pressedClose()), this, SLOT(detailedClosePressed()));

    mMoodDetailedWidget->setGeometry(0, -1 * height(), width(), height());
    mMoodDetailedWidget->setVisible(false);

    connect(mGreyOut, SIGNAL(clicked()), this, SLOT(greyoutClicked()));

    connect(appData, SIGNAL(newMoodAdded(QString)), this, SLOT(newMoodAdded(QString)));

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


void MoodPage::selectedMood(const QString&, const QString& moodKey) {
    mCurrentMood = moodKey;
    moodSelected(moodKey);
}

void MoodPage::resize() {
    auto mainWidgetHeight = height() * 0.9;
    auto xSpacer = width() * 0.03;
    auto mainRect = QRect(xSpacer, height() * 0.05, width() - xSpacer * 2, mainWidgetHeight);
    if (mMoodData->moods().empty()) {
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

void MoodPage::show(const QString& currentMood) {
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
    mCurrentMood = QString();
}

void MoodPage::greyoutClicked() {
    if (mMoodDetailedWidget->isOpen()) {
        detailedClosePressed();
    }
}

void MoodPage::detailedMoodDisplay(QString key) {
    mGreyOut->greyOut(true);

    const auto& moodResult = mMoodData->moods().item(key.toStdString());
    if (moodResult.second) {
        mMoodDetailedWidget->update(moodResult.first);
    }

    mMoodDetailedWidget->pushIn();
}

void MoodPage::moodSelected(QString key) {
    detailedMoodDisplay(key);
}

void MoodPage::detailedClosePressed() {
    mGreyOut->greyOut(false);
    mMoodDetailedWidget->pushOut();
}

void MoodPage::checkForMissingMoods() {
    if (mMoodData->moods().empty()) {
        mPlaceholderWidget->setVisible(true);
        mMoodMenu->setVisible(false);
    } else {
        mPlaceholderWidget->setVisible(false);
        mMoodMenu->setVisible(true);
    }
}
