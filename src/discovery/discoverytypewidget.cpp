/*!
 * \copyright
 * Copyright (C) 2015 - 2020.
 * Released under the GNU General Public License.
 */

#include "discoverytypewidget.h"
#include <QMessageBox>
#include "mainwindow.h"
#include "utils/qt.h"

DiscoveryTypeWidget::DiscoveryTypeWidget(QWidget* parent,
                                         CommLayer* comm,
                                         ControllerWidget* controllerPage)
    : QWidget(parent),
      mComm{comm},
      mControllerPage{controllerPage},
      mIPWidget{new cor::TextInputWidget(parentWidget()->parentWidget()->parentWidget())},
      mHelpView{
          new cor::WebView("Discovery Help", "", parentWidget()->parentWidget()->parentWidget())},
      mHelpViewIsOpen{false},
      mGreyout{new GreyOutOverlay(true, parentWidget()->parentWidget()->parentWidget())} {
    connect(mIPWidget, SIGNAL(textAdded(QString)), this, SLOT(textInputAddedIP(QString)));
    connect(mIPWidget, SIGNAL(cancelClicked()), this, SLOT(closeIPWidget()));
    mIPWidget->setVisible(false);

    connect(mHelpView, SIGNAL(closePressed()), this, SLOT(closeHelpView()));
    mHelpView->setVisible(false);

    connect(mGreyout, SIGNAL(clicked()), this, SLOT(greyOutClicked()));
    mGreyout->greyOut(false);
}

void DiscoveryTypeWidget::fillList(QListWidget* list, std::vector<cor::Controller>& connections) {
    std::vector<QString> strings;
    for (const auto& connection : connections) {
        strings.push_back(connection.name());
    }
    fillList(list, strings);
}

void DiscoveryTypeWidget::fillList(QListWidget* list, std::vector<QString>& connections) {
    for (const auto& connection : connections) {
        // check if item is already in the table, if not, add it
        bool connectionFound = false;
        for (int i = 0; i < list->count(); ++i) {
            QListWidgetItem* item = list->item(i);
            if (item->text() == connection) {
                connectionFound = true;
            }
        }
        if (!connectionFound) {
            list->addItem(connection);
        }
    }
}


void DiscoveryTypeWidget::openIPWidget() {
    mGreyout->greyOut(true);
    mIPWidget->pushIn(IPWidgetPrompt(), IPWidgetDefaultValue());
    mIPWidget->setVisible(true);
    mIPWidget->raise();
}

void DiscoveryTypeWidget::textInputAddedIP(const QString& IP) {
    QHostAddress address(IP);
    if (address.protocol() == QAbstractSocket::IPv4Protocol
        || address.protocol() == QAbstractSocket::IPv6Protocol) {
        checkIfIPExists(IP);
    } else {
        QMessageBox reply;
        reply.setText("Please enter a valid IP address.");
        reply.exec();
    }
}

void DiscoveryTypeWidget::closeIPWidget() {
    mGreyout->greyOut(false);
    mIPWidget->pushOut();
    mIPWidget->setVisible(false);
}

void DiscoveryTypeWidget::resizeHelpView() {
    auto widget = parentWidget()->parentWidget()->parentWidget();
    int xPos;
    int width;
    if (cor::mainWindow()->leftHandMenu()->alwaysOpen()) {
        xPos = cor::mainWindow()->leftHandMenu()->width();
        width = widget->width() - cor::mainWindow()->leftHandMenu()->width();
    } else {
        xPos = 0;
        width = widget->width();
    }
    if (mHelpViewIsOpen) {
        mHelpView->setGeometry(xPos, 0, width, widget->height());
    } else {
        mHelpView->setGeometry(xPos, widget->height(), width, widget->height());
    }
}

void DiscoveryTypeWidget::openHelpView() {
    int xPos;
    if (cor::mainWindow()->leftHandMenu()->alwaysOpen()) {
        xPos = cor::mainWindow()->leftHandMenu()->width();
    } else {
        xPos = 0;
    }
    mHelpViewIsOpen = true;
    mHelpView->setVisible(true);
    mHelpView->updateHTML(discoveryHelpHTML());
    cor::moveWidget(mHelpView, QPoint(xPos, mHelpView->height()), QPoint(xPos, 0));
    mHelpView->raise();
}

void DiscoveryTypeWidget::closeHelpView() {
    int xPos;
    if (cor::mainWindow()->leftHandMenu()->alwaysOpen()) {
        xPos = cor::mainWindow()->leftHandMenu()->width();
    } else {
        xPos = 0;
    }
    mHelpViewIsOpen = false;
    cor::moveWidget(mHelpView, QPoint(xPos, 0), QPoint(xPos, mHelpView->height()));
}

void DiscoveryTypeWidget::greyOutClicked() {
    mGreyout->greyOut(false);
    if (mIPWidget->isOpen()) {
        closeIPWidget();
    }
}
