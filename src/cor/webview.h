#ifndef COR_WEB_VIEW_H
#define COR_WEB_VIEW_H

#include <QWidget>
#include <QLabel>
#include <QPushButton>
#include <QTextBrowser>
#include <QLayout>

#include "cor/topwidget.h"

namespace cor
{

/*!
 * \copyright
 * Copyright (C) 2015 - 2018.
 * Released under the GNU General Public License.
 */

/*!
 * \brief The CorlumaWebView class is a widget that displays local .html in a scollable area
 *        The widget also contains a title and a button to close the widget.
 */
class WebView : public QWidget
{
    Q_OBJECT
public:
    /// constructor
    explicit WebView(QString title, QString htmlPath, QWidget *parent = 0);

signals:
    /// emits whenever wthe close button is pressed
    void closePressed();

private slots:
    /// handles when the close button is pressed internally
    void closeButtonPressed(bool);

protected:
    /// called whenever the widget is repainted
    void paintEvent(QPaintEvent *);

private:

    /// top widget that displays the title and the close button
    cor::TopWidget *mTopWidget;

    /// text browser for displaying the html text
    QTextBrowser *mTextBrowser;

    /// layout for widget
    QGridLayout *mGridLayout;
};

}
#endif // COR_WEB_VIEW_H
