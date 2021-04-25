/*!
 * \copyright
 * Copyright (C) 2015 - 2020.
 * Released under the GNU General Public License.
 */

#include "displaypreviewbridgewidget.h"

#include <QGraphicsOpacityEffect>
#include <QMessageBox>
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
                                                       int rowHeight,
                                                       QWidget* parent)
    : cor::ListItemWidget(key, parent),
      mState{EBridgeDiscoveryState::unknown},
      mComm{comm},
      mSelectedLights{selectedLights},
      mLights{new LightsListMenu(this, true)},
      mImage{new QLabel(this)},
      mCheckBox{new cor::CheckBox(this)},
      mMetadata{new QLabel(this)},
      mManageButton{new QPushButton("Manage", this)},
      mRowHeight{rowHeight} {
    setStyleSheet(cor::kTransparentStylesheet);
    mNameWidget = new QLabel("<b>Name:</b> " + bridge.customName(), this);
    mNameWidget->setAttribute(Qt::WA_TransparentForMouseEvents, true);

    connect(mCheckBox,
            SIGNAL(clicked(ECheckboxState)),
            this,
            SLOT(checkBoxClicked(ECheckboxState)));

    mMetadata->setWordWrap(true);

    connect(mLights, SIGNAL(clickedLight(cor::Light)), this, SLOT(clickedLight(cor::Light)));

    connect(mManageButton, SIGNAL(clicked(bool)), this, SLOT(manageButtonPressed(bool)));

    mImage->setAlignment(Qt::AlignHCenter | Qt::AlignVCenter);
    mMovie = new QMovie(":/images/loading_icon.gif");

    mIsChecked = false;

    updateBridge(bridge);
}

void DisplayPreviewBridgeWidget::updateBridge(const hue::Bridge& bridge) {
    mBridge = bridge;
    mNameWidget->setText(bridge.customName());

    auto updatedLights = mComm->hue()->lightsFromMetadata(bridge.lights().items());
    if (!cor::compareTwoLightVectors(updatedLights, mLights->lights())) {
        mLights->addLights(updatedLights);
    }
    auto prevState = mState;
    handleBridgeState(bridge.state());
    // handle an edge case where the resize needs to be explicitly called
    if (prevState != EBridgeDiscoveryState::connected
        && mState == EBridgeDiscoveryState::connected) {
        resize();
    }
    handleButtonState();
    updateMetadata(bridge);
    highlightLights();
}

void DisplayPreviewBridgeWidget::handleBridgeState(EBridgeDiscoveryState state) {
    auto min = width();
    auto width = int(min * 0.333f);
    auto imageSize = QSize(width, width);
    if (state != EBridgeDiscoveryState::connected) {
        imageSize = QSize(width * 2, width * 2);
    }
    if (state != mState || (mBridgePixmap.size() != QSize(width, width))) {
        if (state == EBridgeDiscoveryState::connected) {
            mBridgePixmap = QPixmap(":images/hue_bridge.png");
            mBridgePixmap = mBridgePixmap.scaled(imageSize.width(),
                                                 imageSize.height(),
                                                 Qt::KeepAspectRatio,
                                                 Qt::SmoothTransformation);
            mImage->setPixmap(mBridgePixmap);
            mCheckBox->setVisible(true);
        } else if (state == EBridgeDiscoveryState::lookingForUsername) {
            mBridgePixmap = QPixmap(":images/hue_bridge_press.png");
            mBridgePixmap = mBridgePixmap.scaled(imageSize.width(),
                                                 imageSize.height(),
                                                 Qt::KeepAspectRatio,
                                                 Qt::SmoothTransformation);
            mImage->setPixmap(mBridgePixmap);
            mCheckBox->setVisible(false);
        } else if (state == EBridgeDiscoveryState::lookingForResponse
                   || state == EBridgeDiscoveryState::testingConnectionInfo) {
            mMovie->setScaledSize(imageSize);
            mImage->setMovie(mMovie);
            mMovie->start();
            mCheckBox->setVisible(false);
        }
        mState = state;
    }
}

void DisplayPreviewBridgeWidget::setChecked(bool checked) {
    mIsChecked = checked;
    update();
}

void DisplayPreviewBridgeWidget::handleButtonState() {
    if (mState == EBridgeDiscoveryState::connected) {
        mManageButton->setText("Manage");
        mManageButton->setStyleSheet("background-color: rgb(48,47,47);");
    } else {
        mManageButton->setText("Delete");
        mManageButton->setStyleSheet("background-color: rgb(110,32,32);");
    }
}

void DisplayPreviewBridgeWidget::clickedLight(cor::Light light) {
    if (mSelectedLights->doesLightExist(light.uniqueID())) {
        emit deselectLight(light.uniqueID());
    } else {
        emit selectLight(light.uniqueID());
    }
}

void DisplayPreviewBridgeWidget::paintEvent(QPaintEvent*) {
    QStyleOption opt;
    opt.initFrom(this);
    QPainter painter(this);

    painter.setRenderHint(QPainter::Antialiasing);
    painter.fillRect(rect(), cor::computeHighlightColor(mSelectedCount, mReachableCount));
}

