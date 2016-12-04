#include "floatinglayout.h"

#include <QDebug>
#include <QApplication>
#include <QScreen>

FloatingLayout::FloatingLayout(QSize size, QWidget *parent) : QWidget(parent) {
    mSingleButton = new QPushButton(this);
    mSingleButton->setCheckable(true);
    mSingleButton->setText("Single");

#ifdef MOBILE_BUILD
    //NOTE: on devices with high DPI size using a floating layout doesnt allow
    //      the views to scale up with the rest of the layout. To avoid this issue,
    //       we use the screens availableSize rather than just the application's size
    QScreen *screen = QApplication::screens().at(0);
    float minWidth = screen->availableSize().width() * 0.133f;
    float minHeight = minWidth * 0.65f;
#else
    float minWidth = size.width() * 0.133f;
    float minHeight = minWidth * 0.7f;
#endif

    mSingleButton->setMinimumWidth(minWidth);
    mSingleButton->setMaximumWidth(minWidth);
    mSingleButton->setMinimumHeight(minHeight);
    mSingleButton->setMaximumHeight(minHeight);
    mSingleButton->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    mSingleButton->setFont(QFont(mSingleButton->font().styleName(), 12, 0));

    connect(mSingleButton, SIGNAL(clicked(bool)), this, SLOT(singleButtonClicked(bool)));

    mMultiButton = new QPushButton(this);
    mMultiButton->setCheckable(true);
    mMultiButton->setText("Multi");
    mMultiButton->setMinimumWidth(minWidth);
    mMultiButton->setMaximumWidth(minWidth);
    mMultiButton->setMinimumHeight(minHeight);
    mMultiButton->setMaximumHeight(minHeight);
    mMultiButton->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    connect(mMultiButton, SIGNAL(clicked(bool)), this, SLOT(multiButtonClicked(bool)));
    mMultiButton->setFont(QFont(mMultiButton->font().styleName(), 12, 0));

    mFloatingLayout = new QHBoxLayout;
    mFloatingLayout->addWidget(mSingleButton);
    mFloatingLayout->addWidget(mMultiButton);

    mFloatingLayout->setContentsMargins(0,0,0,0);
    mFloatingLayout->setSpacing(0);

    setLayout(mFloatingLayout);
    singleButtonClicked(true);
}


void FloatingLayout::move(QPoint topRightPoint) {
    // add floating region to far right of screen under main icon menu
    QPoint topRight = QPoint(topRightPoint.x(),
                             topRightPoint.y() + 5); // +5 for spacing
    QPoint bottomRight = QPoint(topRight.x(), topRight.y() + mSingleButton->height());
    QPoint topLeft = QPoint(topRight.x() - (mSingleButton->width() + mMultiButton->width()), topRight.y());
    this->setGeometry(QRect(topLeft, bottomRight));
}


void FloatingLayout::singleButtonClicked(bool clicked) {
    mFloatingLayoutSingleSelected = true;
    mSingleButton->setChecked(true);
    mMultiButton->setChecked(false);
    emit buttonPressed(QString("Single"));
}

void FloatingLayout::multiButtonClicked(bool clicked) {
    mFloatingLayoutSingleSelected = false;
    mSingleButton->setChecked(false);
    mMultiButton->setChecked(true);
    emit buttonPressed(QString("Multi"));
}


