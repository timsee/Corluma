#ifndef DISCOVERYTOPMENU_H
#define DISCOVERYTOPMENU_H



#include <QPainter>
#include <QStyleOption>
#include <QWidget>
#include "appsettings.h"
#include "connectionbutton.h"
#include "cor/protocols.h"
#include "cor/widgets/slider.h"
#include "lightspage.h"

/*!
 * \copyright
 * Copyright (C) 2015 - 2021.
 * Released under the GNU General Public License.
 *
 *
 * \brief The DiscoveryTopMenu class contains buttons for the top menu of the DiscoveryPage.
 */
class DiscoveryTopMenu : public QWidget {
    Q_OBJECT
public:
    explicit DiscoveryTopMenu(QWidget* parent, AppSettings* appSettings, LightsPage* lightsPage)
        : QWidget(parent),
          mAppSettings{appSettings},
          mLightsPage{lightsPage},
          mArduCorButton{new ConnectionButton("ArduCor", this)},
          mHueButton{new ConnectionButton("Hue", this)},
          mNanoleafButton{new ConnectionButton("Nanoleaf", this)},
          mButtons{mArduCorButton, mHueButton, mNanoleafButton} {
        connect(mArduCorButton, SIGNAL(clicked(bool)), this, SLOT(arduCorButtonPressed(bool)));
        connect(mHueButton, SIGNAL(clicked(bool)), this, SLOT(hueButtonPressed(bool)));
        connect(mNanoleafButton, SIGNAL(clicked(bool)), this, SLOT(nanoleafButtonPressed(bool)));

        const QString stylesheet = "QPushButton { background-color:rgb(33,32,32); } "
                                   "QPushButton:checked { background-color:#4A4949; }";
        mArduCorButton->setStyleSheet(stylesheet);
        mNanoleafButton->setStyleSheet(stylesheet);
        mHueButton->setStyleSheet(stylesheet);

        mOriginalSize = cor::applicationSize();
        resize();
    }


    /// update the state of a ConnectionButton
    void updateDiscoveryButton(EProtocolType type, EConnectionState connectionState) {
        auto label = protocolToName(type);
        for (auto button : mButtons) {
            if (button->text() == label) {
                button->changeState(connectionState);
            }
        }
    }

    /// number of connection buttons in use
    std::size_t buttonCount() {
        auto i = 0;
        for (auto button : mButtons) {
            if (button->isVisible()) {
                ++i;
            }
        }
        return i;
    }

    /// updates the menu
    void updateMenu() {
        mArduCorButton->setVisible(mAppSettings->enabled(EProtocolType::arduCor));
        mNanoleafButton->setVisible(mAppSettings->enabled(EProtocolType::nanoleaf));
        mHueButton->setVisible(mAppSettings->enabled(EProtocolType::hue));
        resize();

        // check that commtype being shown is available, if not, adjust
        if (!mAppSettings->enabled(mLightsPage->currentProtocol())) {
            if (mAppSettings->enabled(EProtocolType::hue)) {
                mLightsPage->switchProtocol(EProtocolType::hue);
            } else if (mAppSettings->enabled(EProtocolType::nanoleaf)) {
                mLightsPage->switchProtocol(EProtocolType::nanoleaf);
            } else if (mAppSettings->enabled(EProtocolType::arduCor)) {
                mLightsPage->switchProtocol(EProtocolType::arduCor);
            }
        }

        auto currentProtocol = mLightsPage->currentProtocol();
        highlightButton(currentProtocol);
        mLightsPage->switchProtocol(currentProtocol);
    }

signals:

    /// signal when the slider changes.
    void buttonPressed(QString);

protected:
    /// called whenever the menu is painted
    void paintEvent(QPaintEvent*) override {
        QStyleOption opt;
        opt.initFrom(this);
        QPainter painter(this);

        painter.setRenderHint(QPainter::Antialiasing);
        painter.fillRect(rect(), QBrush(QColor(33, 32, 32)));
    }

private slots:

    /// handle when arducor button is pressed
    void arduCorButtonPressed(bool) {
        handleButonPressed(EProtocolType::arduCor, "Discovery_ArduCor");
    }

    /// handle when hue button is pressed
    void hueButtonPressed(bool) { handleButonPressed(EProtocolType::hue, "Discovery_Hue"); }

    /// handl when nanoleaf button is pressed
    void nanoleafButtonPressed(bool) {
        handleButonPressed(EProtocolType::nanoleaf, "Discovery_NanoLeaf");
    }

private:
    /// handle when any button is pressed
    void handleButonPressed(EProtocolType type, const QString& buttonName) {
        emit buttonPressed(buttonName);
        highlightButton(type);
    }


    /// highlight a button
    void highlightButton(EProtocolType type) {
        auto label = protocolToName(type);
        for (auto button : mButtons) {
            if (button->text() == label) {
                button->setChecked(true);
            } else {
                button->setChecked(false);
            }
        }
    }

    /// convert a protocol to a name
    QString protocolToName(EProtocolType type) {
        QString label;
        switch (type) {
            case EProtocolType::hue:
                label = "Hue";
                break;
            case EProtocolType::arduCor:
                label = "ArduCor";
                break;
            case EProtocolType::nanoleaf:
                label = "Nanoleaf";
                break;
            default:
                break;
        }
        return label;
    }

    /// resize widget programmatically
    void resize() {
        auto size = mOriginalSize;

        auto ratio = float(size.height()) / size.width();
        auto width = size.width() * 0.15f;
        auto height = size.height() * 0.07f;
        if (ratio > 1.3f) {
            width = size.width() * 0.2f;
            height = size.height() * 0.1f;
        }
        size = QSize(int(width), int(height));


        auto xPos = 0;
        auto buttonCount = 0;
        if (mAppSettings->enabled(EProtocolType::arduCor)) {
            mArduCorButton->setGeometry(xPos, 0, size.width(), size.height());
            xPos += mArduCorButton->width();
            ++buttonCount;
        }

        if (mAppSettings->enabled(EProtocolType::hue)) {
            mHueButton->setGeometry(xPos, 0, size.width(), size.height());
            xPos += mHueButton->width();
            ++buttonCount;
        }

        if (mAppSettings->enabled(EProtocolType::nanoleaf)) {
            mNanoleafButton->setGeometry(xPos, 0, size.width(), size.height());
            xPos += mNanoleafButton->width();
            ++buttonCount;
        }

        int fixedWidth = size.width() * buttonCount;
        int fixedHeight = size.height();
        setFixedSize(QSize(fixedWidth, fixedHeight));
    }

    /// original size of application.
    QSize mOriginalSize;

    /// pointer to app settings
    AppSettings* mAppSettings;

    /// pointer to lights page
    LightsPage* mLightsPage;

    /// button for arducor
    ConnectionButton* mArduCorButton;

    /// button for hues
    ConnectionButton* mHueButton;

    /// button for nanoleafs
    ConnectionButton* mNanoleafButton;

    /// vector of all buttons
    std::vector<ConnectionButton*> mButtons;
};


#endif // DISCOVERYTOPMENU_H
