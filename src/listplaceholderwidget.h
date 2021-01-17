#ifndef LISTPLACEHOLDERWIDGET_H
#define LISTPLACEHOLDERWIDGET_H


#include <QLabel>
#include <QPainter>
#include <QWidget>
/*!
 * \copyright
 * Copyright (C) 2015 - 2021.
 * Released under the GNU General Public License.
 *
 *
 * \brief The ListPlaceholderWidget class exists to add helper text when a list is empty. IE, if its
 * a list of moods but no moods exist, the helper text will tell the user how to make a mood.
 */
class ListPlaceholderWidget : public QWidget {
    Q_OBJECT
public:
    explicit ListPlaceholderWidget(QWidget* parent) : QWidget(parent), mText{new QLabel(this)} {
        mText->setWordWrap(true);
        setAutoFillBackground(true);
        setStyleSheet("background-color:rgb(33,32,32); font-size:20pt;");
    }

    ListPlaceholderWidget(QWidget* parent, const QString& text) : ListPlaceholderWidget(parent) {
        setText(text);
    }

    /// set the text to display
    void setText(const QString& text) { mText->setText(text); }

protected:
    /// resizes the widgets
    void resizeEvent(QResizeEvent*) { resize(); }

    /// handles when the widget is painted
    void paintEvent(QPaintEvent*) {
        QPainter painter(this);
        painter.setRenderHint(QPainter::Antialiasing);
        painter.fillRect(rect(), QBrush(QColor(33, 32, 32)));
    }

private:
    /// resizes the widgets
    void resize() {
        auto offsetWidth = width() / 6;
        mText->setGeometry(offsetWidth, 0, width() - offsetWidth * 2, height());
    }

    /// text to display
    QLabel* mText;
};

#endif // LISTPLACEHOLDERWIDGET_H
