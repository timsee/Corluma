/*!
 * \copyright
 * Copyright (C) 2015 - 2017.
 * Released under the GNU General Public License.
 */

#include "floatinglayout.h"

#include <QDebug>
#include <QApplication>
#include <QScreen>
#include <QSignalMapper>

FloatingLayout::FloatingLayout( QWidget *parent) : QWidget(parent) {
    mFloatingLayout = new QHBoxLayout;

    mFloatingLayout->setContentsMargins(0,0,0,0);
    mFloatingLayout->setSpacing(0);

    setLayout(mFloatingLayout);
}

void FloatingLayout::setupButtons(std::vector<QString> buttonNames, QSize size) {

    if (mButtons.size() > 0) {
        for (uint32_t i = 0; i < mButtons.size(); ++i) {
            mFloatingLayout->removeWidget(mButtons[i]);
        }
    }

#ifdef MOBILE_BUILD
    Q_UNUSED(size);
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


    QSignalMapper *buttonsMapper = new QSignalMapper(this);
    mButtons = std::vector<QPushButton*>(buttonNames.size(), nullptr);
    mNames = buttonNames;
    for (uint32_t i = 0; i < buttonNames.size(); ++i) {
        mButtons[i] = new QPushButton;
        mButtons[i]->setCheckable(true);
        mButtons[i]->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
        mButtons[i]->setFont(QFont(mButtons[i]->font().styleName(), 12, 0));
        mButtons[i]->setMinimumWidth(minWidth);
        mButtons[i]->setMaximumWidth(minWidth);
        mButtons[i]->setMinimumHeight(minHeight);
        mButtons[i]->setMaximumHeight(minHeight);
        mButtons[i]->setText(buttonNames[i]);
        mFloatingLayout->addWidget(mButtons[i]);
        connect(mButtons[i], SIGNAL(clicked(bool)), buttonsMapper, SLOT(map()));
        buttonsMapper->setMapping(mButtons[i], i);
    }

    connect(buttonsMapper, SIGNAL(mapped(int)), this, SLOT(buttonClicked(int)));

}

void FloatingLayout::move(QPoint topRightPoint) {
    // add floating region to far right of screen under main icon menu
    QPoint topRight = QPoint(topRightPoint.x(), topRightPoint.y());
    QPoint bottomRight = QPoint(topRight.x(), topRight.y() + mButtons[0]->height());
    QPoint topLeft = QPoint(topRight.x() - (mButtons[0]->width() + mButtons[0]->width()), topRight.y());
    this->setGeometry(QRect(topLeft, bottomRight));
}

void FloatingLayout::buttonClicked(int buttonIndex) {
    for (uint32_t i = 0; i < mButtons.size(); ++i) {
        mButtons[i]->setChecked(false);
    }
    mButtons[buttonIndex]->setChecked(true);
    emit buttonPressed(mNames[buttonIndex]);
}

