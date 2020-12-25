#ifndef EDITREVIEWGROUPWIDGET_H
#define EDITREVIEWGROUPWIDGET_H

#include <QMessageBox>
#include <QScrollBar>
#include <QWidget>
#include "edit/editpagechildwidget.h"
#include "menu/displaygroupwidget.h"
#include "utils/qt.h"

/*!
 * \copyright
 * Copyright (C) 2015 - 2020.
 * Released under the GNU General Public License.
 *
 *
 * \brief The ReviewGroupWidget class provides a high level view of the previous pages in the edit
 * page, and allows the user to review the edit they are making to a group before finally committing
 * the edit.
 */
class ReviewGroupWidget : public EditPageChildWidget {
    Q_OBJECT
public:
    explicit ReviewGroupWidget(QWidget* parent, CommLayer* comm, GroupData* groups)
        : EditPageChildWidget(parent),
          mComm{comm},
          mGroups{groups},
          mTopLabel{new QLabel("Review:", this)},
          mGroupWidget{new DisplayGroupWidget(this, comm, groups)},
          mCreateButton{new QPushButton("Create", this)} {
        mBottomButtons->hideForward(true);
        mCreateButton->setStyleSheet("background-color:rgb(69,67,67);");
        connect(mCreateButton, SIGNAL(clicked(bool)), this, SLOT(createGroup(bool)));
    }

    /// set to true if editing an existing group, set to false if its a new group
    void editMode(bool isEditMode, std::uint64_t uniqueID) {
        mEditMode = isEditMode;
        if (mEditMode) {
            mCreateButton->setText("Edit");
        } else {
            mCreateButton->setText("Create");
        }
        mUniqueID = uniqueID;
    }

    /// always false, no edits happen from this page
    bool hasEdits() override { return false; }

    /// change the height of rows in scroll widgets
    void changeRowHeight(int height) { mGroupWidget->changeRowHeight(height); }

    /// displays a group in the group widget
    void displayGroup(const QString& name,
                      const cor::EGroupType& type,
                      const QString& description,
                      const std::vector<QString>& lights) {
        // generate a unique ID if and only if its a new group, otherwise, use the unique ID
        // provided when edit mode was turned on.
        std::uint64_t key = mUniqueID;
        if (!mEditMode) {
            key = mGroups->generateNewUniqueKey();
        }
        cor::Group group(key, name, type, lights);
        group.description(description);
        mGroupWidget->updateGroup(group, mEditMode);
    }

    /// getter for the bottom buttons. Only the forward button is used in this widget.
    EditBottomButtons* bottomButtons() { return mBottomButtons; }

    /// getter for unique ID of the group
    std::uint64_t uniqueID() { return mUniqueID; }

    /// true if in edit mode, false if creating new group.
    bool isEditMode() { return mEditMode; }

protected:
    /*!
     * \brief resizeEvent called whenever the widget resizes so that assets can be updated.
     */
    void resizeEvent(QResizeEvent*) override {
        int yPos = 0;
        int buttonHeight = this->height() / 10;

        // column 1
        mTopLabel->setGeometry(0, yPos, this->width(), buttonHeight);
        yPos += mTopLabel->height();

        mGroupWidget->setGeometry(0, yPos, this->width(), buttonHeight * 8);

        auto createWidth = this->width() / 6;
        mCreateButton->setGeometry(this->width() - createWidth,
                                   this->height() - buttonHeight,
                                   createWidth,
                                   buttonHeight);
        mBottomButtons->setGeometry(0, this->height() - buttonHeight, this->width(), buttonHeight);
    }
private slots:

    /// handles when the create button is pressed. Verifies the user wants to create a group, and if
    /// they do, it creates the group and closes the page.
    void createGroup(bool) {
        const auto& group = mGroupWidget->group();
        // ask if the user is sure before creating
        QString text;
        QString title;
        if (mEditMode) {
            text = "Are you sure you want to edit the group named " + group.name() + " with "
                   + QString::number(group.lights().size()) + " lights?";
            title = "Edit Group?";
        } else {
            text = "Are you sure you want to save the group named " + group.name() + " with "
                   + QString::number(group.lights().size()) + " lights?";
            title = "Save Group?";
        }
        auto reply = QMessageBox::question(this, title, text, QMessageBox::Yes | QMessageBox::No);
        if (reply == QMessageBox::Yes) {
            if (mEditMode) {
                // remove the existing group
                // mGroups->removeGroup(mGroupWidget->group().uniqueID());
                qDebug() << "INFO: editing group" << group.toJson();
            } else {
                qDebug() << "INFO: adding new group" << group.toJson();
            }
            // save the group
            mComm->saveNewGroup(group);
            emit updateGroups();
            // close the page.
            emit forceClosePage();
        }
    }


private:
    /// pointer to comm layer
    CommLayer* mComm;

    /// pointer to group data
    GroupData* mGroups;

    /// label for top of widget
    QLabel* mTopLabel;

    /// widget to display the group that has been created or edited
    DisplayGroupWidget* mGroupWidget;

    /// button that creates the group
    QPushButton* mCreateButton;

    /// true if reviewing an existing group and thus editing, false if its a new group entirely
    bool mEditMode;

    /// the unique ID of the group being either edited or created.
    std::uint64_t mUniqueID;
};

#endif // EDITREVIEWGROUPWIDGET_H
