/*!
 * \copyright
 * Copyright (C) 2015 - 2020.
 * Released under the GNU General Public License.
 */

#include "palettescrollarea.h"
#include <QScrollBar>
#include <QScroller>
#include "cor/stylesheets.h"
#include "utils/qt.h"

namespace {

void initScrollArea(QWidget* widget, QScrollArea* scrollArea) {
    QScroller::grabGesture(scrollArea->viewport(), QScroller::LeftMouseButtonGesture);
    scrollArea->setWidget(widget);
    scrollArea->setFrameStyle(QFrame::NoFrame);
    scrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    scrollArea->horizontalScrollBar()->setEnabled(false);
    scrollArea->horizontalScrollBar()->setVisible(false);
}

} // namespace


PaletteScrollArea::PaletteScrollArea(QWidget* parent, const std::vector<cor::Palette>& palettes)
    : QWidget(parent),
      mState{EState::palettes},
      mWidgetHeight{10u},
      mScrollArea{new QScrollArea(this)},
      mParentScrollArea{new QScrollArea(this)},
      mPaletteContainer{new MenuPaletteContainer(mScrollArea)},
      mParentContainer{new MenuGenericGroupContainer(mParentScrollArea)},
      mParentWidget{new cor::GroupButton("", "", this)} {
    initScrollArea(mPaletteContainer, mScrollArea);
    initScrollArea(mParentContainer, mParentScrollArea);

    connect(mPaletteContainer,
            SIGNAL(paletteSelected(cor::Palette)),
            this,
            SLOT(buttonClicked(cor::Palette)));


    mParentWidget->setVisible(false);
    mParentWidget->changeArrowState(cor::EArrowState::open);
    mParentWidget->showSelectAllCheckbox(false);
    connect(mParentWidget,
            SIGNAL(groupButtonPressed(QString)),
            this,
            SLOT(topParentClicked(QString)));
    mParentWidget->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);

    connect(mParentContainer, SIGNAL(parentClicked(QString)), this, SLOT(parentClicked(QString)));
    showPalettes(palettes);
    setStyleSheet(cor::kDarkerGreyBackground);
}



void PaletteScrollArea::clear() {
    mPaletteContainer->clear();
}

void PaletteScrollArea::showPalettes(std::vector<cor::Palette> palettes) {
    mState = EState::palettes;
    mParentWidget->setVisible(false);
    mScrollArea->setVisible(true);
    mParentScrollArea->setVisible(false);
    mParentContainer->setVisible(false);
    mPaletteContainer->setVisible(true);
    mPaletteGroups = {};
    mPaletteContainer->showPalettes(palettes);
    resize();
}


void PaletteScrollArea::showPalettesWithParents(const std::vector<cor::PaletteGroup>& groups) {
    mState = EState::parents;
    mPaletteGroups = groups;

    topParentClicked("");

    // make parent name vector
    std::vector<QString> parentNames;
    for (const auto& paletteGroup : groups) {
        parentNames.push_back(paletteGroup.name());
    }
    mParentContainer->showGroups(parentNames, mWidgetHeight * 0.66);
}



void PaletteScrollArea::buttonClicked(cor::Palette palette) {
    emit paletteClicked(palette);
}

void PaletteScrollArea::topParentClicked(QString) {
    mState = EState::parents;
    mParentWidget->setVisible(false);
    mScrollArea->setVisible(false);
    mParentScrollArea->setVisible(true);
    mParentContainer->setVisible(true);
    mPaletteContainer->setVisible(false);
    resize();
}

void PaletteScrollArea::parentClicked(QString key) {
    mParentWidget->changeText(key);

    for (const auto& paletteGroup : mPaletteGroups) {
        if (paletteGroup.name() == key) {
            mPaletteContainer->showPalettes(paletteGroup.palettes());
        }
    }

    mState = EState::palettesForParent;
    mParentWidget->setVisible(true);
    mScrollArea->setVisible(true);
    mParentScrollArea->setVisible(false);
    mParentContainer->setVisible(false);
    mPaletteContainer->setVisible(true);
    resize();
}

void PaletteScrollArea::resize() {
    int offsetY = 0u;
    QRect rect = QRect(0, offsetY, this->width(), this->height() - offsetY);
    int scrollAreaWidth = int(rect.width() * 1.2);
    if (mState == EState::parents) {
        mParentScrollArea->setGeometry(rect.x(), rect.y(), scrollAreaWidth, rect.height());
        mParentContainer->setGeometry(0, 0, rect.width(), rect.height());
        mParentContainer->resizeWidgets(mWidgetHeight * 0.66);
    } else if (mState == EState::palettesForParent || mState == EState::palettes) {
        auto scrollAreaHeight = this->height();
        if (mParentWidget->isVisible()) {
            mParentWidget->setFixedHeight(mWidgetHeight * 0.66);
            mParentWidget->setGeometry(0, 0, width(), mWidgetHeight * 0.66);
            offsetY += mParentWidget->height();
            scrollAreaHeight -= mParentWidget->height();
        }
        mScrollArea->setGeometry(0, offsetY, scrollAreaWidth, scrollAreaHeight);
        mPaletteContainer->setFixedWidth(rect.width());
    }
}
