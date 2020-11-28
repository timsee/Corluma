#ifndef DISPLAYARDUCORCONTROLLER_H
#define DISPLAYARDUCORCONTROLLER_H


#include <QPainter>
#include <QStyleOption>
#include <QTextEdit>
#include <QWidget>
#include "comm/arducor/controller.h"
#include "comm/commarducor.h"
#include "comm/commlayer.h"
#include "cor/widgets/expandingtextscrollarea.h"
#include "menu/displaymoodmetadata.h"
#include "menu/groupstatelistmenu.h"
#include "menu/lightslistmenu.h"
/*!
 * \copyright
 * Copyright (C) 2015 - 2020.
 * Released under the GNU General Public License.
 *
 *
 * \brief The DisplayGroupWidget class displays all pertinent info for a cor::Controller. This
 * includes the controller's name, the controller's lights, and the metadata about the controller.
 */
class DisplayArduCorControllerWidget : public QWidget {
    Q_OBJECT
public:
    explicit DisplayArduCorControllerWidget(QWidget* parent, CommLayer* comm)
        : QWidget(parent),
          mComm{comm},
          mName{new QLabel(this)},
          mLightsLabel{new QLabel("<b>Lights:</b>", this)},
          mLights{new LightsListMenu(this, false)},
          mMetadata{new cor::ExpandingTextScrollArea(this)} {
        auto font = mName->font();
        font.setPointSize(20);
        mName->setFont(font);

        this->setStyleSheet("background-color:rgb(33,32,32);");
    }

    /// getter for controller represented by the widget
    const cor::Controller& controller() const noexcept { return mController; }

    /// updates the controller's UI elements.
    void updateController(const cor::Controller& controller) {
        mController = controller;
        mName->setText(cor::controllerToGenericName(mController, cor::EArduCorStatus::connected));

        auto lights = mComm->arducor()->lightsFromNames(mController.names());
        mLights->updateLights();
        mLights->showLights(lights);
        updateMetadata(mController);
        resize();
    }

    /// changes the row height of rows in scroll areas.
    void changeRowHeight(int height) { mRowHeight = height; }

    /// reset the widget to showing no group
    void reset() {
        mController = {};
        mName->setText("");
        mLights->showLights({});
        resize();
    }

    /// programmatically resize
    void resize() {
        int yPosColumn1 = 0;
        int yPosColumn2 = 0;
        int buttonHeight = this->height() / 10;
        int xSecondColumnStart = int(this->width() / 2 * 1.05);
        int columnWidth = int((this->width() / 2) * 0.95);
        int xSpacer = this->width() / 20;

        // top of both
        mName->setGeometry(xSpacer / 2, yPosColumn1, this->width() - xSpacer / 2, buttonHeight);
        yPosColumn1 += mName->height();
        yPosColumn2 += mName->height();

        // column 1
        mLightsLabel->setGeometry(xSpacer, yPosColumn1, columnWidth - xSpacer, buttonHeight);
        yPosColumn1 += mLightsLabel->height();

        QRect selectedLightsRect(xSpacer, yPosColumn1, columnWidth - xSpacer, buttonHeight * 8);
        mLights->resize(selectedLightsRect, mRowHeight);
        yPosColumn1 += mLights->height();

        // column 2
        yPosColumn2 += mLightsLabel->height();
        mMetadata->setGeometry(xSecondColumnStart, yPosColumn2, columnWidth, buttonHeight * 4);
        yPosColumn2 += mMetadata->height();
    }

protected:
    /*!
     * \brief resizeEvent called whenever the widget resizes so that assets can be updated.
     */
    void resizeEvent(QResizeEvent*) { resize(); }

    /// paints the dark grey background
    void paintEvent(QPaintEvent*) {
        QStyleOption opt;
        opt.init(this);
        QPainter painter(this);
        painter.fillRect(rect(), QBrush(QColor(32, 31, 31, 255)));
    }


private:
    /// update the metadata for the cor::Controller
    void updateMetadata(const cor::Controller& controller) {
        std::stringstream returnString;

        returnString << "<b>Protocol:</b> " << commTypeToString(controller.type()).toStdString()
                     << "<br>";
        if (controller.type() == ECommType::UDP || controller.type() == ECommType::HTTP) {
            returnString << "<b>IP:</b> " << controller.name().toStdString() << "<br>";
        } else {
            returnString << "<b>Port:</b> " << controller.name().toStdString() << "<br>";
        }
        returnString << "<b>API:</b> " << controller.majorAPI() << "." << controller.minorAPI()
                     << "<br>";
        returnString << "<b>Hardware Count:</b> " << controller.maxHardwareIndex() << "<br>";
        returnString << "<b>Max Packet Size:</b> " << controller.maxPacketSize() << "<br>";
        returnString << "<b>Using CRC: </b>";
        if (controller.isUsingCRC()) {
            returnString << "true<br>";
        } else {
            returnString << "false<br>";
        }
        if (controller.hardwareCapabilities()) {
            returnString << "<b>Raspberry Pi</b>";
        }
        std::string result = returnString.str();
        mMetadata->updateText(QString(result.c_str()));
    }

    /// pointer to comm data
    CommLayer* mComm;

    /// The controller being displayed.
    cor::Controller mController;

    /// name of the group
    QLabel* mName;

    /// label for lights
    QLabel* mLightsLabel;

    /// displays the lights that are part of this group and their current states.
    LightsListMenu* mLights;

    /// widget for metadata
    cor::ExpandingTextScrollArea* mMetadata;

    /// the height of a row in a scroll area
    int mRowHeight;
};


#endif // DISPLAYARDUCORCONTROLLER_H
