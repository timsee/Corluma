/*!
 * \copyright
 * Copyright (C) 2015 - 2019.
 * Released under the GNU General Public License.
 */

#include "topmenu.h"
#include <QGraphicsEffect>
#include "comm/hue/huelight.h"
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
                 cor::DeviceList* data,
                 CommLayer* comm,
                 GroupData* groups,
                 MainWindow* mainWindow,
                 PalettePage* palettePage,
                 ColorPage* colorPage)
    : QWidget(parent),
      mData(data),
      mComm(comm),
      mGroups(groups),
      mShouldGreyOutIcons{true},
      mMainWindow(mainWindow),
      mPalettePage(palettePage),
      mColorPage(colorPage) {
    mLastParentSizeColorMenu = QSize(0u, 0u);
    QSize size = cor::applicationSize();
    mSize = QSize(int(size.height() * 0.1f), int(size.height() * 0.1f));
    mStartSelectLightsButton = mSize.height();
    mFloatingMenuStart = mSize.height();
    mColorMenuType = EColorMenuType::none;


    mRenderTimer = new QTimer(this);
    connect(mRenderTimer, SIGNAL(timeout()), this, SLOT(updateUI()));
    mRenderTimer->start(100);

    //---------------
    // Create Spacer
    //---------------

    mSpacer = new QWidget(this);
    mSpacer->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    mSpacer->setFixedHeight(mSize.height());

    // --------------
    // Setup menu button
    // --------------
    mMenuButton = new QPushButton(this);
    mMenuButton->setVisible(true);
    mMenuButton->setFixedHeight(int(mSize.height() * 0.8));
    cor::resizeIcon(mMenuButton, ":/images/hamburger_icon.png", 0.8f);
    connect(mMenuButton, SIGNAL(clicked(bool)), this, SLOT(menuButtonPressed()));

    // --------------
    // Setup Brightness Slider
    // --------------
    // setup the slider that controls the LED's brightness
    mBrightnessSlider = new cor::Slider(this);
    mBrightnessSlider->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    mBrightnessSlider->slider()->setRange(2, 100);
    mBrightnessSlider->slider()->setValue(2);
    mBrightnessSlider->setFixedHeight(mSize.height() / 2);
    mBrightnessSlider->setHeightPercentage(0.8f);
    mBrightnessSlider->setColor(QColor(255, 255, 255));
    mBrightnessSlider->enable(false);
    connect(mBrightnessSlider, SIGNAL(valueChanged(int)), this, SLOT(brightnessSliderChanged(int)));

    // --------------
    // Setup Main Palette
    // -------------
    mMainPalette = new cor::LightVectorWidget(6, 2, true, this);
    mMainPalette->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    mMainPalette->setFixedHeight(int(mSize.height() * 0.8));
    mMainPalette->setFixedWidth(mSize.width() * 3);

    // --------------
    // Setup on/off switch
    // --------------
    mOnOffSwitch = new cor::Switch(this);
    mOnOffSwitch->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    mOnOffSwitch->setFixedSize(QSize(mSize.width(), mSize.height() / 2));
    connect(mOnOffSwitch, SIGNAL(switchChanged(bool)), this, SLOT(changedSwitchState(bool)));
    mOnOffSwitch->setSwitchState(ESwitchState::disabled);

    std::vector<QString> buttons;

    // --------------
    // Group Floating Layout
    // --------------
    mPaletteFloatinglayout = new FloatingLayout(false, mMainWindow);
    connect(mPaletteFloatinglayout,
            SIGNAL(buttonPressed(QString)),
            this,
            SLOT(floatingLayoutButtonPressed(QString)));
    buttons = {QString("RGB"), QString("HSV"), QString("Preset")};
    mPaletteFloatinglayout->setupButtons(buttons, EButtonSize::small);
    mPaletteFloatinglayout->highlightButton("RGB");
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
    mSelectLightsButton->setFixedSize(mSize.width() * 3, int(mSize.height() * 0.5));

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
    mColorFloatingLayout->setupButtons({QString("RGB"), QString("HSV"), QString("Temperature")},
                                       EButtonSize::small);
    mColorFloatingLayout->highlightButton("RGB");

    mSingleRoutineFloatingLayout = new FloatingLayout(false, mMainWindow);
    connect(mSingleRoutineFloatingLayout,
            SIGNAL(buttonPressed(QString)),
            this,
            SLOT(floatingLayoutButtonPressed(QString)));
    mSingleRoutineFloatingLayout->setVisible(false);
    mSingleRoutineFloatingLayout->setupButtons({QString("Routine")}, EButtonSize::small);

    mLastColorButtonKey = "RGB";

    mSingleColorStateWidget = new SingleColorStateWidget(mMainWindow);
    mSingleColorStateWidget->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    mSingleColorStateWidget->setFixedSize(mSize.width(), mSize.height() / 2);
    showSingleColorStateWidget(true);

    mMultiColorStateWidget = new MultiColorStateWidget(mMainWindow);
    mMultiColorStateWidget->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    mMultiColorStateWidget->setFixedSize(mSize.width() * 3.5, mSize.height() / 2);
    showMultiColorStateWidget(false);

    deviceCountChanged();

    mCurrentPage = EPage::colorPage;
    showFloatingLayout(mCurrentPage);

    resize(0);
}


