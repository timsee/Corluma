/*!
 * \copyright
 * Copyright (C) 2015 - 2020.
 * Released under the GNU General Public License.
 */

#include "displaypreviewbridgewidget.h"

#include <QGraphicsOpacityEffect>
#include <QScroller>
#include <QStyleOption>
#include <QtCore>
#include <QtGui>
#include "utils/qt.h"

namespace hue {

DisplayPreviewBridgeWidget::DisplayPreviewBridgeWidget(const hue::Bridge& bridge,
                                                       const QString& key,
                                                       CommLayer* comm,
                                                       cor::LightList* selectedLights,
                                                       QWidget* parent)
    : cor::ListItemWidget(key, parent),
      mState{EBridgeDiscoveryState::unknown},
      mComm{comm},
      mSelectedLights{selectedLights} {
    const QString styleSheet = "background-color: rgba(0,0,0,0);";
    setStyleSheet(styleSheet);
    mNameWidget = new QLabel("<b>Name:</b> " + bridge.customName(), this);
    mNameWidget->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    mNameWidget->setAttribute(Qt::WA_TransparentForMouseEvents, true);

    // connect(mNameWidget, SIGNAL(updatedField(QString)), this, SLOT(changedName(QString)));

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

    //-----
    // Top right widget
    //----

    mTopRightLayout = new QVBoxLayout;
    mTopRightLayout->addWidget(mNameWidget, 1);
    mTopRightLayout->addWidget(mIPAddress, 1);
    mTopRightLayout->addWidget(mAPI, 1);
    mTopRightLayout->addWidget(mID, 1);
    mTopRightLayout->addWidget(mSpacer, 1);

    //-----
    // mid widget
    //----

    mMidLayout = new QHBoxLayout;
    mMidLayout->addWidget(mImage);
    mMidLayout->addLayout(mTopRightLayout);

    mIsChecked = false;

    updateBridge(bridge);
}

void DisplayPreviewBridgeWidget::updateBridge(const hue::Bridge& bridge) {
    mNameWidget->setText(bridge.customName());
    mIPAddress->setText("<b>IP:</b>  " + bridge.IP());
    mAPI->setText("<b>API:</b>  " + bridge.API());
    mID->setText("<b>ID:</b>  " + bridge.id());
    handleBridgeState(bridge.state());
    highlightLights();
    mBridge = bridge;
}

void DisplayPreviewBridgeWidget::handleBridgeState(EBridgeDiscoveryState state) {
    auto min = width();
    auto width = int(min * 0.333f);
    if (state == EBridgeDiscoveryState::connected) {
        mBridgePixmap = QPixmap(":images/Hue-Bridge.png");
        mImage->setPixmap(
            mBridgePixmap.scaled(width, width, Qt::KeepAspectRatio, Qt::SmoothTransformation));
    } else if (state == EBridgeDiscoveryState::lookingForUsername) {
        mBridgePixmap = QPixmap(":images/pressHueBridgeImage.png");
        mImage->setPixmap(
            mBridgePixmap.scaled(width, width, Qt::KeepAspectRatio, Qt::SmoothTransformation));
    } else if (state == EBridgeDiscoveryState::lookingForResponse
               || state == EBridgeDiscoveryState::testingConnectionInfo) {
        mImage->setMovie(mMovie);
        mMovie->start();
    }
    mState = state;
    mImage->setFixedWidth(width);
    adjustSize();
}

void DisplayPreviewBridgeWidget::setChecked(bool checked) {
    mIsChecked = checked;
    update();
}

void DisplayPreviewBridgeWidget::paintEvent(QPaintEvent*) {
    QStyleOption opt;
    opt.initFrom(this);
    QPainter painter(this);

    painter.setRenderHint(QPainter::Antialiasing);
    painter.fillRect(rect(), cor::computeHighlightColor(mSelectedCount, mReachableCount));

    // draw line at bottom of widget
    QRect area(x(), y(), width(), height());
    QPainter linePainter(this);
    linePainter.setRenderHint(QPainter::Antialiasing);
    linePainter.setBrush(QBrush(QColor(255, 255, 255)));
    QLine spacerLine(QPoint(area.x(), area.height() - 3), QPoint(area.width(), area.height() - 3));
    linePainter.drawLine(spacerLine);
}

void DisplayPreviewBridgeWidget::mouseReleaseEvent(QMouseEvent*) {
    emit clicked(mBridge.id());
}

void DisplayPreviewBridgeWidget::resize() {
    handleBridgeState(mBridge.state());
    auto yPos = 0;

    mMidLayout->setGeometry(QRect(0, yPos, width(), height()));
    mTopRightLayout->setGeometry(QRect(mImage->width(), yPos, width() - mImage->width(), height()));
    yPos += mMidLayout->geometry().height();
}

void DisplayPreviewBridgeWidget::resizeEvent(QResizeEvent*) {
    resize();
}

void DisplayPreviewBridgeWidget::setTitleFontPointSize(int pt) {
    if (pt <= 0) {
        pt = 1;
    }
    QString stylesheet = "font-size:" + QString::number(pt) + "pt;";
    mIPAddress->setStyleSheet(stylesheet);

    QFont font(mIPAddress->font().toString(), pt);
    QFontMetrics fm(font);
    mIPAddress->setMinimumHeight(fm.height());
}

void DisplayPreviewBridgeWidget::highlightLights() {
    mReachableCount = 0u;
    mSelectedCount = 0u;
    for (auto light : mComm->hue()->lightsFromMetadata(mBridge.lights().items())) {
        if (light.isReachable()) {
            mReachableCount++;
        }
        if (mSelectedLights->doesLightExist(light.uniqueID())) {
            mSelectedCount++;
        }
    }
    update();
}

} // namespace hue
