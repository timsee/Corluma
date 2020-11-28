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
                                                 ControllerPage* controllerPage)
    : DiscoveryWidget(parent, comm, controllerPage) {
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
    auto nanoleafList = foundNanoleafs;
    // get all not found bridges
    for (const auto& controller : notFoundNanoleafs) {
        nanoleafList.push_back(controller);
    }

    // loop through all nanoleafs
    for (const auto& nanoleaf : nanoleafList) {
        // check if light already exists in list
        int widgetIndex = -1;
        int i = 0;
        for (const auto& widget : mListWidget->widgets()) {
            auto nanoleafWidget = dynamic_cast<DisplayPreviewNanoleafWidget*>(widget);
            if (widget->key() == nanoleaf.serialNumber()) {
                // standard case, theres a unique ID for this bridge
                widgetIndex = i;
                nanoleafWidget->updateNanoleaf(nanoleaf);
            }
            ++i;
        }

        // if it doesnt exist, add it
        if (widgetIndex == -1) {
            auto widget = new DisplayPreviewNanoleafWidget(nanoleaf, mListWidget->mainWidget());
            widget->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
            connect(widget, SIGNAL(clicked(QString)), this, SLOT(nanoleafClicked(QString)));
            mListWidget->insertWidget(widget);
            resize();
        }
    }

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
            mLabel->setText("Aurora found! Hold the power button for around 5 "
                            "seconds, until the LED to the left of it starts blinking. ");
            break;
        case ENanoleafDiscoveryState::someNanoleafsConnected:
            mLabel->setText("Additional Aurora found! Please hold the power button for around 5 "
                            "seconds, until the LED to the left of it starts blinking. ");
            break;
        case ENanoleafDiscoveryState::allNanoleafsConnected:
            mLabel->setText("All NanoLeaf discovered and fully connected!");
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

void DiscoveryNanoLeafWidget::resizeEvent(QResizeEvent*) {
    resize();
}

void DiscoveryNanoLeafWidget::resize() {
    auto yPos = 0u;
    mLabel->setGeometry(0, 0, int(width() * 0.7), int(height() * 0.25));
    yPos += mLabel->height();
    mListWidget->setGeometry(int(width() * 0.025),
                             yPos,
                             int(width() * 0.95),
                             int(height() * 0.735));
    mGreyout->resize();

    // call resize function of each widget
    auto yHeight = 0u;
    QSize widgetSize(mListWidget->width(), int(mListWidget->height() * 0.33));
    for (auto widget : mListWidget->widgets()) {
        auto nanoleafWidget = dynamic_cast<DisplayPreviewNanoleafWidget*>(widget);
        nanoleafWidget->setGeometry(0, yHeight, widgetSize.width(), widgetSize.height());
        nanoleafWidget->resize();
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
            cor::mainWindow()->showControllerPage();
        }
    }
}

void DiscoveryNanoLeafWidget::greyOutClicked() {
    mGreyout->greyOut(false);
    if (mIPWidget->isOpen()) {
        closeIPWidget();
    }
}
