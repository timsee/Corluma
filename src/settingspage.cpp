/*!
 * \copyright
 * Copyright (C) 2015 - 2019.
 * Released under the GNU General Public License.
 */

#include "settingspage.h"
#include "comm/commhue.h"
#include "listlightwidget.h"
#include "mainwindow.h"
#include "utils/qt.h"

#include <QDebug>
#include <QFileDialog>
#include <QGraphicsOpacityEffect>
#include <QMessageBox>
#include <QPainter>
#include <QScrollBar>
#include <QScroller>
#include <QStandardPaths>
#include <QStyleOption>

#include <algorithm>

#include <QDesktopWidget>

SettingsPage::SettingsPage(QWidget* parent, GroupData* parser, AppSettings* appSettings)
    : QWidget(parent), mGroups(parser) {
    mShowingDebug = true;

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

    mTitles = {"Discover Lights", "Light Info", "Save", "Load", "Reset", "Copyright", "FAQ"};

    mDescriptions = {"Discover new lights.",
                     "Read and manage the hardware information.",
                     "Save light moods and collections to JSON.",
                     "Erase old moods and collections, load new data from JSON.",
                     "Resets all app data and all app settings.",
                     "",
                     ""};

    mButtons = std::vector<SettingsButton*>(mTitles.size());
    mSectionLabels = std::vector<QLabel*>(mSectionTitles.size());

    uint32_t sectionIndex = 0;
    for (uint32_t x = 0; x < mTitles.size(); ++x) {
        if (mTitles[x] == "Save" || mTitles[x] == "Mock Connection" || mTitles[x] == "Copyright") {
            mSectionLabels[sectionIndex] = new QLabel(mSectionTitles[sectionIndex].c_str());
            mSectionLabels[sectionIndex]->setStyleSheet(
                "font:bold; font-size:20pt; color:rgba(61, 142, 201,255);");
            mScrollLayout->addWidget(mSectionLabels[sectionIndex]);
            sectionIndex++;
        }
        mButtons[x] = new SettingsButton(
            QString(mTitles[x].c_str()), QString(mDescriptions[x].c_str()), this);
        connect(mButtons[x],
                SIGNAL(buttonPressed(QString)),
                this,
                SLOT(settingsButtonPressed(QString)));
        mScrollLayout->addWidget(mButtons[x]);
    }

    mCopyrightWidget = new cor::WebView("Copyright", ":/resources/Copyright.html", this);
    mCopyrightWidget->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    mCopyrightWidget->setGeometry(this->geometry());
    connect(mCopyrightWidget, SIGNAL(closePressed()), this, SLOT(hideCurrentWebView()));


    mFAQWidget = new cor::WebView("FAQ", ":/resources/FAQ.html", this);
    mFAQWidget->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    mFAQWidget->setGeometry(this->geometry());
    connect(mFAQWidget, SIGNAL(closePressed()), this, SLOT(hideCurrentWebView()));

    //------------
    // Global Widget
    //------------
    mGlobalWidget = new GlobalSettingsWidget(mScrollAreaWidget, appSettings);
    mGlobalWidget->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    mScrollLayout->addWidget(mGlobalWidget);

    //------------
    // Final Cleanup
    //------------
    this->setLayout(mMainLayout);

    mScrollArea->setWidget(mScrollAreaWidget);
}

// ----------------------------
// Protected
// ----------------------------

void SettingsPage::show() {
    mGlobalWidget->updateUI();
    mGlobalWidget->show();

    mCopyrightWidget->setGeometry(this->geometry());
}

void SettingsPage::resizeEvent(QResizeEvent* event) {
    Q_UNUSED(event);

    mScrollAreaWidget->setFixedWidth(int(this->width() * 0.85f));

    QRect shownWidget = this->geometry();
    QRect hiddenWidget = QRect(0, this->geometry().height(), this->width(), this->height());

    switch (mCurrentWebView) {
        case ECorlumaWebView::copyright:
            mCopyrightWidget->setGeometry(shownWidget);
            mFAQWidget->setGeometry(hiddenWidget);
            break;
        case ECorlumaWebView::FAQ:
            mCopyrightWidget->setGeometry(hiddenWidget);
            mFAQWidget->setGeometry(shownWidget);
            break;
        default:
            mCopyrightWidget->setGeometry(hiddenWidget);
            mFAQWidget->setGeometry(hiddenWidget);
            return;
    }
}


