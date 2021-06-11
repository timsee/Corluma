/*!
 * \copyright
 * Copyright (C) 2015 - 2020.
 * Released under the GNU General Public License.
 */

#include "topmenu.h"

#include <QGraphicsEffect>

#include "lightspage.h"
#include "mainwindow.h"
#include "topmenu.h"
#include "utils/exception.h"
#include "utils/qt.h"

TopMenu::TopMenu(QWidget* parent,
                 cor::LightList* data,
                 CommLayer* comm,
                 AppSettings* appSettings,
                 MainWindow* mainWindow,
                 LightsPage* lightsPage,
                 PalettePage* palettePage,
                 ColorPage* colorPage)
    : QWidget(parent),
      mStartSelectLightsButton{int(cor::applicationSize().height() * 0.1f)},
      mLastParentSizeColorMenu{0, 0},
      mColorMenuType{EColorMenuType::none},
      mFloatingMenuStart{int(cor::applicationSize().height() * 0.1f)},
      mData(data),
      mComm(comm),
      mAppSettings{appSettings},
      mMainWindow(mainWindow),
      mPalettePage(palettePage),
      mColorPage(colorPage),
      mLightsPage{lightsPage},
      mGlobalStateWidget{new GlobalStateWidget(this)},
      mCurrentPage{EPage::colorPage},
      mSize{QSize(int(cor::applicationSize().height() * 0.08f),
                  int(cor::applicationSize().height() * 0.08f))},
      mLastColorButtonKey{"HSV"},
      mRenderTimer{new QTimer(this)},
      mMenuButton{new QPushButton(this)},
      mGlobalBrightness{new GlobalBrightnessWidget(mSize, mComm, mData, this)},
      mSingleLightBrightness{new SingleLightBrightnessWidget(mSize, this)},
      mPaletteFloatingLayout{new FloatingLayout(mMainWindow)},
      mPaletteAndRoutineFloatingLayout{new FloatingLayout(mMainWindow)},
      mMoodsFloatingLayout{new FloatingLayout(mMainWindow)},
      mColorFloatingLayout{new FloatingLayout(mMainWindow)},
      mColorAndRoutineFloatingLayout{new FloatingLayout(mMainWindow)},
      mTimeoutFloatingLayout{new FloatingLayout(mMainWindow)},
      mAddLightsFloatingLayout{new FloatingLayout(mMainWindow)},
      mSingleColorStateWidget{new SingleColorStateWidget(mMainWindow)},
      mMultiColorStateWidget{new MultiColorStateWidget(mMainWindow)},
      mSelectLightsButton{new SelectLightsButton(this)},
      mDiscoveryTopMenu{new DiscoveryTopMenu(mMainWindow, mAppSettings, mLightsPage)} {
    mDiscoveryTopMenu->setVisible(false);
    // start render timer
    connect(mRenderTimer, SIGNAL(timeout()), this, SLOT(updateUI()));
    mRenderTimer->start(100);

    // --------------
    // Setup menu button
    // --------------
    mMenuButton->setVisible(true);
    connect(mMenuButton, SIGNAL(clicked(bool)), this, SLOT(menuButtonPressed()));
    mMenuButton->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    mMenuButton->setStyleSheet("border:none;");

    // --------------
    // Setup Brightness Slider
    // --------------
    mGlobalBrightness->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    mSingleLightBrightness->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);

    // --------------
    // Palette Floating Layout
    // --------------
    connect(mPaletteFloatingLayout,
            SIGNAL(buttonPressed(QString)),
            this,
            SLOT(floatingLayoutButtonPressed(QString)));
    mPaletteFloatingLayout->setupButtons(
        {QString("Preset"), QString("HSV"), QString("New_Palette")},
        EButtonSize::small);
    mPaletteFloatingLayout->highlightButton("Preset");

    connect(mPaletteAndRoutineFloatingLayout,
            SIGNAL(buttonPressed(QString)),
            this,
            SLOT(floatingLayoutButtonPressed(QString)));
    mPaletteAndRoutineFloatingLayout->setupButtons(
        {QString("Preset"), QString("HSV"), QString("New_Palette"), QString("Routine")},
        EButtonSize::small);
    mPaletteAndRoutineFloatingLayout->highlightButton("Preset");

    // --------------
    // Moods Floating Layout
    // --------------
    connect(mMoodsFloatingLayout,
            SIGNAL(buttonPressed(QString)),
            this,
            SLOT(floatingLayoutButtonPressed(QString)));
    mMoodsFloatingLayout->setupButtons({QString("New_Group")}, EButtonSize::small);

    // --------------
    // Color Page Floating Layouts
    // --------------
    connect(mColorFloatingLayout,
            SIGNAL(buttonPressed(QString)),
            this,
            SLOT(floatingLayoutButtonPressed(QString)));
    mColorFloatingLayout->setupButtons({QString("HSV"), QString("Temperature")},
                                       EButtonSize::small);
    mColorFloatingLayout->highlightButton("HSV");


    connect(mColorAndRoutineFloatingLayout,
            SIGNAL(buttonPressed(QString)),
            this,
            SLOT(floatingLayoutButtonPressed(QString)));
    mColorAndRoutineFloatingLayout->setupButtons(
        {QString("HSV"), QString("Temperature"), QString("Routine")},
        EButtonSize::small);
    mColorAndRoutineFloatingLayout->highlightButton("HSV");


    connect(mTimeoutFloatingLayout,
            SIGNAL(buttonPressed(QString)),
            this,
            SLOT(floatingLayoutButtonPressed(QString)));
    mTimeoutFloatingLayout->setupButtons({}, EButtonSize::small);

    mSingleColorStateWidget->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    mSingleColorStateWidget->setFixedSize(mSize.width(), mSize.height() / 2);
    mSingleColorStateWidget->setVisible(false);
    showSingleColorStateWidget(false);

    mMultiColorStateWidget->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    mMultiColorStateWidget->setFixedSize(int(mSize.width() * 2.2), mSize.height() / 2);
    mMultiColorStateWidget->setVisible(false);
    showMultiColorStateWidget(false);


    // --------------
    // Lights Menus
    // --------------

    connect(mAddLightsFloatingLayout,
            SIGNAL(buttonPressed(QString)),
            this,
            SLOT(floatingLayoutButtonPressed(QString)));
    std::vector<QString> verticalButtons = {QString("Plus"), QString("Help")};
    mAddLightsFloatingLayout->setupButtons(verticalButtons, EButtonSize::small);
    mAddLightsFloatingLayout->useDarkTheme();

    connect(mDiscoveryTopMenu,
            SIGNAL(buttonPressed(QString)),
            this,
            SLOT(floatingLayoutButtonPressed(QString)));


    mGlobalStateWidget->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    mGlobalStateWidget->setVisible(true);

    // --------------
    // Select Lights Button
    // --------------
    mSelectLightsButton->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    connect(mSelectLightsButton, SIGNAL(pressed()), this, SLOT(menuButtonPressed()));
    QFontMetrics fm(mSelectLightsButton->font());
    auto textWidth = int(fm.horizontalAdvance(mSelectLightsButton->text()) * 1.1);
    auto selectLightsWidth = int(mSize.width() * 2.5);
    if (selectLightsWidth < textWidth) {
        selectLightsWidth = textWidth;
    }
    mSelectLightsButton->setFixedSize(selectLightsWidth, int(mSize.height() * 0.5));

    showFloatingLayout(mCurrentPage);

    // update the lights menu to reflect the proper state
    updateLightsMenu();

    moveHiddenLayouts();
    hideMenuButton(mMainWindow->leftHandMenu()->alwaysOpen());
    resize(0);
    handleButtonLayouts();
}

