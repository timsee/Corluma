#ifndef COR_WEB_VIEW_H
#define COR_WEB_VIEW_H

#include <QFile>
#include <QLayout>
#include <QPushButton>
#include <QScroller>
#include <QStyleOption>
#include <QTextBrowser>
#include <QTextStream>
#include <QWidget>
#include <QtCore>
#include <QtGui>

#include "cor/widgets/topwidget.h"
#include "utils/qt.h"

namespace cor {

/*!
 * \copyright
 * Copyright (C) 2015 - 2020.
 * Released under the GNU General Public License.
 */

/*!
 * \brief The CorlumaWebView class is a widget that displays local .html in a scollable area
 *        The widget also contains a title and a button to close the widget.
 */
class WebView : public QWidget {
    Q_OBJECT
public:
    /// constructor
    explicit WebView(const QString& title, const QString& htmlPath, QWidget* parent)
        : QWidget(parent) {
        mTextBrowser = new QTextBrowser(this);
        QScroller::grabGesture(mTextBrowser->viewport(), QScroller::LeftMouseButtonGesture);
        mTextBrowser->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
        mTextBrowser->setTextInteractionFlags(Qt::LinksAccessibleByMouse);

        mTopWidget = new cor::TopWidget(title, ":images/closeX.png", this);
        connect(mTopWidget, SIGNAL(clicked(bool)), this, SLOT(closeButtonPressed(bool)));
#ifdef USE_EXPERIMENTAL_FEATURES
        connect(mTopWidget, SIGNAL(debugClicked(bool)), this, SLOT(debugClicked(bool)));
#endif

        if (!htmlPath.isEmpty()) {
            QFile file(htmlPath);
            if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
                qDebug() << "resource not found!";
            }
            QTextStream in(&file);
            mTextBrowser->setHtml(in.readAll());
        }
        mTextBrowser->setOpenExternalLinks(true);

        mGridLayout = new QGridLayout(this);
        mGridLayout->addWidget(mTopWidget, 0, 0, 1, 2);
        mGridLayout->addWidget(mTextBrowser, 1, 0, 1, 2);

        // stretch out the column and rows that need it...
        mGridLayout->setColumnStretch(0, 1);
        mGridLayout->setColumnStretch(1, 5);

        mGridLayout->setRowStretch(0, 1);
        mGridLayout->setRowStretch(1, 10);

        setLayout(mGridLayout);
    }

    /// update the html programmatically
    void updateHTML(const QString& input) { mTextBrowser->setHtml(input); }

#ifdef USE_EXPERIMENTAL_FEATURES
    /// show the debug button
    void useDebugButton(bool useDebugButton) { mTopWidget->showDebugButton(useDebugButton); }
#endif

signals:
    /// emits whenever the close button is pressed
    void closePressed();

#ifdef USE_EXPERIMENTAL_FEATURES
    /// emits when the debug button is pressed.
    void debugPressed();
#endif

private slots:
    /// handles when the close button is pressed internally
    void closeButtonPressed(bool) { emit closePressed(); }

#ifdef USE_EXPERIMENTAL_FEATURES
    /// handles when a debug button is clicked
    void debugClicked(bool) { emit debugPressed(); }
#endif

protected:
    /// called whenever the widget is repainted
    void paintEvent(QPaintEvent*) {
        QStyleOption opt;
        opt.initFrom(this);
        QPainter painter(this);

        painter.setRenderHint(QPainter::Antialiasing);
        painter.fillRect(rect(), QBrush(cor::kLighterBackgroundColor));
    }


private:
    /// top widget that displays the title and the close button
    cor::TopWidget* mTopWidget;

    /// text browser for displaying the html text
    QTextBrowser* mTextBrowser;

    /// layout for widget
    QGridLayout* mGridLayout;
};

} // namespace cor
#endif // COR_WEB_VIEW_H
