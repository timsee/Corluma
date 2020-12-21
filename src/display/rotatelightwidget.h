#ifndef ROTATELIGHTWIDGET_H
#define ROTATELIGHTWIDGET_H

#include <QWidget>
#include "comm/nanoleaf/leafmetadata.h"
#include "comm/nanoleaf/leafpanelimage.h"
#include "cor/objects/page.h"
#include "utils/qt.h"

/*!
 * \copyright
 * Copyright (C) 2015 - 2020.
 * Released under the GNU General Public License.
 *
 *
 * \brief The RotateLightWidget class is a widget for rotating lights. This is used to effectively
 * display lights in the orientation they are mounted in physical reality, since lights like
 * nanoleafs can show their layout information, but don't have a concept of how they are rotated.
 */
class RotateLightWidget : public QWidget, public cor::Page {
    Q_OBJECT
public:
    explicit RotateLightWidget(QWidget* parent);

    /// set the nanoleaf and rotation
    void setNanoleaf(const nano::LeafMetadata& leaf, int rotation);

    /// getter for the rotation
    int rotation() { return mValue; }

    /// pushes widget in
    void pushIn();

    /// pushes widget out
    void pushOut();

    /// resizes widget programmatically
    void resize();

signals:

    /// emits when the value changes
    void valueChanged(int);

    /// emitted when "X" is clicked
    void cancelClicked();

private slots:

    /// handles when OK is clicked
    void clickedOK() { emit valueChanged(mValue); }

    /// handles when cancel is clicked
    void clickedCancel() { emit cancelClicked(); }

protected:
    /// pick up when mouse is pressed
    void mousePressEvent(QMouseEvent*);

    /// pick up when mouse is moved
    void mouseMoveEvent(QMouseEvent*);

    /// used to render the kitchen timer
    void paintEvent(QPaintEvent*);

    /// called when widget is resized
    void resizeEvent(QResizeEvent*);

private:
    /*!
     * \brief handleMouseEvent handles all the mouse events used in the rotatelight widget.
     *        The mousePressEvent and mouseReleaseEvent both map to this function.
     * \param event the mouse event that is getting processed.
     */
    void handleMouseEvent(QMouseEvent* event);

    /// draw the nanoleaf
    void drawNanoleaf();

    /// button that says "OK" and accepts the rotation
    QPushButton* mButtonOK;

    /// button with an "X" that closes the window
    QPushButton* mButtonCancel;

    /// instructions for the rotation
    QLabel* mInstructionLabel;

    /// pixamp to draw the rotated light
    QPixmap mPixmap;

    /// class that generates the nanoleaf image
    nano::LeafPanelImage* mLeafPanelImage;

    /// label to display the light image
    QLabel* mLightImage;

    /// nanoleaf being drawn
    nano::LeafMetadata mLeaf;

    /// rotation value
    int mValue;
};

#endif // ROTATELIGHTWIDGET_H