void DisplayPreviewBridgeWidget::manageButtonPressed(bool) {
    if (mState == EBridgeDiscoveryState::connected) {
        emit bridgeClicked(mBridge.id());
    } else {
        QMessageBox::StandardButton reply;
        QString text =
            "Delete " + mBridge.customName() + "? This bridge has not been fully discovered.";

        reply = QMessageBox::question(this, "Delete?", text, QMessageBox::Yes | QMessageBox::No);
        if (reply == QMessageBox::Yes) {
            // signal to remove from app
            emit deleteBridge(mBridge.id(), EProtocolType::hue);
        }
    }
}

void DisplayPreviewBridgeWidget::resize() {
    handleBridgeState(mBridge.state());
    auto yPosFirstColumn = 0;
    auto yPosSecondColumn = 0;
    auto xSpacer = width() * 0.00;
    auto columnWidth = width() * 0.33;
    auto xSecondColumnStart = columnWidth + xSpacer * 2;

    auto rowHeight = height() / 8;

    mNameWidget->setGeometry(xSpacer, yPosFirstColumn, columnWidth * 2, rowHeight);
    mCheckBox->setGeometry(width() - rowHeight, yPosFirstColumn, rowHeight, rowHeight);
    yPosSecondColumn += mNameWidget->height();
    yPosFirstColumn += mNameWidget->height();

    if (mState == EBridgeDiscoveryState::connected) {
        auto minHeight = std::min(int(columnWidth), int(rowHeight * 4));
        mImage->setGeometry(xSpacer, yPosFirstColumn, columnWidth, minHeight);
        yPosFirstColumn += mImage->height();
        mManageButton->setGeometry(xSpacer, yPosFirstColumn, columnWidth, rowHeight);
        yPosFirstColumn += mManageButton->height();
        mMetadata->setGeometry(xSpacer, yPosFirstColumn, columnWidth, rowHeight * 2);
    } else {
        mMetadata->setGeometry(xSpacer, yPosFirstColumn, columnWidth, rowHeight * 2);
        yPosFirstColumn += rowHeight * 4;
        yPosFirstColumn += mMetadata->height();
        mManageButton->setGeometry(xSpacer, yPosFirstColumn, columnWidth, rowHeight);
    }

    if (mState == EBridgeDiscoveryState::connected) {
        QRect selectedLightsRect(xSecondColumnStart,
                                 yPosSecondColumn,
                                 columnWidth * 2 - xSpacer,
                                 rowHeight * 7);
        mLights->resize(selectedLightsRect, mRowHeight);
        yPosSecondColumn += mLights->height();
    } else {
        mImage->setGeometry(xSecondColumnStart, yPosSecondColumn, columnWidth * 2, rowHeight * 7);
    }
}

void DisplayPreviewBridgeWidget::resizeEvent(QResizeEvent*) {
    resize();
}

void DisplayPreviewBridgeWidget::checkBoxClicked(ECheckboxState state) {
    if (state == ECheckboxState::checked) {
        mCheckBox->checkboxState(ECheckboxState::unchecked);
        emit deselectAllClicked(mBridge.id(), EProtocolType::hue);
        mLights->highlightLights({});
    } else {
        mCheckBox->checkboxState(ECheckboxState::checked);
        emit selectAllClicked(mBridge.id(), EProtocolType::hue);
        // filter out IDs that are not reachables
        std::vector<QString> reachableIDs;
        for (auto lightID : mBridge.lightIDs()) {
            if (mComm->lightByID(lightID).isReachable()) {
                reachableIDs.push_back(lightID);
            }
        }
        mLights->highlightLights(reachableIDs);
    }
}

void DisplayPreviewBridgeWidget::updateMetadata(const hue::Bridge& bridge) {
    if (bridge.state() == EBridgeDiscoveryState::connected) {
        std::stringstream returnString;
        returnString << "<b>Lights: </b>" << bridge.lightIDs().size() << "<br>";
        returnString << "<b>Groups: </b>" << bridge.groups().size() << "<br>";
        returnString << "<b>Schedules: </b>" << bridge.schedules().size() << "<br>";
        std::string result = returnString.str();
        mMetadata->setText(QString(result.c_str()));
    } else if (bridge.state() == EBridgeDiscoveryState::lookingForUsername) {
        mMetadata->setText("Bridge found! Please press the Link Button...");
    } else if (bridge.state() == EBridgeDiscoveryState::lookingForResponse) {
        mMetadata->setText("Looking for bridge...");
    }
}

void DisplayPreviewBridgeWidget::highlightLights() {
    mReachableCount = 0u;
    mSelectedCount = 0u;
    std::vector<QString> lightsToHighlight;
    for (auto light : mComm->hue()->lightsFromMetadata(mBridge.lights().items())) {
        if (light.isReachable()) {
            mReachableCount++;
        }
        if (mSelectedLights->doesLightExist(light.uniqueID())) {
            lightsToHighlight.push_back(light.uniqueID());
            mSelectedCount++;
        }
    }

    handleCheckboxState();
    mLights->highlightLights(lightsToHighlight);
    update();
}

void DisplayPreviewBridgeWidget::removeLights(const std::vector<QString>& keys) {
    mLights->removeLights(keys);
}

void DisplayPreviewBridgeWidget::addLight(const cor::Light& light) {
    mLights->addLight(light);
    resize();
}

} // namespace hue
