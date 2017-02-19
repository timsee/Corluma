#ifndef GREYOUTOVERLAY_H
#define GREYOUTOVERLAY_H

#include <QWidget>

/*!
 * \copyright
 * Copyright (C) 2015 - 2017.
 * Released under the GNU General Public License.
 */

/*!
 * \brief The GreyOutOverlay class is a QWidget that can be applied over any UI and it provides a black
 *        layer with transparency that gives the effect of greying out everything below it. It is useful
 *        for making a user to focus on a popup window that shows up over the rest fo the UI.
 */
class GreyOutOverlay : public QWidget
{
    Q_OBJECT
public:
    /// constructor
    explicit GreyOutOverlay(QWidget *parent = 0);

    /*!
     * \brief resize resize the grey out overlay. should be called on the resizeEvent of whatever the
     *        parent of this overlay is.
     */
    void resize();

protected:

    /// paints the greyout overlay
    void paintEvent(QPaintEvent *);

};

#endif // GREYOUTOVERLAY_H
