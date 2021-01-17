#ifndef CONNECTIONBUTTON_H
#define CONNECTIONBUTTON_H

/*!
 * \copyright
 * Copyright (C) 2015 - 2020.
 * Released under the GNU General Public License.
 */

#include <QPushButton>
#include "cor/protocols.h"

/*!
 * \brief The ConnectionButton class is a simple QPushButton used for displaying connections on the
 * DiscoveryWidget.
 */
class ConnectionButton : public QPushButton {
    Q_OBJECT
public:
    explicit ConnectionButton(QWidget* parent)
        : QPushButton(parent),
          mState{EConnectionState::MAX} {
        setCheckable(true);
        setStyleSheet("text-align:left;");
    }

    /// change the state of the button, changign what icon displays
    void changeState(EConnectionState state) {
        if (mState != state) {
            mState = state;
            switch (state) {
                case EConnectionState::off:
                case EConnectionState::discovered:
                    setIcon(QIcon());
                    break;
                case EConnectionState::discovering:
                    setIcon(QIcon(mNoConnectionIcon));
                    break;
                case EConnectionState::MAX:
                case EConnectionState::connectionError:
                    setIcon(QIcon(mWarningIcon));
                    break;
            }
        }
    }

protected:
    /// resizes icons when the widget resizes.
    void resizeEvent(QResizeEvent*) {
        mWarningIcon = QPixmap(":/images/warning_icon.png");
        mNoConnectionIcon = QPixmap(":/images/no_wifi.png");

        setIconSize(QSize(height() * 0.25, height() * 0.25));

        mWarningIcon = mWarningIcon.scaled(iconSize().width(),
                                           iconSize().height(),
                                           Qt::KeepAspectRatio,
                                           Qt::SmoothTransformation);

        mNoConnectionIcon = mNoConnectionIcon.scaled(iconSize().width(),
                                                     iconSize().height(),
                                                     Qt::KeepAspectRatio,
                                                     Qt::SmoothTransformation);
    }

private:
    /// current state
    EConnectionState mState;

    /// icon for warning state
    QPixmap mWarningIcon;

    /// icon for no connections found.
    QPixmap mNoConnectionIcon;
};

#endif // CONNECTIONBUTTON_H
