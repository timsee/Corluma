/*!
 * \copyright
 * Copyright (C) 2015 - 2020.
 * Released under the GNU General Public License.
 */

#include "discoverywidget.h"
#include <QMessageBox>


DiscoveryWidget::DiscoveryWidget(QWidget* parent, CommLayer* comm, ControllerPage* controllerPage)
    : QWidget(parent),
      mComm{comm},
      mControllerPage{controllerPage},
      mIPWidget{new cor::TextInputWidget(parentWidget()->parentWidget())},
      mGreyout{new GreyOutOverlay(true, parentWidget()->parentWidget())} {
    connect(mIPWidget, SIGNAL(textAdded(QString)), this, SLOT(textInputAddedIP(QString)));
    connect(mIPWidget, SIGNAL(cancelClicked()), this, SLOT(closeIPWidget()));
    mIPWidget->setVisible(false);

    connect(mGreyout, SIGNAL(clicked()), this, SLOT(greyOutClicked()));
    mGreyout->greyOut(false);
}

void DiscoveryWidget::fillList(QListWidget* list, std::vector<cor::Controller>& connections) {
    std::vector<QString> strings;
    for (const auto& connection : connections) {
        strings.push_back(connection.name());
    }
    fillList(list, strings);
}

void DiscoveryWidget::fillList(QListWidget* list, std::vector<QString>& connections) {
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

void DiscoveryWidget::openIPWidget() {
    mGreyout->greyOut(true);
    mIPWidget->pushIn(IPWidgetPrompt(), IPWidgetDefaultValue());
    mIPWidget->raise();
    mIPWidget->setVisible(true);
}

void DiscoveryWidget::textInputAddedIP(const QString& IP) {
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

void DiscoveryWidget::closeIPWidget() {
    mGreyout->greyOut(false);
    mIPWidget->pushOut();
    mIPWidget->setVisible(false);
}

void DiscoveryWidget::greyOutClicked() {
    mGreyout->greyOut(false);
    if (mIPWidget->isOpen()) {
        closeIPWidget();
    }
}
