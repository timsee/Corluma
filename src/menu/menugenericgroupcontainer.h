#ifndef MENUGENERICGROUPCONTAINER_H
#define MENUGENERICGROUPCONTAINER_H


#include <QWidget>
#include "cor/widgets/groupbutton.h"
#include "data/appdata.h"

/*!
 * \copyright
 * Copyright (C) 2015 - 2020.
 * Released under the GNU General Public License.
 *
 *
 * \brief The MenuGenericGroupContainer class displays groups in a scrollable list. These groups can
 * be rooms or just standard groups. Groups emit when they are clicked.
 */
class MenuGenericGroupContainer : public QWidget {
    Q_OBJECT
public:
    explicit MenuGenericGroupContainer(QWidget* parent);

    /// shows the groups with the names provided
    void showGroups(const std::vector<QString>& groups, int buttonHeight);

    //// resize the subgroups programmatically
    void resizeWidgets(int buttonHeight);

    /// expected height of the overall widget based on the top widget's height
    int expectedHeight(int topWidgetHeight);

signals:

    /// emitted when a group button is pressed. This emits its group ID
    void parentClicked(QString key);

protected:
    /// resizes interal widgets when a resize event is triggered
    void resizeEvent(QResizeEvent*);

private slots:

    /// picks up when a group button is pressed.
    void buttonPressed(const QString& key);

private:
    /// number of buttons displayed in row
    int mGroupCount;

    /// add a group to a the list of supported groups
    void addGroup(const QString& group);

    /// stored example of the widget height
    int mWidgetHeight;

    /// stores the group button widgets
    std::vector<cor::GroupButton*> mButtons;
};


#endif // MENUGENERICGROUPCONTAINER_H
