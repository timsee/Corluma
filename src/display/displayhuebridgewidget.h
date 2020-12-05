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
#include "cor/lightlist.h"
#include "cor/widgets/checkbox.h"
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
    explicit DisplayHueBridgeWidget(QWidget* parent,
                                    CommLayer* comm,
                                    cor::LightList* selectedLights)
        : QWidget(parent),
          mComm{comm},
          mSelectedLights{selectedLights},
          mName{new QLabel(this)},
          mLightsLabel{new QLabel("<b>Lights:</b>", this)},
          mLights{new LightsListMenu(this, true)},
          mMetadata{new cor::ExpandingTextScrollArea(this)},
          mCheckBox{new cor::CheckBox(this)} {
        auto font = mName->font();
        font.setPointSize(20);
        mName->setFont(font);

        connect(mLights, SIGNAL(clickedLight(cor::Light)), this, SLOT(lightClicked(cor::Light)));
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
        highlightLights();
        handleCheckboxState();
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
        int headerX = 0;
        int buttonHeight = this->height() / 10;
        int xSecondColumnStart = int(this->width() / 2 * 1.05);
        int columnWidth = int((this->width() / 2) * 0.95);
        int xSpacer = this->width() / 20;

        // top of both
        mName->setGeometry(xSpacer / 2,
                           yPosColumn1,
                           this->width() - xSpacer / 2 - buttonHeight,
                           buttonHeight);
        headerX += mName->width() + mName->geometry().x();
        mCheckBox->setGeometry(headerX, yPosColumn1, buttonHeight, buttonHeight);
        mCheckBox->resize();
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

    ///  highlight lights in the widget
    void highlightLights() { qDebug() << "TODO: highlight hue bridge widget"; }

signals:
    /// handle when a light is clicked
    void lightClicked(QString, bool);

    /// handle when a full controller is clicked
    void controllerClicked(QString, EProtocolType, bool);

    /// handle when deleting a full controller.
    void deleteController(QString, EProtocolType);

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

    /// handles when the mouse is released
    void mouseReleaseEvent(QMouseEvent* event) {
        if (cor::isMouseEventTouchUpInside(event, mCheckBox, false)) {
            if (mCheckBox->checkboxState() == cor::ECheckboxState::clearAll) {
                mCheckBox->checkboxState(cor::ECheckboxState::selectAll);
                emit controllerClicked(mBridge.id(), EProtocolType::hue, false);
                mLights->highlightLights({});
            } else {
                mCheckBox->checkboxState(cor::ECheckboxState::clearAll);
                emit controllerClicked(mBridge.id(), EProtocolType::hue, true);
                mLights->highlightLights(mBridge.lightIDs());
            }
        }
        event->ignore();
    }
private slots:

    /// handle when a light is clicked
    void lightClicked(cor::Light light) { emit lightClicked(light.uniqueID(), true); }

    /// handle when the delete button is pressed for a hue.
    void deleteButtonPressed(bool) { emit deleteController(mBridge.id(), EProtocolType::arduCor); }

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

    /// handle the state of the checkbox.
    void handleCheckboxState() {
        bool anyLightSelected = false;
        for (auto light : mBridge.lightIDs()) {
            if (mSelectedLights->doesLightExist(light)) {
                anyLightSelected = true;
                break;
            }
        }
        if (anyLightSelected) {
            mCheckBox->checkboxState(cor::ECheckboxState::clearAll);
        } else {
            mCheckBox->checkboxState(cor::ECheckboxState::selectAll);
        }
    }


    /// pointer to comm data
    CommLayer* mComm;

    /// list of selected lights
    cor::LightList* mSelectedLights;

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

    /// checkbox to select/deselect the bridge.
    cor::CheckBox* mCheckBox;

    /// the height of a row in a scroll area
    int mRowHeight;
};


#endif // DISPLAYHUEBRIDGEWIDGET_H
