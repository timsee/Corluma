/*!
 * \copyright
 * Copyright (C) 2015 - 2020.
 * Released under the GNU General Public License.
 */

#include "topmenu.h"

#include <QGraphicsEffect>

#include "mainwindow.h"
#include "utils/exception.h"
#include "utils/qt.h"


TopMenu::TopMenu(QWidget* parent,
                 cor::LightList* data,
                 CommLayer* comm,
                 GroupData* groups,
                 MainWindow* mainWindow,
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
      mMainWindow(mainWindow),
      mPalettePage(palettePage),
      mColorPage(colorPage),
      mCurrentPage{EPage::colorPage},
      mSize{QSize(int(cor::applicationSize().height() * 0.1f),
                  int(cor::applicationSize().height() * 0.1f))},
      mPaletteWidth{mSize.width() * 3},
      mLastColorButtonKey{"HSV"},
      mRenderTimer{new QTimer(this)},
      mMainPalette{new cor::LightVectorWidget(6, 2, true, this)},
      mMenuButton{new QPushButton(this)},
      mGlobalBrightness{
          new GlobalBrightnessWidget(mSize, mMainWindow->leftHandMenu()->alwaysOpen(), this)},
      mSingleLightBrightness{
          new SingleLightBrightnessWidget(mSize, mMainWindow->leftHandMenu()->alwaysOpen(), this)},
      mPaletteFloatingLayout{new FloatingLayout(false, mMainWindow)},
      mMoodsFloatingLayout{new FloatingLayout(false, mMainWindow)},
      mColorFloatingLayout{new FloatingLayout(false, mMainWindow)},
      mRoutineFloatingLayout{new FloatingLayout(false, mMainWindow)},
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
    mColorFloatingLayout->setupButtons({QString("HSV"), QString("RGB"), QString("Temperature")},
                                       EButtonSize::small);
    mColorFloatingLayout->highlightButton("HSV");

    mSingleColorStateWidget->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    mSingleColorStateWidget->setFixedSize(mSize.width(), mSize.height() / 2);
    mSingleColorStateWidget->setVisible(false);
    showSingleColorStateWidget(false);

    mMultiColorStateWidget->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    mMultiColorStateWidget->setFixedSize(int(mSize.width() * 3.2), mSize.height() / 2);
    mMultiColorStateWidget->setVisible(false);
    showMultiColorStateWidget(false);


    // --------------
    // Select Lights Button
    // --------------
    mSelectLightsButton->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    connect(mSelectLightsButton, SIGNAL(pressed()), this, SLOT(menuButtonPressed()));
    QFontMetrics fm(mSelectLightsButton->font());
    auto textWidth = fm.horizontalAdvance(mSelectLightsButton->text()) * 1.1;
    auto selectLightsWidth = int(mSize.width() * 2.5);
    if (selectLightsWidth < textWidth) {
        selectLightsWidth = textWidth;
    }
    mSelectLightsButton->setFixedSize(selectLightsWidth, int(mSize.height() * 0.5));
    mSelectLightsButton->setGeometry(mSelectLightsButton->width(),
                                     mStartSelectLightsButton,
                                     mSelectLightsButton->width(),
                                     mSelectLightsButton->height());

    showFloatingLayout(mCurrentPage);

    auto colorMenuWidth = cor::applicationSize().width() - mColorFloatingLayout->width();
    if (mPaletteWidth > colorMenuWidth) {
        mPaletteWidth = colorMenuWidth;
    }
    mMainPalette->setFixedHeight(int(mSize.height() * 0.8));
    mMainPalette->setFixedWidth(mPaletteWidth);

    hideMenuButton(mMainWindow->leftHandMenu()->alwaysOpen());
    resize(0);
    handleButtonLayouts();
}

void TopMenu::lightCountChanged() {
    // handle select lights button
    if (mData->empty() && !mMainWindow->leftHandMenu()->isIn()
        && !mMainWindow->leftHandMenu()->alwaysOpen()) {
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
        mColorPage->show(mData->mainColor(),
                         mData->lights().size(),
                         mComm->bestColorPickerType(mData->lights()));
    } else if (mCurrentPage == EPage::palettePage) {
        showRoutineWidget(false);
        showMultiColorStateWidget(!mData->empty());
        mMultiColorStateWidget->updateState(mData->colorScheme());
        mPalettePage->lightCountChanged(mData->lights().size());
    }
}

void TopMenu::highlightButton(const QString& key) {
    mPaletteFloatingLayout->highlightButton(key);
    mMoodsFloatingLayout->highlightButton(key);
    mColorFloatingLayout->highlightButton(key);
}

