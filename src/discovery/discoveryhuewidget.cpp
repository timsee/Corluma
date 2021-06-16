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
#include "comm/hue/bridgediscovery.h"
#include "comm/upnpdiscovery.h"
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
      mPlaceholderWidget{new ListPlaceholderWidget(
          this,
          "No Hue Bridges lights have been discovered. Give the Bridges up to a minute to discover "
          "automatically. If that doesn't work, click the ? button for more debugging tips, or "
          "click the + to add an IP address manually.")},
      mHasLights{false},
      mBridgeIndex{0},
      mRowHeight{10u},
      mHueDiscoveryState{EHueDiscoveryState::findingIpAddress} {
#ifdef USE_EXPERIMENTAL_FEATURES
    mHelpView->useDebugButton(true);
    connect(mHelpView, SIGNAL(debugPressed()), this, SLOT(debugClicked()));
#endif
}

void DiscoveryHueWidget::hueDiscoveryUpdate(EHueDiscoveryState newState) {
    if (newState != mHueDiscoveryState) {
        mHueDiscoveryState = newState;

        switch (mHueDiscoveryState) {
            case EHueDiscoveryState::findingIpAddress:
                // qDebug() << "Hue Update: Finding IP Address";
                emit connectionStatusChanged(EProtocolType::hue, EConnectionState::discovering);
                break;
            case EHueDiscoveryState::findingDeviceUsername:
                // qDebug() << "Hue Update: Bridge is waiting for link button to be pressed.";
                emit connectionStatusChanged(EProtocolType::hue, EConnectionState::discovering);
                break;
            case EHueDiscoveryState::testingFullConnection:
                // qDebug() << "Hue Update: IP and Username received, testing combination. ";
                emit connectionStatusChanged(EProtocolType::hue, EConnectionState::discovering);
                break;
            case EHueDiscoveryState::bridgeConnected: {
                // qDebug() << "Hue Update: Bridge Connected";
                emit connectionStatusChanged(EProtocolType::hue, EConnectionState::discovered);
                break;
            }
            case EHueDiscoveryState::allBridgesConnected:
                // qDebug() << "Hue Update: All Bridges Connected";
                emit connectionStatusChanged(EProtocolType::hue, EConnectionState::discovered);
                break;
        }
        resize();
    }
}

