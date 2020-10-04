/*!
 * \copyright
 * Copyright (C) 2015 - 2020.
 * Released under the GNU General Public License.
 */
#include "edit/editpage.h"
#include <QInputDialog>
#include <QMessageBox>
#include <QStyleOption>
#include <QtCore>
#include <QtGui>
#include "utils/qt.h"

namespace cor {

EditPage::EditPage(QWidget* parent, CommLayer* comm, GroupData* groups, bool showPreviewButton)
    : QWidget(parent),
      mPreviewButton{new QPushButton("Preview", this)},
      mComm(comm),
      mGroups(groups),
      mPlaceholder{new QWidget(this)},
      mCloseButton{new QPushButton(this)},
      mUsePreviewButton{showPreviewButton} {
    connect(mCloseButton, SIGNAL(clicked(bool)), this, SLOT(closePressed(bool)));
    mPreviewButton->setVisible(mUsePreviewButton);

#ifdef MOBILE_BUILD
    mTopHeight = cor::applicationSize().height() * 0.075;
#else
    mTopHeight = int(cor::applicationSize().height() * 0.1);
#endif
}

void EditPage::setupWidgets(std::vector<EditPageChildWidget*> widgets) {
    mWidgets = widgets;
    mProgressWidget = new EditProgressWidget(this, std::uint32_t(mWidgets.size()));
    // wire up the bottom buttons
    auto i = 0u;
    for (auto widget : mWidgets) {
        widget->index(i);
        ++i;
        connect(widget->bottomButtons(), SIGNAL(leftButtonPressed()), this, SLOT(pageBackwards()));
        connect(widget->bottomButtons(), SIGNAL(rightButtonPressed()), this, SLOT(pageForwards()));
        connect(widget,
                SIGNAL(stateChanged(std::uint32_t, EEditProgressState)),
                this,
                SLOT(widgetChangedState(std::uint32_t, EEditProgressState)));
        connect(widget, SIGNAL(closePage()), this, SLOT(closeFromPagePressed()));
        connect(widget, SIGNAL(updateGroups()), this, SLOT(updateGroupsFromPagePressed()));
    }

    connect(mProgressWidget,
            SIGNAL(changePage(std::uint32_t)),
            this,
            SLOT(changePageFromProgressWidget(std::uint32_t)));
}

void EditPage::pushIn(const QPoint& startPoint, const QPoint& endPoint) {
    setVisible(true);
    raise();
    isOpen(true);
    moveWidget(this, startPoint, endPoint);
}

void EditPage::pushOut(const QPoint& endPoint) {
    moveWidget(this, pos(), endPoint);
    isOpen(false);
}

void EditPage::reset() {
    mProgressWidget->reset();
    showPage(0);
}

void EditPage::changePageFromProgressWidget(std::uint32_t index) {
    showPage(index);
}

void EditPage::showPage(std::uint32_t pageIndex) {
    // update top widget
    editProgressWidget()->changeToPage(pageIndex);

    // update the currently showing widget
    showAndResizePage(pageIndex);

    // do widget specific initializations
    pageChanged(pageIndex);
}

void EditPage::pageForwards() {
    auto currentIndex = editProgressWidget()->currentPage();
    if (currentIndex < editProgressWidget()->numberOfPages()) {
        auto newIndex = currentIndex + 1;
        showPage(newIndex);
    }
}

void EditPage::pageBackwards() {
    auto currentIndex = editProgressWidget()->currentPage();
    if (currentIndex > 0) {
        showPage(currentIndex - 1);
    }
}

void EditPage::closeFromPagePressed() {
    emit closePressed(true);
}

void EditPage::widgetChangedState(std::uint32_t i, EEditProgressState state) {
    editProgressWidget()->updateState(i, state);
    // programmatically check states of other pages based off of the changing page
    if (i < (editProgressWidget()->numberOfPages() - 1)) {
        // first unlock/lock the next page
        auto newIndex = i + 1;
        if (state == EEditProgressState::completed
            && editProgressWidget()->state(newIndex) == EEditProgressState::locked) {
            editProgressWidget()->updateState(newIndex, EEditProgressState::incomplete);
        } else if (state == EEditProgressState::incomplete) {
            editProgressWidget()->updateState(newIndex, EEditProgressState::locked);
        }
        // now compute the final page
        computeStateOfReviewPage();
    }
}

void EditPage::computeStateOfReviewPage() {
    auto editIndex = editProgressWidget()->numberOfPages() - 1;
    bool allWidgetsComplete = true;
    bool noWidgetsLocked = true;
    for (auto i = 0u; i < editIndex; ++i) {
        if (editProgressWidget()->state(i) != EEditProgressState::completed) {
            allWidgetsComplete = false;
        }

        if (editProgressWidget()->state(i) == EEditProgressState::locked) {
            noWidgetsLocked = false;
        }
    }

    if (allWidgetsComplete && noWidgetsLocked) {
        editProgressWidget()->updateState(editIndex, EEditProgressState::completed);
    } else if (!noWidgetsLocked) {
        editProgressWidget()->updateState(editIndex, EEditProgressState::locked);
    }
}


void EditPage::showAndResizePage(std::uint32_t i) {
    for (auto widget : widgets()) {
        widget->setVisible(false);
    }
    widgets()[i]->setVisible(true);
    widgets()[i]->setGeometry(mPlaceholder->geometry());
    widgets()[i]->raise();
}

void EditPage::resizeCloseButton() {
    QPixmap pixmap(":images/closeX.png");
    int closeSize = int(mTopHeight * 0.8);
    int finalSize = int(mTopHeight * 0.5);
    int spacer = (mTopHeight - finalSize) / 4;
    mCloseButton->setIconSize(QSize(finalSize, finalSize));
    mCloseButton->setIcon(
        QIcon(pixmap.scaled(finalSize, finalSize, Qt::KeepAspectRatio, Qt::SmoothTransformation)));
    mCloseButton->setGeometry(spacer, spacer, closeSize, closeSize);
}

void EditPage::resizeEvent(QResizeEvent*) {
    resizeCloseButton();
    int yPos = mTopHeight;
    // width of progress widget should be relatively fixed
    auto progressWidth = int(editProgressWidget()->numberOfPages() * std::uint32_t(mTopHeight));
    editProgressWidget()->setGeometry(this->width() - progressWidth, 0u, progressWidth, mTopHeight);

    auto placeholderWidthPadding = this->width() / 20;
    auto placeholderHeightPadding = this->height() / 20;
    if (mUsePreviewButton) {
        auto previewButtonWidth = this->width() * 1 / 4;
        mPreviewButton->setGeometry(this->width() - previewButtonWidth,
                                    yPos,
                                    previewButtonWidth,
                                    placeholderHeightPadding);
        yPos += mPreviewButton->height();
    } else {
        yPos += placeholderHeightPadding;
    }
    mPlaceholder->setGeometry(placeholderWidthPadding,
                              yPos,
                              this->width() - placeholderWidthPadding * 2,
                              this->height() - yPos - placeholderHeightPadding);
    showAndResizePage(editProgressWidget()->currentPage());
}

void EditPage::paintEvent(QPaintEvent*) {
    QStyleOption opt;
    opt.init(this);
    QPainter painter(this);

    painter.setRenderHint(QPainter::Antialiasing);
    painter.fillRect(rect(), QBrush(QColor(48, 47, 47)));
}

void EditPage::closePressed(bool) {
    // check if any edits have been made.
    if (hasAnyEdits()) {
        QString text = "You have unsaved edit data. Are you sure you want to close? ";
        auto reply =
            QMessageBox::question(this, "Close?", text, QMessageBox::Yes | QMessageBox::No);
        if (reply == QMessageBox::No) {
            return;
        }
    }
    emit pressedClose();
}

void EditPage::updateGroupsFromPagePressed() {
    emit updateGroups();
}

bool EditPage::hasAnyEdits() {
    for (auto widget : widgets()) {
        if (widget->hasEdits()) {
            return true;
        }
    }
    return false;
}

} // namespace cor