void TopMenu::changedSwitchState(bool switchState) {
    mData->turnOn(switchState);
    emit buttonPressed("OnOff");
}


void TopMenu::brightnessSliderChanged(int newBrightness) {
    brightnessUpdate(uint32_t(newBrightness));
}

void TopMenu::brightnessUpdate(uint32_t newValue) {
    mData->updateBrightness(newValue);
    mData->turnOn(true);
    // update the top menu bar
    updateBrightnessSlider();

    if (newValue != 0 && mOnOffSwitch->switchState() != ESwitchState::on) {
        mOnOffSwitch->setSwitchState(ESwitchState::on);
    }

    if (mCurrentPage == EPage::colorPage) {
        mColorPage->updateBrightness(newValue);
    }
    if (mCurrentPage == EPage::palettePage) {
        mPalettePage->updateBrightness(newValue);
    }
    emit brightnessChanged(newValue);
}

void TopMenu::updateBrightnessSlider() {
    mBrightnessSlider->setColor(mData->mainColor());

    if (mData->brightness() != mBrightnessSlider->slider()->value()) {
        mBrightnessSlider->blockSignals(true);
        mBrightnessSlider->slider()->setValue(mData->brightness());
        mBrightnessSlider->blockSignals(false);
    }
}

void TopMenu::deviceCountChanged() {
    if (mData->devices().empty() && !mMainWindow->leftHandMenu()->isIn()
        && !mMainWindow->leftHandMenu()->alwaysOpen()) {
        mSelectLightsButton->pushIn(mStartSelectLightsButton);
    } else if (!mMainWindow->leftHandMenu()->alwaysOpen()) {
        mSelectLightsButton->pushOut(mStartSelectLightsButton);
    }

    if (mShouldGreyOutIcons && !mData->devices().empty()) {
        mBrightnessSlider->enable(true);

        if (mData->isOn()) {
            mOnOffSwitch->setSwitchState(ESwitchState::on);
        } else {
            mOnOffSwitch->setSwitchState(ESwitchState::off);
        }

        updateBrightnessSlider();
        mShouldGreyOutIcons = false;
    }

    if ((!mShouldGreyOutIcons && mData->devices().empty())) {
        mBrightnessSlider->enable(false);
        mBrightnessSlider->slider()->setValue(0);
        mOnOffSwitch->setSwitchState(ESwitchState::disabled);

        mShouldGreyOutIcons = true;
    }

    if (mCurrentPage == EPage::colorPage) {
        adjustSingleColorLayout(false);
        mColorPage->show(mData->mainColor(),
                         uint32_t(mData->brightness()),
                         uint32_t(mData->devices().size()),
                         mData->bestColorPickerType());
    } else if (mCurrentPage == EPage::palettePage) {
        adjustMultiColorLayout(false);
        mPalettePage->lightCountChanged(mData->devices().size());
    }
}

