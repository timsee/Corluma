/*!
 * \copyright
 * Copyright (C) 2015 - 2019.
 * Released under the GNU General Public License.
 */

#include "topmenu.h"

#include <QGraphicsEffect>

#include "comm/hue/huemetadata.h"
#include "mainwindow.h"
#include "utils/exception.h"
#include "utils/qt.h"


QString pageToString(EPage page) {
    switch (page) {
        case EPage::colorPage:
            return "Color";
        case EPage::moodPage:
            return "Moods";
        case EPage::palettePage:
            return "Palette";
        case EPage::discoveryPage:
            return "Discovery";
        case EPage::settingsPage:
            return "Settings";
    }
    THROW_EXCEPTION("Did not recognize page");
}

EPage stringToPage(const QString& string) {
    if (string == "Color") {
        return EPage::colorPage;
    } else if (string == "Moods") {
        return EPage::moodPage;
    } else if (string == "Palette") {
        return EPage::palettePage;
    } else if (string == "Settings") {
        return EPage::settingsPage;
    } else if (string == "Discovery") {
        return EPage::discoveryPage;
    }
    THROW_EXCEPTION("String not recognized as a page");
}

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
      mSize{QSize(int(cor::applicationSize().height() * 0.1f),
                  int(cor::applicationSize().height() * 0.1f))},
      mColorIndex{0},
      mRenderTimer{new QTimer(this)},
      mMainPalette{new cor::LightVectorWidget(6, 2, true, this)},
      mSpacer{new QWidget(this)},
      mMenuButton{new QPushButton(this)},
      mGlobalBrightness{
          new GlobalBrightnessWidget(mSize, mMainWindow->leftHandMenu()->alwaysOpen(), data, this)},
      mSingleLightBrightness{
          new SingleLightBrightnessWidget(mSize, mMainWindow->leftHandMenu()->alwaysOpen(), this)} {
    connect(mRenderTimer, SIGNAL(timeout()), this, SLOT(updateUI()));
    mRenderTimer->start(100);

    //---------------
    // Create Spacer
    //---------------

    mSpacer->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    mSpacer->setFixedHeight(mSize.height());

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
    connect(mGlobalBrightness,
            SIGNAL(brightnessChanged(std::uint32_t)),
            this,
            SLOT(brightnessUpdate(std::uint32_t)));

    mSingleLightBrightness->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    connect(mSingleLightBrightness,
            SIGNAL(brightnessChanged(std::uint32_t)),
            this,
            SLOT(brightnessUpdate(std::uint32_t)));

    // --------------
    // Setup Main Palette
    // -------------
    mMainPalette->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);

    std::vector<QString> buttons;

    // --------------
    // Group Floating Layout
    // --------------
    mPaletteFloatinglayout = new FloatingLayout(false, mMainWindow);
    connect(mPaletteFloatinglayout,
            SIGNAL(buttonPressed(QString)),
            this,
            SLOT(floatingLayoutButtonPressed(QString)));
    buttons = {QString("HSV"), QString("Preset")};
    mPaletteFloatinglayout->setupButtons(buttons, EButtonSize::small);
    mPaletteFloatinglayout->highlightButton("HSV");
    mPaletteFloatinglayout->setVisible(false);

    mMultiRoutineFloatingLayout = new FloatingLayout(false, mMainWindow);
    connect(mMultiRoutineFloatingLayout,
            SIGNAL(buttonPressed(QString)),
            this,
            SLOT(floatingLayoutButtonPressed(QString)));
    mMultiRoutineFloatingLayout->setVisible(false);
    mMultiRoutineFloatingLayout->setupButtons({QString("Routine")}, EButtonSize::small);

    // --------------
    // Select Lights Button
    // --------------
    mSelectLightsButton = new SelectLightsButton(this);
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

    // --------------
    // Moods Floating Layout
    // --------------
    mMoodsFloatingLayout = new FloatingLayout(false, mMainWindow);
    connect(mMoodsFloatingLayout,
            SIGNAL(buttonPressed(QString)),
            this,
            SLOT(floatingLayoutButtonPressed(QString)));
    buttons = {QString("New_Group")};
    mMoodsFloatingLayout->setupButtons(buttons, EButtonSize::small);
    mMoodsFloatingLayout->setVisible(false);

    // --------------
    // Color Page Floating Layouts
    // --------------
    // NOTE: by making the parent the parent of the floating widgets, they don't get cut off if they
    // overextend over top menu
    mColorFloatingLayout = new FloatingLayout(false, mMainWindow);
    connect(mColorFloatingLayout,
            SIGNAL(buttonPressed(QString)),
            this,
            SLOT(floatingLayoutButtonPressed(QString)));
    mColorFloatingLayout->setVisible(false);
    mColorFloatingLayout->setupButtons({QString("HSV"), QString("RGB"), QString("Temperature")},
                                       EButtonSize::small);
    mColorFloatingLayout->highlightButton("HSV");

    mSingleRoutineFloatingLayout = new FloatingLayout(false, mMainWindow);
    connect(mSingleRoutineFloatingLayout,
            SIGNAL(buttonPressed(QString)),
            this,
            SLOT(floatingLayoutButtonPressed(QString)));
    mSingleRoutineFloatingLayout->setVisible(false);
    mSingleRoutineFloatingLayout->setupButtons({QString("Routine")}, EButtonSize::small);

    mLastColorButtonKey = "HSV";

    mSingleColorStateWidget = new SingleColorStateWidget(mMainWindow);
    mSingleColorStateWidget->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    mSingleColorStateWidget->setFixedSize(mSize.width(), mSize.height() / 2);
    mSingleColorStateWidget->setVisible(false);
    showSingleColorStateWidget(false);

    mMultiColorStateWidget = new MultiColorStateWidget(mMainWindow);
    mMultiColorStateWidget->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    mMultiColorStateWidget->setFixedSize(int(mSize.width() * 3.5), mSize.height() / 2);
    mMultiColorStateWidget->setVisible(false);
    showMultiColorStateWidget(false);

    connect(mColorPage,
            SIGNAL(routineUpdate(cor::LightState)),
            this,
            SLOT(updateState(cor::LightState)));

    connect(mPalettePage,
            SIGNAL(routineUpdate(cor::LightState)),
            this,
            SLOT(updateState(cor::LightState)));
    connect(mPalettePage->colorPicker(),
            SIGNAL(schemeUpdate(std::vector<QColor>, std::uint32_t)),
            this,
            SLOT(updateScheme(std::vector<QColor>, std::uint32_t)));

    connect(mPalettePage->colorPicker(),
            SIGNAL(selectionChanged(std::uint32_t, QColor)),
            this,
            SLOT(multiColorSelectionChange(std::uint32_t, QColor)));

    connect(mPalettePage->colorPicker(),
            SIGNAL(schemeUpdated(EColorSchemeType)),
            this,
            SLOT(colorSchemeTypeChanged(EColorSchemeType)));

    mCurrentPage = EPage::colorPage;
    showFloatingLayout(mCurrentPage);

    mPaletteWidth = mSize.width() * 3;
    auto colorMenuWidth = cor::applicationSize().width() - mColorFloatingLayout->width();
    if (mPaletteWidth > colorMenuWidth) {
        mPaletteWidth = colorMenuWidth;
    }
    mMainPalette->setFixedHeight(int(mSize.height() * 0.8));
    mMainPalette->setFixedWidth(mPaletteWidth);

    hideMenuButton(mMainWindow->leftHandMenu()->alwaysOpen());
    resize(0);
}