void TopMenu::showMenu() {
    setVisible(true);
    mRoutineFloatingLayout->setVisible(true);

    raise();
    mColorFloatingLayout->raise();
    mPaletteFloatingLayout->raise();
    mMoodsFloatingLayout->raise();
    mRoutineFloatingLayout->raise();
    mSingleColorStateWidget->raise();
    mMultiColorStateWidget->raise();
}

void TopMenu::resize(int xOffset) {
    auto parentSize = parentWidget()->size();

    moveFloatingLayout();

    int padding = 5;
    int topSpacer = mSize.height() / 8;
    int yPos = topSpacer;
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

    yPos = mMenuButton->height() + padding;
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
        if (mData->hasLightWithProtocol(EProtocolType::arduCor)
            || mData->hasLightWithProtocol(EProtocolType::nanoleaf)) {
            mPalettePage->setMode(EGroupMode::arduinoPresets);
        } else {
            mPalettePage->setMode(EGroupMode::huePresets);
        }
        showRoutineWidget(false);
        handleBrightnessSliders();
    } else if (button == "RGB") {
        if (mColorPage->isOpen()) {
            mLastColorButtonKey = button;
            mColorPage->changePageType(ESingleColorPickerMode::RGB);
        }
    } else if (button == "HSV") {
        if (mColorPage->isOpen()) {
            mLastColorButtonKey = button;
            mColorPage->changePageType(ESingleColorPickerMode::HSV);
        }
        if (mPalettePage->isOpen()) {
            mPalettePage->setMode(EGroupMode::HSV);
            handleBrightnessSliders();
            showRoutineWidget(false);
        }
    } else if (button == "Temperature") {
        mLastColorButtonKey = button;
        mColorPage->changePageType(ESingleColorPickerMode::ambient);
    } else if (button == "Routine") {
        if (mMainWindow->routineWidget()->isOpen()) {
            mRoutineFloatingLayout->highlightButton("");
            mMainWindow->routineWidget()->pushOut();
        } else {
            auto widgetGroup = EWidgetGroup::singleRoutines;
            if (mPalettePage->isOpen()) {
                widgetGroup = EWidgetGroup::multiRoutines;
            }
            mMainWindow->routineWidget()->pushIn(widgetGroup);
        }
    } else if (button == "Settings") {
        emit buttonPressed("Settings");
    } else {
        qDebug() << "I don't recognize that button type..." << button;
    }
}

void TopMenu::showSingleColorStateWidget(bool show) {
    int height = mSize.height() * 2 / 3;
    int width = 0;
    if (mMainWindow->leftHandMenu()->alwaysOpen()) {
        width = mMainWindow->leftHandMenu()->width();
    }

    QPoint startPoint(width, height);
    if (show) {
        mSingleColorStateWidget->setVisible(true);
        mSingleColorStateWidget->pushIn(startPoint);
        mSingleColorStateWidget->updateSyncStatus(ESyncState::notSynced);
    } else {
        mSingleColorStateWidget->pushOut(startPoint);
    }
}

