#ifndef ROUTINEPAGE_H
#define ROUTINEPAGE_H

#include <QWidget>
#include "cor/objects/page.h"
#include "cor/widgets/topwidget.h"
#include "routines/routinecontainer.h"
#include "utils/qt.h"
/*!
 * \copyright
 * Copyright (C) 2015 - 2021.
 * Released under the GNU General Public License.
 */


/*!
 * \brief The RoutinePage class is a page that stores the routinecontainer used to show
 */
class RoutinePage : public QWidget, public cor::Page {
    Q_OBJECT
public:
    explicit RoutinePage(QWidget* parent, ERoutineGroup routineGroup)
        : QWidget(parent),
          mTopWidget{new cor::TopWidget("Routines", ":images/closeX.png", this)},
          mRoutineContainer{new RoutineContainer(this, routineGroup)} {
        connect(mTopWidget, SIGNAL(clicked(bool)), this, SLOT(closeButtonPressed(bool)));
    }

    /// getter for the routine container wrapped by this page.
    RoutineContainer* routines() { return mRoutineContainer; }

    /// displays the page
    void pushIn(const QPoint& startPoint, const QPoint& endPoint) {
        setVisible(true);
        raise();
        isOpen(true);
        moveWidget(this, startPoint, endPoint);
    }

    /// hides the page
    void pushOut(const QPoint& endPoint) {
        moveWidget(this, pos(), endPoint);
        isOpen(false);
    }

signals:
    /*!
     * \brief pressedClose emited when the close button is pressed.
     */
    void closePressed();

protected:
    /// called whenever the widget is repainted
    void paintEvent(QPaintEvent*) {
        QStyleOption opt;
        opt.initFrom(this);
        QPainter painter(this);

        painter.setRenderHint(QPainter::Antialiasing);
        painter.fillRect(rect(), QBrush(cor::kLighterBackgroundColor));
    }

    /// handles when the widget resizes
    void resizeEvent(QResizeEvent*) {
        auto rowHeight = height() / 10;
        auto yPos = 0;
        mTopWidget->setGeometry(0, yPos, width(), rowHeight);
        yPos += mTopWidget->height();
        mRoutineContainer->setGeometry(0, yPos, width(), rowHeight * 9);
    }


private slots:
    /// handles when the close button is pressed internally
    void closeButtonPressed(bool) { emit closePressed(); }

private:
    /// top widget that displays the title and the close button
    cor::TopWidget* mTopWidget;

    /// container for the routines, displays either single, multi, or a set of both.
    RoutineContainer* mRoutineContainer;
};

#endif // ROUTINEPAGE_H
