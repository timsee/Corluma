#ifndef DISPLAYGROUPWIDGET_H
#define DISPLAYGROUPWIDGET_H

#include <QLabel>
#include <QScrollArea>
#include <QScrollBar>
#include <QScroller>
#include <QWidget>
#include "cor/objects/group.h"
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
    explicit DisplayGroupWidget(QWidget* parent, CommLayer* comm)
        : QWidget(parent),
          mName{new QLabel(this)},
          mDescription{new QLabel(this)},
          mDescriptionScrollArea{new QScrollArea(this)},
          mLights{new LightsListMenu(this, comm)} {
        mDescriptionScrollArea->setWidget(mDescription);
        mDescriptionScrollArea->horizontalScrollBar()->setVisible(false);
        QScroller::grabGesture(mDescriptionScrollArea->viewport(),
                               QScroller::LeftMouseButtonGesture);

        auto font = mName->font();
        font.setPointSize(20);
        mName->setFont(font);

        mDescription->setWordWrap(true);
        mDescription->setAlignment(Qt::AlignTop | Qt::AlignLeft);
        mDescription->setStyleSheet("background-color:rgb(33,32,32);");
    }

    /// getter for group represented by the widget
    const cor::Group& group() const noexcept { return mGroup; }

    /// updates the group's UI elements.
    void updateGroup(const cor::Group& group) {
        mGroup = group;
        mName->setText(group.name());
        if (group.description().isEmpty()) {
            mDescription->setVisible(false);
            mDescriptionScrollArea->setVisible(false);
        } else {
            mDescription->setVisible(true);
            mDescriptionScrollArea->setVisible(true);
            mDescription->setText(group.description());
        }
        mLights->updateLights();
        mLights->showGroup(group.lights());
        resize();
    }

    /// changes the row height of rows in scroll areas.
    void changeRowHeight(int height) { mRowHeight = height; }

protected:
    /*!
     * \brief resizeEvent called whenever the widget resizes so that assets can be updated.
     */
    void resizeEvent(QResizeEvent*) { resize(); }

private:
    /// programmatically resize
    void resize() {
        int yPosColumn1 = 0;
        int yPosColumn2 = 0;
        int buttonHeight = this->height() / 10;
        int xSecondColumnStart = int(this->width() / 2 * 1.05);
        int columnWidth = int((this->width() / 2) * 0.95);

        // top of both
        mName->setGeometry(0, yPosColumn1, this->width(), buttonHeight);
        yPosColumn1 += mName->height();
        yPosColumn2 += mName->height();

        // column 1
        QRect selectedLightsRect(0, yPosColumn1, columnWidth, buttonHeight * 7);
        mLights->resize(selectedLightsRect, mRowHeight);
        yPosColumn1 += mLights->height();

        // column 2
        if (mDescription->isVisible()) {
            mDescriptionScrollArea->setGeometry(xSecondColumnStart,
                                                yPosColumn2,
                                                columnWidth,
                                                3 * buttonHeight);
            yPosColumn2 += mDescription->height();
            mDescription->setGeometry(0, 0, int(columnWidth * 0.9), 4 * buttonHeight);
        }
    }

    /// stores the group that is being displayed
    cor::Group mGroup;

    /// name of the group
    QLabel* mName;

    /// description text box
    QLabel* mDescription;

    /// scroll area for the description
    QScrollArea* mDescriptionScrollArea;

    /// displays the lights that are part of this group and their current states.
    LightsListMenu* mLights;

    /// the height of a row in a scroll area
    int mRowHeight;
};

#endif // DISPLAYGROUPWIDGET_H
