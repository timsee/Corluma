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
#include "comm/hue/bridgegroupswidget.h"
#include "comm/hue/bridgescheduleswidget.h"
#include "cor/lightlist.h"
#include "cor/widgets/checkbox.h"
#include "cor/widgets/expandingtextscrollarea.h"
#include "lightinfolistwidget.h"
#include "menu/displaymoodmetadata.h"
#include "menu/groupstatelistmenu.h"
#include "menu/lightslistmenu.h"


enum class EDisplayHueBridgeState { info, lights, groups, schedule };

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
          mInfoButton{new QPushButton("Info", this)},
          mLightsButton{new QPushButton("Lights", this)},
          mGroupsButton{new QPushButton("Groups", this)},
          mScheduleButton{new QPushButton("Schedules", this)},
          mLightInfoWidget{new LightInfoListWidget(this)},
          mSelectedLights{selectedLights},
          mName{new QLabel(this)},
          mLightsLabel{new QLabel("<b>Lights:</b>", this)},
          mLights{new LightsListMenu(this, true)},
          mMetadata{new cor::ExpandingTextScrollArea(this)},
          mCheckBox{new cor::CheckBox(this)},
          mGroupsWidget{new hue::BridgeGroupsWidget(this)},
          mSchedulesWidget{new hue::BridgeSchedulesWidget(this)},
          mRowHeight{10},
          mState{EDisplayHueBridgeState::info} {
        auto font = mName->font();
        font.setPointSize(20);
        mName->setFont(font);

        mInfoButton->setCheckable(true);
        mLightsButton->setCheckable(true);
        mGroupsButton->setCheckable(true);
        mScheduleButton->setCheckable(true);

        mGroupsWidget->setVisible(false);
        mGroupsWidget->isOpen(false);

        mSchedulesWidget->setVisible(false);
        mSchedulesWidget->isOpen(false);


        connect(mInfoButton, SIGNAL(clicked(bool)), this, SLOT(infoButtonPressed(bool)));
        connect(mLightsButton, SIGNAL(clicked(bool)), this, SLOT(lightsButtonPressed(bool)));
        connect(mGroupsButton, SIGNAL(clicked(bool)), this, SLOT(groupsButtonPressed(bool)));
        connect(mScheduleButton, SIGNAL(clicked(bool)), this, SLOT(scheduleButtonPressed(bool)));

        mLightInfoWidget->isOpen(false);
        mLightInfoWidget->setVisible(false);

        connect(mLights, SIGNAL(clickedLight(cor::Light)), this, SLOT(lightClicked(cor::Light)));
        auto styleSheet = "background-color:rgb(33,32,32);";
        mName->setStyleSheet(styleSheet);
        mLightsLabel->setStyleSheet(styleSheet);

        handleButtonHighlight(mInfoButton->text());
    }

    /// getter for hue::Bridge represented by the widget
    const hue::Bridge& bridge() const noexcept { return mBridge; }

    /// getter for lightinfowidget
    LightInfoListWidget* lightInfoWidget() { return mLightInfoWidget; }

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

        auto buttonPos = 0u;
        auto buttonWidth = width() / 4;
        mInfoButton->setGeometry(buttonPos, yPosColumn1, buttonWidth, buttonHeight);
        buttonPos += mInfoButton->width();
        mLightsButton->setGeometry(buttonPos, yPosColumn1, buttonWidth, buttonHeight);
        buttonPos += mLightsButton->width();
        mGroupsButton->setGeometry(buttonPos, yPosColumn1, buttonWidth, buttonHeight);
        buttonPos += mGroupsButton->width();
        mScheduleButton->setGeometry(buttonPos, yPosColumn1, buttonWidth, buttonHeight);
        buttonPos += mScheduleButton->width();

        yPosColumn1 += mInfoButton->height();
        yPosColumn2 += mInfoButton->height();

        if (mState == EDisplayHueBridgeState::lights) {
            mLightInfoWidget->isOpen(true);
            mSchedulesWidget->setVisible(false);
            mGroupsWidget->setVisible(false);
            mGroupsWidget->isOpen(false);
            mSchedulesWidget->isOpen(false);
            mLightInfoWidget->raise();
            mLightInfoWidget->setVisible(true);
            mLightInfoWidget->setGeometry(0, yPosColumn1, width(), height() - yPosColumn1);
            mLightInfoWidget->resize();
        } else if (mState == EDisplayHueBridgeState::groups) {
            mLightInfoWidget->setVisible(false);
            mGroupsWidget->setVisible(true);
            mSchedulesWidget->setVisible(false);
            mSchedulesWidget->isOpen(false);
            mGroupsWidget->setGeometry(0, yPosColumn1, width(), height() - yPosColumn1);
            displayGroups();
        } else if (mState == EDisplayHueBridgeState::schedule) {
            mLightInfoWidget->setVisible(false);
            mGroupsWidget->setVisible(false);
            mSchedulesWidget->setVisible(true);
            mGroupsWidget->isOpen(false);
            mSchedulesWidget->raise();
            mSchedulesWidget->setGeometry(0, yPosColumn1, width(), height() - yPosColumn1);
            displaySchedules();
        } else {
            mLightInfoWidget->setVisible(false);
            mGroupsWidget->setVisible(false);
            mSchedulesWidget->setVisible(false);
            mGroupsWidget->isOpen(false);
            mSchedulesWidget->isOpen(false);
        }

        // column 1
        mLightsLabel->setGeometry(xSpacer, yPosColumn1, columnWidth - xSpacer, buttonHeight);
        yPosColumn1 += mLightsLabel->height();

        QRect selectedLightsRect(xSpacer, yPosColumn1, columnWidth - xSpacer, buttonHeight * 7);
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
        opt.initFrom(this);
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

    /// info button pressed
    void infoButtonPressed(bool) {
        mState = EDisplayHueBridgeState::info;
        handleState();
        handleButtonHighlight(mInfoButton->text());
    }

    /// lights button pressed
    void lightsButtonPressed(bool) {
        mState = EDisplayHueBridgeState::lights;
        mLightInfoWidget->scrollArea()->updateHues(mComm->hue()->discovery()->lights());
        handleState();
        handleButtonHighlight(mLightsButton->text());
    }

    /// groups button pressed
    void groupsButtonPressed(bool) {
        mState = EDisplayHueBridgeState::groups;
        handleState();
        handleButtonHighlight(mGroupsButton->text());
    }

    /// schedule buttons pressed
    void scheduleButtonPressed(bool) {
        mState = EDisplayHueBridgeState::schedule;
        handleState();
        handleButtonHighlight(mScheduleButton->text());
    }

