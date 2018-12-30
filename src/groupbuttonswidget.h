#ifndef LISTGROUPTOPWIDGET_H
#define LISTGROUPTOPWIDGET_H

#include <QWidget>
#include <QGridLayout>

#include "cor/groupbutton.h"
#include "cor/dictionary.h"

#include <vector>

/*!
 * \copyright
 * Copyright (C) 2015 - 2019.
 * Released under the GNU General Public License.
 *
 *
 * \brief The GroupButtonsWidget class is a widget that displays a grid of groups buttons. This is used in
 *        ListRoomWidgets to show subgroups of a room, such as the lights around a desk in a bedroom. This
 *        widget does a bit of magic to shorten names of widgets if the subgroups have matching words at the
 *        start of the widget as the Room. For example, if the room is named "John's Bedroom" and the group is
 *        called "John's Bedroom Desk" the group's name would be shortened to "Desk".
 */
class GroupButtonsWidget : public QWidget
{
    Q_OBJECT

public:
    /// constructor
    explicit GroupButtonsWidget(QWidget *parent, const QString& roomName, const std::vector<QString>& groups);

    /// add a group to a the list of supported groups
    void addGroup(const QString& group);

    /// get list of groups shown by the widget
    std::list<QString> groupNames();

    /// key of the currently selected group
    const QString& currentKey() const { return mCurrentKey; }

    /// update the checked devices of the group that matches the key
    void updateCheckedDevices(const QString& key, uint32_t checkedDeviceCount, uint32_t reachableDeviceCount);

signals:

    /// emitted when a group button is pressed. This emits its actual group name, instead of its displayed group name.
    void groupButtonPressed(QString key);

    /// emitted when a group's toggle button is pressed. This emits its actual group name, instead of its displayed group name.
    void groupSelectAllToggled(QString key, bool);

private slots:

    /// picks up when a group button is pressed.
    void buttonPressed(QString key);

    /// picks up when a group's select all/deselect all button is pressed.
    void buttonToggled(QString key, bool);

protected:

    /// resizes interal widgets when a resize event is triggered
    void resizeEvent(QResizeEvent *);

private:

    /// returns the renamed version of a group when given an actual group name
    QString renamedGroup(QString group);

    /// returns the original group name when given a renamed group
    QString originalGroup(QString group);

    /// creates a converted group name, when given a room and group name.
    QString convertGroupName(QString room, QString group);

    /// stores the key of the currently selected group
    QString mCurrentKey;

    /// stores the name of the room it is representing
    QString mRoomName;

    /// stores the group button widgets
    std::vector<cor::GroupButton*> mButtons;

    /// stores the map of the relabeled named
    cor::Dictionary<std::string> mRelabeledNames;

    /// main layout
    QGridLayout *mLayout;

    /// number of buttons displayed in row
    int mGroupCount;
};

#endif // LISTGROUPTOPWIDGET_H
