#include "multicolorstatewidget.h"

MultiColorStateWidget::MultiColorStateWidget(QWidget* parent) : QWidget(parent) {
    mSyncWidget = new SyncWidget(this);
    mSyncWidget->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);

    mSwatchWidget = new SwatchVectorWidget(6, 1, this);
    mSwatchWidget->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);

    setStyleSheet("background-color:rgb(33, 32, 32);");
}

void MultiColorStateWidget::updateState(std::vector<QColor> colors) {
    mSwatchWidget->updateColors(colors);
}

void MultiColorStateWidget::updateSyncStatus(ESyncState state) {
    mSyncWidget->changeState(state);
}

void MultiColorStateWidget::resize() {
    int xPos = 0;
    mSyncWidget->setGeometry(xPos, 0, width() / 7, height());
    xPos += mSyncWidget->width();
    mSwatchWidget->setGeometry(xPos, 0, width() * 6 / 7, height());
}

void MultiColorStateWidget::resizeEvent(QResizeEvent*) {
    resize();
}
