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

    mDeleteButton = new QPushButton("X", this);
    mDeleteButton->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    connect(mDeleteButton, SIGNAL(clicked()), this, SLOT(deleteButtonPressed()));

    mNameLabel = new QLabel("<b>Name:</b> ", this);
    mNameLabel->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    mNameLabel->setAttribute(Qt::WA_TransparentForMouseEvents, true);

    mNameWidget = new EditableFieldWidget(bridge.customName(), this);
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



    mButtonsLayout = new QGridLayout;
    mButtonsLayout->addWidget(mDiscoverHueButton, 0, 0);
    auto margin = int(this->width() * 0.05);
    mButtonsLayout->setSpacing(margin * 2);
    mButtonsLayout->setContentsMargins(margin, margin, margin, margin);
    mDiscoverHueButton->setEnabled(false);

    //-----
    // Top right widget
    //----

    mTopRightLayout = new QVBoxLayout;
    mTopRightLayout->addLayout(mTopLayout, 1);
    mTopRightLayout->addWidget(mIPAddress, 1);
    mTopRightLayout->addWidget(mAPI, 1);
    mTopRightLayout->addWidget(mID, 1);
    mTopRightLayout->addLayout(mButtonsLayout, 4);
    mTopRightLayout->addWidget(mSpacer, 1);

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
        mDiscoverHueButton->setEnabled(true);
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

    calculateButtonFontSize();
}

void DisplayPreviewBridgeWidget::resizeEvent(QResizeEvent*) {
    resize();
}


void DisplayPreviewBridgeWidget::pressedDiscoverHues() {
    emit discoverHuesPressed(mBridge.id());
}

void DisplayPreviewBridgeWidget::changedName(const QString& newName) {
    emit nameChanged(mBridge.id(), newName);
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

void DisplayPreviewBridgeWidget::deleteButtonPressed() {
    emit deleteBridge(mBridge);
}


void DisplayPreviewBridgeWidget::calculateButtonFontSize() {
    const auto& text = mDiscoverHueButton->text();
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
    mDiscoverHueButton->setFont(font);
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
