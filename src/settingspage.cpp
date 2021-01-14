/*!
 * \copyright
 * Copyright (C) 2015 - 2020.
 * Released under the GNU General Public License.
 */

#include "settingspage.h"

#include <QDebug>
#include <QFileDialog>
#include <QMessageBox>
#include <QPainter>
#include <QScrollBar>
#include <QScroller>
#include <QStyleOption>

#include "comm/commhue.h"
#include "listlightwidget.h"
#include "mainwindow.h"
#include "utils/qt.h"

#ifdef USE_SHARE_UTILS
#include "shareutils/shareutils.hpp"
#endif

#define SHOW_APP_VERSION
//#define USE_DEBUG_OPTIONS
#ifdef USE_DEBUG_OPTIONS
const static char* kDebugSpoof = "DEBUG: Spoof Connection";
#endif

SettingsPage::SettingsPage(QWidget* parent,
                           GroupData* parser,
                           CommLayer* comm,
                           AppSettings* appSettings)
    : QWidget(parent),
      mGroups(parser),
      mScrollArea{new QScrollArea(this)},
      mScrollAreaWidget{new QWidget(this)},
      mScrollLayout{new QVBoxLayout(mScrollAreaWidget)},
      mGlobalWidget{new GlobalSettingsWidget(mScrollAreaWidget, appSettings)},
      mAppVersionLabel{new QLabel(APP_VERSION, this)},
      mCurrentWebView{ECorlumaWebView::none},
      mSectionTitles{"Data", "About"},
      mSectionLabels{mSectionTitles.size()},
      mTitles{"Add or Edit Group",
              "Backup Save Data",
#ifndef MOBILE_BUILD
              "Load Backup",
#endif
#ifdef USE_DEBUG_OPTIONS
              kDebugSpoof,
#endif
              "Reset",
              "Copyright"},
      mButtons{mTitles.size()},
      mCopyrightWidget{new cor::WebView("Copyright", ":/resources/Copyright.html", this)},
      mComm{comm},
      mGreyOut{new GreyOutOverlay(false, parentWidget())} {


#ifndef SHOW_APP_VERSION
    mAppVersionLabel->setVisible(false);
#endif

    mAppVersionLabel->setText(QString("v" + mAppVersionLabel->text()));
    mAppVersionLabel->setStyleSheet("background-color:rgba(0,0,0,0);");

    //------------
    // ScrollArea Widget
    //------------

    mScrollArea->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    mScrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

    mScrollAreaWidget->setObjectName("contentWidget");
    QScroller::grabGesture(mScrollArea->viewport(), QScroller::LeftMouseButtonGesture);
    mScrollAreaWidget->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    mScrollAreaWidget->setContentsMargins(0, 0, 0, 0);
    mScrollAreaWidget->setStyleSheet("QWidget#contentWidget{ background-color: #201F1F; } QLabel { "
                                     "background-color: #201F1F; } ");


    mScrollLayout->setSpacing(7);
    mScrollLayout->setContentsMargins(9, 9, 9, 9);
    mScrollAreaWidget->setLayout(mScrollLayout);

    //------------
    // Scroll Area Contents
    //------------

    std::uint32_t sectionIndex = 0;
    for (std::size_t x = 0u; x < mTitles.size(); ++x) {
        if (mTitles[x] == "Backup Save Data" || mTitles[x] == "Mock Connection"
            || mTitles[x] == "Copyright") {
            mSectionLabels[sectionIndex] = new QLabel(mSectionTitles[sectionIndex].c_str());
            mSectionLabels[sectionIndex]->setStyleSheet(
                "font:bold; font-size:20pt; color:rgba(61, 142, 201,255);");
            mScrollLayout->addWidget(mSectionLabels[sectionIndex]);
            sectionIndex++;
        }
        auto minHeight = height() / 10;
        mButtons[x] = new SettingsButton(QString(mTitles[x].c_str()), minHeight, this);
        mButtons[x]->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Minimum);
        connect(mButtons[x],
                SIGNAL(buttonPressed(QString)),
                this,
                SLOT(settingsButtonPressed(QString)));
        mScrollLayout->addWidget(mButtons[x]);
    }

    mCopyrightWidget->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    mCopyrightWidget->setGeometry(geometry());
    connect(mCopyrightWidget, SIGNAL(closePressed()), this, SLOT(hideCurrentWebView()));

    //------------
    // Global Widget
    //------------
    mGlobalWidget->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    mScrollLayout->addWidget(mGlobalWidget);

    connect(mGreyOut, SIGNAL(clicked()), this, SLOT(greyOutClicked()));


    mScrollArea->setWidget(mScrollAreaWidget);
    enableButtons(false);
}

// ----------------------------
// Protected
// ----------------------------

void SettingsPage::showWidget() {
    raise();
    isOpen(true);

    mGlobalWidget->updateUI();
    mGlobalWidget->show();
    auto mainWindow = qobject_cast<MainWindow*>(parentWidget());
    Q_ASSERT(mainWindow);
    bool anyDiscovered = mainWindow->anyDiscovered();
    // add new group
    mButtons[0]->shouldEnable(anyDiscovered);
    // backup save data
    mButtons[1]->shouldEnable(mGroups->saveExists());
#ifndef MOBILE_BUILD
    // load backup
    mButtons[2]->shouldEnable(anyDiscovered);
#endif
    mGlobalWidget->resize();
    mCopyrightWidget->setGeometry(geometry());
}

void SettingsPage::hideWidget() {
    isOpen(false);
}

