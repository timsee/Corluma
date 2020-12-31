#ifndef DISPLAYARDUCORCONTROLLER_H
#define DISPLAYARDUCORCONTROLLER_H


#include <QMessageBox>
#include <QPainter>
#include <QPushButton>
#include <QStyleOption>
#include <QTextEdit>
#include <QWidget>
#include "comm/arducor/controller.h"
#include "comm/commarducor.h"
#include "comm/commlayer.h"
#include "cor/lightlist.h"
#include "cor/widgets/button.h"
#include "cor/widgets/checkbox.h"
#include "cor/widgets/expandingtextscrollarea.h"
#include "menu/displaymoodmetadata.h"
#include "menu/groupstatelistmenu.h"
#include "menu/lightslistmenu.h"
#include "syncwidget.h"
#include "utils/qt.h"

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
    explicit DisplayArduCorControllerWidget(QWidget* parent,
                                            CommLayer* comm,
                                            cor::LightList* selectedLights)
        : QWidget(parent),
          mComm{comm},
          mSelectedLights{selectedLights},
          mStatus{cor::EArduCorStatus::searching},
          mName{new QLabel(this)},
          mLightsLabel{new QLabel("<b>Lights:</b>", this)},
          mLights{new LightsListMenu(this, true)},
          mCheckBox{new cor::CheckBox(this)},
          mMetadata{new cor::ExpandingTextScrollArea(this)},
          mDeleteButton{new QPushButton("Delete", this)},
          mStateButton{new cor::Button(this, {})},
          mSingleLightIcon{new QLabel(this)},
          mSyncWidget{new SyncWidget(this)},
          mRowHeight{10} {
        auto font = mName->font();
        font.setPointSize(20);
        mName->setFont(font);

        mStateButton->setVisible(false);
        mStateButton->setIconPercent(0.85);

        mSingleLightIcon->setVisible(false);
        mSingleLightIcon->setAlignment(Qt::AlignHCenter);

        connect(mDeleteButton, SIGNAL(clicked(bool)), this, SLOT(deleteButtonPressed(bool)));
        mDeleteButton->setStyleSheet("background-color:rgb(110,30,30);");

        connect(mLights, SIGNAL(clickedLight(cor::Light)), this, SLOT(lightClicked(cor::Light)));
        this->setStyleSheet("background-color:rgb(33,32,32);");
    }

    /// getter for controller represented by the widget
    const cor::Controller& controller() const noexcept { return mController; }

    /// highlight lights
    void highlightLights() {
        auto lights = mComm->arducor()->lightsFromNames(mController.names());
        // highlight relevant lights
        std::vector<QString> lightsToHighlight;
        for (auto light : lights) {
            if (mSelectedLights->doesLightExist(light)) {
                lightsToHighlight.push_back(light.uniqueID());
            }
        }
        mLights->highlightLights(lightsToHighlight);

        handleCheckboxState();
    }

    /// updates the controller's UI elements.
    void updateController(const cor::Controller& controller, cor::EArduCorStatus status) {
        mController = controller;
        mStatus = status;
        mName->setText(cor::controllerToGenericName(mController, status));
        handleCheckboxState();

        auto lights = mComm->arducor()->lightsFromNames(mController.names());
        if (mStatus == cor::EArduCorStatus::searching) {
            // handle when light is not discovered
            mSyncWidget->changeState(ESyncState::syncing);
            mLightsLabel->setVisible(false);
            mLights->setVisible(false);
            mSingleLightIcon->setVisible(false);
            mCheckBox->setVisible(false);
            mStateButton->setVisible(false);
            mSyncWidget->setVisible(true);
        } else if (lights.size() == 1) {
            // hide the lights menu and show the icons when theres only one light
            mLightsLabel->setVisible(false);
            mLights->setVisible(false);
            mSyncWidget->setVisible(false);
            mCheckBox->setVisible(true);
            mSingleLightIcon->setVisible(true);
            mStateButton->setVisible(true);
            mSyncWidget->changeState(ESyncState::synced);
            // update the icons
            updateSingleIcon();
            auto singleLightVector = mComm->arducor()->lightsFromNames(mController.names());
            mStateButton->updateRoutine(singleLightVector[0].state());
        } else {
            // hide the icons and show the lights list when theres only one light
            mLightsLabel->setVisible(true);
            mLights->setVisible(true);
            mSingleLightIcon->setVisible(false);
            mStateButton->setVisible(false);
            mCheckBox->setVisible(true);
            mSyncWidget->setVisible(false);
            mSyncWidget->changeState(ESyncState::synced);
            // update the lights menu
            mLights->updateLights();
            mLights->showLights(lights);
            highlightLights();
        }

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
        int headerX = 0;
        int buttonHeight = this->height() / 10;
        int xSecondColumnStart = int(this->width() / 2 * 1.05);
        int columnWidth = int((this->width() / 2) * 0.95);
        int xSpacer = this->width() / 20;

        // top of both
        auto nameWidth = this->width() - xSpacer / 2 - buttonHeight;
        if (isSingleLight()) {
            nameWidth -= (buttonHeight + xSpacer / 2);
        }
        mName->setGeometry(xSpacer / 2, yPosColumn1, nameWidth, buttonHeight);
        headerX += mName->width() + mName->geometry().x();
        if (isSingleLight()) {
            mStateButton->setGeometry(headerX, yPosColumn1, buttonHeight, buttonHeight);
            headerX += mStateButton->width() + xSpacer / 2;
        }
        mCheckBox->setGeometry(headerX, yPosColumn1, buttonHeight, buttonHeight);
        mCheckBox->resize();


        yPosColumn1 += mName->height();
        yPosColumn2 += mName->height();

        mLightsLabel->setGeometry(xSpacer, yPosColumn1, columnWidth - xSpacer, buttonHeight);
        yPosColumn1 += mLightsLabel->height();

        // column 1
        if (mStatus == cor::EArduCorStatus::searching) {
            mSyncWidget->setGeometry(xSpacer, yPosColumn1, columnWidth, buttonHeight * 4);
            yPosColumn1 += mSyncWidget->height();
        } else if (isSingleLight()) {
            // update the icons
            mSingleLightIcon->setGeometry(xSpacer, yPosColumn1, columnWidth, buttonHeight * 4);
            yPosColumn1 += mSingleLightIcon->height();
            updateSingleIcon();
        } else {
            QRect selectedLightsRect(xSpacer, yPosColumn1, columnWidth - xSpacer, buttonHeight * 7);
            mLights->resize(selectedLightsRect, mRowHeight);
            yPosColumn1 += mLights->height();
        }

        // column 2
        yPosColumn2 += mLightsLabel->height();
        mMetadata->setGeometry(xSecondColumnStart, yPosColumn2, columnWidth, buttonHeight * 4);
        yPosColumn2 += mMetadata->height();

        // put delete button on bottom of column
        mDeleteButton->setGeometry(xSecondColumnStart,
                                   height() - buttonHeight,
                                   columnWidth,
                                   buttonHeight);
    }

