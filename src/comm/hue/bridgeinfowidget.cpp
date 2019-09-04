/*!
 * \copyright
 * Copyright (C) 2015 - 2019.
 * Released under the GNU General Public License.
 */

#include "bridgeinfowidget.h"

#include <QGraphicsOpacityEffect>
#include <QScroller>
#include <QStyleOption>
#include <QtCore>
#include <QtGui>

namespace hue {

BridgeInfoWidget::BridgeInfoWidget(const hue::Bridge& bridge, QWidget* parent)
    : cor::ListItemWidget(bridge.IP, parent),
      mState{EBridgeDiscoveryState::unknown} {
    const QString styleSheet = "background-color: rgba(0,0,0,0);";
    setStyleSheet(styleSheet);

    mDeleteButton = new QPushButton("X", this);
    mDeleteButton->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    connect(mDeleteButton, SIGNAL(clicked()), this, SLOT(deleteButtonPressed()));

    mNameLabel = new QLabel("<b>Name:</b> ", this);
    mNameLabel->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    mNameLabel->setAttribute(Qt::WA_TransparentForMouseEvents, true);

    mNameWidget = new EditableFieldWidget(bridge.customName, this);
    mNameWidget->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    connect(mNameWidget, SIGNAL(updatedField(QString)), this, SLOT(changedName(QString)));

    mTopLayout = new QHBoxLayout;
    mTopLayout->addWidget(mNameLabel, 2);
    mTopLayout->addWidget(mNameWidget, 4);
    mTopLayout->addWidget(mDeleteButton, 1);

    //-----------
    // mid Left Image
    //-----------

    mImage = new QLabel(this);
    mImage->setAlignment(Qt::AlignHCenter | Qt::AlignVCenter);
    mImage->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Expanding);
    mMovie = new QMovie(":/images/loading_icon.gif");

    //-----------
    // mid Right info
    //-----------

    mIPAddress = new QLabel(this);
    mIPAddress->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    mIPAddress->setAttribute(Qt::WA_TransparentForMouseEvents, true);

    mAPI = new QLabel(this);
    mAPI->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    mAPI->setAttribute(Qt::WA_TransparentForMouseEvents, true);

    mID = new QLabel(this);
    mID->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    mID->setAttribute(Qt::WA_TransparentForMouseEvents, true);

    mSpacer = new QLabel(this);
    mSpacer->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    mSpacer->setAttribute(Qt::WA_TransparentForMouseEvents, true);

    //-----------
    // Bottom
    //-----------
    const QString buttonStyleSheet = "background-color: #302F2F;";

    mDiscoverHueButton = new QPushButton("Discover \r\n New Hues", this);
    mDiscoverHueButton->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    connect(mDiscoverHueButton, SIGNAL(clicked()), this, SLOT(pressedDiscoverHues()));
    mDiscoverHueButton->setStyleSheet(buttonStyleSheet);

    mGroupsButton = new QPushButton("View Groups", this);
    mGroupsButton->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    connect(mGroupsButton, SIGNAL(clicked()), this, SLOT(groupsListPressed()));
    mGroupsButton->setStyleSheet(buttonStyleSheet);

    mSchedulesButton = new QPushButton("Schedules", this);
    mSchedulesButton->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    connect(mSchedulesButton, SIGNAL(clicked()), this, SLOT(schedulesListPressed()));
    mSchedulesButton->setStyleSheet(buttonStyleSheet);

    mButtonsLayout = new QHBoxLayout;
    mButtonsLayout->addWidget(mDiscoverHueButton, 2);
    mButtonsLayout->addWidget(mGroupsButton, 2);
    auto margin = int(this->width() * 0.05);
    mButtonsLayout->setSpacing(margin * 2);
    mButtonsLayout->setContentsMargins(margin, margin, margin, margin);
    // mButtonsLayout->addWidget(mSchedulesButton, 2);
    // for now, hide the schedules menu
    mSchedulesButton->setVisible(false);
    mDiscoverHueButton->setEnabled(false);
    mGroupsButton->setEnabled(false);

    //-----
    // Top right widget
    //----

    mTopRightLayout = new QVBoxLayout;
    mTopRightLayout->addLayout(mTopLayout, 1);
    mTopRightLayout->addWidget(mIPAddress, 1);
    mTopRightLayout->addWidget(mAPI, 1);
    mTopRightLayout->addWidget(mID, 1);
    mTopRightLayout->addLayout(mButtonsLayout, 2);
    mTopRightLayout->addWidget(mSpacer, 2);

    //-----
    // mid widget
    //----

    mMidLayout = new QHBoxLayout;
    mMidLayout->addWidget(mImage);
    mMidLayout->addLayout(mTopRightLayout);

    mIsChecked = false;

    updateBridge(bridge);
    calculateButtonFontSize();
}

void BridgeInfoWidget::updateBridge(const hue::Bridge& bridge) {
    if (!(bridge == mBridge)) {
        mNameWidget->setText(bridge.customName);
        mIPAddress->setText("<b>IP:</b>  " + bridge.IP);
        mAPI->setText("<b>API:</b>  " + bridge.api);
        mID->setText("<b>ID:</b>  " + bridge.id);
        handleBridgeState(bridge.state);
        mBridge = bridge;
    }
}

