/*!
 * \copyright
 * Copyright (C) 2015 - 2020.
 * Released under the GNU General Public License.
 */

#include "discoveryhuewidget.h"

#include <QGraphicsOpacityEffect>
#include <QMessageBox>
#include <QScroller>

#include "comm/commhue.h"
#include "mainwindow.h"
#include "utils/qt.h"

DiscoveryHueWidget::DiscoveryHueWidget(QWidget* parent,
                                       CommLayer* comm,
                                       cor::LightList* selectedLights,
                                       ControllerWidget* controllerPage)
    : DiscoveryTypeWidget(parent, comm, controllerPage),
      mBridgeDiscovered{false},
      mSelectedLights{selectedLights},
      mBridgeButtons{std::vector<hue::BridgeButton*>(3, nullptr)},
      mBridgeWidgets{std::vector<hue::DisplayPreviewBridgeWidget*>(3, nullptr)},
      mBridgeIndex{0},
      mRowHeight{10u},
      mHueDiscoveryState{EHueDiscoveryState::findingIpAddress} {}

void DiscoveryHueWidget::hueDiscoveryUpdate(EHueDiscoveryState newState) {
    if (newState != mHueDiscoveryState) {
        mHueDiscoveryState = newState;

        switch (mHueDiscoveryState) {
            case EHueDiscoveryState::findingIpAddress:
                // mLabel->setText(QString("Looking for Bridge..."));
                // qDebug() << "Hue Update: Finding IP Address";
                emit connectionStatusChanged(EProtocolType::hue, EConnectionState::discovering);
                break;
            case EHueDiscoveryState::findingDeviceUsername:
                // mLabel->setText(QString("Bridge Found! Please press Link button..."));
                // qDebug() << "Hue Update: Bridge is waiting for link button to be pressed.";
                emit connectionStatusChanged(EProtocolType::hue, EConnectionState::discovering);
                break;
            case EHueDiscoveryState::testingFullConnection:
                // mLabel->setText(QString("Testing full connection..."));
                // qDebug() << "Hue Update: IP and Username received, testing combination. ";
                emit connectionStatusChanged(EProtocolType::hue, EConnectionState::discovering);
                break;
            case EHueDiscoveryState::bridgeConnected: {
                //                auto totalBridges = mComm->hue()->discovery()->bridges().size()
                //                                    +
                //                                    mComm->hue()->discovery()->notFoundBridges().size();
                //                auto notFoundBridges =
                //                    totalBridges -
                //                    mComm->hue()->discovery()->notFoundBridges().size();
                //                mLabel->setText(
                //                    QString("%1 out of %2 bridges discovered. Searching for
                //                    additional bridges...")
                //                        .arg(QString::number(notFoundBridges),
                //                        QString::number(totalBridges)));
                // qDebug() << "Hue Update: Bridge Connected" << mComm->hueBridge().IP;
                emit connectionStatusChanged(EProtocolType::hue, EConnectionState::discovered);
                break;
            }
            case EHueDiscoveryState::allBridgesConnected:
                //   mLabel->setText(QString(""));
                // qDebug() << "Hue Update: All Bridges Connected" << mComm->hueBridge().IP;
                emit connectionStatusChanged(EProtocolType::hue, EConnectionState::discovered);
                break;
        }
        resize();
    }
}

void DiscoveryHueWidget::handleDiscovery(bool) {
    mComm->hue()->discovery()->startDiscovery();
    hueDiscoveryUpdate(mComm->hue()->discovery()->state());

    updateBridgeGUI();
}

void DiscoveryHueWidget::checkIfIPExists(const QString& IP) {
    if (!mComm->hue()->discovery()->doesIPExist(IP)) {
        mComm->hue()->discovery()->addManualIP(IP);
        closeIPWidget();
    } else {
        QMessageBox reply;
        reply.setText("IP Address already exists.");
        reply.exec();
    }
}

