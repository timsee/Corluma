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
                 GroupData* groups,
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
      mGroups(groups),
      mAppSettings{appSettings},
      mMainWindow(mainWindow),
      mPalettePage(palettePage),
      mColorPage(colorPage),
      mLightsPage{lightsPage},
      mCurrentPage{EPage::colorPage},
      mSize{QSize(int(cor::applicationSize().height() * 0.1f),
                  int(cor::applicationSize().height() * 0.1f))},
      mPaletteWidth{mSize.width() * 3},
      mLastColorButtonKey{"HSV"},
      mRenderTimer{new QTimer(this)},
      mMainPalette{new cor::LightVectorWidget(6, 2, true, this)},
      mMenuButton{new QPushButton(this)},
      mGlobalBrightness{new GlobalBrightnessWidget(mSize,
                                                   mMainWindow->leftHandMenu()->alwaysOpen(),
                                                   mComm,
                                                   mData,
                                                   this)},
      mSingleLightBrightness{
          new SingleLightBrightnessWidget(mSize, mMainWindow->leftHandMenu()->alwaysOpen(), this)},
      mPaletteFloatingLayout{new FloatingLayout(mMainWindow)},
      mMoodsFloatingLayout{new FloatingLayout(mMainWindow)},
      mColorFloatingLayout{new FloatingLayout(mMainWindow)},
      mTimeoutFloatingLayout{new FloatingLayout(mMainWindow)},
      mRoutineFloatingLayout{new FloatingLayout(mMainWindow)},
      mLightsFloatingLayout{new FloatingLayout(mMainWindow)},
      mAddLightsFloatingLayout{new FloatingLayout(mMainWindow)},
      mSingleColorStateWidget{new SingleColorStateWidget(mMainWindow)},
      mMultiColorStateWidget{new MultiColorStateWidget(mMainWindow)},
      mSelectLightsButton{new SelectLightsButton(this)} {
    // start render timer
    connect(mRenderTimer, SIGNAL(timeout()), this, SLOT(updateUI()));
    mRenderTimer->start(100);

    // --------------
    // Setup menu button
    // --------------
    mMenuButton->setVisible(true);
    connect(mMenuButton, SIGNAL(clicked(bool)), this, SLOT(menuButtonPressed()));
    mMenuButton->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);

    // --------------
    // Setup Brightness Slider
    // --------------
    mGlobalBrightness->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    mSingleLightBrightness->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);

    // --------------
    // Setup Main Palette
    // -------------
    mMainPalette->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    mMainPalette->enableButtonInteraction(false);

    // --------------
    // Routine Floating Layout
    // --------------

    connect(mRoutineFloatingLayout,
            SIGNAL(buttonPressed(QString)),
            this,
            SLOT(floatingLayoutButtonPressed(QString)));
    mRoutineFloatingLayout->setVisible(false);
    mRoutineFloatingLayout->setupButtons({QString("Routine")}, EButtonSize::small);

    // --------------
    // Palette Floating Layout
    // --------------
    connect(mPaletteFloatingLayout,
            SIGNAL(buttonPressed(QString)),
            this,
            SLOT(floatingLayoutButtonPressed(QString)));
    mPaletteFloatingLayout->setupButtons({QString("HSV"), QString("Preset")}, EButtonSize::small);
    mPaletteFloatingLayout->highlightButton("HSV");

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
    mMultiColorStateWidget->setFixedSize(int(mSize.width() * 3.2), mSize.height() / 2);
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

    connect(mLightsFloatingLayout,
            SIGNAL(buttonPressed(QString)),
            this,
            SLOT(floatingLayoutButtonPressed(QString)));


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

    auto colorMenuWidth = cor::applicationSize().width() - mColorFloatingLayout->width();
    if (mPaletteWidth > colorMenuWidth) {
        mPaletteWidth = colorMenuWidth;
    }
    mMainPalette->setFixedHeight(int(mSize.height() * 0.8));
    mMainPalette->setFixedWidth(mPaletteWidth);

    // update the lights menu to reflect the proper state
    updateLightsMenu();

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

    handleButtonLayouts();
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
        showRoutineWidget(false);
        mColorPage->update(mData->mainColor(),
                           mData->brightness(),
                           mData->lights().size(),
                           mComm->bestColorPickerType(mData->lights()));
    } else if (mCurrentPage == EPage::palettePage) {
        showRoutineWidget(false);
        showMultiColorStateWidget(!mData->empty());
        mMultiColorStateWidget->updateState(mData->colorScheme());
        mPalettePage->update(mData->lightCount(), mData->multiColorScheme());
    }
}