void TopMenu::lightCountChanged() {
    // handle select lights button
    if (mData->empty() && !mMainWindow->leftHandMenu()->isIn()
        && !mMainWindow->leftHandMenu()->alwaysOpen()
        && (mCurrentPage != EPage::settingsPage && mCurrentPage != EPage::lightsPage)) {
        mSelectLightsButton->pushIn(mStartSelectLightsButton);
    } else if (!mMainWindow->leftHandMenu()->alwaysOpen()) {
        mSelectLightsButton->pushOut(mStartSelectLightsButton);
    }

    handleBrightnessSliders();

    // handle standard cases
    if (mData->empty()) {
        showSingleColorStateWidget(false);

        if (mCurrentPage == EPage::colorPage) {
            showSingleColorStateWidget(false);
        } else if (mCurrentPage == EPage::palettePage) {
            showMultiColorStateWidget(false);
        }

        if ((mCurrentPage == EPage::colorPage || mCurrentPage == EPage::palettePage)
            && !mMainWindow->leftHandMenu()->alwaysOpen() && !mSelectLightsButton->isIn()) {
            mSelectLightsButton->pushIn(mStartSelectLightsButton);
        }
    }

    if (mCurrentPage == EPage::colorPage) {
        moveColorPageMenus(false);
        mColorPage->update(mData->mainColor(),
                           mData->brightness(),
                           mData->lights().size(),
                           mComm->bestColorPickerType(mData->lights()));
    } else if (mCurrentPage == EPage::palettePage) {
        movePalettePageMenus(false);
        showMultiColorStateWidget(!mData->empty());
        mMultiColorStateWidget->updateState(mData->multiColorScheme());
        mPalettePage->update(mData->lightCount(), mData->multiColorScheme());
    }
}

void TopMenu::highlightButton(const QString& key) {
    mPaletteFloatingLayout->highlightButton(key);
    mPaletteAndRoutineFloatingLayout->highlightButton(key);
    mMoodsFloatingLayout->highlightButton(key);
    mColorFloatingLayout->highlightButton(key);
    mColorAndRoutineFloatingLayout->highlightButton(key);
    mTimeoutFloatingLayout->highlightButton(key);
}

void TopMenu::showMenu() {
    setVisible(true);

    raise();
    mColorFloatingLayout->raise();
    mColorAndRoutineFloatingLayout->raise();
    mPaletteFloatingLayout->raise();
    mPaletteAndRoutineFloatingLayout->raise();
    mMoodsFloatingLayout->raise();
    mTimeoutFloatingLayout->raise();
    mSingleColorStateWidget->raise();
    mMultiColorStateWidget->raise();
    mDiscoveryTopMenu->raise();
    mAddLightsFloatingLayout->raise();
    mGlobalStateWidget->raise();
}

