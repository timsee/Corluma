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

EditPage::EditPage(QWidget* parent, CommLayer* comm, GroupData* groups)
    : QWidget(parent),
      mComm(comm),
      mGroups(groups),
      mPlaceholder{new QWidget(this)},
      mCloseButton{new QPushButton(this)},
      mWidgets{4, nullptr},
      mProgressWidget{new EditProgressWidget(this, 4)} {
    connect(mCloseButton, SIGNAL(clicked(bool)), this, SLOT(closePressed(bool)));

#ifdef MOBILE_BUILD
    mTopHeight = cor::applicationSize().height() * 0.075;
#else
    mTopHeight = cor::applicationSize().height() * 0.1;
#endif

    mProgressWidget->updateState(0, EEditProgressState::completed);
    connect(mProgressWidget,
            SIGNAL(changePage(std::uint32_t)),
            this,
            SLOT(changePageFromProgressWidget(std::uint32_t)));

    for (std::uint32_t i = 0; i < mWidgets.size(); ++i) {
        mWidgets[i] = new QWidget(this);
        mWidgets[i]->setVisible(false);
    }
    mWidgets[0]->setStyleSheet("background-color:rgb(255,0,0);");
    mWidgets[1]->setStyleSheet("background-color:rgb(255,255,0);");
    mWidgets[2]->setStyleSheet("background-color:rgb(0,255,0);");
    mWidgets[3]->setStyleSheet("background-color:rgb(0,255,255);");
    mWidgets[3]->setVisible(false);
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

void EditPage::changePageFromProgressWidget(std::uint32_t index) {
    showPage(index);
}

void EditPage::showPage(std::uint32_t pageIndex) {
    // update top widget
    mProgressWidget->changeToPage(pageIndex);

    // update the currently showing widget
    showAndResizePage(pageIndex);
}

void EditPage::showAndResizePage(std::uint32_t i) {
    for (auto widget : mWidgets) {
        widget->setVisible(false);
    }
    mWidgets[i]->setVisible(true);
    mWidgets[i]->setGeometry(mPlaceholder->geometry());
    mWidgets[i]->raise();
}

void EditPage::resizeCloseButton() {
    QPixmap pixmap(":images/closeX.png");
    int closeSize = mTopHeight * 0.8;
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
    auto progressWidth = int(mProgressWidget->numberOfPages() * mTopHeight);
    mProgressWidget->setGeometry(this->width() - progressWidth, 0u, progressWidth, mTopHeight);

    auto placeholderWidthPadding = this->width() / 20;
    auto placeholderHeightPadding = this->height() / 20;
    yPos += placeholderHeightPadding;
    mPlaceholder->setGeometry(placeholderWidthPadding,
                              yPos,
                              this->width() - placeholderWidthPadding * 2,
                              this->height() - yPos - placeholderHeightPadding);
    showAndResizePage(mProgressWidget->currentPage());
}

void EditPage::paintEvent(QPaintEvent*) {
    QStyleOption opt;
    opt.init(this);
    QPainter painter(this);

    painter.setRenderHint(QPainter::Antialiasing);
    painter.fillRect(rect(), QBrush(QColor(48, 47, 47)));
}

void EditPage::closePressed(bool) {
    emit pressedClose();
}


} // namespace cor