void TopMenu::highlightButton(const QString& key) {
    mPaletteFloatingLayout->highlightButton(key);
    mMoodsFloatingLayout->highlightButton(key);
    mColorFloatingLayout->highlightButton(key);
    mTimeoutFloatingLayout->highlightButton(key);
    mRoutineFloatingLayout->highlightButton(key);
}

void TopMenu::showMenu() {
    setVisible(true);
    mRoutineFloatingLayout->setVisible(true);

    raise();
    mColorFloatingLayout->raise();
    mPaletteFloatingLayout->raise();
    mMoodsFloatingLayout->raise();
    mTimeoutFloatingLayout->raise();
    mRoutineFloatingLayout->raise();
    mSingleColorStateWidget->raise();
    mMultiColorStateWidget->raise();
    mLightsFloatingLayout->raise();
    mAddLightsFloatingLayout->raise();
}

void TopMenu::resize(int xOffset) {
    auto parentSize = parentWidget()->size();
    moveFloatingLayout();

    int padding = 5;
    mMenuButton->setGeometry(int(mSize.width() * 0.1),
                             int(mSize.height() * 0.025),
                             int(mSize.width() * 0.75f),
                             int(mSize.height() * 0.75f));
    cor::resizeIcon(mMenuButton, ":/images/hamburger_icon.png", 0.7f);

    mGlobalBrightness->resize();
    mSingleLightBrightness->resize();
    if (mMainWindow->leftHandMenu()->alwaysOpen()) {
        mMainPalette->setVisible(false);
    } else {
        mMainPalette->setVisible(true);
    }

    int yPos = mMenuButton->height() + padding;
    mStartSelectLightsButton = yPos;
    mFloatingMenuStart = yPos;
    yPos += mSize.height() / 2;

    mMainPalette->setGeometry(0, yPos, mPaletteWidth, mSize.height() / 2);
    yPos += mMainPalette->height() + padding;

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
        showRoutineWidget(false);
        handleBrightnessSliders();
        mRoutineFloatingLayout->highlightButton("");
    } else if (button == "HSV") {
        if (mColorPage->isOpen()) {
            mLastColorButtonKey = button;
            mColorPage->showRoutines(false);
            mColorPage->changePageType(ESingleColorPickerMode::HSV);
        }
        if (mPalettePage->isOpen()) {
            mPalettePage->setMode(EGroupMode::wheel);
            handleBrightnessSliders();
            showRoutineWidget(false);
        }
        mRoutineFloatingLayout->highlightButton("");
    } else if (button == "Temperature") {
        mLastColorButtonKey = button;
        mColorPage->showRoutines(false);
        mRoutineFloatingLayout->highlightButton("");
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
    } else {
        qDebug() << "I don't recognize that button type..." << button;
    }
}

void TopMenu::closeRoutinesPage() {
    if (mColorPage->isOpen()) {
        floatingLayoutButtonPressed("HSV");
    } else if (mPalettePage->isOpen()) {
        floatingLayoutButtonPressed("HSV");
    }
}

QPoint TopMenu::colorStateStartPoint() {
    int width = 0;
    if (mMainWindow->leftHandMenu()->alwaysOpen()) {
        width = mMainWindow->leftHandMenu()->width();
    }
    return QPoint(width, mSize.height() * 2 / 3);
}

void TopMenu::showSingleColorStateWidget(bool show) {
    auto startPoint = colorStateStartPoint();
    if (show) {
        mSingleColorStateWidget->setVisible(true);
        mSingleColorStateWidget->pushIn(startPoint);
        mSingleColorStateWidget->updateSyncStatus(ESyncState::notSynced);
    } else {
        mSingleColorStateWidget->pushOut(startPoint);
    }
}

void TopMenu::showMultiColorStateWidget(bool show) {
    auto startPoint = colorStateStartPoint();
    if (show && !mData->empty()) {
        mMultiColorStateWidget->setVisible(true);
        mMultiColorStateWidget->pushIn(startPoint);
        mMultiColorStateWidget->updateSyncStatus(ESyncState::synced);
    } else {
        mMultiColorStateWidget->pushOut(startPoint);
        mSingleLightBrightness->pushOut();
        mMultiColorStateWidget->pushOut(startPoint);
    }
}