void TopMenu::highlightButton(const QString& key) {
    mPaletteFloatinglayout->highlightButton(key);
    mMoodsFloatingLayout->highlightButton(key);
    mColorFloatingLayout->highlightButton(key);
}

void TopMenu::showMenu() {
    this->setVisible(true);
    mColorFloatingLayout->setVisible(true);
    mPaletteFloatinglayout->setVisible(true);
    mMoodsFloatingLayout->setVisible(true);
    mSingleRoutineFloatingLayout->setVisible(true);
    mMultiRoutineFloatingLayout->setVisible(true);

    this->raise();
    mColorFloatingLayout->raise();
    mPaletteFloatinglayout->raise();
    mMoodsFloatingLayout->raise();
    mSingleRoutineFloatingLayout->raise();
    mMultiRoutineFloatingLayout->raise();
    mSingleColorStateWidget->raise();
    mMultiColorStateWidget->raise();
}

void TopMenu::resize(int xOffset) {
    auto parentSize = qobject_cast<QWidget*>(this->parent())->size();

    moveFloatingLayout();

    int yPos = cor::statusBarOffset();
    int padding = 5;
    mMenuButton->setGeometry(0, yPos, mSize.width(), mSize.height());
    mSpacer->setGeometry(mSize.width(), yPos, this->width() - mSize.width(), mSize.height());

    if (!mMainWindow->leftHandMenu()->alwaysOpen()) {
        mOnOffSwitch->setGeometry(
            int(mSize.width() * 1.1), yPos, mSize.width(), mSize.height() / 2);

        mBrightnessSlider->setGeometry(mSize.width() * 2 + 5,
                                       yPos,
                                       this->width() - int(mSize.width() * 2.1),
                                       mSize.height() / 2);

        mMainPalette->setVisible(true);
    } else {
        mOnOffSwitch->setGeometry(
            int(mSize.width() * 0.1), yPos, mSize.width(), mSize.height() / 2);

        mBrightnessSlider->setGeometry(
            mSize.width() + 5, yPos, this->width() - int(mSize.width() * 1.1), mSize.height() / 2);

        mMainPalette->setVisible(false);
    }

    yPos += mMenuButton->height() + padding;
    mStartSelectLightsButton = yPos;
    mFloatingMenuStart = yPos;

    mMainPalette->setGeometry(0, yPos, int(this->width() * 0.8), mSize.height() / 2);
    yPos += mMainPalette->height() + padding;

    yPos += mBrightnessSlider->height() + 20;

    mSelectLightsButton->resize(mStartSelectLightsButton);

    this->setGeometry(xOffset, 0, parentSize.width() - xOffset, yPos);
}


void TopMenu::resizeEvent(QResizeEvent*) {
    resize(this->geometry().x());
}

//--------------------
// FloatingLayout Setup
//--------------------

