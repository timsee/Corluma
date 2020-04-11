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

SettingsPage::SettingsPage(QWidget* parent,
                           GroupData* parser,
                           CommLayer* comm,
                           AppSettings* appSettings,
                           ShareUtils* shareUtils)
    : QWidget(parent),
      mGroups(parser),
      mShareUtils(shareUtils),
      mComm{comm},
      mGreyOut{new GreyOutOverlay(false, parentWidget())},
      mLightInfoWidget{new LightInfoListWidget(parentWidget(), appSettings)} {
    mShowingDebug = true;

    // --------------
    // Setup Light Info Widget
    // --------------

    mLightInfoWidget->isOpen(false);

    connect(mLightInfoWidget, SIGNAL(pressedClose()), this, SLOT(lightInfoClosePressed()));
    mLightInfoWidget->setGeometry(0, -1 * height(), width(), height());

    mCurrentWebView = ECorlumaWebView::none;

    //------------
    // Top Layout
    //------------

    mTopWidget = new cor::TopWidget("", ":images/closeX.png", this);
    connect(mTopWidget, SIGNAL(clicked(bool)), this, SLOT(closeButtonPressed(bool)));

    //------------
    // ScrollArea Widget
    //------------

    mScrollArea = new QScrollArea(this);
    mScrollArea->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    mScrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

    mScrollAreaWidget = new QWidget(this);
    mScrollAreaWidget->setObjectName("contentWidget");
    QScroller::grabGesture(mScrollArea->viewport(), QScroller::LeftMouseButtonGesture);
    mScrollAreaWidget->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    mScrollAreaWidget->setContentsMargins(0, 0, 0, 0);
    mScrollAreaWidget->setStyleSheet("QWidget#contentWidget{ background-color: #201F1F; } QLabel { "
                                     "background-color: #201F1F; } ");

    mScrollLayout = new QVBoxLayout(mScrollAreaWidget);
    mScrollLayout->setSpacing(7);
    mScrollLayout->setContentsMargins(9, 9, 9, 9);
    mScrollAreaWidget->setLayout(mScrollLayout);

    //------------
    // Main Layout
    //------------

    mMainLayout = new QVBoxLayout(this);
    mMainLayout->addWidget(mTopWidget, 1);
    mMainLayout->addWidget(mScrollArea, 12);
    mMainLayout->setContentsMargins(9, 9, 9, 9);
    mMainLayout->setSpacing(6);

    //------------
    // Scroll Area Contents
    //------------

    mSectionTitles = {"Data", "About"};

    mTitles = {"Find New Lights",
               "View/Edit Lights",
               "Add New Group",
               "Backup Save Data",
#ifndef MOBILE_BUILD
               "Load Backup",
#endif
               "Reset",
               "Copyright"};

    mButtons = std::vector<SettingsButton*>(mTitles.size());
    mSectionLabels = std::vector<QLabel*>(mSectionTitles.size());

    uint32_t sectionIndex = 0;
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

    mCopyrightWidget = new cor::WebView("Copyright", ":/resources/Copyright.html", this);
    mCopyrightWidget->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    mCopyrightWidget->setGeometry(geometry());
    connect(mCopyrightWidget, SIGNAL(closePressed()), this, SLOT(hideCurrentWebView()));

    //------------
    // Global Widget
    //------------
    mGlobalWidget = new GlobalSettingsWidget(mScrollAreaWidget, appSettings);
    mGlobalWidget->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    mScrollLayout->addWidget(mGlobalWidget);

    connect(mGreyOut, SIGNAL(clicked()), this, SLOT(greyOutClicked()));

    //------------
    // Final Cleanup
    //------------
    setLayout(mMainLayout);

    mScrollArea->setWidget(mScrollAreaWidget);
    enableButtons(false);
}

// ----------------------------
// Protected
// ----------------------------

void SettingsPage::show() {
    mGlobalWidget->updateUI();
    mGlobalWidget->show();
    auto mainWindow = qobject_cast<MainWindow*>(parentWidget());
    Q_ASSERT(mainWindow);
    bool anyDiscovered = mainWindow->anyDiscovered();
    mGlobalWidget->hideTimeout(!anyDiscovered);
    mButtons[1]->shouldEnable(anyDiscovered);
    mButtons[2]->shouldEnable(mGroups->saveExists());
#ifndef MOBILE_BUILD
    mButtons[3]->shouldEnable(anyDiscovered);
#endif
    mGlobalWidget->resize();
    mCopyrightWidget->setGeometry(geometry());
}

void SettingsPage::resizeEvent(QResizeEvent*) {
    for (auto button : mButtons) {
        button->setMinimumHeight(height() / 10);
    }
    mScrollAreaWidget->setFixedWidth(int(width() * 0.85f));

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
            return;
    }


    mLightInfoWidget->resize();

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
    int requestID = 7;
    mShareUtils->sendFile(mGroups->savePath(), "CorlumaSave", "application/json", requestID);
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

    // load no data, deleting everything.
    mGroups->loadExternalData("");
}

void SettingsPage::paintEvent(QPaintEvent*) {
    QStyleOption opt;
    opt.init(this);
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
    } else if (title == "View/Edit Lights") {
        lightInfoWidgetClicked();
    } else if (title == "Add New Group") {
        addNewGroupButtonPressed();
    } else if (title == "Find New Lights") {
        emit clickedDiscovery();
    } else if (title == "Copyright") {
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


void SettingsPage::lightInfoWidgetClicked() {
    mGreyOut->resize();
    mGreyOut->greyOut(true);

    mLightInfoWidget->isOpen();
    mLightInfoWidget->scrollArea()->updateHues(mComm->hue()->discovery()->lights());
    mLightInfoWidget->scrollArea()->updateNanoLeafs(mComm->nanoleaf()->lights().items());
    mLightInfoWidget->scrollArea()->updateAruCorLights(mComm->arducor()->arduCorLights());
    mLightInfoWidget->resize();
    mLightInfoWidget->pushIn();
}


void SettingsPage::lightInfoClosePressed() {
    mGreyOut->greyOut(false);
    mLightInfoWidget->pushOut();
}

void SettingsPage::greyOutClicked() {
    if (mLightInfoWidget->isOpen()) {
        lightInfoClosePressed();
    }
}

void SettingsPage::pushIn(const QPoint& startPoint, const QPoint& endPoint) {
    setVisible(true);
    moveWidget(this, startPoint, endPoint);
    raise();
    show();
    isOpen(true);
}

void SettingsPage::pushOut(const QPoint& endPoint) {
    moveWidget(this, pos(), endPoint);
    isOpen(false);
}

void SettingsPage::enableButtons(bool enable) {
    if (enable) {
        for (auto button : mButtons) {
            button->shouldEnable(true);
        }
    } else {
        for (auto button : mButtons) {
            if (button->text() == "Copyright" || button->text() == "Find New Lights") {
                button->shouldEnable(true);
            } else {
                button->shouldEnable(false);
            }
        }
    }
}
