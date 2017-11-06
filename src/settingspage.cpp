/*!
 * \copyright
 * Copyright (C) 2015 - 2017.
 * Released under the GNU General Public License.
 */

#include "settingspage.h"
#include "comm/commhue.h"
#include "listdevicewidget.h"
#include "corlumautils.h"

#include <QFileDialog>
#include <QDebug>
#include <QSignalMapper>
#include <QScroller>
#include <QMessageBox>
#include <QStandardPaths>
#include <QStyleOption>
#include <QGraphicsOpacityEffect>
#include <QPainter>
#include <QScrollBar>
#include <QPropertyAnimation>

#include <algorithm>


#include <QDesktopWidget>
#include <QScreen>

SettingsPage::SettingsPage(QWidget *parent) :
    QWidget(parent) {
    mShowingDebug = true;

    mCurrentWebView = ECorlumaWebView::eNone;

    //------------
    // Top Layout
    //------------

    mTopWidget = new CorlumaTopWidget("", ":images/closeX.png");
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
    mScrollAreaWidget->setContentsMargins(0,0,0,0);
    mScrollAreaWidget->setStyleSheet("QWidget#contentWidget{ background-color: #201F1F; } QLabel { background-color: #201F1F; } CorlumaSlider { background-color: #201F1F; } ");

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
    mMainLayout->setContentsMargins(9,9,9,9);
    mMainLayout->setSpacing(6);

    //------------
    // Scroll Area Contents
    //------------

    mSectionTitles = { "Data",
                       "About",
                       "Debug"};

    mTitles = { "Hue Info",
                "Discover New Hues",
                "Save",
                "Load",
                "Reset",
                "Copyright",
                "FAQ",
                "Mock Connection"};

    mDescriptions = { "Read and manage the hardware information stored on the Hue Bridge.",
                      "Discover new Hue lights.",
                      "Save light moods and collections to JSON.",
                      "Erase old moods and collections, load new data from JSON.",
                      "Resets all app data and all app settings.",
                      "",
                      "",
                      "Mock a device state update packet to get past the start screen."};

    mButtons = std::vector<SettingsButton*>(mTitles.size());
    mSectionLabels = std::vector<QLabel*>(mSectionTitles.size());

    uint32_t sectionIndex = 0;
    for (uint32_t x = 0; x < mTitles.size(); ++x) {
        if (mTitles[x].compare("Save") == 0
                || mTitles[x].compare("Mock Connection") == 0
                || mTitles[x].compare("Copyright") == 0) {
            mSectionLabels[sectionIndex] = new QLabel(mSectionTitles[sectionIndex].c_str());
            mSectionLabels[sectionIndex]->setStyleSheet("font:bold; font-size:20pt; color:rgba(61, 142, 201,255);");
            mScrollLayout->addWidget(mSectionLabels[sectionIndex]);
            sectionIndex++;
        }
        mButtons[x] = new SettingsButton(QString(mTitles[x].c_str()), QString(mDescriptions[x].c_str()));
        connect(mButtons[x], SIGNAL(buttonPressed(QString)), this, SLOT(settingsButtonPressed(QString)));
        mScrollLayout->addWidget(mButtons[x]);
    }

    mCopyrightWidget = new CorlumaWebView("Copyright", ":/resources/Copyright.html", this);
    mCopyrightWidget->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    mCopyrightWidget->setGeometry(this->geometry());
    connect(mCopyrightWidget, SIGNAL(closePressed()), this, SLOT(hideCurrentWebView()));


    mFAQWidget = new CorlumaWebView("FAQ", ":/resources/FAQ.html", this);
    mFAQWidget->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    mFAQWidget->setGeometry(this->geometry());
    connect(mFAQWidget, SIGNAL(closePressed()), this, SLOT(hideCurrentWebView()));

    //------------
    // Global Widget
    //------------
    mGlobalWidget = new GlobalSettingsWidget(mScrollAreaWidget);
    mGlobalWidget->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    mScrollLayout->addWidget(mGlobalWidget);

    //------------
    // Final Cleanup
    //------------
    this->setLayout(mMainLayout);

    mScrollArea->setWidget(mScrollAreaWidget);
    mScrollAreaWidget->setFixedWidth(mScrollArea->width() * 0.9f);

}

SettingsPage::~SettingsPage() {
}

// ----------------------------
// Protected
// ----------------------------

void SettingsPage::show() {
    mGlobalWidget->updateUI();
    mGlobalWidget->show();

    mCopyrightWidget->setGeometry(this->geometry());
}

