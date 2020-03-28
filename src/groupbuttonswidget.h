#ifndef LISTGROUPTOPWIDGET_H
#define LISTGROUPTOPWIDGET_H

#include <QWidget>
#include <vector>

#include "cor/dictionary.h"
#include "cor/protocols.h"
#include "cor/widgets/groupbutton.h"

/*!
 * \copyright
 * Copyright (C) 2015 - 2020.
 * Released under the GNU General Public License.
 *
 *
 * \brief The GroupButtonsWidget class is a widget that displays a grid of groups buttons. This is
 * used in ParentGroupWidget to show subgroups of a room, such as the lights around a desk in a
 * bedroom. This widget does a bit of magic to shorten names of widgets if the subgroups have
 * matching words at the start of the widget as the Room. For example, if the room is named "John's
 * Bedroom" and the group is called "John's Bedroom Desk" the group's name would be shortened to
 * "Desk".
 */
class GroupButtonsWidget : public QWidget {
    Q_OBJECT

public:
    /// constructor
    explicit GroupButtonsWidget(QWidget* parent,
                                cor::EWidgetType type,
                                const QString& roomName,
                                const std::vector<QString>& groups);

    /// add a group to a the list of supported groups
    void addGroup(const QString& group);

    /// removes a group from the list of supported groups
    void removeGroup(const QString& group);

    /// true if empty, false if theres at least one button
    bool empty() { return mButtons.empty(); }

    /// get list of groups shown by the widget
    std::vector<QString> groupNames();

    /// key of the currently selected group
    const QString& currentKey() const { return mCurrentKey; }

    /// deselect current group
    void deselectGroup();

    /// update the checked devices of the group that matches the key
    void updateCheckedLights(const QString& key,
                             std::uint32_t checkedLightCount,
                             std::uint32_t reachableLightCount);

    /// getter for type of widget
    cor::EWidgetType type() const noexcept { return mType; }

    /// expected height of the overall widget based on the top widget's height
    int expectedHeight(int topWidgetHeight);

    /// the end point in the y dimension of a group given a key
    int groupEndPointY(int topWidgetHeight, const QString& key);

signals:

    /// emitted when a group button is pressed. This emits its actual group name, instead of its
    /// displayed group name.
    void groupButtonPressed(QString key);

    /// emitted when a group's toggle button is pressed. This emits its actual group name, instead
    /// of its displayed group name.
    void groupSelectAllToggled(QString key, bool selectAll);

protected:
    /// resizes interal widgets when a resize event is triggered
    void resizeEvent(QResizeEvent*);

private slots:

    /// picks up when a group button is pressed.
    void buttonPressed(const QString& key);

    /// picks up when a group's select all/deselect all button is pressed.
    void buttonToggled(QString key, bool selectAll) {
        emit groupSelectAllToggled(originalGroup(key), selectAll);
    }

private:
    /// resizes prorgrammatically
    void resize();

    /// ttype of widget
    cor::EWidgetType mType;

    /// returns the renamed version of a group when given an actual group name
    QString renamedGroup(const QString& group);

    /// returns the original group name when given a renamed group
    QString originalGroup(const QString& group);

    /// creates a converted group name, when given a room and group name.
    QString convertGroupName(const QString& room, const QString& group);

    /// stores the key of the currently selected group
    QString mCurrentKey;

    /// stores the name of the room it is representing
    QString mRoomName;

    /// stores the group button widgets
    std::vector<cor::GroupButton*> mButtons;

    /// stores the map of the relabeled named
    cor::Dictionary<std::string> mRelabeledNames;

    /// number of buttons displayed in row
    int mGroupCount;
};

#endif // LISTGROUPTOPWIDGET_H
