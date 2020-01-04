/*!
 * \copyright
 * Copyright (C) 2015 - 2019.
 * Released under the GNU General Public License.
 */

#include "discoverywidget.h"


DiscoveryWidget::DiscoveryWidget(QWidget* parent) : QWidget(parent), mComm{nullptr} {}

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