void SettingsPage::resizeEvent(QResizeEvent *event) {
    Q_UNUSED(event);

    mScrollAreaWidget->setFixedWidth(this->width() * 0.9f);

    QRect shownWidget = this->geometry();
    QRect hiddenWidget = QRect(0, this->geometry().height(), this->width(), this->height());

    switch (mCurrentWebView) {
        case ECorlumaWebView::eCopyright:
            mCopyrightWidget->setGeometry(shownWidget);
            mFAQWidget->setGeometry(hiddenWidget);
            break;
        case ECorlumaWebView::eFAQ:
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
    const QString downloadsFolder = QStandardPaths::writableLocation(QStandardPaths::DownloadLocation);
    dialog.setDirectory(downloadsFolder);
    QStringList fileNames;
    if (dialog.exec()) {
        fileNames = dialog.selectedFiles();
        for (auto& name : fileNames){
            if (!mGroups->loadExternalData(name)) {
                qDebug() << "WARNING: loading external data failed at " << name;
            }
        }
    }
}

void SettingsPage::saveButtonClicked() {
    QString fileName = QFileDialog::getSaveFileName(this,
          tr("Save Group Data"), "CorlumaGroups.json",
          tr("JSON (*.json)"));
    if (fileName.isEmpty()) {
        qDebug() << "WARNING: save file name empty";
        return;
    }
    if (!mGroups->saveFile(fileName)) {
        qDebug() << "WARNING: Save failed!";
    }
}

void SettingsPage::resetButtonClicked() {
    QMessageBox::StandardButton reply;
    QString text = "Reset all App Data? This will remove the settings, saved collections, and saved moods. This cannot be undone.";
    reply = QMessageBox::question(this, "Reset?", text,
                                  QMessageBox::Yes|QMessageBox::No);
    if (reply == QMessageBox::Yes) {
        resetToDefaults();
    }
}

void SettingsPage::resetToDefaults() {
    mGlobalWidget->checkBoxClicked(ECommType::eHue, true);
    mGlobalWidget->checkBoxClicked(ECommType::eUDP, false);
#ifndef MOBILE_BUILD
    mGlobalWidget->checkBoxClicked(ECommType::eSerial, false);
#endif //MOBILE_BUILD

    mGlobalWidget->advanceModeButtonPressed(false);
    mGlobalWidget->timeoutButtonPressed(true);

    // load no data, deleting everything.
    mGroups->loadExternalData("");
}

void SettingsPage::removeDebug() {
    if (mShowingDebug) {
        mShowingDebug = false;
        // parse section labels for debug and remove it from layout
        for (uint32_t x = 0; x < mSectionLabels.size(); ++x) {
            if (mSectionLabels[x]->text().compare("Debug") == 0) {
                mScrollLayout->removeWidget(mSectionLabels[x]);
                mSectionLabels[x]->setVisible(false);
            }
        }

        // parse section labels for debug and remove it from layout
        for (uint32_t x = 0; x < mButtons.size(); ++x) {
            if (mButtons[x]->text().compare("Mock Connection") == 0) {
                mScrollLayout->removeWidget(mButtons[x]);
                mButtons[x]->setVisible(false);
            }
        }

        // adjust the size to new missing content
        adjustSize();
    }
}

void SettingsPage::paintEvent(QPaintEvent *) {
    QStyleOption opt;
    opt.init(this);
    QPainter painter(this);

    painter.setRenderHint(QPainter::Antialiasing);
    painter.fillRect(this->rect(), QBrush(QColor(48, 47, 47)));
}

void SettingsPage::settingsButtonPressed(QString title) {
   // qDebug() << "settings button pressed: " << title;
    if (title.compare("Debug") == 0) {
        std::list<SLightDevice> debugDevices = mGroups->loadDebugData();
        if (debugDevices.size() > 0) {
            mComm->loadDebugData(debugDevices);
            emit debugPressed();
        } else {
            qDebug() << "WARNING: Debug devices not found!";
        }
    } else if (title.compare("Reset") == 0) {
        resetButtonClicked();
    } else if (title.compare("Load") == 0) {
        loadButtonClicked();
    } else if (title.compare("Save") == 0) {
        saveButtonClicked();
    } else if (title.compare("Discover New Hues") == 0) {
        emit clickedHueDiscovery();
    } else if (title.compare("Hue Info") == 0) {
        emit clickedHueInfoWidget();
    } else if (title.compare("Copyright") == 0) {
        showWebView(ECorlumaWebView::eCopyright);
    } else if (title.compare("FAQ") == 0) {
        showWebView(ECorlumaWebView::eFAQ);
    }
}


void SettingsPage::showWebView(ECorlumaWebView newWebView) {
    if (newWebView != mCurrentWebView) {
        mCurrentWebView = newWebView;

        CorlumaWebView *widget;
        switch (newWebView) {
            case ECorlumaWebView::eCopyright:
                widget = mCopyrightWidget;
                break;
            case ECorlumaWebView::eFAQ:
                widget = mFAQWidget;
                break;
            default:
                // none, return
                return;
        }
        QSize size = this->size();
        widget->setGeometry(0, widget->height(), size.width(), size.height());
        QPropertyAnimation *animation = new QPropertyAnimation(widget, "pos");
        animation->setDuration(TRANSITION_TIME_MSEC);
        animation->setStartValue(widget->pos());
        animation->setEndValue(QPoint(0, 0));
        animation->start();
        widget->raise();
    }
}

void SettingsPage::hideCurrentWebView() {

    CorlumaWebView *widget;
    switch (mCurrentWebView) {
        case ECorlumaWebView::eCopyright:
            widget = mCopyrightWidget;
            break;
        case ECorlumaWebView::eFAQ:
            widget = mFAQWidget;
            break;
        default:
            // none, return
            return;
    }

    QPropertyAnimation *animation = new QPropertyAnimation(widget, "pos");
    animation->setDuration(TRANSITION_TIME_MSEC);
    animation->setStartValue(widget->pos());
    animation->setEndValue(QPoint(0, widget->height()));
    animation->start();
    widget->raise();

    mCurrentWebView = ECorlumaWebView::eNone;
}
