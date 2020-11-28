#ifndef DISPLAYHUEBRIDGEWIDGET_H
#define DISPLAYHUEBRIDGEWIDGET_H

#include <QLabel>
#include <QPainter>
#include <QStyleOption>
#include <QWidget>
#include "comm/hue/bridge.h"
#include "cor/widgets/lightvectorwidget.h"
#include "cor/widgets/listitemwidget.h"
#include "syncwidget.h"
#include "utils/qt.h"

#include <QPainter>
#include <QStyleOption>
#include <QTextEdit>
#include <QWidget>
#include "comm/arducor/controller.h"
#include "comm/commhue.h"
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
 * \brief The DisplayHueBridgeWidget class displays all pertinent info for a hue::Bridge. This
 * includes the bridge's name, lights, schedules, and groups.
 */
class DisplayHueBridgeWidget : public QWidget {
    Q_OBJECT
public:
    explicit DisplayHueBridgeWidget(QWidget* parent, CommLayer* comm)
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

    /// getter for hue::Bridge represented by the widget
    const hue::Bridge& bridge() const noexcept { return mBridge; }

    /// updates the controller's UI elements.
    void updateBridge(const hue::Bridge& bridge) {
        mBridge = bridge;
        mName->setText(mBridge.name());


        mBridge.lights();
        auto lights = mComm->hue()->lightsFromMetadata(bridge.lights().items());
        mLights->showLights(lights);
        updateMetadata(mBridge);
        resize();
    }

    /// changes the row height of rows in scroll areas.
    void changeRowHeight(int height) { mRowHeight = height; }

    /// reset the widget to showing no group
    void reset() {
        mBridge = {};
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
    /// update the metadata for the hue::Bridge
    void updateMetadata(const hue::Bridge& bridge) {
        std::stringstream returnString;

        returnString << "<b>IP:</b> " << bridge.IP().toStdString() << "<br>";
        returnString << "<b>API:</b> " << bridge.API().toStdString() << "<br>";
        returnString << "<b>MAC Address:</b> " << bridge.macaddress().toStdString() << "<br>";
        returnString << "<b>ID:</b> " << bridge.id().toStdString() << "<br>";

        std::string result = returnString.str();
        mMetadata->updateText(QString(result.c_str()));
    }

    /// pointer to comm data
    CommLayer* mComm;

    /// The controller being displayed.
    hue::Bridge mBridge;

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


#endif // DISPLAYHUEBRIDGEWIDGET_H