void SettingsPage::loadButtonClicked() {
    QFileDialog dialog(this);
    dialog.setFileMode(QFileDialog::AnyFile);
    dialog.setNameFilter(tr("JSON (*.json)"));
    dialog.setViewMode(QFileDialog::Detail);
    dialog.setStyleSheet("color:silver;");
    const QString downloadsFolder
        = QStandardPaths::writableLocation(QStandardPaths::DownloadLocation);
    dialog.setDirectory(downloadsFolder);
    QStringList fileNames;
    if (dialog.exec()) {
        fileNames = dialog.selectedFiles();
        for (auto& name : fileNames) {
            if (!mGroups->loadExternalData(name)) {
                qDebug() << "WARNING: loading external data failed at " << name;
            }
        }
    }
}

void SettingsPage::saveButtonClicked() {
    QString fileName = QFileDialog::getSaveFileName(
        this, tr("Save Group Data"), "CorlumaGroups.json", tr("JSON (*.json)"));
    if (fileName.isEmpty()) {
        qDebug() << "WARNING: save file name empty";
        return;
    }


    //    if (!mGroups->saveFile(fileName)) {
    //        qDebug() << "WARNING: Save failed!";
    //    }
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

    mGlobalWidget->timeoutButtonPressed(true);

    // load no data, deleting everything.
    mGroups->loadExternalData("");
}

void SettingsPage::paintEvent(QPaintEvent*) {
    QStyleOption opt;
    opt.init(this);
    QPainter painter(this);

    painter.setRenderHint(QPainter::Antialiasing);
    painter.fillRect(this->rect(), QBrush(QColor(48, 47, 47)));
}

void SettingsPage::settingsButtonPressed(const QString& title) {
    // qDebug() << "settings button pressed: " << title;
    if (title.compare("Reset") == 0) {
        resetButtonClicked();
    } else if (title.compare("Load") == 0) {
        loadButtonClicked();
    } else if (title.compare("Save") == 0) {
        saveButtonClicked();
    } else if (title.compare("Light Info") == 0) {
        emit clickedInfoWidget();
    } else if (title.compare("Discover Lights") == 0) {
        emit clickedDiscovery();
    } else if (title.compare("Copyright") == 0) {
        showWebView(ECorlumaWebView::copyright);
    } else if (title.compare("FAQ") == 0) {
        showWebView(ECorlumaWebView::FAQ);
    } else if (title.compare("Mock Connection") == 0) {
        auto mainWindow = qobject_cast<MainWindow*>(this->parentWidget());
        Q_ASSERT(mainWindow);
        mainWindow->anyDiscovered(true);
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
            case ECorlumaWebView::FAQ:
                widget = mFAQWidget;
                break;
            default:
                // none, return
                return;
        }

        cor::moveWidget(widget, this->size(), QPoint(0, widget->height()), QPoint(0, 0));

        widget->raise();
    }
}

void SettingsPage::hideCurrentWebView() {
    cor::WebView* widget;
    switch (mCurrentWebView) {
        case ECorlumaWebView::copyright:
            widget = mCopyrightWidget;
            break;
        case ECorlumaWebView::FAQ:
            widget = mFAQWidget;
            break;
        default:
            // none, return
            return;
    }

    cor::moveWidget(widget, this->size(), QPoint(0, widget->height()), QPoint(0, 0));

    mCurrentWebView = ECorlumaWebView::none;
}

void SettingsPage::pushIn(const QSize& size, const QPoint& startPoint, const QPoint& endPoint) {
    this->setVisible(true);
    moveWidget(this, size, startPoint, endPoint);
    this->raise();
    this->show();
    this->isOpen(true);
}

void SettingsPage::pushOut(const QPoint& endPoint) {
    moveWidget(this, this->size(), this->pos(), endPoint);
    this->isOpen(false);
}
