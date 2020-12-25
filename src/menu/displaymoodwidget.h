#ifndef DISPLAYMOODWIDGET_H
#define DISPLAYMOODWIDGET_H

#include <QPainter>
#include <QStyleOption>
#include <QTextEdit>
#include <QWidget>
#include "comm/commlayer.h"
#include "cor/objects/group.h"
#include "cor/widgets/expandingtextscrollarea.h"
#include "menu/displaymoodmetadata.h"
#include "menu/groupstatelistmenu.h"
#include "menu/lightslistmenu.h"

/*!
 * \copyright
 * Copyright (C) 2015 - 2020.
 * Released under the GNU General Public License.
 *
 *
 * \brief The DisplayGroupWidget class displays all pertinent info for a cor::Group. This includes
 * the group's name, the group's lights, and the description if one exists.
 */
class DisplayMoodWidget : public QWidget {
    Q_OBJECT
public:
    explicit DisplayMoodWidget(QWidget* parent, CommLayer* comm, GroupData* groups)
        : QWidget(parent),
          mComm{comm},
          mGroups{groups},
          mName{new QLabel(this)},
          mLightsLabel{new QLabel("Lights:", this)},
          mGroupsLabel{new QLabel("Groups:", this)},
          mDescription{new cor::ExpandingTextScrollArea(this)},
          mMetadata{new DisplayMoodMetadata(this, comm, groups)},
          mLights{new LightsListMenu(this, false)},
          mGroupDefaults{new GroupStateListMenu(this, false)},
          mRowHeight{10} {
        auto font = mName->font();
        font.setPointSize(20);
        mName->setFont(font);

        this->setStyleSheet("background-color:rgb(33,32,32);");
    }

    /// getter for group represented by the widget
    const cor::Mood& mood() const noexcept { return mMood; }

    /// updates the group's UI elements.
    void updateMood(const cor::Mood& mood, bool moodExistsAlready) {
        mMood = mComm->addMetadataToMood(mood);
        mName->setText(mMood.name());
        if (mMood.description().isEmpty()) {
            mDescription->setVisible(false);
        } else {
            mDescription->setVisible(true);
            mDescription->updateText(mMood.description());
        }


        if (mMood.defaults().empty()) {
            mGroupDefaults->setVisible(false);
            mGroupsLabel->setVisible(false);
        } else {
            mGroupDefaults->setVisible(true);
            mGroupsLabel->setVisible(true);
            mGroupDefaults->showStates(mMood.defaults());
        }
        mLights->updateLights();
        mLights->showLights(mMood.lights());
        mMetadata->update(mood, moodExistsAlready);
        resize();
    }

    /// changes the row height of rows in scroll areas.
    void changeRowHeight(int height) { mRowHeight = height; }

    /// reset the widget to showing no group
    void reset() {
        mMood = {};
        mName->setText("");
        mDescription->setVisible(false);
        mLights->showLights({});
        mMetadata->reset();
        mGroupDefaults->clear();
        resize();
    }

    /// programmatically resize
    void resize() {
        int yPosColumn1 = 0;
        int yPosColumn2 = 0;
        int buttonHeight = this->height() / 10;
        int xSecondColumnStart = int(this->width() / 2 * 1.05);
        int columnWidth = int((this->width() / 2) * 0.95);
        int xSpacer = this->width() / 20;

        // top of both
        mName->setGeometry(xSpacer / 2, yPosColumn1, this->width() - xSpacer / 2, buttonHeight);
        yPosColumn1 += mName->height();
        yPosColumn2 += mName->height();

        // column 1
        mLightsLabel->setGeometry(xSpacer, yPosColumn1, columnWidth - xSpacer, buttonHeight);
        yPosColumn1 += mLightsLabel->height();

        QRect selectedLightsRect(xSpacer, yPosColumn1, columnWidth - xSpacer, buttonHeight * 8);
        mLights->resize(selectedLightsRect, mRowHeight);
        yPosColumn1 += mLights->height();

        // column 2
        if (mGroupsLabel->isVisible()) {
            mGroupsLabel->setGeometry(xSecondColumnStart, yPosColumn2, columnWidth, buttonHeight);
        }
        yPosColumn2 += mLightsLabel->height();

        int defaultsHeight;
        int descriptionHeight;
        int metadataHeight;
        if (mGroupDefaults->isVisible() && mDescription->isVisible()) {
            defaultsHeight = buttonHeight * 5;
            descriptionHeight = buttonHeight * 2;
            metadataHeight = buttonHeight * 3;
        } else if (mGroupDefaults->isVisible() && !mDescription->isVisible()) {
            defaultsHeight = buttonHeight * 5;
            descriptionHeight = 0;
            metadataHeight = buttonHeight * 5;
        } else if (!mGroupDefaults->isVisible() && mDescription->isVisible()) {
            defaultsHeight = 0;
            descriptionHeight = buttonHeight * 5;
            metadataHeight = buttonHeight * 5;
        } else {
            defaultsHeight = 0;
            descriptionHeight = 0;
            metadataHeight = buttonHeight * 10;
        }
        if (mGroupDefaults->isVisible()) {
            QRect groupStatesRect(xSecondColumnStart, yPosColumn2, columnWidth, defaultsHeight);
            mGroupDefaults->resize(groupStatesRect, mRowHeight);
            yPosColumn2 += mGroupDefaults->height();
        }

        if (mDescription->isVisible()) {
            mDescription->setGeometry(xSecondColumnStart,
                                      yPosColumn2,
                                      columnWidth,
                                      descriptionHeight);

            // add an additional spacer
            yPosColumn2 += mDescription->height() + buttonHeight;
        }
        mMetadata->setGeometry(xSecondColumnStart, yPosColumn2, columnWidth, metadataHeight);
    }

protected:
    /*!
     * \brief resizeEvent called whenever the widget resizes so that assets can be updated.
     */
    void resizeEvent(QResizeEvent*) { resize(); }

    /// paints the dark grey background
    void paintEvent(QPaintEvent*) {
        QStyleOption opt;
        opt.init(this);
        QPainter painter(this);
        painter.fillRect(rect(), QBrush(QColor(32, 31, 31, 255)));
    }


private:
    /// pointer to comm data
    CommLayer* mComm;

    /// pointer to group data
    GroupData* mGroups;

    /// stores the group that is being displayed
    cor::Mood mMood;

    /// name of the group
    QLabel* mName;

    /// label for lights
    QLabel* mLightsLabel;

    /// label for groups
    QLabel* mGroupsLabel;

    /// description text box
    cor::ExpandingTextScrollArea* mDescription;

    /// metadata, displays info like the mood's parent, subgroups, etc.
    DisplayMoodMetadata* mMetadata;

    /// displays the lights that are part of this group and their current states.
    LightsListMenu* mLights;

    /// list of group defaults
    GroupStateListMenu* mGroupDefaults;

    /// the height of a row in a scroll area
    int mRowHeight;
};

#endif // DISPLAYMOODWIDGET_H