void DiscoveryHueWidget::deleteLight(const QString& id) {
    int index = -1;
    auto storedIndex = mBridgeIndex;
    for (auto i = 0u; i < mBridgeWidgets.size(); ++i) {
        if (mBridgeWidgets[i] != nullptr) {
            if (mBridgeWidgets[i]->bridge().id() == id) {
                index = i;
                delete mBridgeWidgets[i];
                mBridgeWidgets[i] = nullptr;
                delete mBridgeButtons[i];
                mBridgeButtons[i] = nullptr;
            }
        }
    }
    if (index != -1) {
        for (auto i = 0u; i < mBridgeWidgets.size(); ++i) {
            if (mBridgeWidgets[i] != nullptr) {
                mBridgeIndex = i;
                break;
            }
        }
        if (storedIndex == mBridgeIndex) {
            mBridgeIndex = 0u;
        }
    }
    resize();
}

void DiscoveryHueWidget::updateBridgeGUI() {
    auto bridgeList = mComm->hue()->bridges().items();
    // get all not found bridges
    for (const auto& bridge : mComm->hue()->discovery()->notFoundBridges()) {
        bridgeList.push_back(bridge);
    }

    // loop through all bridges
    for (const auto& bridge : bridgeList) {
        // check if light already exists in list
        int widgetIndex = -1;
        int i = 0;
        for (const auto& widget : mBridgeWidgets) {
            if (widget != nullptr) {
                if (widget->bridge().id().isEmpty() && widget->key() == bridge.IP()) {
                    // if theres no unique ID yet for this bridge, check if we can add a username
                    if (!bridge.username().isEmpty()) {
                        // if we can add a username, remove the current widget and re-add with a new
                        // username
                        qDebug() << " TODO: remove widget " << bridge.IP();
                        break;
                    } else {
                        // only update if we still don't have a username
                        widgetIndex = i;
                        widget->updateBridge(bridge);
                    }
                } else if (widget->key() == bridge.id()) {
                    // standard case, theres a unique ID for this bridge
                    widgetIndex = i;
                    widget->updateBridge(bridge);
                }
            }
            ++i;
        }

        // if it doesnt exist, add it
        if (widgetIndex == -1) {
            // check if there is a free index
            int newIndex = -1;
            for (auto i = 0u; i < mBridgeWidgets.size(); ++i) {
                if (mBridgeWidgets[i] == nullptr) {
                    newIndex = i;
                    break;
                }
            }
            if (newIndex != -1) {
                // if the bridge's key can be its id, use that. If not, we'll use a temporary widget
                // that uses an IP address, but as soon as the real id is available, we'll delete
                // this widget, and make a new one with the id as the key.
                QString key = bridge.id();
                if (key.isEmpty()) {
                    key = bridge.IP();
                }
                auto widget = new hue::DisplayPreviewBridgeWidget(bridge,
                                                                  key,
                                                                  mComm,
                                                                  mSelectedLights,
                                                                  mRowHeight,
                                                                  this);
                widget->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
                connect(widget, SIGNAL(bridgeClicked(QString)), this, SLOT(bridgePressed(QString)));
                connect(widget, SIGNAL(selectLight(QString)), this, SLOT(lightSelected(QString)));
                connect(widget,
                        SIGNAL(deselectLight(QString)),
                        this,
                        SLOT(lightDeselected(QString)));
                connect(widget,
                        SIGNAL(selectAllClicked(QString, EProtocolType)),
                        this,
                        SLOT(clickedSelectAll(QString, EProtocolType)));
                connect(widget,
                        SIGNAL(deselectAllClicked(QString, EProtocolType)),
                        this,
                        SLOT(clickedDeselectAll(QString, EProtocolType)));
                connect(widget,
                        SIGNAL(deleteBridge(QString, EProtocolType)),
                        this,
                        SLOT(deleteBridgePressed(QString, EProtocolType)));
                mBridgeWidgets[newIndex] = widget;

                // make a new button for the widget
                auto bridgeName = bridge.customName();
                if (key.isEmpty()) {
                    bridgeName = bridge.IP();
                }
                mBridgeButtons[newIndex] = new hue::BridgeButton(bridgeName, this);
                connect(mBridgeButtons[newIndex],
                        SIGNAL(clicked(QString)),
                        this,
                        SLOT(bridgeButtonPressed(QString)));
                resize();
            } else {
                qDebug() << "INFO: Discovered a bridge but there are already "
                         << mBridgeWidgets.size() << " bridges";
            }
        }
    }
}


