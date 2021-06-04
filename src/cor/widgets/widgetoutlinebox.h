#ifndef WIDGETOUTLINEBOX_H
#define WIDGETOUTLINEBOX_H
#include <QPainterPath>
#include <QStyleOption>
#include <QWidget>

#include "cor/stylesheets.h"
#include "utils/painterutils.h"

namespace cor {

/*!
 * \copyright
 * Copyright (C) 2015 - 2021.
 * Released under the GNU General Public License.
 */

/*!
 * \brief The WidgetOutlineBox class is a simple widget that gets overlaid over widgets that needs a
 * border that is always on top. This allows those widgets to have simpler logic in handling an
 * always on top outline.
 *
 */
class WidgetOutlineBox : public QWidget {
    Q_OBJECT

public:
    explicit WidgetOutlineBox(EPaintRectOptions options, QWidget* parent)
        : QWidget(parent),
          mRectOptions{options} {
        setStyleSheet(cor::kTransparentStylesheet);
        setAttribute(Qt::WA_TransparentForMouseEvents, true);
    }


    /// change which sides are painted on the outline
    void changePaintRectOptions(EPaintRectOptions options) {
        mRectOptions = options;
        update();
    }

protected:
    /// renders the widget
    virtual void paintEvent(QPaintEvent*) {
        QStyleOption opt;
        opt.initFrom(this);
        QPainter painter(this);
        QPen pen(Qt::white, 5);
        painter.setPen(pen);

        paintRect(painter, rect(), mRectOptions);
    }

private:
    /// option to paint the rect sides
    EPaintRectOptions mRectOptions;
};

} // namespace cor
#endif // WIDGETOUTLINEBOX_H