void TopMenu::resize(int xOffset) {
    auto parentSize = parentWidget()->size();
    moveFloatingLayout();


    auto yPos = 0;
    int padding = 5;
    // global state widget is always in top right
    QSize globalStateWidgetSize = QSize(width(), mSize.height() * 0.1);
    mGlobalStateWidget->setGeometry(0,
                                    0,
                                    globalStateWidgetSize.width(),
                                    globalStateWidgetSize.height());
    yPos += mGlobalStateWidget->height();
    mMenuStart = yPos;

    auto xPosTop = 0;
    mMenuButton->setGeometry(0, yPos, mSize.width(), mSize.height());
    cor::resizeIcon(mMenuButton, ":/images/hamburger_icon.png", 0.8f);
    xPosTop += mMenuButton->x() + mMenuButton->width();

    auto shownSlider = brightnessSliderRect();
    auto hiddenSlider = QRect(shownSlider.topLeft().x(),
                              -1 * shownSlider.height(),
                              shownSlider.width(),
                              shownSlider.height());
    if (mGlobalBrightness->isIn()) {
        mGlobalBrightness->setGeometry(shownSlider);
    } else {
        mGlobalBrightness->setGeometry(hiddenSlider);
    }

    if (mSingleLightBrightness->isIn()) {
        mSingleLightBrightness->setGeometry(shownSlider);
    } else {
        mSingleLightBrightness->setGeometry(hiddenSlider);
    }

    yPos += mMenuButton->height() + padding;
    mStartSelectLightsButton = yPos;
    mFloatingMenuStart = yPos;
    yPos += mSize.height() * 0.6;

    mSelectLightsButton->resize(mStartSelectLightsButton);

    setGeometry(xOffset, 0, parentSize.width() - xOffset, yPos);
}


void TopMenu::resizeEvent(QResizeEvent*) {
    resize(geometry().x());
}

//--------------------
// FloatingLayout Setup
//--------------------

void TopMenu::floatingLayoutButtonPressed(const QString& button) {
    if (button == "New_Group") {
        if (mCurrentPage == EPage::moodPage) {
            mMainWindow->editButtonClicked(true);
        } else {
            mMainWindow->editButtonClicked(false);
        }
        highlightButton("");
    } else if (button == "Preset") {
        mPalettePage->setMode(EGroupMode::presets);
        handleBrightnessSliders();
    } else if (button == "HSV") {
        if (mColorPage->isOpen()) {
            mLastColorButtonKey = button;
            mColorPage->showRoutines(false);
            mColorPage->changePageType(ESingleColorPickerMode::HSV);
        }
        if (mPalettePage->isOpen()) {
            mPalettePage->setMode(EGroupMode::wheel);
            handleBrightnessSliders();
        }
    } else if (button == "Temperature") {
        mLastColorButtonKey = button;
        mColorPage->showRoutines(false);
        mColorPage->changePageType(ESingleColorPickerMode::ambient);
    } else if (button == "Routine") {
        if (mColorPage->isOpen()) {
            mColorPage->showRoutines(true);
            mColorFloatingLayout->highlightButton("");
        } else if (mPalettePage->isOpen()) {
            mPalettePage->setMode(EGroupMode::routines);
            mPaletteFloatingLayout->highlightButton("");
        }
    } else if (button == "Settings") {
        emit buttonPressed("Settings");
    } else if (button.contains("Discovery_")) {
        emit buttonPressed(button);
    } else if (button.contains("Plus") || button.contains("Help")) {
        emit buttonPressed(button);
        mAddLightsFloatingLayout->highlightButton("");
    } else if (button == "New_Palette") {
        mPalettePage->pushInNewPalettePage();
    } else {
        qDebug() << "I don't recognize that button type..." << button;
    }
}

void TopMenu::closeRoutinesPage() {
    mColorFloatingLayout->highlightButton("HSV");
    mColorAndRoutineFloatingLayout->highlightButton("HSV");

    mPaletteFloatingLayout->highlightButton("Preset");
    mPaletteAndRoutineFloatingLayout->highlightButton("Preset");
}

int TopMenu::widthOffsetFromLeftMenu() {
    int width = 0;
    if (mMainWindow->leftHandMenu()->alwaysOpen()) {
        width = mMainWindow->leftHandMenu()->width();
    }
    return width;
}

QPoint TopMenu::colorStateStartPoint() {
    return QPoint(widthOffsetFromLeftMenu(), mSize.height() * 2 / 3);
}


QRect TopMenu::brightnessSliderRect() {
    int offsetForMenuButton = 0;
    if (!mMainWindow->leftHandMenu()->alwaysOpen()) {
        offsetForMenuButton += mMenuButton->width();
    }
    auto topLeft =
        QPoint(offsetForMenuButton, mGlobalStateWidget->height() + mSize.height() * 0.025);

    auto sliderWidth = width() - topLeft.x() - mSize.width() - offsetForMenuButton;
    if (mMainWindow->leftHandMenu()->alwaysOpen()) {
        sliderWidth -= mSize.width();
    } else {
        sliderWidth += mSize.width() / 2;
    }
    auto brightnessHeight = mSize.height() * 0.75;

    auto brightnessOffset = (mSize.height() - brightnessHeight) / 2;
    return QRect(topLeft.x(), topLeft.y() + brightnessOffset, sliderWidth, brightnessHeight);
}