signals:
    /// emits when a light should be selected
    void selectLight(QString);

    /// emits when a light should be deselected
    void deselectLight(QString);

    /// handle when the controller is clicked
    void selectControllerLights(QString, EProtocolType);

    /// handle when all the lights from a controller should be deselected.
    void deselectControllerLights(QString, EProtocolType);

    /// delete a controller
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

    /// handle when a mouse release event occurs.
    void mouseReleaseEvent(QMouseEvent* event) {
        if (cor::isMouseEventTouchUpInside(event, mCheckBox, false)) {
            if (mCheckBox->checkboxState() == cor::ECheckboxState::clearAll) {
                mCheckBox->checkboxState(cor::ECheckboxState::selectAll);
                emit deselectControllerLights(mController.name(), EProtocolType::arduCor);
                mLights->highlightLights({});
            } else {
                mCheckBox->checkboxState(cor::ECheckboxState::clearAll);
                emit selectControllerLights(mController.name(), EProtocolType::arduCor);
                mLights->highlightLights(mController.names());
            }
        }
        event->ignore();
    }


private slots:
    /// light clicked
    void lightClicked(cor::Light light) {
        if (mSelectedLights->doesLightExist(light.uniqueID())) {
            emit deselectLight(light.uniqueID());
        } else {
            emit selectLight(light.uniqueID());
        }
    }

    /// delete is clicked for a controller.
    void deleteButtonPressed(bool) {
        QMessageBox::StandardButton reply;
        QString text =
            "Delete " + mController.name() + "? This will remove it from the app memory.";

        reply = QMessageBox::question(this, "Delete?", text, QMessageBox::Yes | QMessageBox::No);
        if (reply == QMessageBox::Yes) {
            // signal to remove from app
            emit deleteController(mController.name(), EProtocolType::arduCor);
        }
    }