void DiscoveryHueWidget::handleDiscovery(bool) {
    mComm->hue()->discovery()->startDiscovery();
    mHasLights = (!mComm->hue()->discovery()->notFoundBridges().empty()
                  || !mComm->hue()->discovery()->bridges().empty());

    if (mHasLights) {
        mPlaceholderWidget->setVisible(false);
    } else {
        mPlaceholderWidget->setVisible(true);
    }

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

void DiscoveryHueWidget::deleteLight(const cor::LightID& id) {
    int index = -1;
    auto storedIndex = mBridgeIndex;
    for (auto i = 0u; i < mBridgeWidgets.size(); ++i) {
        if (mBridgeWidgets[i] != nullptr) {
            if (mBridgeWidgets[i]->bridge().id() == id.toString()) {
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

                    if (mBridgeButtons[i]->text() != bridge.customName()) {
                        mBridgeButtons[i]->setText(bridge.customName());
                    }
                } else if (widget->key() == bridge.id()) {
                    // standard case, theres a unique ID for this bridge
                    widgetIndex = i;
                    widget->updateBridge(bridge);

                    if (mBridgeButtons[i]->text() != bridge.customName()) {
                        mBridgeButtons[i]->setText(bridge.customName());
                    }
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
                connect(widget,
                        SIGNAL(selectLight(cor::LightID)),
                        this,
                        SLOT(lightSelected(cor::LightID)));
                connect(widget,
                        SIGNAL(deselectLight(cor::LightID)),
                        this,
                        SLOT(lightDeselected(cor::LightID)));
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
                if (newIndex > 0) {
                    highlightBridgeButtons();
                }
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

void DiscoveryHueWidget::handleDeletedLights(const std::vector<cor::LightID>& keys) {
    for (const auto& widget : mBridgeWidgets) {
        if (widget != nullptr) {
            widget->removeLights(keys);
        }
    }
    // update the bridges in the GUI.
    updateBridgeGUI();
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


void DiscoveryHueWidget::lightSelected(cor::LightID key) {
    emit selectLight(key);
}

void DiscoveryHueWidget::lightDeselected(cor::LightID key) {
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
    highlightBridgeButtons();
}

void DiscoveryHueWidget::highlightBridgeButtons() {
    for (auto i = 0u; i < mBridgeButtons.size(); ++i) {
        if (mBridgeButtons[i] != nullptr) {
            mBridgeButtons[i]->setChecked(i == mBridgeIndex);
        }
    }
}


void DiscoveryHueWidget::handleBridgeNameUpdate(const QString&, const QString&) {
    updateBridgeGUI();
}

void DiscoveryHueWidget::updateLightNames() {
    updateBridgeGUI();
}

void DiscoveryHueWidget::deleteBridgePressed(QString key, EProtocolType protocol) {
    emit deleteController(key, protocol);
}

void DiscoveryHueWidget::resize() {
    int yPos = 0;
    auto rowHeight = height() / 10;

    auto buttonWidth = width() / mBridgeButtons.size();

    // check if we should display the buttons, buttons should display only if theres more than
    // one
    int totalButtons = 0;
    for (auto button : mBridgeButtons) {
        if (button != nullptr) {
            totalButtons++;
        }
    }
    bool buttonsShown = false;
    if (totalButtons < 2) {
        for (auto button : mBridgeButtons) {
            if (button != nullptr) {
                button->setVisible(false);
            }
        }
    } else {
        buttonsShown = true;
        int buttonCount = 0;
        for (auto button : mBridgeButtons) {
            if (button != nullptr) {
                button->setVisible(true);
                button->setGeometry(buttonWidth * buttonCount, yPos, buttonWidth, rowHeight);
                buttonCount++;
            }
        }
        yPos += rowHeight;
    }

    std::uint32_t index = 0u;
    auto height = rowHeight * 9;
    if (buttonsShown) {
        height = rowHeight * 8;
    }
    if (yPos == 0) {
        yPos += this->height() * 0.05;
    }
    auto lightRect = QRect(int(width() * 0.03), yPos, int(width() - width() * 0.06), height);
    mPlaceholderWidget->setGeometry(lightRect);
    for (auto widget : mBridgeWidgets) {
        if (widget != nullptr) {
            if (index == mBridgeIndex) {
                widget->setVisible(true);
            } else {
                widget->setVisible(false);
            }
            widget->setGeometry(lightRect);
        }
        index++;
    }

    resizeHelpView();
}

void DiscoveryHueWidget::resizeEvent(QResizeEvent*) {
    mGreyout->resize();
    resize();
}

void DiscoveryHueWidget::newHuesFound(const std::vector<cor::LightID>& IDs) {
    auto lights = mComm->lightsByIDs(IDs);
    for (auto light : lights) {
        auto bridge = mComm->hue()->bridgeFromLight(light);
        for (const auto& widget : mBridgeWidgets) {
            if (widget != nullptr) {
                if (widget->bridge().id() == bridge.id()) {
                    widget->addLight(light);
                }
            }
        }
    }
    updateBridgeGUI();
}

QString DiscoveryHueWidget::discoveryHelpHTML() {
    std::stringstream sstream;
    sstream << "<b>General Tips</b>";
    sstream << "<ul>";
    sstream << "<li> Give Corluma up to a minute to automatically discover the Bridge via UPnP or "
               "NUPnP. </li>";
    sstream << "<li> If this fails, you know the IP address of the bridge, enter the IP "
               "address by clicking the + button. </li>";
    sstream << "<li> If the IP address fails, or you do not know it, verify the IP address in the "
               "Hue App. "
               "</li>";
    sstream << "<li> If the Hue App can't connect, verify that the Bridge is connected via "
               "ethernet and is receiving power. "
               "</li>";
    sstream << "</ul>";
    sstream << "<b>Debugging Connections</b>";
    sstream << "<ul>";
    if (!QSslSocket::supportsSsl()) {
        sstream << "<li> SSL support was not found, so the app cannot received NUPnP packets. This "
                   "means discovery relies on UPnP, which will take up to a minute. </li>";
    } else if (mComm->hue()->discovery()->receivedNUPnPTraffic()) {
        sstream << "<li> Rceived NUPnP traffic. All known Bridges have been updated.</li>";
    } else {
        sstream << "<li> NUPnP has not sent a response. This typically takes less than 30 seconds, "
                   "and cannot be achieved on a VPN. </li>";
    }
    if (mComm->UPnP()->isStateConnected() && mComm->UPnP()->hasReceivedTraffic()) {
        sstream << "<li> UPnP is active, and has received traffic. New bridges can be found "
                   "without an IP address.</li>";
    } else if (mComm->UPnP()->isStateConnected() && !mComm->UPnP()->hasReceivedTraffic()) {
        sstream << "<li> UPnP is active, but has not yet received traffic. </li>";
    } else {
        sstream << "<li> ERROR: UPnP is not active. </li>";
    }
#ifdef USE_EXPERIMENTAL_FEATURES
    sstream
        << "<li> Discovery Time: "
        << cor::makePrettyTimeOutput(mComm->hue()->discovery()->lastDiscoveryTime()).toStdString()
        << " </li>";
    sstream << "<li> Last Send Time: "
            << cor::makePrettyTimeOutput(mComm->hue()->lastSendTime()).toStdString() << " </li>";
    sstream << "<li> Last Receive Time: "
            << cor::makePrettyTimeOutput(mComm->hue()->lastReceiveTime()).toStdString() << " </li>";
    sstream << "<li> Last Request Header: " << mComm->hue()->lastRequestHeader().toStdString()
            << " </li>";
    sstream << "<li> Last Request: " << mComm->hue()->lastRequest().toStdString() << " </li>";
    sstream << "<li> Last Response: " << mComm->hue()->lastResponse().toStdString() << " </li>";
#endif
    sstream << "</ul>";
    return QString(sstream.str().c_str());
}

#ifdef USE_EXPERIMENTAL_FEATURES
void DiscoveryHueWidget::debugClicked() {
    qDebug() << " Debug clicked!";
}
#endif
