#include "mainviewport.h"

#include "mainwindow.h"
#include "utils/qt.h"

/*!
 * \copyright
 * Copyright (C) 2015 - 2020.
 * Released under the GNU General Public License.
 */

MainViewport::MainViewport(MainWindow* parent,
                           CommLayer* comm,
                           cor::LightList* data,
                           AppData* appData,
                           PaletteData* palettes,
                           AppSettings* settings,
                           DataSyncTimeout* dataSyncTimeout)
    : QWidget(parent),
      mComm{comm},
      mData{data},
      mAppData{appData},
      mAppSettings{settings},
      mMainWindow{parent},
      mLightsPage{new LightsPage(parent, comm, data, settings)},
      mColorPage{new ColorPage(parent)},
      mPalettePage{new PalettePage(parent, palettes)},
      mMoodPage{new MoodPage(parent, appData, comm)},
      mTimeoutPage{new TimeoutPage(parent, comm, data, dataSyncTimeout)},
      mSettingsPage{new SettingsPage(parent, mAppData, mComm, mAppSettings)} {
    // NOTE: this is mood page so that it doesn't default to light page on so when light page
    //      is turned on, we can use standard functions
    mPageIndex = EPage::moodPage;

    // --------------
    // Setup Pages
    // --------------

    mLightsPage->isOpen(true);
    mLightsPage->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);

    mColorPage->isOpen(false);
    mColorPage->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);

    mPalettePage->isOpen(false);
    mPalettePage->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);

    mMoodPage->isOpen(false);
    mMoodPage->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    connect(mMoodPage, SIGNAL(clickedEditButton(bool)), parent, SLOT(editButtonClicked(bool)));

    mTimeoutPage->isOpen(false);
    mTimeoutPage->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);

    mSettingsPage->isOpen(false);
    mSettingsPage->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
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

    if (mPageIndex != EPage::lightsPage) {
        mLightsPage->setGeometry(offsetGeometry);
    }

    if (mPageIndex != EPage::palettePage) {
        mPalettePage->setGeometry(offsetGeometry);
    }

    if (mPageIndex != EPage::moodPage) {
        mMoodPage->setGeometry(offsetGeometry);
    }

    if (mPageIndex != EPage::timeoutPage) {
        mTimeoutPage->setGeometry(offsetGeometry);
    }

    if (mPageIndex != EPage::settingsPage) {
        mSettingsPage->setGeometry(offsetGeometry);
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
        case EPage::timeoutPage:
            widget = qobject_cast<QWidget*>(mTimeoutPage);
            break;
        case EPage::lightsPage:
            widget = qobject_cast<QWidget*>(mLightsPage);
            break;
        case EPage::settingsPage:
            widget = qobject_cast<QWidget*>(mSettingsPage);
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
        case EPage::timeoutPage:
            widget = mTimeoutPage;
            break;
        case EPage::lightsPage:
            widget = mLightsPage;
            break;
        case EPage::settingsPage:
            widget = mSettingsPage;
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

    if (page != EPage::lightsPage) {
        mLightsPage->hideWidgets();
    }

    if (page != EPage::settingsPage) {
        mSettingsPage->hideWidget();
    }

    if (page == EPage::colorPage) {
        mColorPage->update(mData->mainColor(),
                           mData->brightness(),
                           mData->lights().size(),
                           mComm->bestColorPickerType(mData->lights()));
        mColorPage->setVisible(true);
    } else if (page == EPage::moodPage) {
        loadMoodPage();
        mMoodPage->setVisible(true);
    } else if (page == EPage::palettePage) {
        mPalettePage->resize();
        mPalettePage->update(mData->lightCount(), mData->multiColorScheme());
        mPalettePage->setVisible(true);
    } else if (page == EPage::timeoutPage) {
        mTimeoutPage->update(mAppSettings->timeoutEnabled(), mAppSettings->timeout());
        mTimeoutPage->setVisible(true);
    } else if (page == EPage::lightsPage) {
        mLightsPage->showWidgets();
    } else if (page == EPage::settingsPage) {
        mSettingsPage->showWidget();
    }
}

void MainViewport::loadMoodPage() {
    mMoodPage->show(mData->findCurrentMood(mAppData->moods()->moods()));
}

void MainViewport::hideMainPage(EPage page) {
    auto pageObject = mainPage(page);
    auto widget = mainWidget(page);
    pageObject->isOpen(false);
    int x = widget->width() * -1;

    cor::moveWidget(widget, pos(), QPoint(x, widget->pos().y()));
}