void DiscoveryHueWidget::showWidget() {
    for (auto widget : mBridgeWidgets) {
        if (widget != nullptr) {
            widget->updateBridge(widget->bridge());
        }
    }
}

void DiscoveryHueWidget::greyOutClicked() {
    mGreyout->greyOut(false);

    if (mIPWidget->isOpen()) {
        closeIPWidget();
    }
}

void DiscoveryHueWidget::highlightLights() {
    for (auto widget : mBridgeWidgets) {
        if (widget != nullptr) {
            widget->highlightLights();
        }
    }
}

void DiscoveryHueWidget::bridgePressed(const QString& key) {
    const auto& bridgeResult = mComm->hue()->bridges().item(key.toStdString());
    if (bridgeResult.second) {
        mControllerPage->showHueBridge(bridgeResult.first);
        emit showControllerWidget();
    }
}


void DiscoveryHueWidget::lightSelected(QString key) {
    emit selectLight(key);
}

void DiscoveryHueWidget::lightDeselected(QString key) {
    emit deselectLight(key);
}

void DiscoveryHueWidget::clickedSelectAll(QString key, EProtocolType protocol) {
    emit selectControllerLights(key, protocol);
}

void DiscoveryHueWidget::clickedDeselectAll(QString key, EProtocolType protocol) {
    emit deselectControllerLights(key, protocol);
}

void DiscoveryHueWidget::textInputAddedIP(const QString& IP) {
    QHostAddress address(IP);
    if (address.protocol() == QAbstractSocket::IPv4Protocol
        || address.protocol() == QAbstractSocket::IPv6Protocol) {
        if (!mComm->hue()->discovery()->doesIPExist(IP)) {
            mComm->hue()->discovery()->addManualIP(IP);
            closeIPWidget();
        } else {
            QMessageBox reply;
            reply.setText("IP Address already exists.");
            reply.exec();
        }
    } else {
        QMessageBox reply;
        reply.setText("Please enter a valid IP address.");
        reply.exec();
    }
}

void DiscoveryHueWidget::bridgeButtonPressed(QString key) {
    for (auto i = 0u; i < mBridgeButtons.size(); ++i) {
        if (mBridgeButtons[i]->text() == key) {
            mBridgeIndex = i;
            resize();
            break;
        }
    }
}

void DiscoveryHueWidget::deleteBridgePressed(QString key, EProtocolType protocol) {
    emit deleteController(key, protocol);
}

void DiscoveryHueWidget::resize() {
    int yPos = 0;
    auto rowHeight = height() / 10;

    auto buttonWidth = width() / mBridgeButtons.size();

    // check if we should display the buttons, buttons should display only if theres more than one
    int totalButtons = 0;
    for (auto button : mBridgeButtons) {
        if (button != nullptr) {
            totalButtons++;
        }
    }
    if (totalButtons < 2) {
        for (auto button : mBridgeButtons) {
            if (button != nullptr) {
                button->setVisible(false);
            }
        }
    } else {
        int buttonCount = 0;
        for (auto button : mBridgeButtons) {
            if (button != nullptr) {
                button->setVisible(true);
                button->setGeometry(buttonWidth * buttonCount, yPos, buttonWidth, rowHeight);
                buttonCount++;
            }
        }
    }
    yPos += rowHeight;

    std::uint32_t index = 0u;
    for (auto widget : mBridgeWidgets) {
        if (widget != nullptr) {
            if (index == mBridgeIndex) {
                widget->setVisible(true);
            } else {
                widget->setVisible(false);
            }
            widget->setGeometry(int(width() * 0.025), yPos, int(width() * 0.95), rowHeight * 9);
        }
        index++;
    }
}

void DiscoveryHueWidget::resizeEvent(QResizeEvent*) {
    mGreyout->resize();
    resize();
}
