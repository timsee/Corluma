#include "loadingscreen.h"
#include <QTimer>


LoadingScreen::LoadingScreen(QWidget* parent)
    : QWidget(parent),
      mImage{new QLabel(this)},
      mText{new QLabel("Looking for Lights...", this)},
      mIsReady{false} {
    mText->setAlignment(Qt::AlignCenter);

    QTimer::singleShot(5000, this, &LoadingScreen::startupTimerFinished);
}

void LoadingScreen::resizeEvent(QResizeEvent*) {
    auto yPos = 0;
    mImage->setGeometry(0, yPos, width(), height() * 0.8);
    yPos += mImage->height();

    mText->setGeometry(0, yPos, width(), height() * 0.2);
}
