#ifndef COR_STATUS_ICON_H
#define COR_STATUS_ICON_H

#include <QGraphicsEffect>
#include <QLabel>
#include <QObject>
#include <QWidget>

namespace cor {

/*!
 * \copyright
 * Copyright (C) 2015 - 2019.
 * Released under the GNU General Public License.
 *
 * \brief The StatusIcon class is a very simple QWidget that displays
 *        an indicator of the current status of a device. If it is showing red,
 *        the device cannot be reached. If it showing black or white, it is off or on respectively.
 *        It also shows the current brightness as a shade of grey.
 */
class StatusIcon : public QWidget {
    Q_OBJECT
public:
    /// constructor
    explicit StatusIcon(QWidget* parent) : QWidget(parent) {
        mIcon = new QLabel(this);
        mIcon->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);

        mBlackIcon = new QLabel(this);
        mIcon->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);

        this->setStyleSheet("background-color:rgba(0,0,0,0);");
    }

    /*!
     * \brief update update the icon's state.
     * \param isReachable true if reachable, false otherwise
     * \param isOn true if on, false otherwise.
     * \param brightness brightness between 0 and 100.
     */
    void update(bool isReachable, bool isOn, double brightness) {
        mIcon->setFixedSize(this->width(), this->height());
        QPixmap buttonIcon;
        if (!isReachable) {
            buttonIcon = QPixmap(":/images/redButton.png");
        } else if (!isOn) {
            buttonIcon = QPixmap(":/images/blackButton.png");
        } else {
            buttonIcon = QPixmap(":/images/whiteButton.png");

            QPixmap blackPixmap(":/images/blackButton.png");
            blackPixmap = blackPixmap.scaled(int(mIcon->height() * 0.5f),
                                             int(mIcon->height() * 0.5f),
                                             Qt::KeepAspectRatio,
                                             Qt::SmoothTransformation);
            mBlackIcon->setPixmap(blackPixmap);

            mBlackIcon->setGeometry(mIcon->geometry());

            // make black icon inverse of brightness
            double blackBrightness = 1.0 - (brightness / 100.0);
            auto effect = new QGraphicsOpacityEffect(mBlackIcon);
            effect->setOpacity(blackBrightness);
            mBlackIcon->setGraphicsEffect(effect);
        }
        buttonIcon = buttonIcon.scaled(int(mIcon->height() * 0.5f),
                                       int(mIcon->height() * 0.5f),
                                       Qt::KeepAspectRatio,
                                       Qt::SmoothTransformation);
        mIcon->setPixmap(buttonIcon);
    }

private:
    /// black icon used for overlaying
    QLabel* mBlackIcon;

    /// main icon
    QLabel* mIcon;
};

} // namespace cor

#endif // COR_STATUS_ICON_H
