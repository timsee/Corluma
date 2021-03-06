#ifndef EDITNAMEANDDESCRIPTIONWIDGET_H
#define EDITNAMEANDDESCRIPTIONWIDGET_H

#include <QDebug>
#include <QLabel>
#include <QLineEdit>
#include <QTextEdit>
#include <QWidget>
#include "cor/objects/group.h"
#include "edit/editpagechildwidget.h"
#include "edit/grouproomcheckboxwidget.h"

namespace {

constexpr int kNameMaxSize = 25;

constexpr int kDescriptionMaxSize = 200;

} // namespace

/*!
 * \copyright
 * Copyright (C) 2015 - 2020.
 * Released under the GNU General Public License.
 *
 *
 * \brief The EditNameAndDescriptionWidget class allows the user to set the name and the description
 * of a group, room, or mood. It provides two fields: a name field, which is required, and a
 * description field, which is optional. The widget checks that the name and description are both
 * shorter than a maximum length, and that the name isn't used by any reserved strings for the name
 * such as "Miscellaneous".
 */
class ChooseMetadataWidget : public EditPageChildWidget {
    Q_OBJECT
public:
    /// constructor
    explicit ChooseMetadataWidget(QWidget* parent, bool isMood)
        : EditPageChildWidget(parent),
          mIsMood{isMood},
          mNameLabel{new QLabel("Name:", this)},
          mNameInput{new QLineEdit(this)},
          mDescriptionLabel{new QLabel("Description (Optional):", this)},
          mDescriptionInput{new QTextEdit(this)},
          mGroupRoomWidget{new GroupRoomCheckboxWidget(this)} {
        mBottomButtons->hideBack(true);
        mBottomButtons->enableForward(false);

        mNameInput->setMaxLength(kNameMaxSize);
        connect(mNameInput, SIGNAL(textEdited(QString)), this, SLOT(lineEditChanged(QString)));

        if (isMood) {
            mGroupRoomWidget->setVisible(false);
        }
        connect(mGroupRoomWidget, SIGNAL(boxChecked()), this, SLOT(groupRoomBoxChecked()));

        connect(mDescriptionInput, SIGNAL(textChanged()), this, SLOT(descriptionChanged()));
    }

    /// getter for the name for set by the page
    QString name() { return mNameInput->text(); }

    /// getter for the description set by the page.
    QString description() { return mDescriptionInput->toPlainText(); }

    /// getter for the group type
    cor::EGroupType groupType() {
        if (mGroupRoomWidget->groupChecked()) {
            return cor::EGroupType::group;
        } else {
            return cor::EGroupType::room;
        }
    }

    /// true if any information does not match the original information, false otherwise;
    bool hasEdits() override {
        return !(mOriginalName == name() && mOriginalDescription == description());
    }

    /// prefill the metadata with existing data to edit
    void prefill(const QString& name, const QString& description, const cor::EGroupType& type) {
        mOriginalName = name;
        mOriginalDescription = description;
        mIsMood = (type == cor::EGroupType::mood);
        mNameInput->setText(name);
        mDescriptionInput->setText(description);
        bool lockAsGroup = (type == cor::EGroupType::group);
        if (mIsMood) {
            mGroupRoomWidget->setVisible(false);
        } else {
            mGroupRoomWidget->setVisible(true);
            mGroupRoomWidget->lockSelection(lockAsGroup);
        }
        conditionsMet();
    }

    /// clears all data currently on the page.
    void clear() {
        mOriginalName = "";
        mOriginalDescription = "";
        mNameInput->setText("");
        mDescriptionInput->setText("");
        mGroupRoomWidget->unlockSelection();
        mBottomButtons->enableForward(false);
    }

protected:
    /*!
     * \brief resizeEvent called whenever the widget resizes so that assets can be updated.
     */
    void resizeEvent(QResizeEvent*) override {
        int yPos = 0;
        int heightSpacer = this->height() / 10;

        mNameLabel->setGeometry(0, yPos, this->width(), heightSpacer);
        yPos += mNameLabel->height();

        mNameInput->setGeometry(0, yPos, this->width(), heightSpacer);
        yPos += mNameInput->height();

        if (!mIsMood) {
            mGroupRoomWidget->setGeometry(0, yPos, this->width(), heightSpacer * 1.5);
            yPos += mGroupRoomWidget->height();
        }

        mDescriptionLabel->setGeometry(0, yPos, this->width(), heightSpacer);
        yPos += mDescriptionLabel->height();

        if (mIsMood) {
            mDescriptionInput->setGeometry(0, yPos, this->width(), heightSpacer * 5);
        } else {
            mDescriptionInput->setGeometry(0, yPos, this->width(), heightSpacer * 3.5);
        }
        yPos += mDescriptionInput->height();

        // add a spacer
        yPos += heightSpacer;

        mBottomButtons->setGeometry(0, yPos, this->width(), heightSpacer);
    }

private slots:

    /// QLineEdit had its text changed, check if its a valid name or not.
    void lineEditChanged(QString) { conditionsMet(); }

    /// called when the groupRoom widget has a box that is checked.
    void groupRoomBoxChecked() { conditionsMet(); }

    /// the description text changed, verify that its less than the maximum length
    void descriptionChanged() {
        QString text = mDescriptionInput->toPlainText();
        if (text.length() > kDescriptionMaxSize) {
            // get count of number of characters over
            auto diff = text.length() - kDescriptionMaxSize;
            text.chop(diff);
            mDescriptionInput->setText(text);
            QTextCursor cursor(mDescriptionInput->textCursor());
            cursor.movePosition(QTextCursor::End, QTextCursor::MoveAnchor);
            mDescriptionInput->setTextCursor(cursor);
        }
    }

private:
    /// true if all fields have met the conditions needed to proceed, false if any widget has an
    /// invalid input.
    bool conditionsMet() {
        if (!mIsMood) {
            // check if the a room or group is checked
            if (!(mGroupRoomWidget->roomChecked() || mGroupRoomWidget->groupChecked())) {
                emit stateChanged(mIndex, EEditProgressState::incomplete);
                mBottomButtons->enableForward(false);
                return false;
            }
        }

        // check if name is correct
        if (mNameInput->text().size() > 2) {
            auto name = mNameInput->text();
            if ((name != "zzzzMiscellaneous") && (name != "Miscellaneous")
                && (name != "zzzAVAIALBLE_DEVICES")) {
                emit stateChanged(mIndex, EEditProgressState::completed);
                mBottomButtons->enableForward(true);
                return true;
            }
        }
        emit stateChanged(mIndex, EEditProgressState::incomplete);
        mBottomButtons->enableForward(false);
        return false;
    }

    /// true if a mood, false if a group or room.
    bool mIsMood;

    /// label for the name input
    QLabel* mNameLabel;

    /// line edit for the name
    QLineEdit* mNameInput;

    /// label for the description
    QLabel* mDescriptionLabel;

    /// text field for the description
    QTextEdit* mDescriptionInput;

    /// stores the original name, used to check for changes
    QString mOriginalName;

    /// stores the original description, used to check for changes
    QString mOriginalDescription;

    /// widget for choosing whether it is a group or a room.
    GroupRoomCheckboxWidget* mGroupRoomWidget;
};

#endif // EDITNAMEANDDESCRIPTIONWIDGET_H