void BridgeInfoWidget::handleBridgeState(EBridgeDiscoveryState state) {
    if (state != mState) {
        auto min = width();
        if (state == EBridgeDiscoveryState::connected) {
            mBridgePixmap = QPixmap(":images/Hue-Bridge.png");
            mDiscoverHueButton->setEnabled(true);
            mGroupsButton->setEnabled(true);
            auto width = int(min * 0.333f);
            mImage->setPixmap(
                mBridgePixmap.scaled(width, width, Qt::KeepAspectRatio, Qt::SmoothTransformation));
        } else if (state == EBridgeDiscoveryState::lookingForUsername) {
            mBridgePixmap = QPixmap(":images/pressHueBridgeImage.png");
            auto width = int(min * 0.333f);
            mImage->setPixmap(
                mBridgePixmap.scaled(width, width, Qt::KeepAspectRatio, Qt::SmoothTransformation));
        } else if (state == EBridgeDiscoveryState::lookingForResponse) {
            mImage->setMovie(mMovie);
            mMovie->start();
        }
        resize();
        mState = state;
        adjustSize();
    }
}

void BridgeInfoWidget::setChecked(bool checked) {
    mIsChecked = checked;
    update();
}

void BridgeInfoWidget::paintEvent(QPaintEvent*) {
    QStyleOption opt;
    opt.init(this);
    QPainter painter(this);

    painter.setRenderHint(QPainter::Antialiasing);
    if (mIsChecked) {
        painter.fillRect(rect(), QBrush(QColor(61, 142, 201, 255)));
    } else {
        // TODO: could I make this transparent in all cases?
        painter.fillRect(rect(), QBrush(QColor(32, 31, 31, 255)));
    }

    // draw line at bottom of widget
    QRect area(x(), y(), width(), height());
    QPainter linePainter(this);
    linePainter.setRenderHint(QPainter::Antialiasing);
    linePainter.setBrush(QBrush(QColor(255, 255, 255)));
    QLine spacerLine(QPoint(area.x(), area.height() - 3), QPoint(area.width(), area.height() - 3));
    linePainter.drawLine(spacerLine);
}

void BridgeInfoWidget::mouseReleaseEvent(QMouseEvent*) {
    emit clicked(mBridge.id);
}

void BridgeInfoWidget::resize() {
    auto min = width();
    // auto min = std::min(width(), height());
    if (mBridge.state != EBridgeDiscoveryState::lookingForResponse) {
        auto width = int(min * 0.45f);
        mImage->setFixedWidth(width);
        mImage->setPixmap(
            mBridgePixmap.scaled(width, width, Qt::KeepAspectRatio, Qt::SmoothTransformation));
    } else {
        mImage->setFixedWidth(int(min * 0.45f));
    }
    auto yPos = 0;

    mMidLayout->setGeometry(QRect(0, yPos, width(), height()));
    mTopRightLayout->setGeometry(QRect(mImage->width(), yPos, width() - mImage->width(), height()));
    yPos += mMidLayout->geometry().height();

    calculateButtonFontSize();
}

void BridgeInfoWidget::resizeEvent(QResizeEvent*) {
    resize();
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

void BridgeInfoWidget::changedName(const QString& newName) {
    emit nameChanged(mBridge.id, newName);
}

void BridgeInfoWidget::setTitleFontPointSize(int pt) {
    if (pt <= 0) {
        pt = 1;
    }
    QString stylesheet = "font-size:" + QString::number(pt) + "pt;";
    mIPAddress->setStyleSheet(stylesheet);

    QFont font(mIPAddress->font().toString(), pt);
    QFontMetrics fm(font);
    mIPAddress->setMinimumHeight(fm.height());
}

void BridgeInfoWidget::deleteButtonPressed() {
    emit deleteBridge(mBridge);
}


void BridgeInfoWidget::calculateButtonFontSize() {
    const auto& text = mGroupsButton->text();
    auto widget = mDiscoverHueButton;
    // calcuate the text's size
    auto systemFontWidth = widget->fontMetrics().boundingRect(text).width();
    // calculate the button's size
    auto buttonWidth = widget->width() * 0.9;
    QFont font(widget->font());
    auto fontPtSize = widget->font().pointSize();
    if (systemFontWidth > buttonWidth) {
        for (auto i = fontPtSize - 1; i > 0; --i) {
            font.setPointSize(i);
            widget->setFont(font);
            auto newFontWidth = widget->fontMetrics().boundingRect(text).width();
            if (newFontWidth < buttonWidth) {
                // font is small enough to fit
                break;
            }
        }
    } else {
        QFont defaultFont;
        for (auto i = 1; i < defaultFont.pointSize(); ++i) {
            font.setPointSize(i);
            widget->setFont(font);
            auto newFontWidth = widget->fontMetrics().boundingRect(text).width();
            if (newFontWidth > buttonWidth) {
                // font is big enough to fit
                break;
            }
        }
    }
    mSchedulesButton->setFont(font);
    mDiscoverHueButton->setFont(font);
    mGroupsButton->setFont(font);
}

} // namespace hue
