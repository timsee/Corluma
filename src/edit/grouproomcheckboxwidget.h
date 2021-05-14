#ifndef GROUPROOMCHECKBOXWIDGET_H
#define GROUPROOMCHECKBOXWIDGET_H

#include <QDebug>
#include <QLabel>
#include <QWidget>
#include "cor/widgets/checkbox.h"

/*!
 * \copyright
 * Copyright (C) 2015 - 2020.
 * Released under the GNU General Public License.
 *
 *
 * \brief The GroupRoomCheckboxWidget class is a subwidget for the ChooseMetadataWidget that allows
 * the user to choose whether a group is a standard group, or a special case where its a room.
 * Lights can belong to any number of groups, but can only belong to one room. Rooms are meant to
 * refer to the physical location of a light (such as a Bedroom, Kitchen, or Dining Room), while a
 * group can be used for any purpose (IE, a desk, all first floor lights, or a Nightstand). If a
 * group is being edited instead of being created, the user cannot change whether the existing group
 * is a group or a room.
 */
class GroupRoomCheckboxWidget : public QWidget {
    Q_OBJECT
public:
    explicit GroupRoomCheckboxWidget(QWidget* parent)
        : QWidget(parent),
          mGroupButton{new QPushButton("Group", this)},
          mRoomButton{new QPushButton("Room", this)},
          mDescription{new QLabel(this)},
          mLastSize{0, 0} {
        mDescription->setWordWrap(true);
        mGroupButton->setCheckable(true);
        mRoomButton->setCheckable(true);


        connect(mGroupButton, SIGNAL(clicked(bool)), this, SLOT(groupCheckboxPressed(bool)));
        connect(mRoomButton, SIGNAL(clicked(bool)), this, SLOT(roomCheckboxPressed(bool)));
    }

    /// true if group is checked
    bool groupChecked() { return mGroupButton->isChecked(); }

    /// true if room is checked
    bool roomChecked() { return mRoomButton->isChecked(); }

    /// true if editing and making changes is locked
    bool isLocked() { return mLock; }

    /// locks selection, used for editing existing groups
    void lockSelection(bool groupShouldBeChecked) {
        mLock = true;
        if (groupShouldBeChecked) {
            mGroupButton->setChecked(true);
            mRoomButton->setChecked(false);
        } else {
            mGroupButton->setChecked(false);
            mRoomButton->setChecked(true);
        }
        updateDescription();
        mGroupButton->setEnabled(false);
        mRoomButton->setEnabled(false);
    }

    /// unlocks selection, resetting the widget for creating new groups
    void unlockSelection() {
        mGroupButton->setEnabled(true);
        mRoomButton->setEnabled(true);
        mGroupButton->setChecked(false);
        mRoomButton->setChecked(false);
        mLock = false;
    }

signals:

    /// emits when the user interacts with a checkbox
    void boxChecked();

private slots:

    /// handles when the group checkbox is pressed
    void groupCheckboxPressed(bool checked) {
        mRoomButton->setChecked(!checked);
        mGroupButton->setChecked(checked);
        updateDescription();
        emit boxChecked();
    }

    /// handles when the room checkbox is pressed
    void roomCheckboxPressed(bool checked) {
        mGroupButton->setChecked(!checked);
        mRoomButton->setChecked(checked);
        updateDescription();
        emit boxChecked();
    }

protected:
    /// called when the app resizes
    void resizeEvent(QResizeEvent*) { resize(); }

private:
    /// handles resizing the widget
    void resize() {
        int yPos = 0;
        int widgetHeight = this->height() / 3;

        mGroupButton->setGeometry(0, yPos, this->width() / 2, widgetHeight);
        mRoomButton->setGeometry(this->width() / 2, yPos, this->width() / 2, widgetHeight);
        yPos += mGroupButton->height();

        mDescription->setGeometry(0, yPos, this->width(), widgetHeight * 2);


        QSize iconSize(widgetHeight * 0.8, widgetHeight * 0.8);
        if (iconSize != mLastSize) {
            mLastSize = iconSize;
            QPixmap pixmap(":/images/groups_icon.png");
            pixmap = pixmap.scaled(mLastSize.width(),
                                   mLastSize.height(),
                                   Qt::KeepAspectRatio,
                                   Qt::SmoothTransformation);
            mGroupButton->setIcon(QIcon(pixmap));
            mGroupButton->setIconSize(mLastSize);
            QPixmap pixmap2(":/images/room_icon.png");
            pixmap2 = pixmap2.scaled(mLastSize.width(),
                                     mLastSize.height(),
                                     Qt::KeepAspectRatio,
                                     Qt::SmoothTransformation);
            mRoomButton->setIcon(QIcon(pixmap2));
            mRoomButton->setIconSize(mLastSize);
        }
    }

    /// updates the description of a room or a group to help out the user in making their selection
    void updateDescription() {
        QString string;
        if (groupChecked()) {
            string = "A light can belong to multiple groups. A group must have "
                     "more than one light. Examples: Desk Lights, First Floor";
        } else {
            string =
                "A light can only belong to one room. Rooms are used for physical locations of "
                "lights. Examples: Kitchen, Living Room.";
        }

        if (mLock) {
            string = " <b>NOTE: Whether it is a group or a room cannot be changed during edit.</b>";
        }
        mDescription->setText(string);
    }

    QPushButton* mGroupButton;

    QPushButton* mRoomButton;

    /// the label for showing a description
    QLabel* mDescription;

    /// true if locked, false if not.
    bool mLock;

    /// last size used for the icon.
    QSize mLastSize;
};

#endif // GROUPROOMCHECKBOXWIDGET_H
