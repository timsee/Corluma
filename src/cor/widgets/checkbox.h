#ifndef COR_CHECK_BOX_H
#define COR_CHECK_BOX_H

#include <QLabel>
#include <QLayout>
#include <QPushButton>
#include <QWidget>
#include "cor/stylesheets.h"
#include "utils/exception.h"
#include "utils/qt.h"

/// checkbox state
enum class ECheckboxState { unchecked, checked, disabled };
Q_DECLARE_METATYPE(ECheckboxState)

namespace cor {

/*!
 * \copyright
 * Copyright (C) 2015 - 2020.
 * Released under the GNU General Public License.
 *
 *
 * \brief The CheckBox class is a simple widget designed to give a checkbox
 *        with a label.
 */
class CheckBox : public QWidget {
    Q_OBJECT
public:
    /// constructor
    explicit CheckBox(QWidget* parent)
        : QWidget(parent),
          mButton{new QLabel(this)},
          mState{ECheckboxState::disabled} {
        mButton->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
        mButton->setAlignment(Qt::AlignCenter);
        mButton->setStyleSheet(cor::kTransparentStylesheet);
    }

    /// getter for checkbox state
    ECheckboxState checkboxState() { return mState; }

    /// setter for checkbox state
    void checkboxState(ECheckboxState state) {
        mState = state;
        mButton->setPixmap(currentPixmap());
    }

signals:

    /// emit when the checkbox is clicked.
    void clicked(ECheckboxState);

protected:
    /// handles when the widget is clicked
    void mouseReleaseEvent(QMouseEvent* event) override {
        if (cor::isMouseEventTouchUpInside(event, mButton, false)) {
            emit clicked(mState);
        }
    }

    /// handles when the widget is resized.
    void resizeEvent(QResizeEvent*) override { resize(); }

private:
    /// programmatically resize
    void resize() {
        mButton->setGeometry(0, 0, this->width(), this->height());
        const auto& size = iconSize();
        mClearAllPixmap = QPixmap(":/images/checkbox_checked.png");
        mClearAllPixmap = mClearAllPixmap.scaled(size.width(),
                                                 size.height(),
                                                 Qt::IgnoreAspectRatio,
                                                 Qt::SmoothTransformation);

        mSelectAllPixmap = QPixmap(":/images/checkbox_unchecked.png");
        mSelectAllPixmap = mSelectAllPixmap.scaled(size.width(),
                                                   size.height(),
                                                   Qt::IgnoreAspectRatio,
                                                   Qt::SmoothTransformation);

        mDisabledPixmap = QPixmap(":/images/checkbox_disabled.png");
        mDisabledPixmap = mDisabledPixmap.scaled(size.width(),
                                                 size.height(),
                                                 Qt::IgnoreAspectRatio,
                                                 Qt::SmoothTransformation);

        mButton->setPixmap(currentPixmap());
    }

    /// getter for current pixmap
    const QPixmap& currentPixmap() {
        switch (mState) {
            case ECheckboxState::disabled:
                return mDisabledPixmap;
            case ECheckboxState::checked:
                return mClearAllPixmap;
            case ECheckboxState::unchecked:
                return mSelectAllPixmap;
        }
        THROW_EXCEPTION("Do not recognize pixmap");
    }

    /// getter for icon size
    QSize iconSize() {
        auto side = int(std::min(width() * 0.9, height() * 0.85));
        return {side, side};
    }

    /// button used for checkbox
    QLabel* mButton;

    /// state of the button
    ECheckboxState mState;

    /// pixmap for the select all button
    QPixmap mSelectAllPixmap;

    /// pixmap for the clear all button
    QPixmap mClearAllPixmap;

    /// pixmap for disabled button
    QPixmap mDisabledPixmap;
};

} // namespace cor
#endif // COR_CHECK_BOX_H
