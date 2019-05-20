/*!
 * \copyright
 * Copyright (C) 2015 - 2019.
 * Released under the GNU General Public License.
 */

#include "discoverywidget.h"


DiscoveryWidget::DiscoveryWidget(QWidget* parent) : QWidget(parent), mComm{nullptr} {}

void DiscoveryWidget::fillList(QListWidget* list, std::list<cor::Controller>& connections) {
    std::list<QString> strings;
    for (auto&& connection : connections) {
        strings.push_back(connection.name);
    }
    fillList(list, strings);
}

void DiscoveryWidget::fillList(QListWidget* list, std::list<QString>& connections) {
    for (auto&& connection : connections) {
        // check if item is already in the table, if not, add it
        bool connectionFound = false;
        for (int i = 0; i < list->count(); ++i) {
            QListWidgetItem* item = list->item(i);
            if (item->text().compare(connection) == 0) {
                connectionFound = true;
            }
        }
        if (!connectionFound) {
            list->addItem(connection);
        }
    }
}
