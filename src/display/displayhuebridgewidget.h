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

#include <QMessageBox>
#include <QPainter>
#include <QStyleOption>
#include <QWidget>
#include "comm/arducor/controller.h"
#include "comm/commhue.h"
#include "comm/commlayer.h"
#include "comm/hue/bridgegroupswidget.h"
#include "comm/hue/bridgescheduleswidget.h"
#include "comm/hue/lightdiscovery.h"
#include "cor/lightlist.h"
#include "cor/widgets/checkbox.h"
#include "cor/widgets/expandingtextscrollarea.h"
#include "cor/widgets/textinputwidget.h"
#include "greyoutoverlay.h"
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
          mChangeName{new QPushButton("Change Bridge Name", this)},
          mDeleteButton{new QPushButton("Delete Bridge", this)},
          mMetadata{new cor::ExpandingTextScrollArea(this)},
          mCheckBox{new cor::CheckBox(this)},
          mGroupsWidget{new hue::BridgeGroupsWidget(this)},
          mSchedulesWidget{new hue::BridgeSchedulesWidget(this)},
          mGreyout{new GreyOutOverlay(true, parentWidget()->parentWidget())},
          mChangeNameInput{new cor::TextInputWidget(parentWidget()->parentWidget())},
          mHueLightDiscovery{new hue::LightDiscovery(parentWidget()->parentWidget(), comm)},
          mRowHeight{10},
          mChangeBridgeName{false},
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

        connect(mChangeName, SIGNAL(clicked(bool)), this, SLOT(handleChangeBridgeNamePressed()));
        connect(mDeleteButton, SIGNAL(clicked(bool)), this, SLOT(deleteButtonPressed(bool)));
        mDeleteButton->setStyleSheet("background-color:rgb(110,30,30);");

        connect(mChangeNameInput, SIGNAL(textAdded(QString)), this, SLOT(nameChanged(QString)));
        connect(mChangeNameInput, SIGNAL(cancelClicked()), this, SLOT(closeNameWidget()));
        mChangeNameInput->setVisible(false);

        connect(mLightInfoWidget,
                SIGNAL(deleteLight(QString)),
                this,
                SLOT(deleteLightFromBridge(QString)));

        connect(mGreyout, SIGNAL(clicked()), this, SLOT(greyOutClicked()));
        mGreyout->greyOut(false);

        connect(mInfoButton, SIGNAL(clicked(bool)), this, SLOT(infoButtonPressed(bool)));
        connect(mLightsButton, SIGNAL(clicked(bool)), this, SLOT(lightsButtonPressed(bool)));
        connect(mGroupsButton, SIGNAL(clicked(bool)), this, SLOT(groupsButtonPressed(bool)));
        connect(mScheduleButton, SIGNAL(clicked(bool)), this, SLOT(scheduleButtonPressed(bool)));

        mLightInfoWidget->isOpen(false);
        mLightInfoWidget->setVisible(false);
        connect(mLightInfoWidget,
                SIGNAL(changeLightName(QString, QString)),
                this,
                SLOT(handleChangeNamePressed(QString, QString)));
        connect(mLightInfoWidget,
                SIGNAL(findNewLightClicked()),
                this,
                SLOT(handleFindLightClicked()));

        mHueLightDiscovery->setVisible(false);
        mHueLightDiscovery->isOpen(false);
        connect(mHueLightDiscovery, SIGNAL(closePressed()), this, SLOT(hueDiscoveryClosePressed()));
        connect(mLights, SIGNAL(clickedLight(cor::Light)), this, SLOT(lightClicked(cor::Light)));
        auto styleSheet = "background-color:rgb(33,32,32);";
        mName->setStyleSheet(styleSheet);
        mLightsLabel->setStyleSheet(styleSheet);

        handleButtonHighlight(mInfoButton->text());
    }

    /// called when the widget is shown
    void showWidget() {
        // reset the widget to the info page
        infoButtonPressed(true);
    }

    /// getter for hue::Bridge represented by the widget
    const hue::Bridge& bridge() const noexcept { return mBridge; }

    /// getter for lightinfowidget
    LightInfoListWidget* lightInfoWidget() { return mLightInfoWidget; }

    /// updates the controller's UI elements.
    void updateBridge(const hue::Bridge& bridge) {
        mBridge = bridge;
        mName->setText(mBridge.customName());


        mBridge.lights();
        auto lights = mComm->hue()->lightsFromMetadata(bridge.lights().items());
        mLights->showLights(lights);
        highlightLights();
        handleCheckboxState();
        updateMetadata(mBridge);
        resize();
    }

    /// changes the row height of rows in scroll areas.
    void changeRowHeight(int height) {
        mRowHeight = height;
        mLightInfoWidget->changeRowHeight(height);
    }

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

        mGreyout->resize();
        mHueLightDiscovery->resize();

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

        yPosColumn2 += buttonHeight;
        mChangeName->setGeometry(xSecondColumnStart, yPosColumn2, columnWidth, buttonHeight);
        yPosColumn2 += mChangeName->height();

        mDeleteButton->setGeometry(xSecondColumnStart, yPosColumn2, columnWidth, buttonHeight);
        yPosColumn2 += mDeleteButton->height();
    }

    ///  highlight lights in the widget
    void highlightLights() { qDebug() << "TODO: highlight hue bridge widget"; }

    /// remove lights by their keys from the list.
    void removeLights(const std::vector<QString>& keys) { mLights->removeLights(keys); }

    /// handles when the hue discovery finds a light.
    void newHueFound(QString uniqueID) {
        auto light = mComm->lightByID(uniqueID);
        if (light.isValid()) {
            if (light.protocol() == EProtocolType::hue) {
                auto metadata = mComm->hue()->metadataFromLight(light);
                mLightInfoWidget->addLight(metadata);
                mLightInfoWidget->changeRowHeight(mRowHeight);
                mLights->addLight(light);
                // update the bridge of this hue
                auto bridge = mComm->hue()->discovery()->bridgeFromLight(metadata);
                updateBridge(bridge);
                handleState();
                qDebug() << " added this light: " << light;
            }
        } else {
            qDebug() << " INFO: invalid light found from discovery... this shouldn't happen...";
        }
    }

