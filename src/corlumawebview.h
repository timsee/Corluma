#ifndef CORLUMAWEBVIEW_H
#define CORLUMAWEBVIEW_H

#include <QWidget>
#include <QLabel>
#include <QPushButton>
#include <QTextBrowser>
#include <QLayout>

#include "corlumatopwidget.h"

/*!
 * \copyright
 * Copyright (C) 2015 - 2017.
 * Released under the GNU General Public License.
 */

/*!
 * \brief The CorlumaWebView class is a widget that displays local .html in a scollable area
 *        The widget also contains a title and a button to close the widget.
 */
class CorlumaWebView : public QWidget
{
    Q_OBJECT
public:
    /// constructor
    explicit CorlumaWebView(QString title, QString htmlPath, QWidget *parent = 0);

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
    CorlumaTopWidget *mTopWidget;

    /// text browser for displaying the html text
    QTextBrowser *mTextBrowser;

    /// layout for widget
    QGridLayout *mGridLayout;
};

#endif // CORLUMAWEBVIEW_H