void TopMenu::floatingLayoutButtonPressed(const QString& button) {
    if (button == "Discovery") {
        mMainWindow->pushInDiscovery();
    } else if (button == "New_Group") {
        if (mCurrentPage == EPage::moodPage) {
            mMainWindow->editButtonClicked(true);
        } else {
            mMainWindow->editButtonClicked(false);
        }
    } else if (button == "Preset_Groups") {
        if (mData->hasLightWithProtocol(EProtocolType::arduCor)
            || mData->hasLightWithProtocol(EProtocolType::nanoleaf)) {
            mPalettePage->setMode(EGroupMode::arduinoPresets);
        } else {
            mPalettePage->setMode(EGroupMode::huePresets);
        }
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

        if (mPalettePage->isOpen()) {
            mPalettePage->setMode(EGroupMode::RGB);
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
    int height = mSize.height() * 2 / 3 + cor::statusBarOffset();
    int width = 0;
    if (!mMainWindow->leftHandMenu()->alwaysOpen()) {
        height += mSize.height();
    } else {
        width = mMainWindow->leftHandMenu()->width();
    }
    QPoint shownPoint(width, height);
    QPoint hiddenPoint(-mSingleColorStateWidget->size().width(), height);
    if (show) {
        cor::moveWidget(
            mSingleColorStateWidget, mSingleColorStateWidget->size(), hiddenPoint, shownPoint);
        mSingleColorStateWidget->updateSyncStatus(ESyncState::notSynced);
    } else {
        cor::moveWidget(
            mSingleColorStateWidget, mSingleColorStateWidget->size(), shownPoint, hiddenPoint);
    }
}

void TopMenu::showMultiColorStateWidget(bool show) {
    int height = mSize.height() * 2 / 3 + cor::statusBarOffset();
    int width = 0;
    if (!mMainWindow->leftHandMenu()->alwaysOpen()) {
        height += mSize.height();
    } else {
        width = mMainWindow->leftHandMenu()->width();
    }
    QPoint shownPoint(width, height);
    QPoint hiddenPoint(-mMultiColorStateWidget->size().width(), height);
    if (show) {
        cor::moveWidget(
            mMultiColorStateWidget, mMultiColorStateWidget->size(), hiddenPoint, shownPoint);
        mMultiColorStateWidget->updateSyncStatus(ESyncState::notSynced);
    } else {
        cor::moveWidget(
            mMultiColorStateWidget, mMultiColorStateWidget->size(), shownPoint, hiddenPoint);
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
                    == this->parentWidget()->geometry().width()
                           - mSingleRoutineFloatingLayout->width()) {
                    auto parentSize = qobject_cast<QWidget*>(this->parent())->size();
                    cor::moveWidget(
                        mSingleRoutineFloatingLayout,
                        mSingleRoutineFloatingLayout->size(),
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
                    == this->parentWidget()->geometry().width()
                           - mMultiRoutineFloatingLayout->width()) {
                    auto parentSize = qobject_cast<QWidget*>(this->parent())->size();
                    cor::moveWidget(
                        mMultiRoutineFloatingLayout,
                        mMultiRoutineFloatingLayout->size(),
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
                showSingleColorStateWidget(true);
                break;
            case EPage::moodPage:
                pullLeftFloatingLayout(mMoodsFloatingLayout);
                break;
            case EPage::palettePage:
                pullLeftFloatingLayout(mPaletteFloatinglayout);
                adjustMultiColorLayout(false);
                showMultiColorStateWidget(true);
                break;
            default:
                break;
        }

        mCurrentPage = newPage;
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
    auto parentSize = qobject_cast<QWidget*>(this->parent())->size();
    QPoint topRight(parentSize.width(), mFloatingMenuStart);

    if (mCurrentPage == EPage::colorPage) {
        adjustSingleColorLayout(true);
    } else if (mCurrentPage == EPage::palettePage) {
        adjustMultiColorLayout(true);
    }
    currentFloatingLayout()->move(topRight);

    moveHiddenLayouts();
}


void TopMenu::adjustSingleColorLayout(bool skipTransition) {
    bool hasArduino = mData->hasLightWithProtocol(EProtocolType::arduCor);
    bool hasNanoLeaf = mData->hasLightWithProtocol(EProtocolType::nanoleaf);

    // get the size of the parent
    auto parentSize = qobject_cast<QWidget*>(this->parent())->size();
    // get teh desired endpoint
    bool hasMulti = hasArduino || hasNanoLeaf;
    if (!hasMulti && !mData->devices().empty()
        && mData->bestColorPickerType() == EColorPickerType::CT) {
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
                        mSingleRoutineFloatingLayout->size(),
                        mSingleRoutineFloatingLayout->pos(),
                        endPoint);
    }
}

void TopMenu::adjustMultiColorLayout(bool skipTransition) {
    bool hasArduino = mData->hasLightWithProtocol(EProtocolType::arduCor);
    bool hasNanoLeaf = mData->hasLightWithProtocol(EProtocolType::nanoleaf);

    // get the size of the parent
    auto parentSize = qobject_cast<QWidget*>(this->parent())->size();
    // get teh desired endpoint
    bool hasLights = !mData->devices().empty();
    QPoint endPoint;
    if (hasLights && (hasArduino || hasNanoLeaf)) {
        endPoint = QPoint(parentSize.width() - mMultiRoutineFloatingLayout->width(),
                          mFloatingMenuStart + mPaletteFloatinglayout->height());
    } else {
        if (mPalettePage->routineWidgetIsOpen()) {
            mMultiRoutineFloatingLayout->highlightButton("");
            mPalettePage->handleRoutineWidget(false);
        }
        endPoint = QPoint(parentSize.width() - mMultiRoutineFloatingLayout->width()
                              + mMultiRoutineFloatingLayout->height(),
                          mFloatingMenuStart + mPaletteFloatinglayout->height());
    }


    if (skipTransition) {
        mMultiRoutineFloatingLayout->setGeometry(endPoint.x(),
                                                 endPoint.y(),
                                                 mMultiRoutineFloatingLayout->width(),
                                                 mMultiRoutineFloatingLayout->height());
    } else {
        cor::moveWidget(mMultiRoutineFloatingLayout,
                        mMultiRoutineFloatingLayout->size(),
                        mMultiRoutineFloatingLayout->pos(),
                        endPoint);
    }
}

void TopMenu::pushRightFloatingLayout(FloatingLayout* layout) {
    auto parentSize = qobject_cast<QWidget*>(this->parent())->size();
    cor::moveWidget(layout,
                    layout->size(),
                    QPoint(parentSize.width() - layout->width(), mFloatingMenuStart),
                    QPoint(parentSize.width() + layout->width(), mFloatingMenuStart));
}


void TopMenu::pullLeftFloatingLayout(FloatingLayout* layout) {
    auto parentSize = qobject_cast<QWidget*>(this->parent())->size();
    cor::moveWidget(layout,
                    layout->size(),
                    QPoint(parentSize.width() + layout->width(), mFloatingMenuStart),
                    QPoint(parentSize.width() - layout->width(), mFloatingMenuStart));
}

void TopMenu::moveHiddenLayouts() {
    auto parentSize = qobject_cast<QWidget*>(this->parent())->size();
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
    if (mData->devices() != mLastDevices) {
        // get copy of data representation of lights
        auto currentDevices = mData->devices();
        // update devices to be comm representation instead of data representaiton
        for (auto&& device : currentDevices) {
            device = mComm->lightByID(device.uniqueID());
        }
        mLastDevices = currentDevices;

        mMainPalette->updateDevices(currentDevices);
        updateBrightnessSlider();
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
        && mData->devices().empty()) {
        mSelectLightsButton->pushIn(mStartSelectLightsButton);
    }
}

void TopMenu::pushOutTapToSelectButton() {
    if (mSelectLightsButton->isIn()) {
        mSelectLightsButton->pushOut(mStartSelectLightsButton);
    }
}


void TopMenu::updateRoutine(const QJsonObject& routineObject) {
    if (mCurrentPage == EPage::colorPage) {
        ERoutine routine = stringToRoutine(routineObject["routine"].toString());
        QColor color;
        color.setHsvF(routineObject["hue"].toDouble(),
                      routineObject["sat"].toDouble(),
                      routineObject["bri"].toDouble());
        mSingleColorStateWidget->updateState(color, routine);
    }
}

void TopMenu::updateScheme(const std::vector<QColor>& colors) {
    if (mCurrentPage == EPage::palettePage) {
        mMultiColorStateWidget->updateState(colors);
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
