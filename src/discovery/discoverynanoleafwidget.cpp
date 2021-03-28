/*!
 * \copyright
 * Copyright (C) 2015 - 2020.
 * Released under the GNU General Public License.
 */

#include "discoverynanoleafwidget.h"
#include <QMessageBox>
#include <QScroller>
#include "comm/commnanoleaf.h"
#include "display/displaypreviewnanoleafwidget.h"
#include "mainwindow.h"
#include "utils/qt.h"

DiscoveryNanoLeafWidget::DiscoveryNanoLeafWidget(QWidget* parent,
                                                 CommLayer* comm,
                                                 cor::LightList* selectedLights,
                                                 ControllerWidget* controllerPage)
    : DiscoveryTypeWidget(parent, comm, controllerPage),
      mSelectedLights{selectedLights},
      mListWidget{new cor::ListWidget(this, cor::EListType::grid)},
      mPlaceholderWidget{new ListPlaceholderWidget(
          this,
          "No Nanoleaf lights have been discovered. Give the lights up to a minute to discover "
          "automatically. If that doesn't work, click the ? button for more debugging tips, or "
          "click the + to add an IP address manually.")},
      mHasLights{false},
      mLabel{new QLabel(this)} {
    mListWidget->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    QScroller::grabGesture(mListWidget->viewport(), QScroller::LeftMouseButtonGesture);
}


void DiscoveryNanoLeafWidget::handleDiscovery(bool) {
    // starts discovery if its not already started
    mComm->nanoleaf()->discovery()->startDiscovery();

    const auto& foundNanoleafs = mComm->nanoleaf()->discovery()->foundLights().items();
    const auto& notFoundNanoleafs = mComm->nanoleaf()->discovery()->notFoundLights();
    const auto& unknownLights = mComm->nanoleaf()->discovery()->unknownLights();
    mHasLights = (!foundNanoleafs.empty() || !notFoundNanoleafs.empty() || !unknownLights.empty());

    // loop through all found nanoleafs
    for (const auto& nanoleaf : foundNanoleafs) {
        handleNanoleaf(nanoleaf, nano::ELeafDiscoveryState::connected);
    }

    // loop through all not found nanoleafs
    for (const auto& nanoleaf : notFoundNanoleafs) {
        handleNanoleaf(nanoleaf, nano::ELeafDiscoveryState::reverifying);
    }

    // loop through all partially discovered nanoleafs
    for (const auto& nanoleaf : unknownLights) {
        // figure out the discovery state of each unknown light
        nano::ELeafDiscoveryState status;
        if (!nanoleaf.IPVerified()) {
            status = nano::ELeafDiscoveryState::searchingIP;
        } else if (nanoleaf.authToken().isEmpty()) {
            status = nano::ELeafDiscoveryState::searchingAuth;
        } else {
            status = nano::ELeafDiscoveryState::reverifying;
        }
        handleNanoleaf(nanoleaf, status);
    }

    removeDuplicatedNanoleafs();

    // handle button updates
    if (mComm->discoveryErrorsExist(EProtocolType::nanoleaf)) {
        emit connectionStatusChanged(EProtocolType::nanoleaf, EConnectionState::connectionError);
    } else if (!foundNanoleafs.empty()) {
        emit connectionStatusChanged(EProtocolType::nanoleaf, EConnectionState::discovered);
    } else {
        emit connectionStatusChanged(EProtocolType::nanoleaf, EConnectionState::off);
    }

    if (mHasLights) {
        mListWidget->setVisible(true);
        mPlaceholderWidget->setVisible(false);
    } else {
        mListWidget->setVisible(false);
        mPlaceholderWidget->setVisible(true);
    }
}

void DiscoveryNanoLeafWidget::handleNanoleaf(const nano::LeafMetadata& nanoleaf,
                                             nano::ELeafDiscoveryState status) {
    auto light = mComm->nanoleaf()->lightFromMetadata(nanoleaf);
    cor::LightState state;
    if (light.second) {
        state = light.first.state();
    }
    // check if light already exists in list
    int widgetIndex = -1;
    int i = 0;
    for (const auto& widget : mListWidget->widgets()) {
        auto nanoleafWidget = dynamic_cast<DisplayPreviewNanoleafWidget*>(widget);
        if (widget->key() == nanoleaf.serialNumber()) {
            // standard case, theres a unique ID for this bridge
            widgetIndex = i;
            nanoleafWidget->updateNanoleaf(nanoleaf, state, status);
            nanoleafWidget->setShouldHighlight(
                mSelectedLights->doesLightExist(nanoleaf.serialNumber()));
        }
        ++i;
    }

    // if it doesnt exist, add it
    if (widgetIndex == -1) {
        auto widget =
            new DisplayPreviewNanoleafWidget(nanoleaf, state, status, mListWidget->mainWidget());
        widget->setShouldHighlight(mSelectedLights->doesLightExist(nanoleaf.serialNumber()));
        connect(widget, SIGNAL(clicked(QString)), this, SLOT(nanoleafClicked(QString)));
        mListWidget->insertWidget(widget);
        resize();
    }
}