void TopMenu::moveColorPageMenus(bool skipTransition) {
    if (mData->empty()) {
        if (skipTransition) {
            mColorFloatingLayout->setGeometry(-1 * mColorFloatingLayout->width(),
                                              mFloatingMenuStart,
                                              mColorFloatingLayout->width(),
                                              mColorFloatingLayout->height());
            mColorAndRoutineFloatingLayout->setGeometry(
                -1 * mColorAndRoutineFloatingLayout->width(),
                mFloatingMenuStart,
                mColorAndRoutineFloatingLayout->width(),
                mColorAndRoutineFloatingLayout->height());
        } else {
            leftPushOutFloatingLayout(mColorAndRoutineFloatingLayout);
            leftPushOutFloatingLayout(mColorFloatingLayout);
        }
    } else {
        if (shouldShowRoutineWidget()) {
            if (skipTransition) {
                mColorFloatingLayout->setGeometry(-1 * mColorFloatingLayout->width(),
                                                  mFloatingMenuStart,
                                                  mColorFloatingLayout->width(),
                                                  mColorFloatingLayout->height());
                mColorAndRoutineFloatingLayout->setGeometry(
                    widthOffsetFromLeftMenu(),
                    mFloatingMenuStart,
                    mColorAndRoutineFloatingLayout->width(),
                    mColorAndRoutineFloatingLayout->height());
            } else {
                leftPushInFloatingLayout(mColorAndRoutineFloatingLayout);
                leftPushOutFloatingLayout(mColorFloatingLayout);
            }
        } else {
            if (skipTransition) {
                mColorFloatingLayout->setGeometry(widthOffsetFromLeftMenu(),
                                                  mFloatingMenuStart,
                                                  mColorFloatingLayout->width(),
                                                  mColorFloatingLayout->height());
                mColorAndRoutineFloatingLayout->setGeometry(
                    -1 * mColorAndRoutineFloatingLayout->width(),
                    mFloatingMenuStart,
                    mColorAndRoutineFloatingLayout->width(),
                    mColorAndRoutineFloatingLayout->height());
            } else {
                leftPushInFloatingLayout(mColorFloatingLayout);
                leftPushOutFloatingLayout(mColorAndRoutineFloatingLayout);
            }
        }
    }
}


void TopMenu::movePalettePageMenus(bool skipTransition) {
    if (mData->empty()) {
        if (skipTransition) {
            mPaletteFloatingLayout->setGeometry(-1 * mPaletteFloatingLayout->width(),
                                                mFloatingMenuStart,
                                                mPaletteFloatingLayout->width(),
                                                mPaletteFloatingLayout->height());
            mPaletteAndRoutineFloatingLayout->setGeometry(
                -1 * mPaletteAndRoutineFloatingLayout->width(),
                mFloatingMenuStart,
                mPaletteAndRoutineFloatingLayout->width(),
                mPaletteAndRoutineFloatingLayout->height());
        } else {
            leftPushOutFloatingLayout(mPaletteAndRoutineFloatingLayout);
            leftPushOutFloatingLayout(mPaletteFloatingLayout);
        }
    } else {
        if (shouldShowRoutineWidget()) {
            if (skipTransition) {
                mPaletteFloatingLayout->setGeometry(-1 * mPaletteFloatingLayout->width(),
                                                    mFloatingMenuStart,
                                                    mPaletteFloatingLayout->width(),
                                                    mPaletteFloatingLayout->height());
                mPaletteAndRoutineFloatingLayout->setGeometry(
                    widthOffsetFromLeftMenu(),
                    mFloatingMenuStart,
                    mPaletteAndRoutineFloatingLayout->width(),
                    mPaletteAndRoutineFloatingLayout->height());
            } else {
                leftPushInFloatingLayout(mPaletteAndRoutineFloatingLayout);
                leftPushOutFloatingLayout(mPaletteFloatingLayout);
            }
        } else {
            if (skipTransition) {
                mPaletteFloatingLayout->setGeometry(widthOffsetFromLeftMenu(),
                                                    mFloatingMenuStart,
                                                    mPaletteFloatingLayout->width(),
                                                    mPaletteFloatingLayout->height());
                mPaletteAndRoutineFloatingLayout->setGeometry(
                    -1 * mPaletteAndRoutineFloatingLayout->width(),
                    mFloatingMenuStart,
                    mPaletteAndRoutineFloatingLayout->width(),
                    mPaletteAndRoutineFloatingLayout->height());
            } else {
                leftPushInFloatingLayout(mPaletteFloatingLayout);
                leftPushOutFloatingLayout(mPaletteAndRoutineFloatingLayout);
            }
        }
    }
}

