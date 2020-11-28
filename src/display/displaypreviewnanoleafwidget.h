#ifndef DISPLAYPREVIEWNANOLEAFWIDGET_H
#define DISPLAYPREVIEWNANOLEAFWIDGET_H

#include <QLabel>
#include <QPainter>
#include <QStyleOption>
#include <QWidget>
#include "comm/nanoleaf/leafmetadata.h"
#include "cor/widgets/listitemwidget.h"
#include "utils/qt.h"

/*!
 * \copyright
 * Copyright (C) 2015 - 2020.
 * Released under the GNU General Public License.
 *
 *
 * \brief The DisplayPreviewNanoleafWidget class displays a Nanoleaf preview on the dsicovery page.
 */
class DisplayPreviewNanoleafWidget : public cor::ListItemWidget {
    Q_OBJECT
public:
    explicit DisplayPreviewNanoleafWidget(const nano::LeafMetadata& leafMetadata, QWidget* parent)
        : cor::ListItemWidget(leafMetadata.serialNumber(), parent),
          mName{new QLabel(leafMetadata.name(), this)},
          mLeafMetadata{leafMetadata} {
        updateNanoleaf(leafMetadata);
    }

    /// getter for the nanoleaf
    const nano::LeafMetadata& nanoleaf() { return mLeafMetadata; }

    /// update the nanoleaf
    void updateNanoleaf(const nano::LeafMetadata& metadata) {
        mLeafMetadata = metadata;
        if (!mLeafMetadata.name().isEmpty()) {
            mName->setText(mLeafMetadata.name());
        } else {
            // handle edge case where name is not filled.
            if (!mLeafMetadata.hardwareName().isEmpty()) {
                mName->setText(mLeafMetadata.hardwareName());
            } else {
                mName->setText(mLeafMetadata.IP());
            }
        }
    }

    /// resize the widget programmatically.
    void resize() { mName->setGeometry(0, 0, this->width(), this->height()); }
signals:

    /// emits when a nanoleaf is clicked
    void clicked(QString);

protected:
    /*!
     * \brief resizeEvent called every time the main window is resized.
     */
    void resizeEvent(QResizeEvent*) { resize(); }

    /*!
     * \brief paintEvent used to draw the background of the widget.
     */
    void paintEvent(QPaintEvent*) {
        QStyleOption opt;
        opt.init(this);
        QPainter painter(this);

        painter.setRenderHint(QPainter::Antialiasing);
        painter.fillRect(rect(), QBrush(QColor(32, 31, 31, 255)));

        // draw line at bottom of widget
        QRect area(x(), y(), width(), height());
        QPainter linePainter(this);
        linePainter.setRenderHint(QPainter::Antialiasing);
        linePainter.setBrush(QBrush(QColor(255, 255, 255)));
        QLine spacerLine(QPoint(area.x(), area.height() - 3),
                         QPoint(area.width(), area.height() - 3));
        linePainter.drawLine(spacerLine);
    }

    /*!
     * \brief mouseReleaseEvent picks up when a click (or a tap on mobile) is released.
     */
    virtual void mouseReleaseEvent(QMouseEvent*) { emit clicked(mLeafMetadata.name()); }

private:
    /// name of the nanoleaf.
    QLabel* mName;

    /// metadata widget for the nanoleaf
    nano::LeafMetadata mLeafMetadata;
};

#endif // DISPLAYPREVIEWNANOLEAFWIDGET_H
