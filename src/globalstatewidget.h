#ifndef GLOBALSTATEWIDGET_H
#define GLOBALSTATEWIDGET_H

#include <QPainter>
#include <QWidget>
#include "cor/objects/lightstate.h"
#include "cor/stylesheets.h"
#include "cor/widgets/palettewidget.h"
/*!
 * \copyright
 * Copyright (C) 2015 - 2021.
 * Released under the GNU General Public License.
 */

/*!
 * \brief The GlobalStateWidget class is a widget that displays in the top right of the app above
 * most other widgets that shows the state of lights currently selected globally in the app.
 */
class GlobalStateWidget : public QWidget {
    Q_OBJECT
public:
    explicit GlobalStateWidget(QWidget* parent)
        : QWidget(parent),
          mPaletteWidget{new cor::PaletteWidget(this)} {
        mPaletteWidget->shouldForceSquares(true);
    }

    /// updates the palette to new light states.
    void update(const std::vector<cor::LightState>& states) { mPaletteWidget->show(states); }

signals:

    /// emits when the widget is clicked.
    void clicked();

protected:
    /*!
     * \brief resizeEvent called every time the main window is resized.
     */
    virtual void resizeEvent(QResizeEvent*) {
        mPaletteWidget->setGeometry(QRect(0, 0, width(), height()));
    }

    /// handles when a mouse is released.
    virtual void mouseReleaseEvent(QMouseEvent*) { emit clicked(); }

    /// paints the background of the widget.
    virtual void paintEvent(QPaintEvent*) {
        QPainter painter(this);
        painter.setRenderHint(QPainter::Antialiasing);
        painter.fillRect(rect(), QBrush(QColor(33, 32, 32)));
    }

private:
    /// displays the light states in the widget.
    cor::PaletteWidget* mPaletteWidget;
};

#endif // GLOBALSTATEWIDGET_H