void TopMenu::showSingleColorStateWidget(bool show) {
    auto stateOffset = (mSize.height() - mSingleColorStateWidget->height()) / 2;
    auto startPoint = QPoint(parentWidget()->width() - mSingleColorStateWidget->width(),
                             mMenuStart + stateOffset);
    if (show) {
        mSingleColorStateWidget->setVisible(true);
        mSingleColorStateWidget->pushIn(startPoint);
        mSingleColorStateWidget->updateSyncStatus(ESyncState::notSynced);
    } else {
        mSingleColorStateWidget->pushOut(startPoint);
    }
}

void TopMenu::showMultiColorStateWidget(bool show) {
    auto startPoint =
        QPoint(parentWidget()->width() - mMultiColorStateWidget->width(), mFloatingMenuStart);

    if (show && !mData->empty()) {
        mMultiColorStateWidget->setVisible(true);
        mMultiColorStateWidget->pushIn(startPoint);
        mMultiColorStateWidget->updateSyncStatus(ESyncState::synced);
    } else {
        mMultiColorStateWidget->pushOut(startPoint);
        mSingleLightBrightness->pushOut(brightnessSliderRect().topLeft());
        mMultiColorStateWidget->pushOut(startPoint);
    }
}

void TopMenu::moveColorStateWidgets() {
    if (mSingleColorStateWidget->isIn()) {
        auto startPoint =
            QPoint(parentWidget()->width() - mSingleColorStateWidget->width(), mMenuStart);
        mSingleColorStateWidget->move(startPoint);
    }

    if (mMultiColorStateWidget->isIn()) {
        auto startPoint =
            QPoint(parentWidget()->width() - mMultiColorStateWidget->width(), mFloatingMenuStart);
        mMultiColorStateWidget->move(startPoint);
    }
}


void TopMenu::showFloatingLayout(EPage newPage) {
    if (newPage != mCurrentPage) {
        // move old menu back
        switch (mCurrentPage) {
            case EPage::colorPage: {
                showSingleColorStateWidget(false);
            } break;
            case EPage::moodPage:
                break;
            case EPage::palettePage: {
                showMultiColorStateWidget(false);
            } break;
            default:
                break;
        }
        // move new menu forward
        switch (newPage) {
            case EPage::colorPage:
                showSingleColorStateWidget(false);
                break;
            case EPage::moodPage:
                break;
            case EPage::palettePage:
                showMultiColorStateWidget(true);
                mMultiColorStateWidget->updateState(mData->multiColorScheme());
                break;
            default:
                break;
        }

        mCurrentPage = newPage;
        handleButtonLayouts();
        handleBrightnessSliders();
    }
}

FloatingLayout* TopMenu::currentFloatingLayout() {
    FloatingLayout* layout = nullptr;
    switch (mCurrentPage) {
        case EPage::moodPage:
            layout = mMoodsFloatingLayout;
            break;
        case EPage::palettePage: {
            if (shouldShowRoutineWidget()) {
                layout = mPaletteAndRoutineFloatingLayout;
            } else {
                layout = mPaletteFloatingLayout;
            }
            break;
        }
        case EPage::colorPage: {
            if (shouldShowRoutineWidget()) {
                layout = mColorAndRoutineFloatingLayout;
            } else {
                layout = mColorFloatingLayout;
            }
            break;
        }
        case EPage::timeoutPage:
            layout = mTimeoutFloatingLayout;
            break;
        default:
            break;
    }
    return layout;
}


void TopMenu::moveFloatingLayout() {
    auto parentSize = parentWidget()->size();
    QPoint topRight(parentSize.width(), mFloatingMenuStart);

    // moves the color state widgets, whether they are in or out
    moveColorStateWidgets();

    if (mCurrentPage == EPage::settingsPage) {
        moveHiddenLayouts();
    } else if (mData->empty() && mCurrentPage == EPage::moodPage) {
        // special case where the current floating layout is shown regardless of if the data is
        // empty
        currentFloatingLayout()->move(topRight);
    } else if (mCurrentPage == EPage::lightsPage) {
        moveLightsMenu();
    } else if (mCurrentPage == EPage::colorPage) {
        moveColorPageMenus(true);
    } else if (mCurrentPage == EPage::palettePage) {
        movePalettePageMenus(true);
    } else if (mData->empty() && mCurrentPage != EPage::lightsPage) {
        // floating layout is not shown
        currentFloatingLayout()->move(
            QPoint(parentWidget()->size().width() + currentFloatingLayout()->width(),
                   mFloatingMenuStart));
        moveHiddenLayouts();
    } else if (mCurrentPage != EPage::lightsPage) {
        // floating layout should be shown, since data is showing.
        currentFloatingLayout()->move(topRight);
        moveHiddenLayouts();
    }
}


bool TopMenu::shouldShowRoutineWidget() {
    bool hasArduino = mData->hasLightWithProtocol(EProtocolType::arduCor);
    bool hasNanoLeaf = mData->hasLightWithProtocol(EProtocolType::nanoleaf);

    // get the desired endpoint
    bool hasMulti = hasArduino || hasNanoLeaf;

    bool shouldMoveIn = false;
    if (mCurrentPage == EPage::colorPage) {
        if (!hasMulti && !mData->empty()
            && mComm->bestColorPickerType(mData->lights()) == EColorPickerType::CT) {
            mColorPage->changePageType(ESingleColorPickerMode::ambient);
            mColorFloatingLayout->highlightButton("Temperature");
        }
        if (hasMulti && !mData->empty()) {
            shouldMoveIn = true;
        }
    } else if (mCurrentPage == EPage::palettePage) {
        // only push in if has lights capable of multiple colors
        if (hasMulti && !mData->empty()) {
            shouldMoveIn = true;
        }
    }
    return shouldMoveIn;
}