void TopMenu::showMultiColorStateWidget(bool show) {
    int height = mSize.height() * 2 / 3;
    int width = 0;
    if (mMainWindow->leftHandMenu()->alwaysOpen()) {
        width = mMainWindow->leftHandMenu()->width();
    }

    QPoint startPoint(width, height);
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

void TopMenu::showFloatingLayout(EPage newPage) {
    if (newPage != mCurrentPage) {
        mRoutineFloatingLayout->highlightButton("");
        // move old menu back
        switch (mCurrentPage) {
            case EPage::colorPage: {
                if (mMainWindow->routineWidget()->isOpen()) {
                    mMainWindow->routineWidget()->pushOut();
                }
                showSingleColorStateWidget(false);
            } break;
            case EPage::moodPage:
                break;
            case EPage::palettePage: {
                if (mMainWindow->routineWidget()->isOpen()) {
                    mMainWindow->routineWidget()->pushOut();
                }
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
        default:
            break;
    }
    return layout;
}

void TopMenu::moveFloatingLayout() {
    auto parentSize = parentWidget()->size();
    QPoint topRight(parentSize.width(), mFloatingMenuStart);
    showRoutineWidget(true);

    if (mCurrentPage == EPage::palettePage) {
        showMultiColorStateWidget(true);
    }

    if (!mData->empty()) {
        currentFloatingLayout()->move(topRight);
    }

    moveHiddenLayouts();
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
        // only push in if light count shows more than one light, has lights capable of multiple
        // colors, and the palette page is showing the HSV picker
        if (hasMulti && mData->lightCount() > 1 && (mPalettePage->mode() == EGroupMode::HSV)) {
            shouldMoveIn = true;
        }
    }
    QPoint endPoint;
    if (shouldMoveIn) {
        endPoint = QPoint(parentSize.width() - mRoutineFloatingLayout->width(),
                          mFloatingMenuStart + mColorFloatingLayout->height());
    } else {
        if (mMainWindow->routineWidget()->isOpen()) {
            mRoutineFloatingLayout->highlightButton("");
            mMainWindow->routineWidget()->pushOut();
        }
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

void TopMenu::moveHiddenLayouts() {
    auto parentSize = parentWidget()->size();
    FloatingLayout* layout = currentFloatingLayout();
    QPoint topRight(parentSize.width() + layout->width(), mFloatingMenuStart);
    if (layout != mColorFloatingLayout) {
        mColorFloatingLayout->setGeometry(topRight.x(),
                                          topRight.y() + mColorFloatingLayout->height(),
                                          mColorFloatingLayout->width(),
                                          mColorFloatingLayout->height());
        mRoutineFloatingLayout->setGeometry(topRight.x(),
                                            topRight.y(),
                                            mRoutineFloatingLayout->width(),
                                            mRoutineFloatingLayout->height());
    }
    if (layout != mMoodsFloatingLayout) {
        mMoodsFloatingLayout->setGeometry(topRight.x(),
                                          topRight.y(),
                                          mMoodsFloatingLayout->width(),
                                          mMoodsFloatingLayout->height());
    }
    if (layout != mPaletteFloatingLayout) {
        mPaletteFloatingLayout->setGeometry(topRight.x(),
                                            topRight.y(),
                                            mPaletteFloatingLayout->width(),
                                            mPaletteFloatingLayout->height());
        mRoutineFloatingLayout->setGeometry(topRight.x(),
                                            topRight.y(),
                                            mRoutineFloatingLayout->width(),
                                            mRoutineFloatingLayout->height());
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
    if (!mSelectLightsButton->isIn() && !mMainWindow->leftHandMenu()->alwaysOpen()
        && mData->empty()) {
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
        mGlobalBrightness->updateColor(mData->mainColor());
        if (routine <= cor::ERoutineSingleColorEnd) {
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
            if (mPalettePage->mode() == EGroupMode::HSV) {
                if (mPalettePage->colorPicker()->currentScheme() == EColorSchemeType::custom) {
                    mGlobalBrightness->pushOut();
                    if (!mSingleLightBrightness->isIn()) {
                        auto color = mData->lights().begin()->state().color();
                        mSingleLightBrightness->updateColor(color);
                        mSingleLightBrightness->updateBrightness(color.valueF() * 100.0);
                    }
                    mSingleLightBrightness->pushIn();
                } else {
                    updateGlobalBrightness = true;
                }
            } else {
                updateGlobalBrightness = true;
            }
        } else if (mCurrentPage == EPage::colorPage || mCurrentPage == EPage::moodPage) {
            updateGlobalBrightness = true;
        }

        if (updateGlobalBrightness) {
            mGlobalBrightness->pushIn();
            mGlobalBrightness->lightCountChanged(mData->isOn(),
                                                 mData->mainColor(),
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
            pushRightFloatingLayout(mMoodsFloatingLayout);
            pushRightFloatingLayout(mPaletteFloatingLayout);
            break;
        case EPage::moodPage:
            mColorFloatingLayout->setVisible(false);
            mMoodsFloatingLayout->setVisible(true);
            mPaletteFloatingLayout->setVisible(false);
            pushRightFloatingLayout(mColorFloatingLayout);
            pullLeftFloatingLayout(mMoodsFloatingLayout);
            pushRightFloatingLayout(mPaletteFloatingLayout);
            break;
        case EPage::palettePage:
            if (mData->empty()) {
                pushRightFloatingLayout(mPaletteFloatingLayout);
            } else {
                pullLeftFloatingLayout(mPaletteFloatingLayout);
            }
            mColorFloatingLayout->setVisible(false);
            mMoodsFloatingLayout->setVisible(false);
            mPaletteFloatingLayout->setVisible(true);
            pushRightFloatingLayout(mColorFloatingLayout);
            pushRightFloatingLayout(mMoodsFloatingLayout);
            break;
        case EPage::settingsPage:
        case EPage::discoveryPage:
            mColorFloatingLayout->setVisible(false);
            mMoodsFloatingLayout->setVisible(false);
            mPaletteFloatingLayout->setVisible(false);
            pushRightFloatingLayout(mColorFloatingLayout);
            pushRightFloatingLayout(mMoodsFloatingLayout);
            pushRightFloatingLayout(mPaletteFloatingLayout);
            break;
        case EPage::MAX:
            break;
    }
}
