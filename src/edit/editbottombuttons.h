#ifndef EDITBOTTOMPAGE_H
#define EDITBOTTOMPAGE_H

#include <QGraphicsOpacityEffect>
#include <QPushButton>
#include <QWidget>


/*!
 * \copyright
 * Copyright (C) 2015 - 2020.
 * Released under the GNU General Public License.
 *
 *
 * \brief The EditBottomButtons class provides forward and backwards buttons for editing pages.
 */
class EditBottomButtons : public QWidget {
    Q_OBJECT
public:
    explicit EditBottomButtons(QWidget* parent)
        : QWidget(parent),
          mLeftButton{new QPushButton(this)},
          mRightButton{new QPushButton(this)} {
        connect(mLeftButton, SIGNAL(clicked(bool)), this, SLOT(handleLeftButton(bool)));
        connect(mRightButton, SIGNAL(clicked(bool)), this, SLOT(handleRightButton(bool)));
    }

    /// true to hide the back button, false to show it
    void hideBack(bool backHidden) { mLeftButton->setHidden(backHidden); }

    /// true to hide the forward button, false to show it.
    void hideForward(bool forwardHidden) { mRightButton->setHidden(forwardHidden); }

    /// true to enable the back button, false to disable it.
    void enableBack(bool enable) { enablePushButton(mLeftButton, enable); }

    /// true to enable the forward button, false to disable it.
    void enableForward(bool enable) { enablePushButton(mRightButton, enable); }

signals:

    /// signals when the left button is pressed
    void leftButtonPressed();

    /// signals when the right button is pressed
    void rightButtonPressed();

protected:
    /*!
     * \brief resizeEvent called whenever the widget resizes so that assets can be updated.
     */
    void resizeEvent(QResizeEvent*) {
        if (this->height() != mLeftButton->height()) {
            resizeButtonIcons();
        }

        mLeftButton->setGeometry(0, 0, this->height(), this->height());
        mRightButton->setGeometry(this->width() - this->height(),
                                  0,
                                  this->height(),
                                  this->height());
    }

private slots:

    /// handles when the left button is pressed
    void handleLeftButton(bool) { emit leftButtonPressed(); }

    /// handles when the right button is pressed
    void handleRightButton(bool) { emit rightButtonPressed(); }

private:
    /// resizes the buttons icons
    void resizeButtonIcons() {
        QPixmap pixmap(":images/arrowLeft.png");
        int iconSide = int(this->height() * 0.8);
        mLeftButton->setIconSize(QSize(iconSide, iconSide));
        mLeftButton->setIcon(QIcon(
            pixmap.scaled(iconSide, iconSide, Qt::KeepAspectRatio, Qt::SmoothTransformation)));


        QPixmap pixmap2(":images/arrowRight.png");
        mRightButton->setIconSize(QSize(iconSide, iconSide));
        mRightButton->setIcon(QIcon(
            pixmap2.scaled(iconSide, iconSide, Qt::KeepAspectRatio, Qt::SmoothTransformation)));
    }

    /// enables a push button and sets its opacity
    void enablePushButton(QPushButton* button, bool enable) {
        button->setEnabled(enable);
        auto effect = new QGraphicsOpacityEffect(button);
        if (enable) {
            effect->setOpacity(1.0);
        } else {
            effect->setOpacity(0.33);
        }
        button->setGraphicsEffect(effect);
    }

    /// left button
    QPushButton* mLeftButton;

    /// right button
    QPushButton* mRightButton;
};

#endif // EDITBOTTOMPAGE_H
