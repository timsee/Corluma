#ifndef DISPLAYARDUCORCONTROLLER_H
#define DISPLAYARDUCORCONTROLLER_H


#include <QPainter>
#include <QPushButton>
#include <QStyleOption>
#include <QTextEdit>
#include <QWidget>
#include "comm/arducor/controller.h"
#include "comm/commarducor.h"
#include "comm/commlayer.h"
#include "cor/lightlist.h"
#include "cor/widgets/checkbox.h"
#include "cor/widgets/expandingtextscrollarea.h"
#include "menu/displaymoodmetadata.h"
#include "menu/groupstatelistmenu.h"
#include "menu/lightslistmenu.h"
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
          mName{new QLabel(this)},
          mLightsLabel{new QLabel("<b>Lights:</b>", this)},
          mLights{new LightsListMenu(this, true)},
          mCheckBox{new cor::CheckBox(this)},
          mMetadata{new cor::ExpandingTextScrollArea(this)},
          mDeleteButton{new QPushButton("Delete", this)} {
        auto font = mName->font();
        font.setPointSize(20);
        mName->setFont(font);

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
    }

    /// updates the controller's UI elements.
    void updateController(const cor::Controller& controller) {
        mController = controller;
        mName->setText(cor::controllerToGenericName(mController, cor::EArduCorStatus::connected));

        handleCheckboxState();

        auto lights = mComm->arducor()->lightsFromNames(mController.names());
        mLights->updateLights();
        mLights->showLights(lights);
        highlightLights();

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
        yPosColumn1 += mLightsLabel->height();
        mMetadata->setGeometry(xSpacer, yPosColumn1, columnWidth, buttonHeight * 4);
        yPosColumn1 += mMetadata->height();
        mDeleteButton->setGeometry(xSpacer, yPosColumn1, columnWidth, buttonHeight);
        yPosColumn1 += mDeleteButton->height();


        // column 2
        mLightsLabel->setGeometry(xSecondColumnStart,
                                  yPosColumn2,
                                  columnWidth - xSpacer,
                                  buttonHeight);
        yPosColumn2 += mLightsLabel->height();

        QRect selectedLightsRect(xSecondColumnStart,
                                 yPosColumn2,
                                 columnWidth - xSpacer,
                                 buttonHeight * 8);
        mLights->resize(selectedLightsRect, mRowHeight);
        yPosColumn2 += mLights->height();
    }

signals:
    /// handle when a light is clicked
    void lightClicked(QString, bool);

    /// handle when the controller is clicked
    void controllerClicked(QString, EProtocolType, bool);

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
        opt.init(this);
        QPainter painter(this);
        painter.fillRect(rect(), QBrush(QColor(32, 31, 31, 255)));
    }

    /// handle when a mouse release event occurs.
    void mouseReleaseEvent(QMouseEvent* event) {
        if (cor::isMouseEventTouchUpInside(event, mCheckBox, false)) {
            if (mCheckBox->checkboxState() == cor::ECheckboxState::clearAll) {
                mCheckBox->checkboxState(cor::ECheckboxState::selectAll);
                emit controllerClicked(mController.name(), EProtocolType::arduCor, false);
                mLights->highlightLights({});
            } else {
                mCheckBox->checkboxState(cor::ECheckboxState::clearAll);
                emit controllerClicked(mController.name(), EProtocolType::arduCor, true);
                mLights->highlightLights(mController.names());
            }
        }
        event->ignore();
    }


private slots:
    /// light clicked
    void lightClicked(cor::Light light) { emit lightClicked(light.uniqueID(), true); }

    /// delete is clicked for a controller.
    void deleteButtonPressed(bool) {
        emit deleteController(mController.name(), EProtocolType::arduCor);
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

    /// pointer to comm data
    CommLayer* mComm;

    /// list of selected lights.
    cor::LightList* mSelectedLights;

    /// The controller being displayed.
    cor::Controller mController;

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

    /// the height of a row in a scroll area
    int mRowHeight;
};


#endif // DISPLAYARDUCORCONTROLLER_H
