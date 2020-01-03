#include "mainviewport.h"

#include "mainwindow.h"
#include "utils/qt.h"

/*!
 * \copyright
 * Copyright (C) 2015 - 2019.
 * Released under the GNU General Public License.
 */

MainViewport::MainViewport(MainWindow* parent,
                           CommLayer* comm,
                           cor::DeviceList* data,
                           GroupData* groups,
                           AppSettings* settings)
    : QWidget(parent),
      mGroups{groups},
      mComm{comm},
      mData{data},
      mAppSettings{settings} {
    // NOTE: this is mood page so that it doesn't default to light page on so when light page
    //      is turned on, we can use standard functions
    mPageIndex = EPage::moodPage;

    // --------------
    // Setup Pages
    // --------------

    mColorPage = new ColorPage(parent);
    mColorPage->isOpen(false);
    mColorPage->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    connect(mColorPage,
            SIGNAL(routineUpdate(QJsonObject)),
            parent,
            SLOT(routineChanged(QJsonObject)));

    mPalettePage = new PalettePage(parent);
    mPalettePage->isOpen(false);
    mPalettePage->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    connect(mPalettePage, SIGNAL(speedUpdate(int)), parent, SLOT(speedChanged(int)));
    connect(mPalettePage,
            SIGNAL(routineUpdate(QJsonObject)),
            parent,
            SLOT(routineChanged(QJsonObject)));
    connect(mPalettePage,
            SIGNAL(schemeUpdate(std::vector<QColor>)),
            parent,
            SLOT(schemeChanged(std::vector<QColor>)));

    mMoodPage = new MoodPage(parent, groups);
    mMoodPage->isOpen(false);
    mMoodPage->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    connect(mMoodPage,
            SIGNAL(clickedSelectedMood(std::uint64_t)),
            parent,
            SLOT(moodSelected(std::uint64_t)));
    connect(mMoodPage, SIGNAL(clickedEditButton(bool)), parent, SLOT(editButtonClicked(bool)));
    connect(mMoodPage, SIGNAL(moodUpdate(std::uint64_t)), parent, SLOT(moodChanged(std::uint64_t)));
}


void MainViewport::resize(const QRect& geometry) {
    setGeometry(geometry);
    mainWidget(mPageIndex)->setGeometry(geometry);

    QWidget* parent = parentWidget();
    QRect offsetGeometry(parent->width() + geometry.width(),
                         pos().y(),
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
}

void MainViewport::pageChanged(EPage pageIndex, bool skipTransition) {
    if (pageIndex != mPageIndex) {
        showMainPage(pageIndex, skipTransition);
        hideMainPage(mPageIndex);
        mPageIndex = pageIndex;
    }
}

QWidget* MainViewport::mainWidget(EPage page) {
    QWidget* widget;
    switch (page) {
        case EPage::colorPage:
            widget = qobject_cast<QWidget*>(mColorPage);
            break;
        case EPage::moodPage:
            widget = qobject_cast<QWidget*>(mMoodPage);
            break;
        case EPage::palettePage:
            widget = qobject_cast<QWidget*>(mPalettePage);
            break;
        default:
            THROW_EXCEPTION("Widget not supported by main widget");
    }
    Q_ASSERT(widget);
    return widget;
}

cor::Page* MainViewport::mainPage(EPage page) {
    cor::Page* widget;
    switch (page) {
        case EPage::colorPage:
            widget = mColorPage;
            break;
        case EPage::moodPage:
            widget = mMoodPage;
            break;
        case EPage::palettePage:
            widget = mPalettePage;
            break;
        default:
            THROW_EXCEPTION("Widget not recognized by mainviewport");
    }
    Q_ASSERT(widget);
    return widget;
}

void MainViewport::showMainPage(EPage page, bool skipTransition) {
    auto pageObject = mainPage(page);
    auto widget = mainWidget(page);
    auto x = width() + widget->width();
    pageObject->isOpen(true);

    if (skipTransition) {
        widget->setGeometry(pos().x(), pos().y(), widget->width(), widget->height());
    } else {
        cor::moveWidget(widget, QPoint(x, pos().y()), pos());
    }

    if (page == EPage::colorPage) {
        mColorPage->show(mData->mainColor(),
                         uint32_t(mData->brightness()),
                         uint32_t(mData->devices().size()),
                         mComm->bestColorPickerType(mData->devices()));
        mColorPage->setVisible(true);
    } else if (page == EPage::moodPage) {
        loadMoodPage();
        mMoodPage->setVisible(true);
    } else if (page == EPage::palettePage) {
        mPalettePage->resize();
        mPalettePage->show(mData->lightCount(),
                           mData->brightness(),
                           mData->colorScheme(),
                           mData->hasLightWithProtocol(EProtocolType::arduCor),
                           mData->hasLightWithProtocol(EProtocolType::nanoleaf));
        mPalettePage->setVisible(true);
    }
}

void MainViewport::loadMoodPage() {
    mMoodPage->show(mData->findCurrentMood(mGroups->moods()),
                    mGroups->moods(),
                    mGroups->rooms().items());
}

void MainViewport::hideMainPage(EPage page) {
    auto pageObject = mainPage(page);
    auto widget = mainWidget(page);
    pageObject->isOpen(false);
    int x = widget->width() * -1;

    cor::moveWidget(widget, pos(), QPoint(x, widget->pos().y()));
    if (page == EPage::colorPage) {
        mColorPage->handleRoutineWidget(false);
    } else if (page == EPage::palettePage) {
        mPalettePage->handleRoutineWidget(false);
    }
}

void MainViewport::lightCountChanged() {
    if (mPageIndex == EPage::colorPage) {
        mColorPage->show(mData->mainColor(),
                         uint32_t(mData->brightness()),
                         uint32_t(mData->devices().size()),
                         mComm->bestColorPickerType(mData->devices()));
    } else if (mPageIndex == EPage::palettePage) {
        mPalettePage->show(mData->lightCount(),
                           mData->brightness(),
                           mData->colorScheme(),
                           mData->hasLightWithProtocol(EProtocolType::arduCor),
                           mData->hasLightWithProtocol(EProtocolType::nanoleaf));
    }
}