void TopMenu::leftPushOutFloatingLayout(FloatingLayout* layout) {
    if (layout->geometry().x() != (-1 * layout->width())) {
        cor::moveWidget(layout,
                        QPoint(widthOffsetFromLeftMenu(), mFloatingMenuStart),
                        QPoint(-1 * layout->width(), mFloatingMenuStart));
    }
}


void TopMenu::rightPushOutFloatingLayout(FloatingLayout* layout) {
    auto parentSize = parentWidget()->size();
    if (layout->geometry().x() != (parentSize.width() + layout->width())) {
        cor::moveWidget(layout,
                        QPoint(parentSize.width() - layout->width(), mFloatingMenuStart),
                        QPoint(parentSize.width() + layout->width(), mFloatingMenuStart));
    }
}


void TopMenu::leftPushInFloatingLayout(FloatingLayout* layout) {
    if (layout->geometry().x() != widthOffsetFromLeftMenu()) {
        cor::moveWidget(layout,
                        QPoint(-1 * layout->width(), mFloatingMenuStart),
                        QPoint(widthOffsetFromLeftMenu(), mFloatingMenuStart));
    }
}

void TopMenu::rightPushInFloatingLayout(FloatingLayout* layout) {
    auto parentSize = parentWidget()->size();
    if (layout->geometry().x() != (parentSize.width() - layout->width())) {
        cor::moveWidget(layout,
                        QPoint(parentSize.width() + layout->width(), mFloatingMenuStart),
                        QPoint(parentSize.width() - layout->width(), mFloatingMenuStart));
    }
}

void TopMenu::pushInLeftLightsMenu() {
    // add check to avoid showing menu when the controller widget is visible.
    auto parentSize = parentWidget()->size();
    if (mDiscoveryTopMenu->geometry().x() != (parentSize.width() - mDiscoveryTopMenu->width())) {
        cor::moveWidget(mDiscoveryTopMenu,
                        QPoint(-1 * mDiscoveryTopMenu->width(), mFloatingMenuStart),
                        QPoint(widthOffsetFromLeftMenu(), mFloatingMenuStart));
    }

    rightPushInFloatingLayout(mAddLightsFloatingLayout);
}

void TopMenu::pushOutLightsMenus() {
    if (mDiscoveryTopMenu->geometry().x() != (-1 * mDiscoveryTopMenu->width())) {
        cor::moveWidget(mDiscoveryTopMenu,
                        QPoint(widthOffsetFromLeftMenu(), mFloatingMenuStart),
                        QPoint(-1 * mDiscoveryTopMenu->width(), mFloatingMenuStart));
    }

    rightPushOutFloatingLayout(mAddLightsFloatingLayout);
}

void TopMenu::moveHiddenLayouts() {
    FloatingLayout* layout = currentFloatingLayout();
    auto rightWidgetStartPoint = parentWidget()->width();

    // color layouts
    if (layout != mColorFloatingLayout) {
        mColorFloatingLayout->setGeometry(-1 * mColorFloatingLayout->width(),
                                          mFloatingMenuStart,
                                          mColorFloatingLayout->width(),
                                          mColorFloatingLayout->height());
    }
    if (layout != mColorAndRoutineFloatingLayout) {
        mColorAndRoutineFloatingLayout->setGeometry(-1 * mColorAndRoutineFloatingLayout->width(),
                                                    mFloatingMenuStart,
                                                    mColorAndRoutineFloatingLayout->width(),
                                                    mColorAndRoutineFloatingLayout->height());
    }


    // palette layouts
    if (layout != mPaletteFloatingLayout) {
        mPaletteFloatingLayout->setGeometry(mPaletteFloatingLayout->width() * -1,
                                            mFloatingMenuStart,
                                            mPaletteFloatingLayout->width(),
                                            mPaletteFloatingLayout->height());
    }
    if (layout != mPaletteAndRoutineFloatingLayout) {
        mPaletteAndRoutineFloatingLayout->setGeometry(
            mPaletteAndRoutineFloatingLayout->width() * -1,
            mFloatingMenuStart,
            mPaletteAndRoutineFloatingLayout->width(),
            mPaletteAndRoutineFloatingLayout->height());
    }


    // lights layout
    if (mCurrentPage != EPage::lightsPage) {
        mDiscoveryTopMenu->setGeometry(mDiscoveryTopMenu->width() * -1,
                                       mFloatingMenuStart,
                                       mDiscoveryTopMenu->width(),
                                       mDiscoveryTopMenu->height());
        mAddLightsFloatingLayout->setGeometry(rightWidgetStartPoint,
                                              mFloatingMenuStart,
                                              mAddLightsFloatingLayout->width(),
                                              mAddLightsFloatingLayout->height());
    }


    // mood layout
    if (layout != mMoodsFloatingLayout) {
        mMoodsFloatingLayout->setGeometry(rightWidgetStartPoint,
                                          mFloatingMenuStart,
                                          mMoodsFloatingLayout->width(),
                                          mMoodsFloatingLayout->height());
    }

    // timeout layout
    if (layout != mTimeoutFloatingLayout) {
        mTimeoutFloatingLayout->setGeometry(mTimeoutFloatingLayout->width() * -1,
                                            mFloatingMenuStart,
                                            mTimeoutFloatingLayout->width(),
                                            mTimeoutFloatingLayout->height());
    }
}

