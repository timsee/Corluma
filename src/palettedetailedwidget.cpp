#include "palettedetailedwidget.h"
#include <QPainter>
#include <QStyleOption>
#include "utils/qt.h"

PaletteDetailedWidget::PaletteDetailedWidget(QWidget* parent)
    : QWidget(parent),
      mCloseButton{new QPushButton(this)},
      mSyncButton{new QPushButton(this)},
      mEditButton{new QPushButton(this)},
      mName{new QLabel(this)},
      mSyncWidget{new SyncWidget(this)},
      mPaletteWidget{new cor::PaletteWidget(this)},
      mPalette{} {
    mName->setStyleSheet("font-size:16pt;");
    mSyncButton->setText("Sync to lights");
    connect(mSyncButton, SIGNAL(clicked(bool)), this, SLOT(syncButtonPressed(bool)));
    connect(mCloseButton, SIGNAL(clicked(bool)), this, SLOT(closeButtonPressed(bool)));
}

void PaletteDetailedWidget::update(const cor::Palette& palette) {
    mPalette = palette;
    mName->setText(palette.name());
    mPaletteWidget->show(palette.colors());
    mSyncWidget->changeState(ESyncState::hidden);
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


    auto syncButtonWidth = std::min(width() - rowHeight * 3, int(rowHeight * 2.5));
    mCloseButton->setGeometry(0, yPos, rowHeight, rowHeight);
    mSyncWidget->setGeometry(width() - rowHeight * 2 - syncButtonWidth, yPos, rowHeight, rowHeight);
    mSyncButton->setGeometry(width() - rowHeight - syncButtonWidth,
                             yPos,
                             syncButtonWidth,
                             rowHeight);
    mEditButton->setGeometry(width() - rowHeight, yPos, rowHeight, rowHeight);
    yPos += mCloseButton->height();

    mName->setGeometry(width() * 0.05, yPos, width() * 0.95, rowHeight);
    yPos += mName->height();


    auto xSpacing = width() * 0.1;
    auto ySpacing = height() * 0.05;
    mPaletteWidget->setGeometry(xSpacing,
                                yPos + ySpacing,
                                width() - xSpacing * 2,
                                rowHeight * 6 - ySpacing * 2);
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

    //  auto widthPoint = int(parentWidget()->width() * 0.875f - topMenu()->width());
    // QPoint finishPoint(widthPoint, int(parentWidget()->height() * 0.125f));
    // cor::moveWidget(topMenu(), QPoint(widthPoint, int(-1 * parentWidget()->height())),
    // finishPoint);

    // mFloatingMenu->setVisible(true);
    raise();
    setVisible(true);
    // mFloatingMenu->raise();
    resize();
}

void PaletteDetailedWidget::pushOut() {
    isOpen(false);

    moveWidget(this,
               QPoint(int(parentWidget()->width() * 0.125f), int(parentWidget()->height() * 0.25f)),
               QPoint(int(parentWidget()->width() * 0.125f), int(-1 * parentWidget()->height())));

    // auto widthPoint = int(parentWidget()->width() * 0.875f - topMenu()->size().width());
    // QPoint startPoint(widthPoint, int(parentWidget()->height() * 0.125f));
    // cor::moveWidget(topMenu(), startPoint, QPoint(widthPoint, int(-1 *
    // parentWidget()->height())));
}

void PaletteDetailedWidget::updateSyncStatus(ESyncState state) {
    mSyncWidget->changeState(state);
}
