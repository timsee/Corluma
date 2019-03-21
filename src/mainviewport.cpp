#include "mainviewport.h"
#include "mainwindow.h"

#include "utils/qt.h"

/*!
 * \copyright
 * Copyright (C) 2015 - 2019.
 * Released under the GNU General Public License.
 */

MainViewport::MainViewport(MainWindow *parent,
                           CommLayer *comm,
                           cor::DeviceList *data,
                           GroupData *groups,
                           AppSettings *settings) : QWidget(parent), mGroups{groups}, mComm{comm}, mData{data}, mAppSettings{settings} {
    //NOTE: this is mood page so that it doesn't default to light page on so when light page
    //      is turned on, we can use standard functions
    mPageIndex = EPage::moodPage;

    // --------------
    // Setup Pages
    // --------------

    mLightPage = new LightPage(parent, data, comm, groups, settings);
    mLightPage->isOpen(false);
    mLightPage->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    mLightPage->setVisible(false);

    mColorPage = new ColorPage(parent);
    mColorPage->isOpen(false);
    mColorPage->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    connect(mColorPage, SIGNAL(routineUpdate(QJsonObject)),  parent, SLOT(routineChanged(QJsonObject)));
    connect(mColorPage, SIGNAL(schemeUpdate(std::vector<QColor>)),  parent, SLOT(schemeChanged(std::vector<QColor>)));

    mPalettePage = new PalettePage(parent);
    mPalettePage->isOpen(false);
    mPalettePage->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    connect(mPalettePage, SIGNAL(speedUpdate(int)),  parent, SLOT(speedChanged(int)));
    connect(mPalettePage, SIGNAL(routineUpdate(QJsonObject)),  parent, SLOT(routineChanged(QJsonObject)));

    mMoodPage = new MoodPage(parent, groups);
    mMoodPage->isOpen(false);
    mMoodPage->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    connect(mMoodPage, SIGNAL(clickedSelectedMood(std::uint64_t)), parent, SLOT(moodSelected(std::uint64_t)));
    connect(mMoodPage, SIGNAL(clickedEditButton(bool)),  parent, SLOT(editButtonClicked(bool)));
    connect(mMoodPage, SIGNAL(moodUpdate(std::uint64_t)),  parent, SLOT(moodChanged(std::uint64_t)));


    connect(mLightPage, SIGNAL(changedDeviceCount()), this, SLOT(lightCountChanged()));
    connect(mMoodPage, SIGNAL(changedDeviceCount()), this, SLOT(lightCountChanged()));
}


void MainViewport::resize(const QRect& geometry) {
    this->setGeometry(geometry);
    mainWidget(mPageIndex)->setGeometry(geometry);

    QWidget *parentWidget = qobject_cast<QWidget*>(this->parent());
    QRect offsetGeometry(parentWidget->width() + geometry.width(),
                         this->pos().y(),
                         this->geometry().width(),
                         this->geometry().height());

    if (mPageIndex != EPage::colorPage) {
        mColorPage->setGeometry(offsetGeometry);
    }

    if (mPageIndex != EPage::palettePage) {
        mPalettePage->setGeometry(offsetGeometry);
    }

    if (mPageIndex != EPage::moodPage) {
        mMoodPage->setGeometry(offsetGeometry);
    }

    if (mPageIndex != EPage::lightPage) {
        mLightPage->setGeometry(offsetGeometry);
    }
}

void MainViewport::pageChanged(EPage pageIndex) {
    if (pageIndex != mPageIndex) {
        showMainPage(pageIndex);
        hideMainPage(mPageIndex);
        mPageIndex = pageIndex;
    }
}

QWidget* MainViewport::mainWidget(EPage page) {
    QWidget *widget;
    switch (page) {
        case EPage::colorPage:
            widget = qobject_cast<QWidget*>(mColorPage);
            break;
        case EPage::lightPage:
            widget = qobject_cast<QWidget*>(mLightPage);
            break;
        case EPage::moodPage:
            widget = qobject_cast<QWidget*>(mMoodPage);
            break;
        case EPage::palettePage:
            widget = qobject_cast<QWidget*>(mPalettePage);
            break;
    }
    Q_ASSERT(widget);
    return widget;
}

cor::Page* MainViewport::mainPage(EPage page) {
    cor::Page *widget;
    switch (page) {
        case EPage::colorPage:
            widget = mColorPage;
            break;
        case EPage::lightPage:
            widget = mLightPage;
            break;
        case EPage::moodPage:
            widget = mMoodPage;
            break;
        case EPage::palettePage:
            widget = mPalettePage;
            break;
    }
    Q_ASSERT(widget);
    return widget;
}

void MainViewport::showMainPage(EPage page) {
    auto pageObject = mainPage(page);
    auto widget = mainWidget(page);
    int x = this->width() + widget->width();
    pageObject->isOpen(true);
        //widget->raise();

    cor::moveWidget(widget,
                    this->size(),
                    QPoint(x, this->pos().y()),
                    this->pos());

    if (page == EPage::lightPage) {
        mLightPage->show();
        mLightPage->setVisible(true);
    } else if (page == EPage::colorPage) {
        mColorPage->show(mData->mainColor(),
                         uint32_t(mData->brightness()),
                         mData->colorScheme(),
                         mData->palette(),
                         mData->devices().size());
        mColorPage->setVisible(true);
    } else if (page == EPage::moodPage) {
        mMoodPage->show(mData->findCurrentMood(mGroups->moods()),
                        mGroups->moods(),
                        mGroups->roomList());
        mMoodPage->setVisible(true);
    } else if (page == EPage::palettePage) {
        mPalettePage->resize();
        mPalettePage->show(mData->mainColor(),
                           mData->devices().size(),
                           mData->hasLightWithProtocol(EProtocolType::arduCor),
                           mData->hasLightWithProtocol(EProtocolType::nanoleaf));
        mPalettePage->setVisible(true);
    }
}

void MainViewport::hideMainPage(EPage page) {
    auto pageObject = mainPage(page);
    auto widget = mainWidget(page);
    pageObject->isOpen(false);
    int x = widget->width() * -1;

    cor::moveWidget(widget,
                    this->size(),
                    this->pos(),
                    QPoint(x, widget->pos().y()));

    if (page == EPage::lightPage) {
        mLightPage->hide();
    }
}

void MainViewport::lightCountChanged() {
    if (mPageIndex == EPage::colorPage) {
        mColorPage->show(mData->mainColor(),
                         uint32_t(mData->brightness()),
                         mData->colorScheme(),
                         mData->palette(),
                         mData->devices().size());
    } else if (mPageIndex == EPage::palettePage) {
        mPalettePage->show(mData->mainColor(),
                           mData->devices().size(),
                           mData->hasLightWithProtocol(EProtocolType::arduCor),
                           mData->hasLightWithProtocol(EProtocolType::nanoleaf));
    }
}

