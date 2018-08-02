/*!
 * \copyright
 * Copyright (C) 2015 - 2018.
 * Released under the GNU General Public License.
 */

#include <QtCore>
#include <QtGui>
#include <QStyleOption>
#include <QScroller>
#include <QGraphicsOpacityEffect>

#include "bridgeinfowidget.h"

namespace hue
{

BridgeInfoWidget::BridgeInfoWidget(const hue::Bridge& bridge, QWidget *parent) : QWidget(parent), mBridge(bridge) {
    const QString styleSheet = "background-color: rgba(0,0,0,0);";
    this->setStyleSheet(styleSheet);

    //-----------
    // Top Left Image
    //-----------

    mImage = new QLabel(this);
    mImage->setAlignment(Qt::AlignHCenter | Qt::AlignVCenter);
    mImage->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    mBridgePixmap = QPixmap(":images/Hue-Bridge.png");

    //-----------
    // Top Right info
    //-----------

    mNameLayout = new QHBoxLayout;

    mNameLabel = new QLabel("<b>Name:</b> ", this);
    mNameLabel->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    mNameLabel->setAttribute(Qt::WA_TransparentForMouseEvents, true);

    mNameWidget = new EditableFieldWidget(bridge.customName, this);
    mNameWidget->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    connect(mNameWidget, SIGNAL(updatedField(QString)), this, SLOT(changedName(QString)));

    mNameLayout->addWidget(mNameLabel);
    mNameLayout->addWidget(mNameWidget);

    mIPAddress  = new QLabel("<b>IP:</b>  " + bridge.IP, this);
    mIPAddress->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    mIPAddress->setAttribute(Qt::WA_TransparentForMouseEvents, true);
    setTitleFontPointSize(14);

    mAPI = new QLabel("<b>API:</b>  " + bridge.api, this);
    mAPI->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    mAPI->setAttribute(Qt::WA_TransparentForMouseEvents, true);

    mID = new QLabel("<b>ID:</b>  " + bridge.id, this);
    mID->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    mID->setAttribute(Qt::WA_TransparentForMouseEvents, true);

    mSpacer = new QLabel(this);
    mSpacer->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    mSpacer->setAttribute(Qt::WA_TransparentForMouseEvents, true);

    //-----
    // Top right widget
    //----

    mTopRightLayout = new QVBoxLayout;
    mTopRightLayout->addLayout(mNameLayout, 1);
    mTopRightLayout->addWidget(mIPAddress,  1);
    mTopRightLayout->addWidget(mAPI,        1);
    mTopRightLayout->addWidget(mID,         1);
    mTopRightLayout->addWidget(mSpacer,     1);

    //-----
    // Top widget
    //----

    mTopLayout = new QHBoxLayout;
    mTopLayout->addWidget(mImage, 2);
    mTopLayout->addLayout(mTopRightLayout, 8);

    //-----------
    // Bottom
    //-----------

    mDiscoverHueButton = new QPushButton("Discover Hues", this);
    mDiscoverHueButton->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    connect(mDiscoverHueButton, SIGNAL(clicked()), this, SLOT(pressedDiscoverHues()));

    mGroupsButton = new QPushButton("Groups", this);
    mGroupsButton->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    connect(mGroupsButton, SIGNAL(clicked()), this, SLOT(groupsListPressed()));

    mSchedulesButton = new QPushButton("Schedules", this);
    mSchedulesButton->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    connect(mSchedulesButton, SIGNAL(clicked()), this, SLOT(schedulesListPressed()));

    mButtonsLayout = new QHBoxLayout;
    mButtonsLayout->addWidget(mDiscoverHueButton);
    mButtonsLayout->addWidget(mGroupsButton);
    mButtonsLayout->addWidget(mSchedulesButton);

    mLayout = new QVBoxLayout(this);
    mLayout->addLayout(mTopLayout, 4);
    mLayout->addLayout(mButtonsLayout, 1);

    mIsChecked = false;
}


void BridgeInfoWidget::setChecked(bool checked) {
    mIsChecked = checked;
    repaint();
}

void BridgeInfoWidget::setHeight(int height) {
    int finalHeight = std::max(height, this->height());
    mHeight = finalHeight;
    this->setFixedHeight(height);
}

void BridgeInfoWidget::paintEvent(QPaintEvent *event) {
    Q_UNUSED(event);
    QStyleOption opt;
    opt.init(this);
    QPainter painter(this);

    painter.setRenderHint(QPainter::Antialiasing);
    if (mIsChecked) {
        painter.fillRect(this->rect(), QBrush(QColor(61, 142, 201, 255)));
    } else {
        //TODO: could I make this transparent in all cases?
        painter.fillRect(this->rect(), QBrush(QColor(32, 31, 31, 255)));
    }

    // draw line at bottom of widget
    QRect area(this->x(), this->y(), this->width(), this->height());
    QPainter linePainter(this);
    linePainter.setRenderHint(QPainter::Antialiasing);
    linePainter.setBrush(QBrush(QColor(255, 255, 255)));
    QLine spacerLine(QPoint(area.x(), area.height() - 3), QPoint(area.width(), area.height() - 3));
    linePainter.drawLine(spacerLine);
}

void BridgeInfoWidget::mouseReleaseEvent(QMouseEvent *event) {
    Q_UNUSED(event);
    emit clicked(mBridge.id);
}

void BridgeInfoWidget::resizeEvent(QResizeEvent *) {
    int width = this->width() * 0.333f;
    mImage->setPixmap(mBridgePixmap.scaled(width,
                                           width,
                                           Qt::KeepAspectRatio,
                                           Qt::SmoothTransformation));
}


void BridgeInfoWidget::pressedDiscoverHues() {
    emit discoverHuesPressed(mBridge.id);
}

void BridgeInfoWidget::groupsListPressed() {
    emit groupsPressed(mBridge.id);
}

void BridgeInfoWidget::schedulesListPressed() {
    emit schedulesPressed(mBridge.id);
}

void BridgeInfoWidget::changedName(QString newName) {
    emit nameChanged(mBridge.id, newName);
}

void BridgeInfoWidget::setTitleFontPointSize(int pt) {
    if (pt <= 0) pt = 1;
    QString stylesheet = "font-size:" + QString::number(pt)+ "pt;";
    mIPAddress->setStyleSheet(stylesheet);

    QFont font(mIPAddress->font().toString(), pt);
    QFontMetrics fm(font);
    mIPAddress->setMinimumHeight(fm.height());
}

}