void TopMenu::updateUI() {
    // get copy of data representation of lights
    auto currentLights = mComm->commLightsFromVector(mData->lights());
    if (currentLights != mLastDevices) {
        mLastDevices = currentLights;
        mGlobalStateWidget->update(cor::lightStatesFromLights(mLastDevices, true));
    }
}


void TopMenu::hideMenuButton(bool shouldHide) {
    mMenuButton->setVisible(!shouldHide);
}

void TopMenu::menuButtonPressed() {
    if (mSelectLightsButton->isIn()) {
        mSelectLightsButton->pushOut(mStartSelectLightsButton);
    }
    emit buttonPressed("Menu");
}

void TopMenu::pushInTapToSelectButton() {
    if (!mSelectLightsButton->isIn() && !mMainWindow->leftHandMenu()->alwaysOpen() && mData->empty()
        && mCurrentPage != EPage::lightsPage) {
        mSelectLightsButton->pushIn(mStartSelectLightsButton);
    }
}

void TopMenu::pushOutTapToSelectButton() {
    if (mSelectLightsButton->isIn()) {
        mSelectLightsButton->pushOut(mStartSelectLightsButton);
    }
}


void TopMenu::updateState(const cor::LightState& state) {
    ERoutine routine = state.routine();
    if (mCurrentPage == EPage::colorPage) {
        if (!mSingleColorStateWidget->isIn()) {
            showSingleColorStateWidget(true);
        }
        QColor color = state.color();
        if (state.temperature() != -1) {
            color = cor::colorTemperatureToRGB(state.temperature());
            color.setHsvF(color.hueF(), color.saturationF(), state.color().valueF());
        }
        mSingleColorStateWidget->updateState(color, routine);
    } else if (mCurrentPage == EPage::palettePage) {
        if (!mMultiColorStateWidget->isIn()) {
            showMultiColorStateWidget(true);
        }
        mMultiColorStateWidget->updateState(mData->multiColorScheme());
    }

    if (mGlobalBrightness->isIn()) {
        // set slider color
        mGlobalBrightness->updateColor(mData->mainColor());
        // set slider position only if on a non-palette page page.
        if (mCurrentPage != EPage::palettePage) {
            // catch edge case where its only hues and we're updating to a palette, in this case,
            // don't adjust the global brightness
            if (!(mData->onlyLightsWithProtocol(EProtocolType::hue)
                  && state.routine() > cor::ERoutineSingleColorEnd)) {
                mGlobalBrightness->updateBrightness(mData->mainColor().valueF() * 100.0);
            }
        }
    }
}

void TopMenu::updateScheme(const std::vector<QColor>& colors, std::uint32_t index) {
    if (mCurrentPage == EPage::palettePage) {
        if (!mMultiColorStateWidget->isIn()) {
            showMultiColorStateWidget(true);
        }
        mMultiColorStateWidget->updateState(colors);
        if (mPalettePage->colorPicker()->currentScheme() == EColorSchemeType::custom) {
            mSingleLightBrightness->updateColor(colors[index]);
        }
    }

    if (mGlobalBrightness->isIn()) {
        auto color = mData->mainColor();
        mGlobalBrightness->updateColor(color);
    }
}

void TopMenu::handleBrightnessSliders() {
    if (mData->empty()) {
        // hide both, its empty
        mSingleLightBrightness->pushOut(
            QPoint(brightnessSliderRect().topLeft().x(), -1 * mSingleLightBrightness->height()));
        mGlobalBrightness->pushOut(
            QPoint(brightnessSliderRect().topLeft().x(), -1 * mGlobalBrightness->height()));
    } else {
        bool updateGlobalBrightness = false;
        if (mCurrentPage == EPage::palettePage) {
            if (mPalettePage->mode() == EGroupMode::wheel) {
                if (mPalettePage->colorPicker()->currentScheme() == EColorSchemeType::custom) {
                    QPoint point(brightnessSliderRect().x(), -1 * mGlobalBrightness->height());
                    mGlobalBrightness->pushOut(point);
                    if (!mSingleLightBrightness->isIn()) {
                        mSingleLightBrightness->pushIn(brightnessSliderRect().topLeft());
                    }
                    auto paletteColors = mPalettePage->palette().colors();
                    if (!paletteColors.empty()) {
                        auto color = paletteColors[0];
                        mSingleLightBrightness->updateColor(color);
                        mSingleLightBrightness->updateBrightness(
                            std::uint32_t(color.valueF() * 100.0));
                    }
                } else {
                    updateGlobalBrightness = true;
                }
            } else {
                updateGlobalBrightness = true;
            }
        } else if (mCurrentPage == EPage::colorPage || mCurrentPage == EPage::moodPage
                   || mCurrentPage == EPage::lightsPage || mCurrentPage == EPage::settingsPage) {
            updateGlobalBrightness = true;
        }

        if (updateGlobalBrightness) {
            mGlobalBrightness->pushIn(brightnessSliderRect().topLeft());
            mGlobalBrightness->lightCountChanged(mData->isOn(),
                                                 mData->mainColor(),
                                                 mData->brightness(),
                                                 mData->lights().size());
            QPoint point(brightnessSliderRect().topLeft().x(),
                         -1 * mSingleLightBrightness->height());
            mSingleLightBrightness->pushOut(point);
        }
    }
}

