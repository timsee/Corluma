#ifndef EDITREVIEWMOODIDGET_H
#define EDITREVIEWMOODIDGET_H

#include <QMessageBox>
#include <QScrollBar>
#include <QWidget>
#include "cor/lightlist.h"
#include "edit/editpagechildwidget.h"
#include "menu/displaymoodwidget.h"
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
class ReviewMoodWidget : public EditPageChildWidget {
    Q_OBJECT
public:
    explicit ReviewMoodWidget(QWidget* parent,
                              CommLayer* comm,
                              GroupData* groups,
                              cor::LightList* data)
        : EditPageChildWidget(parent),
          mComm{comm},
          mGroups{groups},
          mData{data},
          mTopLabel{new QLabel("Review:", this)},
          mMoodWidget{new DisplayMoodWidget(this, comm, groups)},
          mCreateButton{new QPushButton("Create", this)} {
        mBottomButtons->hideForward(true);
        mCreateButton->setStyleSheet("background-color:rgb(69,67,67);");
        connect(mCreateButton, SIGNAL(clicked(bool)), this, SLOT(createMood(bool)));
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
    void changeRowHeight(int height) { mMoodWidget->changeRowHeight(height); }

    /// displays a group in the group widget
    void displayMood(const QString& name,
                     const QString& description,
                     const std::vector<cor::Light>& lights,
                     const std::vector<cor::GroupState>& defaultStates) {
        // generate a unique ID if and only if its a new group, otherwise, use the unique ID
        // provided when edit mode was turned on.
        std::uint64_t key = mUniqueID;
        if (!mEditMode) {
            key = mGroups->generateNewUniqueKey();
        }
        cor::Mood mood(key, name, lights);
        mood.defaults(defaultStates);
        mood.description(description);
        mMoodWidget->updateMood(mood, mEditMode);
    }

    /// return the mood represented by the page.
    cor::Mood mood() { return mMoodWidget->mood(); }

    /// getter for the bottom buttons. Only the forward button is used in this widget.
    EditBottomButtons* bottomButtons() { return mBottomButtons; }

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

        mMoodWidget->setGeometry(0, yPos, this->width(), buttonHeight * 8);

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
    void createMood(bool) {
        const auto& mood = mMoodWidget->mood();
        // ask if the user is sure before creating
        QString text;
        QString title;
        if (mEditMode) {
            text = "Are you sure you want to edit the mood named " + mood.name() + "with "
                   + QString::number(mood.lights().size()) + " lights?";
            title = "Edit Mood?";
        } else {
            text = "Are you sure you want to save the mood named " + mood.name() + " with "
                   + QString::number(mood.lights().size()) + " lights?";
            title = "Save Mood?";
        }
        auto reply = QMessageBox::question(this, title, text, QMessageBox::Yes | QMessageBox::No);
        if (reply == QMessageBox::Yes) {
            // remove mood if it exists
            mGroups->removeGroup(mood.uniqueID());

            // make a new mood
            mGroups->saveNewMood(mood);
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

    /// data layer
    cor::LightList* mData;

    /// label for top of widget
    QLabel* mTopLabel;

    /// widget to display the group that has been created or edited
    DisplayMoodWidget* mMoodWidget;

    /// button that creates the group
    QPushButton* mCreateButton;

    /// true if reviewing an existing group and thus editing, false if its a new group entirely
    bool mEditMode;

    /// the unique ID of the group being either edited or created.
    std::uint64_t mUniqueID;
};

#endif // EDITREVIEWMOODIDGET_H