void TopMenu::brightnessUpdate(std::uint32_t newValue) {
    if (mCurrentPage == EPage::colorPage) {
        mColorPage->updateBrightness(newValue);
    }
    if (mCurrentPage == EPage::palettePage) {
        mPalettePage->updateBrightness(newValue);
        if (mPalettePage->mode() == EGroupMode::HSV) {
            // update the color scheme color in mData
            auto scheme = mData->colorScheme();
            auto color = scheme[mPalettePage->colorPicker()->selectedLight()];
            color.setHsvF(color.hueF(), color.saturationF(), newValue / 100.0);
            scheme[mPalettePage->colorPicker()->selectedLight()] = color;
            mData->updateColorScheme(scheme);
        }
        mMultiColorStateWidget->updateState(mData->colorScheme());
    }
}

void TopMenu::lightCountChanged() {
    // handle select lights button
    if (mData->lights().empty() && !mMainWindow->leftHandMenu()->isIn()
        && !mMainWindow->leftHandMenu()->alwaysOpen()) {
        mSelectLightsButton->pushIn(mStartSelectLightsButton);
    } else if (!mMainWindow->leftHandMenu()->alwaysOpen()) {
        mSelectLightsButton->pushOut(mStartSelectLightsButton);
    }

    handleBrightnessSliders();

    // handle standard cases
    if (mData->lights().empty()) {
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
        adjustSingleColorLayout(false);
        mColorPage->show(mData->mainColor(),
                         std::uint32_t(mData->brightness()),
                         std::uint32_t(mData->lights().size()),
                         mComm->bestColorPickerType(mData->lights()));
    } else if (mCurrentPage == EPage::palettePage) {
        showMultiColorStateWidget(!mData->lights().empty());
        mMultiColorStateWidget->updateState(mData->colorScheme());
        mPalettePage->lightCountChanged(mData->lights().size());
    }
}

