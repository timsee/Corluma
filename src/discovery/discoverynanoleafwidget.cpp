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
      mSelectedLights{selectedLights} {
    mLabel = new QLabel(this);
    mLabel->setWordWrap(true);
    mLabel->setText("Looking for NanoLeaf...");
    mLabel->setAlignment(Qt::AlignLeft | Qt::AlignBottom);

    mListWidget = new cor::ListWidget(this, cor::EListType::linear);
    mListWidget->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    QScroller::grabGesture(mListWidget->viewport(), QScroller::LeftMouseButtonGesture);
}


void DiscoveryNanoLeafWidget::handleDiscovery(bool) {
    // starts discovery if its not already started
    mComm->nanoleaf()->discovery()->startDiscovery();

    const auto& foundNanoleafs = mComm->nanoleaf()->discovery()->foundLights().items();
    const auto& notFoundNanoleafs = mComm->nanoleaf()->discovery()->notFoundLights();
    const auto& unknownLights = mComm->nanoleaf()->discovery()->unknownLights();

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

    ENanoleafDiscoveryState discoveryState = mComm->nanoleaf()->discovery()->state();
    switch (discoveryState) {
        case ENanoleafDiscoveryState::connectionError:
        case ENanoleafDiscoveryState::discoveryOff:
            mLabel->setText("Connection Error");
            break;
        case ENanoleafDiscoveryState::lookingForPreviousNanoleafs:
            mLabel->setText("Testing previous connection data..");
            break;
        case ENanoleafDiscoveryState::nothingFound:
            mLabel->setText("Looking for a NanoLeaf Aurora. This may take up to a minute...");
            break;
        case ENanoleafDiscoveryState::unknownNanoleafsFound:
            mLabel->setText("Nanoleaf found!");
            break;
        case ENanoleafDiscoveryState::someNanoleafsConnected:
            mLabel->setText("Additional Nanoleaf found!");
            break;
        case ENanoleafDiscoveryState::allNanoleafsConnected:
            mLabel->setText("");
            break;
    }

    // handle button updates
    if (mComm->discoveryErrorsExist(EProtocolType::nanoleaf)) {
        emit connectionStatusChanged(EProtocolType::nanoleaf, EConnectionState::connectionError);
    } else if (!foundNanoleafs.empty()) {
        emit connectionStatusChanged(EProtocolType::nanoleaf, EConnectionState::discovered);
    } else {
        emit connectionStatusChanged(EProtocolType::nanoleaf, EConnectionState::off);
    }
}

void DiscoveryNanoLeafWidget::handleNanoleaf(const nano::LeafMetadata& nanoleaf,
                                             nano::ELeafDiscoveryState status) {
    // check if light already exists in list
    int widgetIndex = -1;
    int i = 0;
    for (const auto& widget : mListWidget->widgets()) {
        auto nanoleafWidget = dynamic_cast<DisplayPreviewNanoleafWidget*>(widget);
        if (widget->key() == nanoleaf.serialNumber()) {
            // standard case, theres a unique ID for this bridge
            widgetIndex = i;
            nanoleafWidget->updateNanoleaf(nanoleaf, status);
            nanoleafWidget->setShouldHighlight(
                mSelectedLights->doesLightExist(nanoleaf.serialNumber()));
        }
        ++i;
    }

    // if it doesnt exist, add it
    if (widgetIndex == -1) {
        auto widget = new DisplayPreviewNanoleafWidget(nanoleaf, status, mListWidget->mainWidget());
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
    mListWidget->setGeometry(int(width() * 0.025), yPos, int(width() * 0.95), int(height() * 0.9));
    mGreyout->resize();
    mListWidget->mainWidget()->setFixedWidth(width());

    // call resize function of each widget
    auto yHeight = 0u;
    QSize widgetSize(mListWidget->width(), int(mListWidget->height() * 0.33));
    for (auto widget : mListWidget->widgets()) {
        auto nanoleafWidget = dynamic_cast<DisplayPreviewNanoleafWidget*>(widget);
        nanoleafWidget->setGeometry(0, yHeight, widgetSize.width(), widgetSize.height());
        yHeight += nanoleafWidget->height();
    }
    mListWidget->mainWidget()->setFixedHeight(yHeight);
    mListWidget->mainWidget()->setFixedWidth(width());
}

void DiscoveryNanoLeafWidget::nanoleafClicked(QString nanoleaf) {
    for (auto widget : mListWidget->widgets()) {
        auto nanoleafWidget = dynamic_cast<DisplayPreviewNanoleafWidget*>(widget);
        if (nanoleafWidget->nanoleaf().name() == nanoleaf) {
            mControllerPage->showNanoleaf(nanoleafWidget->nanoleaf());
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