private:
    /// display the groups widget
    void displayGroups() {
        mGroupsWidget->updateGroups(mBridge.groupsWithIDs(), mBridge.roomsWithIDs());
        mGroupsWidget->isOpen(true);
        mGroupsWidget->setVisible(true);
        mGroupsWidget->show();
        mGroupsWidget->resize();
        mGroupsWidget->raise();
    }

    /// display the schedules widget
    void displaySchedules() {
        mSchedulesWidget->updateSchedules(mBridge.schedules().items());
        mSchedulesWidget->isOpen(true);
        mSchedulesWidget->setVisible(true);
        mSchedulesWidget->show();
        mSchedulesWidget->resize();
        mSchedulesWidget->raise();
    }

    /// handle states
    void handleState() { resize(); }

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

    /// handle the highlight of buttons
    void handleButtonHighlight(const QString& key) {
        mInfoButton->setChecked(key == mInfoButton->text());
        mLightsButton->setChecked(key == mLightsButton->text());
        mGroupsButton->setChecked(key == mGroupsButton->text());
        mScheduleButton->setChecked(key == mScheduleButton->text());
    }

    /// pointer to comm data
    CommLayer* mComm;

    /// button for showing light info
    QPushButton* mInfoButton;

    /// button for showing info on specific lights
    QPushButton* mLightsButton;

    /// button for showing groups
    QPushButton* mGroupsButton;

    /// button showing schedules
    QPushButton* mScheduleButton;

    /// shows the info for each light
    LightInfoListWidget* mLightInfoWidget;

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

    /// widget for showing the groups on the bridge.
    hue::BridgeGroupsWidget* mGroupsWidget;

    /// widget for showing the schedules on the bridge.
    hue::BridgeSchedulesWidget* mSchedulesWidget;

    /// the height of a row in a scroll area
    int mRowHeight;

    /// state of the widget
    EDisplayHueBridgeState mState;
};


#endif // DISPLAYHUEBRIDGEWIDGET_H
