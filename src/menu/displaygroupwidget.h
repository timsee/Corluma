#ifndef DISPLAYGROUPWIDGET_H
#define DISPLAYGROUPWIDGET_H

#include <QPainter>
#include <QStyleOption>
#include <QTextEdit>
#include <QWidget>
#include "cor/objects/group.h"
#include "cor/widgets/expandingtextscrollarea.h"
#include "menu/displaygroupmetadata.h"
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
class DisplayGroupWidget : public QWidget {
    Q_OBJECT
public:
    explicit DisplayGroupWidget(QWidget* parent, CommLayer* comm, GroupData* groups)
        : QWidget(parent),
          mName{new QLabel(this)},
          mDescription{new cor::ExpandingTextScrollArea(this)},
          mMetadata{new DisplayGroupMetadata(this, groups)},
          mLights{new LightsListMenu(this, comm, false)} {
        auto font = mName->font();
        font.setPointSize(20);
        mName->setFont(font);

        this->setStyleSheet("background-color:rgb(33,32,32);");
    }

    /// getter for group represented by the widget
    const cor::Group& group() const noexcept { return mGroup; }

    /// updates the group's UI elements.
    void updateGroup(const cor::Group& group) {
        mGroup = group;
        mName->setText(group.name());
        if (group.description().isEmpty()) {
            mDescription->setVisible(false);
        } else {
            mDescription->setVisible(true);
            mDescription->updateText(group.description());
        }
        mLights->updateLights();
        mLights->showGroup(group.lights());
        mMetadata->update(group);
        resize();
    }

    /// changes the row height of rows in scroll areas.
    void changeRowHeight(int height) { mRowHeight = height; }

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
        QRect selectedLightsRect(xSpacer, yPosColumn1, columnWidth - xSpacer, buttonHeight * 7);
        mLights->resize(selectedLightsRect, mRowHeight);
        yPosColumn1 += mLights->height();

        // column 2
        if (mDescription->isVisible()) {
            mDescription->setGeometry(xSecondColumnStart,
                                      yPosColumn2,
                                      columnWidth,
                                      3 * buttonHeight);
            // add an additional spacer
            yPosColumn2 += mDescription->height() + buttonHeight;
        }

        mMetadata->setGeometry(xSecondColumnStart, yPosColumn2, columnWidth, 6 * buttonHeight);
    }

    /// stores the group that is being displayed
    cor::Group mGroup;

    /// name of the group
    QLabel* mName;

    /// description text box
    cor::ExpandingTextScrollArea* mDescription;

    /// metadata, displays info like the group's parent, subgroups, etc.
    DisplayGroupMetadata* mMetadata;

    /// displays the lights that are part of this group and their current states.
    LightsListMenu* mLights;

    /// the height of a row in a scroll area
    int mRowHeight;
};

#endif // DISPLAYGROUPWIDGET_H