void TopMenu::handleButtonLayouts() {
    mColorFloatingLayout->setVisible(false);
    mColorAndRoutineFloatingLayout->setVisible(false);
    mMoodsFloatingLayout->setVisible(false);
    mPaletteFloatingLayout->setVisible(false);
    mPaletteAndRoutineFloatingLayout->setVisible(false);
    mTimeoutFloatingLayout->setVisible(false);
    mDiscoveryTopMenu->setVisible(false);
    mAddLightsFloatingLayout->setVisible(false);
    switch (mCurrentPage) {
        case EPage::colorPage:
            moveColorPageMenus(false);
            mColorFloatingLayout->setVisible(true);
            mColorAndRoutineFloatingLayout->setVisible(true);
            pushOutLightsMenus();
            rightPushOutFloatingLayout(mMoodsFloatingLayout);
            leftPushOutFloatingLayout(mPaletteFloatingLayout);
            leftPushOutFloatingLayout(mPaletteAndRoutineFloatingLayout);
            break;
        case EPage::moodPage:
            mMoodsFloatingLayout->setVisible(true);
            pushOutLightsMenus();
            leftPushOutFloatingLayout(mColorFloatingLayout);
            leftPushOutFloatingLayout(mColorAndRoutineFloatingLayout);
            rightPushInFloatingLayout(mMoodsFloatingLayout);
            leftPushOutFloatingLayout(mPaletteFloatingLayout);
            leftPushOutFloatingLayout(mPaletteAndRoutineFloatingLayout);
            break;
        case EPage::palettePage:
            movePalettePageMenus(false);
            mPaletteFloatingLayout->setVisible(true);
            mPaletteAndRoutineFloatingLayout->setVisible(true);
            pushOutLightsMenus();
            leftPushOutFloatingLayout(mColorFloatingLayout);
            leftPushOutFloatingLayout(mColorAndRoutineFloatingLayout);
            rightPushOutFloatingLayout(mMoodsFloatingLayout);
            break;
        case EPage::timeoutPage:
            if (mData->empty()) {
                leftPushOutFloatingLayout(mTimeoutFloatingLayout);
            } else {
                leftPushInFloatingLayout(mTimeoutFloatingLayout);
            }
            mTimeoutFloatingLayout->setVisible(true);
            pushOutLightsMenus();
            leftPushOutFloatingLayout(mColorFloatingLayout);
            leftPushOutFloatingLayout(mColorAndRoutineFloatingLayout);
            rightPushOutFloatingLayout(mMoodsFloatingLayout);
            leftPushOutFloatingLayout(mPaletteFloatingLayout);
            leftPushOutFloatingLayout(mPaletteAndRoutineFloatingLayout);
            break;
        case EPage::lightsPage:
            mDiscoveryTopMenu->setVisible(true);
            mAddLightsFloatingLayout->setVisible(true);
            pushInLeftLightsMenu();
            leftPushOutFloatingLayout(mColorFloatingLayout);
            leftPushOutFloatingLayout(mColorAndRoutineFloatingLayout);
            rightPushOutFloatingLayout(mMoodsFloatingLayout);
            leftPushOutFloatingLayout(mPaletteFloatingLayout);
            leftPushOutFloatingLayout(mPaletteAndRoutineFloatingLayout);
            break;
        case EPage::settingsPage:
            mMoodsFloatingLayout->setVisible(true);
            pushOutLightsMenus();
            leftPushOutFloatingLayout(mColorFloatingLayout);
            leftPushOutFloatingLayout(mColorAndRoutineFloatingLayout);
            rightPushOutFloatingLayout(mMoodsFloatingLayout);
            leftPushOutFloatingLayout(mPaletteFloatingLayout);
            leftPushOutFloatingLayout(mPaletteAndRoutineFloatingLayout);
            break;
        case EPage::MAX:
            break;
    }
}


void TopMenu::updateLightsMenu() {
    mDiscoveryTopMenu->updateMenu();
    mAddLightsFloatingLayout->highlightButton("");
    pushInLeftLightsMenu();
}

void TopMenu::moveLightsMenu() {
    mDiscoveryTopMenu->setGeometry(widthOffsetFromLeftMenu(),
                                   mFloatingMenuStart,
                                   mDiscoveryTopMenu->width(),
                                   mDiscoveryTopMenu->height());
    mAddLightsFloatingLayout->move(QPoint(parentWidget()->width(), mFloatingMenuStart));
}

void TopMenu::updateLightsButton(EProtocolType type, EConnectionState state) {
    mDiscoveryTopMenu->updateDiscoveryButton(type, state);
}
