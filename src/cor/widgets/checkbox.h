#ifndef COR_CHECK_BOX_H
#define COR_CHECK_BOX_H

#include <QLabel>
#include <QLayout>
#include <QPushButton>
#include <QWidget>
#include "utils/exception.h"
namespace cor {

/// checkbox state
enum class ECheckboxState { selectAll, clearAll, disabled };

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
        const QString transparentStyleSheet = "background-color: rgba(0,0,0,0);";
        mButton->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
        mButton->setStyleSheet(transparentStyleSheet);
        mButton->setAttribute(Qt::WA_TransparentForMouseEvents);
    }

    /// getter for checkbox state
    ECheckboxState checkboxState() { return mState; }

    /// setter for checkbox state
    void checkboxState(ECheckboxState state) {
        mState = state;
        mButton->setPixmap(currentPixmap());
    }

    /// programmatically resize
    void resize() {
        mButton->setGeometry(0, 0, this->width(), this->height());
        const auto& size = iconSize();
        mClearAllPixmap = QPixmap(":/images/selectAllIcon.png");
        mClearAllPixmap = mClearAllPixmap.scaled(size.width(),
                                                 size.height(),
                                                 Qt::IgnoreAspectRatio,
                                                 Qt::SmoothTransformation);

        mSelectAllPixmap = QPixmap(":/images/uncheckedBox.png");
        mSelectAllPixmap = mSelectAllPixmap.scaled(size.width(),
                                                   size.height(),
                                                   Qt::IgnoreAspectRatio,
                                                   Qt::SmoothTransformation);

        mDisabledPixmap = QPixmap(":/images/disabledX.png");
        mDisabledPixmap = mDisabledPixmap.scaled(size.width(),
                                                 size.height(),
                                                 Qt::IgnoreAspectRatio,
                                                 Qt::SmoothTransformation);

        mButton->setPixmap(currentPixmap());
    }

signals:

    /// emit when the checkbox is clicked.
    void checkBoxClicked(bool);

private slots:

    /// picks up when the select all button is pressed
    void buttonPressed(bool clicked) { emit checkBoxClicked(clicked); }

private:
    /// getter for current pixmap
    const QPixmap& currentPixmap() {
        switch (mState) {
            case ECheckboxState::disabled:
                return mDisabledPixmap;
            case ECheckboxState::clearAll:
                return mClearAllPixmap;
            case ECheckboxState::selectAll:
                return mSelectAllPixmap;
        }
        THROW_EXCEPTION("Do not recognize pixmap");
    }

    /// getter for icon size
    QSize iconSize() { return {int(height() * 0.75), int(height() * 0.75)}; }

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
