#include "palettedetailedwidget.h"
#include <QPainter>
#include <QStyleOption>
#include "utils/qt.h"

PaletteDetailedWidget::PaletteDetailedWidget(QWidget* parent)
    : QWidget(parent),
      mCloseButton{new QPushButton(this)},
      mSyncButton{new QPushButton(this)},
      mEditButton{new QPushButton(this)},
      mDeleteButton{new QPushButton("Delete", this)},
      mName{new QLabel(this)},
      mSyncWidget{new SyncWidget(this)},
      mPaletteWidget{new cor::PaletteWidget(this)},
      mPalette{} {
    mName->setStyleSheet("font-size:16pt;");
    mSyncButton->setText("Sync to lights");
    connect(mSyncButton, SIGNAL(clicked(bool)), this, SLOT(syncButtonPressed(bool)));
    connect(mCloseButton, SIGNAL(clicked(bool)), this, SLOT(closeButtonPressed(bool)));
    connect(mDeleteButton, SIGNAL(clicked(bool)), this, SLOT(deleteButtonPressed(bool)));
    connect(mEditButton, SIGNAL(clicked(bool)), this, SLOT(editButtonPressed(bool)));
}

void PaletteDetailedWidget::update(const cor::Palette& palette, bool isReserved) {
    mPalette = palette;
    mName->setText(palette.name());
    mPaletteWidget->show(palette.colors());
    mSyncWidget->changeState(ESyncState::hidden);
    mEditButton->setVisible(!isReserved);
    mDeleteButton->setVisible(!isReserved);
}

void PaletteDetailedWidget::paintEvent(QPaintEvent*) {
    QStyleOption opt;
    opt.initFrom(this);
    QPainter painter(this);

    painter.setRenderHint(QPainter::Antialiasing);
    painter.fillRect(rect(), QBrush(QColor(48, 47, 47)));
}


void PaletteDetailedWidget::resize() {
    QSize size = parentWidget()->size();
    setFixedSize(int(size.width() * 0.75f), int(size.height() * 0.5f));

    auto rowHeight = height() / 9;
    auto yPos = 0;

    auto xSpacing = width() * 0.1;
    auto xSpacingTop = width() * 0.025;
    auto ySpacing = height() * 0.05;

    auto syncButtonWidth = std::min(width() - rowHeight * 4, int(rowHeight * 3));
    mCloseButton->setGeometry(0, yPos, rowHeight, rowHeight);
    mName->setGeometry(mCloseButton->width() + xSpacingTop,
                       yPos,
                       width() - mCloseButton->width() - xSpacingTop,
                       rowHeight);

    yPos += mCloseButton->height();
    mSyncWidget->setGeometry(width() - rowHeight * 2 - syncButtonWidth, yPos, rowHeight, rowHeight);
    mSyncButton->setGeometry(width() - syncButtonWidth, yPos, syncButtonWidth, rowHeight);
    yPos += mSyncButton->height();

    mPaletteWidget->setGeometry(xSpacing,
                                yPos + ySpacing,
                                width() - xSpacing * 2,
                                rowHeight * 6 - ySpacing * 2);

    yPos += mPaletteWidget->height() + ySpacing * 2;
    mDeleteButton->setGeometry(0, yPos, width() / 3, rowHeight);
    mEditButton->setGeometry(width() - rowHeight, yPos, rowHeight, rowHeight);

    resizeIcons();
}


void PaletteDetailedWidget::resizeIcons() {
    QSize newSize = QSize(int(mCloseButton->width() * 0.8f), int(mCloseButton->height() * 0.8f));
    auto closePixmap = QPixmap(":images/closeX.png");
    closePixmap = closePixmap.scaled(newSize.width(),
                                     newSize.height(),
                                     Qt::KeepAspectRatio,
                                     Qt::SmoothTransformation);
    mCloseButton->setIcon(QIcon(closePixmap));
    mCloseButton->setIconSize(newSize);

    auto editPixmap = QPixmap(":images/edit_icon.png");
    editPixmap = editPixmap.scaled(newSize.width(),
                                   newSize.height(),
                                   Qt::KeepAspectRatio,
                                   Qt::SmoothTransformation);
    mEditButton->setIcon(QIcon(editPixmap));
    mEditButton->setIconSize(newSize);
}

void PaletteDetailedWidget::pushIn() {
    isOpen(true);

    moveWidget(
        this,
        QPoint(int(parentWidget()->width() * 0.125f), int(-1 * parentWidget()->height())),
        QPoint(int(parentWidget()->width() * 0.125f), int(parentWidget()->height() * 0.25f)));

    raise();
    setVisible(true);
    resize();
}

void PaletteDetailedWidget::pushOut() {
    isOpen(false);

    moveWidget(this,
               QPoint(int(parentWidget()->width() * 0.125f), int(parentWidget()->height() * 0.25f)),
               QPoint(int(parentWidget()->width() * 0.125f), int(-1 * parentWidget()->height())));
}

void PaletteDetailedWidget::updateSyncStatus(ESyncState state) {
    mSyncWidget->changeState(state);
}