void DiscoveryNanoLeafWidget::removeDuplicatedNanoleafs() {
    std::vector<QString> widgetsToRemove;
    for (const auto& widget : mListWidget->widgets()) {
        auto nanoleafWidget = dynamic_cast<DisplayPreviewNanoleafWidget*>(widget);
        if (nanoleafWidget->nanoleaf().serialNumber().contains("Unknown--")) {
            // check if this unknown has been discovered
            // get its IP address
            QStringList splitIP = nanoleafWidget->nanoleaf().serialNumber().split("//");
            auto IP = splitIP[1];
            for (const auto& innerWidget : mListWidget->widgets()) {
                auto innerNanoleafWidget = dynamic_cast<DisplayPreviewNanoleafWidget*>(innerWidget);
                if (innerNanoleafWidget->status() == nano::ELeafDiscoveryState::connected
                    && innerNanoleafWidget->nanoleaf().IP().contains(IP)) {
                    widgetsToRemove.push_back(nanoleafWidget->nanoleaf().serialNumber());
                }
            }
        }
    }

    bool shouldResize = !widgetsToRemove.empty();
    for (auto widgetToRemove : widgetsToRemove) {
        mListWidget->removeWidget(widgetToRemove);
    }
    if (shouldResize) {
        resize();
    }
}

void DiscoveryNanoLeafWidget::checkIfIPExists(const QString& IP) {
    if (!mComm->nanoleaf()->discovery()->doesIPExist(IP)) {
        mComm->nanoleaf()->discovery()->addIP(IP);
        closeIPWidget();
    } else {
        QMessageBox reply;
        reply.setText("IP Address already exists.");
        reply.exec();
    }
}

void DiscoveryNanoLeafWidget::deleteLight(const QString& light) {
    std::vector<QString> widgetsToRemove;
    for (const auto& widget : mListWidget->widgets()) {
        auto nanoleafWidget = dynamic_cast<DisplayPreviewNanoleafWidget*>(widget);
        if (nanoleafWidget->nanoleaf().serialNumber() == light) {
            widgetsToRemove.emplace_back(nanoleafWidget->nanoleaf().serialNumber());
        }
    }
    bool shouldResize = !widgetsToRemove.empty();
    for (auto widgetToRemove : widgetsToRemove) {
        mListWidget->removeWidget(widgetToRemove);
    }
    if (shouldResize) {
        resize();
    }
}

void DiscoveryNanoLeafWidget::resizeEvent(QResizeEvent*) {
    resize();
}

void DiscoveryNanoLeafWidget::resize() {
    auto yPos = 0u;
    mLabel->setGeometry(0, 0, int(width() * 0.7), int(height() * 0.1));
    yPos += mLabel->height();
    auto lightRect = QRect(int(width() * 0.025), yPos, int(width() * 0.95), int(height() * 0.9));
    mListWidget->setGeometry(lightRect);
    mPlaceholderWidget->setGeometry(lightRect);
    mGreyout->resize();
    mListWidget->mainWidget()->setFixedWidth(width() * 0.9);

    mListWidget->setPreferredWidgetHeight(height() / 2.7);
    mListWidget->resizeWidgets();

    resizeHelpView();
}

void DiscoveryNanoLeafWidget::nanoleafClicked(QString nanoleaf) {
    for (auto widget : mListWidget->widgets()) {
        auto nanoleafWidget = dynamic_cast<DisplayPreviewNanoleafWidget*>(widget);
        if (nanoleafWidget->nanoleaf().name() == nanoleaf) {
            mControllerPage->showNanoleaf(nanoleafWidget->nanoleaf(), nanoleafWidget->status());
            emit showControllerWidget();
        }
    }
}

void DiscoveryNanoLeafWidget::greyOutClicked() {
    mGreyout->greyOut(false);
    if (mIPWidget->isOpen()) {
        closeIPWidget();
    }
}

void DiscoveryNanoLeafWidget::highlightLights() {
    for (auto widget : mListWidget->widgets()) {
        auto nanoleafWidget = dynamic_cast<DisplayPreviewNanoleafWidget*>(widget);
        nanoleafWidget->setShouldHighlight(
            mSelectedLights->doesLightExist(nanoleafWidget->nanoleaf().serialNumber()));
    }
}

QString DiscoveryNanoLeafWidget::discoveryHelpHTML() {
    std::stringstream sstream;
    sstream << "<b>General Tips</b>";
    sstream << "<ul>";
    sstream
        << "<li> Original Nanoleafs must be using firmware v2.1.0 or later to connect "
           "(released in 2017). Nanoleaf Shapes and Panels are supported on all firmwares. </li>";
    sstream << "<li> Corluma assumes that the Nanoleaf has already completed first time setup and "
               "can connect to your local Wifi. If this has not happened yet, use the official "
               "Nanoleaf app to discover the lights once first. </li>";
    sstream << "<li> Nanoleafs can sometimes become unresponsive. To test this, try pressing their "
               "power button once. If they do not turn on, unplug and replug them. </li>";
    sstream << "</ul>";

    sstream << "<b>Debugging Connections</b>";
    sstream << "<ul>";
    if (mComm->UPnP()->isStateConnected() && mComm->UPnP()->hasReceivedTraffic()) {
        sstream << "<li> UPnP is active, and has received traffic. New Nanoleafs can be found "
                   "without an IP address.</li>";
    } else if (mComm->UPnP()->isStateConnected() && !mComm->UPnP()->hasReceivedTraffic()) {
        sstream << "<li> UPnP is active, but has not yet received traffic. </li>";
    } else {
        sstream << "<li> ERROR: UPnP is not active. </li>";
    }
#ifdef USE_EXPERIMENTAL_FEATURES
    sstream << "<li> Discovery Time: "
            << cor::makePrettyTimeOutput(mComm->nanoleaf()->discovery()->lastDiscoveryTime())
                   .toStdString()
            << " </li>";
    sstream << "<li> Last Send Time: "
            << cor::makePrettyTimeOutput(mComm->nanoleaf()->lastSendTime()).toStdString()
            << " </li>";
    sstream << "<li> Last Receive Time: "
            << cor::makePrettyTimeOutput(mComm->nanoleaf()->lastReceiveTime()).toStdString()
            << " </li>";
#endif
    sstream << "</ul>";
    return QString(sstream.str().c_str());
}
