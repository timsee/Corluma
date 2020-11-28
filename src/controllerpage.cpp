#include "controllerpage.h"
#include <QPainter>
#include <QStyleOption>

ControllerPage::ControllerPage(QWidget* parent, CommLayer* comm)
    : QWidget(parent),
      mComm{comm},
      mTopWidget{new cor::TopWidget("", ":images/arrowLeft.png", this)},
      mArduCorWidget{new DisplayArduCorControllerWidget(this, comm)},
      mNanoleafWidget{new DisplayNanoleafControllerWidget(this, comm)},
      mHueBridgeWidget{new DisplayHueBridgeWidget(this, comm)} {
    connect(mTopWidget, SIGNAL(clicked(bool)), this, SLOT(backButtonPressed(bool)));
}

void ControllerPage::showPage(QPoint topLeft) {
    setVisible(true);
    setGeometry(topLeft.x(), topLeft.y(), width(), height());
    raise();
    show();
    isOpen(true);
}

void ControllerPage::hidePage() {
    setVisible(false);
    isOpen(false);
}

void ControllerPage::backButtonPressed(bool) {
    emit backButtonPressed();
}

void ControllerPage::showArduCor(const cor::Controller& controller) {
    mArduCorWidget->updateController(controller);
    mArduCorWidget->setVisible(true);
    mNanoleafWidget->setVisible(false);
    mHueBridgeWidget->setVisible(false);
}

void ControllerPage::showNanoleaf(const nano::LeafMetadata& metadata) {
    mNanoleafWidget->updateLeafMetadata(metadata);
    mArduCorWidget->setVisible(false);
    mNanoleafWidget->setVisible(true);
    mHueBridgeWidget->setVisible(false);
}

void ControllerPage::showHueBridge(const hue::Bridge& bridge) {
    mHueBridgeWidget->updateBridge(bridge);
    mArduCorWidget->setVisible(false);
    mNanoleafWidget->setVisible(false);
    mHueBridgeWidget->setVisible(true);
}

void ControllerPage::changeRowHeight(int height) {
    mArduCorWidget->changeRowHeight(height);
    mNanoleafWidget->changeRowHeight(height);
    mHueBridgeWidget->changeRowHeight(height);
}

void ControllerPage::resizeEvent(QResizeEvent*) {
    auto yPos = 0u;
    mTopWidget->setGeometry(0, 0, this->width(), this->height() / 12);
    yPos += mTopWidget->height();

    mArduCorWidget->setGeometry(0, yPos, this->width(), this->height() * 11 / 12);
    mNanoleafWidget->setGeometry(0, yPos, this->width(), this->height() * 11 / 12);
    mHueBridgeWidget->setGeometry(0, yPos, this->width(), this->height() * 11 / 12);
}

void ControllerPage::paintEvent(QPaintEvent*) {
    QStyleOption opt;
    opt.init(this);
    QPainter painter(this);

    painter.setRenderHint(QPainter::Antialiasing);
    painter.fillRect(rect(), QBrush(QColor(48, 47, 47)));
}