private:
    /// update the metadata for the cor::Controller
    void updateMetadata(const cor::Controller& controller) {
        std::stringstream returnString;

        if (mStatus == cor::EArduCorStatus::connected) {
            returnString << "<b>Protocol:</b> " << commTypeToString(controller.type()).toStdString()
                         << "<br>";
        }
        if (controller.type() == ECommType::UDP || controller.type() == ECommType::HTTP) {
            returnString << "<b>IP:</b> " << controller.name().toStdString() << "<br>";
        } else {
            returnString << "<b>Port:</b> " << controller.name().toStdString() << "<br>";
        }
        returnString << "<b>API:</b> " << controller.majorAPI() << "." << controller.minorAPI()
                     << "<br>";
        if (mStatus == cor::EArduCorStatus::connected) {
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
        }
        std::string result = returnString.str();
        mMetadata->updateText(QString(result.c_str()));
    }

    /// handle checkbox state
    void handleCheckboxState() {
        bool anyLightSelected = false;
        for (auto light : mController.names()) {
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

    /// update the icon for the single light case.
    void updateSingleIcon() {
        int side = iconSide();
        if (mHardwareIcon.size() != QSize(side, side)) {
            mHardwareIcon = lightHardwareTypeToPixmap(mController.hardwareTypes()[0]);
            mHardwareIcon =
                mHardwareIcon.scaled(side, side, Qt::KeepAspectRatio, Qt::SmoothTransformation);
            mSingleLightIcon->setPixmap(mHardwareIcon);
        }
    }

    /// true if single light, false if multiple lights
    bool isSingleLight() { return mController.hardwareTypes().size() == 1; }

    /// getter for iconSide
    int iconSide() { return std::min(height() * 2 / 10, width() / 2); }

    /// pointer to comm data
    CommLayer* mComm;

    /// list of selected lights.
    cor::LightList* mSelectedLights;

    /// The controller being displayed.
    cor::Controller mController;

    /// status of the controller (whether it is connected or searching)
    cor::EArduCorStatus mStatus;

    /// name of the group
    QLabel* mName;

    /// label for lights
    QLabel* mLightsLabel;

    /// displays the lights that are part of this group and their current states.
    LightsListMenu* mLights;

    /// checkbox for selecting/deselecting the controller.
    cor::CheckBox* mCheckBox;

    /// widget for metadata
    cor::ExpandingTextScrollArea* mMetadata;

    /// button for deleting the currently selected controller
    QPushButton* mDeleteButton;

    /// state button for the single light case
    cor::Button* mStateButton;

    /// single light icon to show the hardware type.
    QLabel* mSingleLightIcon;

    /// sync widget to display when searching for the light
    SyncWidget* mSyncWidget;

    /// stores the hardware icon in the single light case
    QPixmap mHardwareIcon;

    /// the height of a row in a scroll area
    int mRowHeight;
};


#endif // DISPLAYARDUCORCONTROLLER_H
