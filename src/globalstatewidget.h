#ifndef GLOBALSTATEWIDGET_H
#define GLOBALSTATEWIDGET_H

#include <QPainter>
#include <QWidget>
#include "cor/objects/lightstate.h"
#include "cor/stylesheets.h"
#include "cor/widgets/palettewidget.h"
#include "listplaceholderwidget.h"

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
        mPaletteWidget->showInSingleLine(true);
        mPaletteWidget->shouldPreferPalettesOverRoutines(true);
        mPaletteWidget->setBrightnessMode(cor::EBrightnessMode::none);
        setStyleSheet(cor::kTransparentStylesheet);
    }

    /// updates the palette to new light states.
    void update(const std::vector<cor::LightState>& states) {
        if (states.empty()) {
            mPaletteWidget->setVisible(false);
        } else {
            mPaletteWidget->setVisible(true);
        }
        mPaletteWidget->show(states);
    }

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

private:
    /// displays the light states in the widget.
    cor::PaletteWidget* mPaletteWidget;
};

#endif // GLOBALSTATEWIDGET_H