void TopMenu::moveColorStateWidgets() {
    auto startPoint = colorStateStartPoint();
    if (mSingleColorStateWidget->isIn()) {
        mSingleColorStateWidget->move(startPoint);
    }

    if (mMultiColorStateWidget->isIn()) {
        mMultiColorStateWidget->move(startPoint);
    }
}


void TopMenu::showFloatingLayout(EPage newPage) {
    if (newPage != mCurrentPage) {
        mRoutineFloatingLayout->highlightButton("");
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
                mMultiColorStateWidget->updateState(mData->colorScheme());
                break;
            default:
                break;
        }

        mCurrentPage = newPage;
        showRoutineWidget(false);
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
        case EPage::palettePage:
            layout = mPaletteFloatingLayout;
            break;
        case EPage::colorPage:
            layout = mColorFloatingLayout;
            break;
        case EPage::timeoutPage:
            layout = mTimeoutFloatingLayout;
            break;
        case EPage::lightsPage:
            layout = mLightsFloatingLayout;
            break;
        default:
            break;
    }
    return layout;
}


void TopMenu::moveFloatingLayout() {
    auto parentSize = parentWidget()->size();
    QPoint topRight(parentSize.width(), mFloatingMenuStart);
    showRoutineWidget(true);

    // moves the color state widgets, whether they are in or out
    moveColorStateWidgets();

    bool skipTranisition = true;
    showRoutineWidget(skipTranisition);

    if (mCurrentPage == EPage::settingsPage) {
        moveHiddenLayouts();
    } else if (mData->empty() && mCurrentPage == EPage::moodPage) {
        // special case where the current floating layout is shown regardless of if the data is
        // empty
        currentFloatingLayout()->move(topRight);
    } else if (mCurrentPage == EPage::lightsPage) {
        moveLightsMenu();
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


void TopMenu::showRoutineWidget(bool skipTransition) {
    bool hasArduino = mData->hasLightWithProtocol(EProtocolType::arduCor);
    bool hasNanoLeaf = mData->hasLightWithProtocol(EProtocolType::nanoleaf);

    // get the size of the parent
    auto parentSize = parentWidget()->size();
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
    QPoint endPoint;
    if (shouldMoveIn) {
        endPoint = QPoint(parentSize.width() - mRoutineFloatingLayout->width(),
                          mFloatingMenuStart + mColorFloatingLayout->height());
    } else {
        endPoint = QPoint(
            parentSize.width() - mRoutineFloatingLayout->width() + mRoutineFloatingLayout->height(),
            mFloatingMenuStart + mColorFloatingLayout->height());
    }

    if (skipTransition) {
        mRoutineFloatingLayout->setGeometry(endPoint.x(),
                                            endPoint.y(),
                                            mRoutineFloatingLayout->width(),
                                            mRoutineFloatingLayout->height());
    } else {
        cor::moveWidget(mRoutineFloatingLayout, mRoutineFloatingLayout->pos(), endPoint);
    }
}

void TopMenu::pushRightFloatingLayout(FloatingLayout* layout) {
    auto parentSize = parentWidget()->size();
    if (layout->geometry().x() != (parentSize.width() + layout->width())) {
        cor::moveWidget(layout,
                        QPoint(parentSize.width() - layout->width(), mFloatingMenuStart),
                        QPoint(parentSize.width() + layout->width(), mFloatingMenuStart));
    }
}

void TopMenu::pullLeftFloatingLayout(FloatingLayout* layout) {
    auto parentSize = parentWidget()->size();
    if (layout->geometry().x() != (parentSize.width() - layout->width())) {
        cor::moveWidget(layout,
                        QPoint(parentSize.width() + layout->width(), mFloatingMenuStart),
                        QPoint(parentSize.width() - layout->width(), mFloatingMenuStart));
    }
}

void TopMenu::pullLeftLightsMenu() {
    // add check to avoid showing menu when the controller widget is visible.
    auto parentSize = parentWidget()->size();
    if (mLightsFloatingLayout->geometry().x()
        != (parentSize.width() - mLightsFloatingLayout->width())) {
        cor::moveWidget(
            mLightsFloatingLayout,
            QPoint(parentSize.width() + mLightsFloatingLayout->width(), mFloatingMenuStart),
            QPoint(parentSize.width() - mLightsFloatingLayout->width(), mFloatingMenuStart));
    }

    auto addLightsStartPoint = mLightsFloatingLayout->height() + mLightsFloatingLayout->pos().y();
    if (mAddLightsFloatingLayout->geometry().x()
        != (parentSize.width() - mAddLightsFloatingLayout->width())) {
        cor::moveWidget(
            mAddLightsFloatingLayout,
            QPoint(parentSize.width() + mAddLightsFloatingLayout->width(), addLightsStartPoint),
            QPoint(parentSize.width() - mAddLightsFloatingLayout->width(), addLightsStartPoint));
    }
}

void TopMenu::pushRightLightsMenus() {
    auto parentSize = parentWidget()->size();
    if (mLightsFloatingLayout->geometry().x()
        != (parentSize.width() + mLightsFloatingLayout->width())) {
        cor::moveWidget(
            mLightsFloatingLayout,
            QPoint(parentSize.width() - mLightsFloatingLayout->width(), mFloatingMenuStart),
            QPoint(parentSize.width() + mLightsFloatingLayout->width(), mFloatingMenuStart));
    }

    auto addLightsStartPoint = mLightsFloatingLayout->height() + mLightsFloatingLayout->pos().y();
    if (mAddLightsFloatingLayout->geometry().x()
        != (parentSize.width() + mAddLightsFloatingLayout->width())) {
        cor::moveWidget(
            mAddLightsFloatingLayout,
            QPoint(parentSize.width() - mAddLightsFloatingLayout->width(), addLightsStartPoint),
            QPoint(parentSize.width() + mAddLightsFloatingLayout->width(), addLightsStartPoint));
    }
}

void TopMenu::moveHiddenLayouts() {
    auto parentSize = parentWidget()->size();
    FloatingLayout* layout = currentFloatingLayout();
    QPoint topRight;
    if (layout == nullptr) {
        topRight = QPoint(parentSize.width(), mFloatingMenuStart);
    } else {
        topRight = QPoint(parentSize.width() + layout->width(), mFloatingMenuStart);
    }
    if (layout != mColorFloatingLayout) {
        mColorFloatingLayout->setGeometry(topRight.x(),
                                          topRight.y() + mColorFloatingLayout->height(),
                                          mColorFloatingLayout->width(),
                                          mColorFloatingLayout->height());
    }

    if (layout != mLightsFloatingLayout) {
        mLightsFloatingLayout->setGeometry(topRight.x(),
                                           topRight.y() + mColorFloatingLayout->height(),
                                           mLightsFloatingLayout->width(),
                                           mLightsFloatingLayout->height());
        mAddLightsFloatingLayout->setGeometry(topRight.x(),
                                              topRight.y(),
                                              mAddLightsFloatingLayout->width(),
                                              mAddLightsFloatingLayout->height());
    }
    if (layout != mMoodsFloatingLayout) {
        mMoodsFloatingLayout->setGeometry(topRight.x(),
                                          topRight.y(),
                                          mMoodsFloatingLayout->width(),
                                          mMoodsFloatingLayout->height());
    }
    if (layout != mTimeoutFloatingLayout) {
        mTimeoutFloatingLayout->setGeometry(topRight.x(),
                                            topRight.y(),
                                            mTimeoutFloatingLayout->width(),
                                            mTimeoutFloatingLayout->height());
    }
    if (layout != mPaletteFloatingLayout) {
        mPaletteFloatingLayout->setGeometry(topRight.x(),
                                            topRight.y(),
                                            mPaletteFloatingLayout->width(),
                                            mPaletteFloatingLayout->height());
    }
}

void TopMenu::updateUI() {
    // get copy of data representation of lights
    auto currentLights = mComm->commLightsFromVector(mData->lights());
    if (currentLights != mLastDevices) {
        mLastDevices = currentLights;
        mMainPalette->updateLights(currentLights);
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
        mMultiColorStateWidget->updateState(mData->colorScheme());
    }

    if (mGlobalBrightness->isIn()) {
        // set slider color
        mGlobalBrightness->updateColor(mData->mainColor());
        // set slider position
        // catch edge case where its only hues and we're updating to a palette, in this case, don't
        // adjust the global brightness
        if (!(mData->onlyLightsWithProtocol(EProtocolType::hue)
              && state.routine() > cor::ERoutineSingleColorEnd)) {
            mGlobalBrightness->updateBrightness(mData->mainColor().valueF() * 100.0);
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
        mSingleLightBrightness->pushOut();
        mGlobalBrightness->pushOut();
    } else {
        bool updateGlobalBrightness = false;
        if (mCurrentPage == EPage::palettePage) {
            if (mPalettePage->mode() == EGroupMode::wheel) {
                if (mPalettePage->colorPicker()->currentScheme() == EColorSchemeType::custom) {
                    mGlobalBrightness->pushOut();
                    if (!mSingleLightBrightness->isIn()) {
                        mSingleLightBrightness->pushIn();
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
            mGlobalBrightness->pushIn();
            mGlobalBrightness->lightCountChanged(mData->isOn(),
                                                 mData->mainColor(),
                                                 mData->brightness(),
                                                 mData->lights().size());
            mSingleLightBrightness->pushOut();
        }
    }
}

void TopMenu::handleButtonLayouts() {
    switch (mCurrentPage) {
        case EPage::colorPage:
            if (mData->empty()) {
                pushRightFloatingLayout(mColorFloatingLayout);
            } else {
                pullLeftFloatingLayout(mColorFloatingLayout);
            }
            mColorFloatingLayout->setVisible(true);
            mMoodsFloatingLayout->setVisible(false);
            mPaletteFloatingLayout->setVisible(false);
            mTimeoutFloatingLayout->setVisible(false);
            mLightsFloatingLayout->setVisible(false);
            mAddLightsFloatingLayout->setVisible(false);
            pushRightLightsMenus();
            pushRightFloatingLayout(mMoodsFloatingLayout);
            pushRightFloatingLayout(mPaletteFloatingLayout);
            break;
        case EPage::moodPage:
            mColorFloatingLayout->setVisible(false);
            mMoodsFloatingLayout->setVisible(true);
            mPaletteFloatingLayout->setVisible(false);
            mTimeoutFloatingLayout->setVisible(false);
            mLightsFloatingLayout->setVisible(false);
            mAddLightsFloatingLayout->setVisible(false);
            pushRightLightsMenus();
            pushRightFloatingLayout(mColorFloatingLayout);
            pullLeftFloatingLayout(mMoodsFloatingLayout);
            pushRightFloatingLayout(mPaletteFloatingLayout);
            break;
        case EPage::palettePage:
            if (mData->empty() && mPaletteFloatingLayout->isVisible()) {
                pushRightFloatingLayout(mPaletteFloatingLayout);
            } else if (!mData->empty()) {
                pullLeftFloatingLayout(mPaletteFloatingLayout);
            }
            mColorFloatingLayout->setVisible(false);
            mMoodsFloatingLayout->setVisible(false);
            mPaletteFloatingLayout->setVisible(true);
            mTimeoutFloatingLayout->setVisible(false);
            mLightsFloatingLayout->setVisible(false);
            mAddLightsFloatingLayout->setVisible(false);
            pushRightLightsMenus();
            pushRightFloatingLayout(mColorFloatingLayout);
            pushRightFloatingLayout(mMoodsFloatingLayout);
            break;
        case EPage::timeoutPage:
            if (mData->empty()) {
                pushRightFloatingLayout(mTimeoutFloatingLayout);
            } else {
                pullLeftFloatingLayout(mTimeoutFloatingLayout);
            }
            mColorFloatingLayout->setVisible(false);
            mMoodsFloatingLayout->setVisible(false);
            mPaletteFloatingLayout->setVisible(false);
            mTimeoutFloatingLayout->setVisible(true);
            mLightsFloatingLayout->setVisible(false);
            mAddLightsFloatingLayout->setVisible(false);
            pushRightLightsMenus();
            pushRightFloatingLayout(mMoodsFloatingLayout);
            pushRightFloatingLayout(mPaletteFloatingLayout);
            break;
        case EPage::lightsPage:
            mColorFloatingLayout->setVisible(false);
            mMoodsFloatingLayout->setVisible(false);
            mPaletteFloatingLayout->setVisible(false);
            mLightsFloatingLayout->setVisible(true);
            mAddLightsFloatingLayout->setVisible(true);
            pullLeftLightsMenu();
            pushRightFloatingLayout(mColorFloatingLayout);
            pushRightFloatingLayout(mMoodsFloatingLayout);
            pushRightFloatingLayout(mPaletteFloatingLayout);
            break;
        case EPage::settingsPage:
            mColorFloatingLayout->setVisible(false);
            mMoodsFloatingLayout->setVisible(true);
            mPaletteFloatingLayout->setVisible(false);
            mTimeoutFloatingLayout->setVisible(false);
            mLightsFloatingLayout->setVisible(false);
            mAddLightsFloatingLayout->setVisible(false);
            pushRightLightsMenus();
            pushRightFloatingLayout(mColorFloatingLayout);
            pushRightFloatingLayout(mMoodsFloatingLayout);
            pushRightFloatingLayout(mPaletteFloatingLayout);
            break;
        case EPage::MAX:
            break;
    }
}


void TopMenu::updateLightsMenu() {
    std::vector<QString> buttons;

    // hide and show buttons based on their usage
    if (mAppSettings->enabled(EProtocolType::hue)) {
        buttons.emplace_back("Discovery_Hue");
    }

    if (mAppSettings->enabled(EProtocolType::nanoleaf)) {
        buttons.emplace_back("Discovery_NanoLeaf");
    }

    if (mAppSettings->enabled(EProtocolType::arduCor)) {
        buttons.emplace_back("Discovery_ArduCor");
    }

    // check that commtype being shown is available, if not, adjust
    if (!mAppSettings->enabled(mLightsPage->currentProtocol())) {
        if (mAppSettings->enabled(EProtocolType::hue)) {
            mLightsPage->switchProtocol(EProtocolType::hue);
        } else if (mAppSettings->enabled(EProtocolType::nanoleaf)) {
            mLightsPage->switchProtocol(EProtocolType::nanoleaf);
        } else if (mAppSettings->enabled(EProtocolType::arduCor)) {
            mLightsPage->switchProtocol(EProtocolType::arduCor);
        }
    }

    auto currentProtocol = mLightsPage->currentProtocol();
    // check that if only one is available that the top menu doesn't show.
    if (buttons.size() == 1) {
        std::vector<QString> emptyVector;
        mLightsFloatingLayout->setupButtons(emptyVector, EButtonSize::small);
        mAddLightsFloatingLayout->highlightButton("");
    } else {
        mLightsFloatingLayout->setupButtons(buttons, EButtonSize::rectangle);
        mAddLightsFloatingLayout->highlightButton("");
        if (currentProtocol == EProtocolType::nanoleaf) {
            mLightsFloatingLayout->highlightButton("Discovery_NanoLeaf");
        } else if (currentProtocol == EProtocolType::arduCor) {
            mLightsFloatingLayout->highlightButton("Discovery_ArduCor");
        } else if (currentProtocol == EProtocolType::hue) {
            mLightsFloatingLayout->highlightButton("Discovery_Hue");
        }
    }
    mLightsPage->switchProtocol(currentProtocol);

    pullLeftLightsMenu();
}

void TopMenu::moveLightsMenu() {
    auto parentSize = parentWidget()->size();
    FloatingLayout* layout = currentFloatingLayout();
    QPoint verticalStart(parentSize.width() + layout->width(), mFloatingMenuStart);
    if (mLightsFloatingLayout->buttonCount() == 0) {
        // theres no horizontal floating layout, so move vertical to top right
        verticalStart =
            QPoint(parentSize.width(), mLightsFloatingLayout->size().height() + mFloatingMenuStart);
    } else {
        // theres a horizontal and vertical menu, horizontal is in top right
        QPoint topRight(parentSize.width(), mFloatingMenuStart);
        mLightsFloatingLayout->move(topRight);
        // vertical is directly under it.
        verticalStart =
            QPoint(parentSize.width(), mLightsFloatingLayout->size().height() + mFloatingMenuStart);
    }

    mAddLightsFloatingLayout->move(verticalStart);
}

void TopMenu::updateLightsButton(EProtocolType type, EConnectionState state) {
    mLightsFloatingLayout->updateDiscoveryButton(type, state);
}
