#ifndef NOWIFIWIDGET_H
#define NOWIFIWIDGET_H

#include <QWidget>
#include <QLabel>
#include <QLayout>

/*!
 * \copyright
 * Copyright (C) 2015 - 2019.
 * Released under the GNU General Public License.
 *
 * \brief The NoWifiWidget class is a simple widget that pop ups when there is no wifi
 *        detected on the device currently in use.
 */
class NoWifiWidget : public QWidget
{
    Q_OBJECT
public:
    /// constructor
    explicit NoWifiWidget(QWidget *parent);

private slots:
    /// called whenever the screen is painted
    void paintEvent(QPaintEvent *);

    /// callend whenever the window resizes
    void resizeEvent(QResizeEvent *);

private:

    /// label for displaying a text message
    QLabel *mText;

    /// label for displaying an image
    QLabel *mImage;

    /// page's layout
    QVBoxLayout *mLayout;

    /// pixmap for image's label
    QPixmap mPixmap;
};

#endif // NOWIFIWIDGET_H