signals:
    /// emits when a light should be selected
    void selectLight(QString);

    /// emits when a light should be deselected
    void deselectLight(QString);

    /// handle when a full controller should be selected
    void selectControllerLights(QString, EProtocolType);

    /// handle when a full controller should be deselected
    void deselectControllerLights(QString, EProtocolType);

    /// handle when deleting a full controller.
    void deleteController(QString, EProtocolType);

    /// signals that specific light should be deleted.
    void deleteLight(QString);

    /// emits when a bridge name is changed.
    void controllerNameChanged(QString bridgeID, QString name);

    /// signals when a light name is changed, signaling the lights unique ID and its current name.
    void lightNameChanged(QString uniqueID, QString name);

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
                emit deselectControllerLights(mBridge.id(), EProtocolType::hue);
                mLights->highlightLights({});
            } else {
                mCheckBox->checkboxState(cor::ECheckboxState::clearAll);
                emit selectControllerLights(mBridge.id(), EProtocolType::hue);
                mLights->highlightLights(mBridge.lightIDs());
            }
        }
        event->ignore();
    }
private slots:

    /// handle when a light is clicked
    void lightClicked(cor::Light light) {
        if (mSelectedLights->doesLightExist(light.uniqueID())) {
            emit deselectLight(light.uniqueID());
        } else {
            emit selectLight(light.uniqueID());
        }
    }

    /// handle when the delete button is pressed for a hue.
    void deleteButtonPressed(bool) {
        QMessageBox::StandardButton reply;
        QString text = "Delete " + mBridge.customName()
                       + "? This will remove it and all of its lights from the app memory.";

        reply = QMessageBox::question(this, "Delete?", text, QMessageBox::Yes | QMessageBox::No);
        if (reply == QMessageBox::Yes) {
            // signal to remove from app
            emit deleteController(mBridge.id(), EProtocolType::hue);
        }
    }

    /// signals when a light should be deleted from the bridge
    void deleteLightFromBridge(QString lightID) {
        auto light = mComm->lightByID(lightID);
        QMessageBox::StandardButton reply;
        QString text = "Delete " + light.name() + "? This will remove it from the Hue Bridge.";
        reply = QMessageBox::question(this, "Delete?", text, QMessageBox::Yes | QMessageBox::No);
        if (reply == QMessageBox::Yes) {
            mLightInfoWidget->deleteLightFromDisplay(lightID);
            emit deleteLight(lightID);
        }
    }

    /// info button pressed
    void infoButtonPressed(bool) {
        mState = EDisplayHueBridgeState::info;
        handleState();
        handleButtonHighlight(mInfoButton->text());
    }

    /// lights button pressed
    void lightsButtonPressed(bool) {
        mState = EDisplayHueBridgeState::lights;
        mLightInfoWidget->scrollArea()->updateHues(mBridge.lights().items());
        mLightInfoWidget->changeRowHeight(mRowHeight);
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

    /// handles when the change bridge name button is pressed.
    void handleChangeBridgeNamePressed() {
        mChangeBridgeName = true;
        mGreyout->greyOut(true);
        mChangeNameInput->pushIn("Change Name of Hue Bridge", mBridge.customName());
        mChangeNameInput->setVisible(true);
        mChangeNameInput->raise();
    }

    /// handle when the name change is pressed.
    void handleChangeNamePressed(QString uniqueID, QString name) {
        mLightToChangeName = uniqueID;
        mChangeBridgeName = false;
        mGreyout->greyOut(true);
        mChangeNameInput->pushIn("Change Name of Hue: ", name);
        mChangeNameInput->setVisible(true);
        mChangeNameInput->raise();
    }

    /// change a name of a hue light or bridge.
    void nameChanged(const QString& name) {
        if (!name.isEmpty()) {
            if (mChangeBridgeName) {
                auto result = mComm->hue()->discovery()->changeName(mBridge, name);
                if (result) {
                    mName->setText(name);
                    emit controllerNameChanged(mBridge.id(), name);
                } else {
                    qDebug() << "WARNING: could not change bridge name, id: " << mBridge.id()
                             << " to: " << name;
                }
            } else {
                mLightInfoWidget->updateLightName(mLightToChangeName, name);
                emit lightNameChanged(mLightToChangeName, name);
            }
            mGreyout->greyOut(false);
            mChangeNameInput->pushOut();
            mChangeNameInput->raise();
        }
    }

    /// close the name widget
    void closeNameWidget() {
        mGreyout->greyOut(false);
        mChangeNameInput->pushOut();
    }

    /// grey out clicked
    void greyOutClicked() {
        mChangeNameInput->pushOut();
        mGreyout->greyOut(false);

        if (mHueLightDiscovery->isOpen()) {
            hueDiscoveryClosePressed();
        }
    }

    /// handles when the find light button is clicked
    void handleFindLightClicked() {
        mGreyout->greyOut(true);
        mHueLightDiscovery->isOpen(true);
        mHueLightDiscovery->resize();
        mHueLightDiscovery->setVisible(true);
        mHueLightDiscovery->show(mBridge);
        mHueLightDiscovery->raise();
    }

    /// handles when the hue discovery widget has its closed button pressed.
    void hueDiscoveryClosePressed() {
        mGreyout->greyOut(false);
        mHueLightDiscovery->isOpen(false);
        mHueLightDiscovery->setVisible(false);
        mHueLightDiscovery->hide();
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
        for (const auto& light : mBridge.lightIDs()) {
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

    /// button to change name.
    QPushButton* mChangeName;

    /// button for deleting the currently selected nanoleaf
    QPushButton* mDeleteButton;

    /// widget for metadata
    cor::ExpandingTextScrollArea* mMetadata;

    /// checkbox to select/deselect the bridge.
    cor::CheckBox* mCheckBox;

    /// widget for showing the groups on the bridge.
    hue::BridgeGroupsWidget* mGroupsWidget;

    /// widget for showing the schedules on the bridge.
    hue::BridgeSchedulesWidget* mSchedulesWidget;

    /// widget for greying out widgets in the background
    GreyOutOverlay* mGreyout;

    /// input to change the name of a light
    cor::TextInputWidget* mChangeNameInput;

    /// widget for discovering hue lights
    hue::LightDiscovery* mHueLightDiscovery;

    /// the height of a row in a scroll area
    int mRowHeight;

    /// false to change light name, true to change bridge name
    bool mChangeBridgeName;

    /// name of light to change
    QString mLightToChangeName;

    /// state of the widget
    EDisplayHueBridgeState mState;
};


#endif // DISPLAYHUEBRIDGEWIDGET_H