void SettingsPage::resizeEvent(QResizeEvent*) {
    // resize the app version label
    QFontMetrics fm(mAppVersionLabel->font());
    auto textWidth = int(fm.horizontalAdvance(mAppVersionLabel->text()) * 1.1);
    mAppVersionLabel->setGeometry(this->width() - textWidth,
                                  this->height() - mAppVersionLabel->height(),
                                  textWidth,
                                  mAppVersionLabel->height());

    mScrollArea->setGeometry(this->width() * 0.03,
                             this->height() * 0.03,
                             width() * 0.94,
                             height() * 0.97 - mAppVersionLabel->height());

    mScrollAreaWidget->setFixedWidth(int(width() * 0.9f));
    // mScrollAreaWidget->setFixedHeight(1.1 * (mGlobalWidget->y() + mGlobalWidget->height()));

    QRect shownWidget = geometry();
    QRect hiddenWidget = QRect(0, geometry().height(), width(), height());

    switch (mCurrentWebView) {
        case ECorlumaWebView::copyright:
            mCopyrightWidget->setGeometry(shownWidget);
            break;
        case ECorlumaWebView::FAQ:
            mCopyrightWidget->setGeometry(hiddenWidget);
            break;
        default:
            mCopyrightWidget->setGeometry(hiddenWidget);
    }


    mGreyOut->resize();
}


void SettingsPage::loadButtonClicked() {
    QFileDialog dialog(this);
    dialog.setFileMode(QFileDialog::AnyFile);
    dialog.setNameFilter(tr("JSON (*.json)"));
    dialog.setViewMode(QFileDialog::Detail);
    dialog.setStyleSheet("color:silver;");
    const QString downloadsFolder =
        QStandardPaths::writableLocation(QStandardPaths::DownloadLocation);
    dialog.setDirectory(downloadsFolder);
    QStringList fileNames;
    if (dialog.exec()) {
        fileNames = dialog.selectedFiles();
        for (auto& name : fileNames) {
            emit clickedLoadJSON(name);
        }
    }
}

void SettingsPage::saveButtonClicked() {
#if defined(Q_OS_IOS) || defined(Q_OS_ANDROID)
#ifdef USE_SHARE_UTILS
    int requestID = 7;
    cor::shareUtils()->sendFile(mGroups->savePath(), "CorlumaSave", "application/json", requestID);
#endif
#else
    auto fileName = QFileDialog::getSaveFileName(this,
                                                 tr("Save Group Data"),
                                                 "CorlumaGroups.json",
                                                 tr("JSON (*.json)"));
    if (fileName.isEmpty()) {
        qDebug() << "WARNING: save file name empty";
        return;
    } else {
        mGroups->save(fileName);
    }
#endif
}

void SettingsPage::resetButtonClicked() {
    QMessageBox::StandardButton reply;
    QString text = "Reset all App Data? This will remove the settings, saved collections, and "
                   "saved moods. This cannot be undone.";
    reply = QMessageBox::question(this, "Reset?", text, QMessageBox::Yes | QMessageBox::No);
    if (reply == QMessageBox::Yes) {
        resetToDefaults();
    }
}

void SettingsPage::resetToDefaults() {
    mGlobalWidget->checkBoxClicked(EProtocolType::hue, true);
    mGlobalWidget->checkBoxClicked(EProtocolType::arduCor, false);
    mGlobalWidget->checkBoxClicked(EProtocolType::nanoleaf, false);

    mGlobalWidget->timeoutCheckboxPressed(true);

    mGroups->removeAppData();
    // load no data, deleting everything.
    mGroups->loadExternalData("");
    AppSettings::setToDefaults();
}

void SettingsPage::paintEvent(QPaintEvent*) {
    QStyleOption opt;
    opt.initFrom(this);
    QPainter painter(this);

    painter.setRenderHint(QPainter::Antialiasing);
    painter.fillRect(rect(), QBrush(QColor(48, 47, 47)));
}

void SettingsPage::settingsButtonPressed(const QString& title) {
    // qDebug() << "settings button pressed: " << title;
    if (title == "Reset") {
        resetButtonClicked();
    } else if (title == "Load Backup") {
        loadButtonClicked();
    } else if (title == "Backup Save Data") {
        saveButtonClicked();
    } else if (title == "Add or Edit Group") {
        addOrEditGroupPressed();
    }
#ifdef USE_DEBUG_OPTIONS
    else if (title == kDebugSpoof) {
        emit enableDebugMode();
        emit closePressed();
    }
#endif // USE_DEBUG_OPTIONS
    else if (title == "Copyright") {
        showWebView(ECorlumaWebView::copyright);
    } else if (title == "FAQ") {
        showWebView(ECorlumaWebView::FAQ);
    }
}


void SettingsPage::showWebView(ECorlumaWebView newWebView) {
    if (newWebView != mCurrentWebView) {
        mCurrentWebView = newWebView;

        cor::WebView* widget;
        switch (newWebView) {
            case ECorlumaWebView::copyright:
                widget = mCopyrightWidget;
                break;
            default:
                // none, return
                return;
        }

        cor::moveWidget(widget, QPoint(0, widget->height()), QPoint(0, 0));

        widget->raise();
    }
}

void SettingsPage::hideCurrentWebView() {
    cor::WebView* widget;
    switch (mCurrentWebView) {
        case ECorlumaWebView::copyright:
            widget = mCopyrightWidget;
            break;
        default:
            // none, return
            return;
    }

    cor::moveWidget(widget, QPoint(0, 0), QPoint(0, widget->height()));

    mCurrentWebView = ECorlumaWebView::none;
}

void SettingsPage::greyOutClicked() {}

void SettingsPage::enableButtons(bool enable) {
    if (enable) {
        for (auto button : mButtons) {
            button->shouldEnable(true);
        }
    } else {
        for (auto button : mButtons) {
            if (button->text() == "Copyright") {
                button->shouldEnable(true);
            } else {
                button->shouldEnable(false);
            }
        }
    }
}
