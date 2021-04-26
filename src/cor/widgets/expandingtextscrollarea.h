#ifndef EXPANDINGTEXTSCROLLAREA_H
#define EXPANDINGTEXTSCROLLAREA_H

#include <QScrollArea>
#include <QScrollBar>
#include <QScroller>
#include <QTextEdit>

#include "cor/stylesheets.h"

namespace cor {

/*!
 * \copyright
 * Copyright (C) 2015 - 2020.
 * Released under the GNU General Public License.
 *
 *
 * @brief The ExpandingTextScrollArea class is a QScrollArea designed to show text in a fixed width.
 * For all overflow text, it autoexpands the scroll widget to fit the text.
 */
class ExpandingTextScrollArea : public QScrollArea {
    Q_OBJECT
public:
    explicit ExpandingTextScrollArea(QWidget* parent)
        : QScrollArea(parent),
          mText{new QTextEdit(this)} {
        setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
        setWidget(mText);
        QScroller::grabGesture(viewport(), QScroller::LeftMouseButtonGesture);
        setStyleSheet(cor::kDarkerGreyBackground);

        horizontalScrollBar()->setVisible(false);

        mText->setReadOnly(true);
        mText->setWordWrapMode(QTextOption::WordWrap);
        mText->setAlignment(Qt::AlignTop | Qt::AlignLeft);
    }

    /// update the text in the widget
    void updateText(const QString& text) {
        mText->setHtml(text);
        resize();
    }

protected:
    /// called when resized
    void resizeEvent(QResizeEvent*) { resize(); }

private:
    /// programmatically resizes the widget
    void resize() {
        auto textWidth = int(this->width());
        // set width in case it adjusts height
        mText->setFixedWidth(textWidth);
        // set height
        mText->setFixedHeight(int(mText->document()->size().height()));
    }

    /// description text box
    QTextEdit* mText;
};

} // namespace cor

#endif // EXPANDINGTEXTSCROLLAREA_H