void TopMenu::highlightButton(const QString& key) {
    mPaletteFloatinglayout->highlightButton(key);
    mMoodsFloatingLayout->highlightButton(key);
    mColorFloatingLayout->highlightButton(key);
}

void TopMenu::showMenu() {
    setVisible(true);
    mColorFloatingLayout->setVisible(true);
    mPaletteFloatinglayout->setVisible(true);
    mMoodsFloatingLayout->setVisible(true);
    mSingleRoutineFloatingLayout->setVisible(true);
    mMultiRoutineFloatingLayout->setVisible(true);

    raise();
    mColorFloatingLayout->raise();
    mPaletteFloatinglayout->raise();
    mMoodsFloatingLayout->raise();
    mSingleRoutineFloatingLayout->raise();
    mMultiRoutineFloatingLayout->raise();
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

    mMainPalette->setGeometry(0, yPos, mPaletteWidth, mSize.height() / 2);
    yPos += mMainPalette->height() + padding;
    yPos += mGlobalBrightness->height() + 20;

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
        }
    } else if (button == "Temperature") {
        mLastColorButtonKey = button;
        mColorPage->changePageType(ESingleColorPickerMode::ambient);
    } else if (button == "Routine") {
        if (mColorPage->isOpen()) {
            if (mColorPage->routineWidgetIsOpen()) {
                mSingleRoutineFloatingLayout->highlightButton("");
                mColorPage->handleRoutineWidget(false);
            } else {
                mColorPage->handleRoutineWidget(true);
            }
        }
        if (mPalettePage->isOpen()) {
            if (mPalettePage->routineWidgetIsOpen()) {
                mMultiRoutineFloatingLayout->highlightButton("");
                mPalettePage->handleRoutineWidget(false);
            } else {
                mPalettePage->handleRoutineWidget(true);
            }
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
    } else {
        height += mSize.height();
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
    } else {
        height += mSize.height();
    }

    QPoint startPoint(width, height);
    if (show && !mData->lights().empty()) {
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
        // move old menu back
        switch (mCurrentPage) {
            case EPage::colorPage: {
                pushRightFloatingLayout(mColorFloatingLayout);
                mSingleRoutineFloatingLayout->highlightButton("");
                if (mSingleRoutineFloatingLayout->geometry().x()
                    == parentWidget()->geometry().width() - mSingleRoutineFloatingLayout->width()) {
                    auto parentSize = parentWidget()->size();
                    cor::moveWidget(
                        mSingleRoutineFloatingLayout,
                        QPoint(parentSize.width() - mSingleRoutineFloatingLayout->width(),
                               mFloatingMenuStart + mColorFloatingLayout->height()),
                        QPoint(parentSize.width() + mSingleRoutineFloatingLayout->width(),
                               mFloatingMenuStart + mColorFloatingLayout->height()));
                }
                if (mColorPage->routineWidgetIsOpen()) {
                    mColorPage->handleRoutineWidget(false);
                }
                showSingleColorStateWidget(false);
            } break;
            case EPage::moodPage:
                pushRightFloatingLayout(mMoodsFloatingLayout);
                break;
            case EPage::palettePage: {
                pushRightFloatingLayout(mPaletteFloatinglayout);
                mMultiRoutineFloatingLayout->highlightButton("");
                if (mMultiRoutineFloatingLayout->geometry().x()
                    == parentWidget()->geometry().width() - mMultiRoutineFloatingLayout->width()) {
                    auto parentSize = parentWidget()->size();
                    cor::moveWidget(
                        mMultiRoutineFloatingLayout,
                        QPoint(parentSize.width() - mMultiRoutineFloatingLayout->width(),
                               mFloatingMenuStart + mPaletteFloatinglayout->height()),
                        QPoint(parentSize.width() + mMultiRoutineFloatingLayout->width(),
                               mFloatingMenuStart + mPaletteFloatinglayout->height()));
                }
                if (mPalettePage->routineWidgetIsOpen()) {
                    mPalettePage->handleRoutineWidget(false);
                }
                showMultiColorStateWidget(false);
            } break;
            default:
                break;
        }

        // move new menu forward
        switch (newPage) {
            case EPage::colorPage:
                pullLeftFloatingLayout(mColorFloatingLayout);
                adjustSingleColorLayout(false);
                showSingleColorStateWidget(false);
                break;
            case EPage::moodPage:
                pullLeftFloatingLayout(mMoodsFloatingLayout);
                break;
            case EPage::palettePage:
                pullLeftFloatingLayout(mPaletteFloatinglayout);
                showMultiColorStateWidget(true);
                mMultiColorStateWidget->updateState(mData->colorScheme());
                break;
            default:
                break;
        }

        mCurrentPage = newPage;
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
            layout = mPaletteFloatinglayout;
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

    if (mCurrentPage == EPage::colorPage) {
        adjustSingleColorLayout(true);
    } else if (mCurrentPage == EPage::palettePage) {
        showMultiColorStateWidget(true);
    }
    currentFloatingLayout()->move(topRight);

    moveHiddenLayouts();
}


void TopMenu::adjustSingleColorLayout(bool skipTransition) {
    bool hasArduino = mData->hasLightWithProtocol(EProtocolType::arduCor);
    bool hasNanoLeaf = mData->hasLightWithProtocol(EProtocolType::nanoleaf);

    // get the size of the parent
    auto parentSize = parentWidget()->size();
    // get the desired endpoint
    bool hasMulti = hasArduino || hasNanoLeaf;
    if (!hasMulti && !mData->lights().empty()
        && mComm->bestColorPickerType(mData->lights()) == EColorPickerType::CT) {
        mColorPage->changePageType(ESingleColorPickerMode::ambient);
        mColorFloatingLayout->highlightButton("Temperature");
    }
    QPoint endPoint;
    if (hasMulti) {
        endPoint = QPoint(parentSize.width() - mSingleRoutineFloatingLayout->width(),
                          mFloatingMenuStart + mColorFloatingLayout->height());
    } else {
        if (mColorPage->routineWidgetIsOpen()) {
            mSingleRoutineFloatingLayout->highlightButton("");
            mColorPage->handleRoutineWidget(false);
        }
        endPoint = QPoint(parentSize.width() - mSingleRoutineFloatingLayout->width()
                              + mSingleRoutineFloatingLayout->height(),
                          mFloatingMenuStart + mColorFloatingLayout->height());
    }

    if (skipTransition) {
        mSingleRoutineFloatingLayout->setGeometry(endPoint.x(),
                                                  endPoint.y(),
                                                  mSingleRoutineFloatingLayout->width(),
                                                  mSingleRoutineFloatingLayout->height());
    } else {
        cor::moveWidget(mSingleRoutineFloatingLayout,
                        mSingleRoutineFloatingLayout->pos(),
                        endPoint);
    }
}

void TopMenu::pushRightFloatingLayout(FloatingLayout* layout) {
    auto parentSize = parentWidget()->size();
    cor::moveWidget(layout,
                    QPoint(parentSize.width() - layout->width(), mFloatingMenuStart),
                    QPoint(parentSize.width() + layout->width(), mFloatingMenuStart));
}


void TopMenu::pullLeftFloatingLayout(FloatingLayout* layout) {
    auto parentSize = parentWidget()->size();
    cor::moveWidget(layout,
                    QPoint(parentSize.width() + layout->width(), mFloatingMenuStart),
                    QPoint(parentSize.width() - layout->width(), mFloatingMenuStart));
}

void TopMenu::moveHiddenLayouts() {
    auto parentSize = parentWidget()->size();
    QPoint topRight(parentSize.width(), mFloatingMenuStart);
    FloatingLayout* layout = currentFloatingLayout();
    if (layout != mColorFloatingLayout) {
        mColorFloatingLayout->setGeometry(topRight.x(),
                                          topRight.y() + mColorFloatingLayout->height(),
                                          mColorFloatingLayout->width(),
                                          mColorFloatingLayout->height());
        mSingleRoutineFloatingLayout->setGeometry(topRight.x(),
                                                  topRight.y(),
                                                  mSingleRoutineFloatingLayout->width(),
                                                  mSingleRoutineFloatingLayout->height());
    }
    if (layout != mMoodsFloatingLayout) {
        mMoodsFloatingLayout->setGeometry(topRight.x(),
                                          topRight.y(),
                                          mMoodsFloatingLayout->width(),
                                          mMoodsFloatingLayout->height());
    }
    if (layout != mPaletteFloatinglayout) {
        mPaletteFloatinglayout->setGeometry(topRight.x(),
                                            topRight.y(),
                                            mPaletteFloatinglayout->width(),
                                            mPaletteFloatinglayout->height());
        mMultiRoutineFloatingLayout->setGeometry(topRight.x(),
                                                 topRight.y() + mPaletteFloatinglayout->height(),
                                                 mMultiRoutineFloatingLayout->width(),
                                                 mMultiRoutineFloatingLayout->height());
    }
}

void TopMenu::updateUI() {
    if (mData->lights() != mLastDevices) {
        // get copy of data representation of lights
        auto currentLights = mComm->commLightsFromVector(mData->lights());
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
        && mData->lights().empty()) {
        mSelectLightsButton->pushIn(mStartSelectLightsButton);
    }
}

void TopMenu::pushOutTapToSelectButton() {
    if (mSelectLightsButton->isIn()) {
        mSelectLightsButton->pushOut(mStartSelectLightsButton);
    }
}


void TopMenu::updateState(const cor::LightState& state) {
    mData->updateState(state);
    if (mCurrentPage == EPage::colorPage) {
        if (!mSingleColorStateWidget->isIn()) {
            showSingleColorStateWidget(true);
        }
        ERoutine routine = state.routine();
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
        // update brightness of the slider only if the wheel isn't HSV
        bool isHSVWheel =
            (mCurrentPage == EPage::palettePage && mPalettePage->mode() == EGroupMode::HSV)
            || (mCurrentPage == EPage::colorPage
                && mColorPage->pageType() == ESingleColorPickerMode::HSV);
        if (!isHSVWheel) {
            mGlobalBrightness->updateBrightness(mData->mainColor().valueF() * 100.0);
        }
    }
}

void TopMenu::updateScheme(const std::vector<QColor>& colors, std::uint32_t index) {
    mData->updateColorScheme(colors);
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

void TopMenu::multiColorSelectionChange(std::uint32_t index, const QColor& color) {
    mColorIndex = index;
    if (mCurrentPage == EPage::palettePage) {
        if (mPalettePage->colorPicker()->currentScheme() == EColorSchemeType::custom) {
            mGlobalBrightness->updateColor(mData->mainColor());
            mSingleLightBrightness->updateColor(color);
            mSingleLightBrightness->updateBrightness(color.valueF() * 100.0);
        }
    }
}

void TopMenu::colorSchemeTypeChanged(EColorSchemeType scheme) {
    handleBrightnessSliders();
}

void TopMenu::handleBrightnessSliders() {
    if (mData->lights().empty()) {
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

void TopMenu::dataInSync(bool inSync) {
    ESyncState state = ESyncState::syncing;
    if (inSync) {
        state = ESyncState::synced;
    }
    if (mCurrentPage == EPage::colorPage) {
        mSingleColorStateWidget->updateSyncStatus(state);
    } else if (mCurrentPage == EPage::palettePage) {
        mMultiColorStateWidget->updateSyncStatus(state);
    }
}
