#ifndef DISPLAYNANOLEAFCONTROLLERWIDGET_H
#define DISPLAYNANOLEAFCONTROLLERWIDGET_H

#include <QMessageBox>
#include <QPainter>
#include <QPushButton>
#include <QStyleOption>
#include <QTextEdit>
#include <QWidget>
#include "comm/commlayer.h"
#include "comm/commnanoleaf.h"
#include "comm/nanoleaf/leafmetadata.h"
#include "cor/widgets/button.h"
#include "cor/widgets/checkbox.h"
#include "cor/widgets/expandingtextscrollarea.h"
#include "cor/widgets/textinputwidget.h"
#include "displaynanoleafscheduleswidget.h"
#include "greyoutoverlay.h"
#include "menu/displaymoodmetadata.h"
#include "menu/groupstatelistmenu.h"
#include "menu/lightslistmenu.h"

/*!
 * \copyright
 * Copyright (C) 2015 - 2020.
 * Released under the GNU General Public License.
 *
 *
 * \brief The DisplayGroupWidget class displays all pertinent info for a Nanoleaf. This includes
 * the Nanoleaf's name, firmware, and other metadata, as well as what is connected to the nanoleaf.
 */
class DisplayNanoleafControllerWidget : public QWidget {
    Q_OBJECT
public:
    explicit DisplayNanoleafControllerWidget(QWidget* parent, CommLayer* comm)
        : QWidget(parent),
          mComm{comm},
          mDisplayLights{new QLabel(this)},
          mName{new QLabel(this)},
          mStateButton{new cor::Button(this, {})},
          mCheckBox{new cor::CheckBox(this)},
          mChangeName{new QPushButton("Change Name", this)},
          mDeleteButton{new QPushButton("Delete", this)},
          mMetadata{new cor::ExpandingTextScrollArea(this)},
          mSchedulesLabel{new QLabel("<b>Schedules:</b>", this)},
          mSchedulesWidget{new DisplayNanoleafSchedulesWidget(this)},
          mGreyout{new GreyOutOverlay(true, parentWidget()->parentWidget())},
          mChangeNameInput{new cor::TextInputWidget(parentWidget()->parentWidget())} {
        auto font = mName->font();
        font.setPointSize(20);
        mName->setFont(font);
        connect(mChangeNameInput, SIGNAL(textAdded(QString)), this, SLOT(nameChanged(QString)));
        connect(mChangeNameInput, SIGNAL(cancelClicked()), this, SLOT(closeNameWidget()));
        mChangeNameInput->setVisible(false);

        connect(mDeleteButton, SIGNAL(clicked(bool)), this, SLOT(deleteButtonPressed(bool)));
        mDeleteButton->setStyleSheet("background-color:rgb(110,30,30);");

        connect(mGreyout, SIGNAL(clicked()), this, SLOT(greyOutClicked()));
        mGreyout->greyOut(false);

        connect(mChangeName, SIGNAL(clicked(bool)), this, SLOT(handleChangeNamePressed()));
        this->setStyleSheet("background-color:rgb(33,32,32);");
    }

    /// getter for controller represented by the widget
    const nano::LeafMetadata& metadata() const noexcept { return mLeaf; }

    /// updates the meatadata's UI elements.
    void updateLeafMetadata(const nano::LeafMetadata& leafMetadata, bool isSelected) {
        mLeaf = leafMetadata;

        if (isSelected) {
            mCheckBox->checkboxState(cor::ECheckboxState::clearAll);
        } else {
            mCheckBox->checkboxState(cor::ECheckboxState::selectAll);
        }

        if (!leafMetadata.name().isEmpty()) {
            mName->setText(leafMetadata.name());
        } else {
            mName->setText(leafMetadata.hardwareName());
        }
        auto light = mComm->nanoleaf()->lightFromMetadata(leafMetadata);
        if (light.second) {
            mStateButton->updateRoutine(light.first.state());
        }
        auto scheduleResult = mComm->nanoleaf()->findSchedules(leafMetadata.serialNumber());
        if (scheduleResult.second) {
            mSchedulesWidget->updateSchedules(scheduleResult.first.items(),
                                              light.first.state().isOn());
        }
        updateMetadata(leafMetadata);
        resize();
    }

    /// changes the row height of rows in scroll areas.
    void changeRowHeight(int height) { mRowHeight = height; }

    /// reset the widget to showing no group
    void reset() {
        mLeaf = {};
        mName->setText("");
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

        mGreyout->resize();

        // top of both
        mName->setGeometry(xSpacer / 2,
                           yPosColumn1,
                           this->width() - xSpacer / 2 - buttonHeight * 2,
                           buttonHeight);
        headerX += mName->width() + mName->x();

        mStateButton->setGeometry(headerX, yPosColumn1, buttonHeight, buttonHeight);
        headerX += mStateButton->width();
        mCheckBox->setGeometry(headerX, yPosColumn1, buttonHeight, buttonHeight);
        mCheckBox->resize();
        headerX += mCheckBox->width();

        yPosColumn1 += mName->height();
        yPosColumn2 += mName->height();

        mDisplayLights->setGeometry(xSpacer / 2,
                                    yPosColumn1,
                                    this->width() - xSpacer / 2,
                                    buttonHeight * 4);
        yPosColumn1 += mDisplayLights->height();
        yPosColumn2 += mDisplayLights->height();


        // column 1
        mMetadata->setGeometry(xSpacer, yPosColumn1, columnWidth, buttonHeight * 4);
        yPosColumn1 += mMetadata->height();
        mChangeName->setGeometry(xSpacer, yPosColumn1, columnWidth, buttonHeight);

        // column 2
        mSchedulesLabel->setGeometry(xSecondColumnStart,
                                     yPosColumn2,
                                     columnWidth,
                                     buttonHeight * 0.5);
        yPosColumn2 += mSchedulesLabel->height();
        mSchedulesWidget->setGeometry(xSecondColumnStart,
                                      yPosColumn2,
                                      columnWidth,
                                      buttonHeight * 3.5);
        yPosColumn2 += mSchedulesWidget->height();

        mDeleteButton->setGeometry(xSecondColumnStart, yPosColumn2, columnWidth, buttonHeight);
        yPosColumn2 += mDeleteButton->height();
    }
signals:

    /// emit when a light is deleted
    void deleteLight(QString);

    /// emit when a light is clicked
    void lightClicked(QString, bool);

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

    /// handle mouse release events
    void mouseReleaseEvent(QMouseEvent* event) {
        if (cor::isMouseEventTouchUpInside(event, mCheckBox, false)) {
            if (mCheckBox->checkboxState() == cor::ECheckboxState::clearAll) {
                mCheckBox->checkboxState(cor::ECheckboxState::selectAll);
                emit lightClicked(mLeaf.serialNumber(), false);
            } else {
                mCheckBox->checkboxState(cor::ECheckboxState::clearAll);
                emit lightClicked(mLeaf.serialNumber(), true);
            }
        }
        event->ignore();
    }

private slots:
    /// handle when the name change is pressed
    void handleChangeNamePressed() {
        mGreyout->greyOut(true);
        mChangeNameInput->pushIn("Change Name of Nanoleaf", mLeaf.name());
        mChangeNameInput->raise();
        mChangeNameInput->setVisible(true);
    }

    /// change a name of a nanoleaf.
    void nameChanged(const QString& name) {
        if (!name.isEmpty()) {
            mComm->nanoleaf()->renameLight(mLeaf, name);
            mLeaf.name(name);
            updateLeafMetadata(mLeaf, mCheckBox->checkboxState() == cor::ECheckboxState::clearAll);
            mGreyout->greyOut(false);
            mChangeNameInput->pushOut();
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
    }

    /*!
     * \brief deleteButtonPressed delete button pressed, which triggers deleting a nanoleaf
     */
    void deleteButtonPressed(bool) {
        QMessageBox::StandardButton reply;
        QString text = "Delete " + mLeaf.name() + "? This will remove it from the app memory.";

        reply = QMessageBox::question(this, "Delete?", text, QMessageBox::Yes | QMessageBox::No);
        if (reply == QMessageBox::Yes) {
            // signal to remove from app
            emit deleteLight(mLeaf.serialNumber());
        }
    }

private:
    /// update the metadata for the nano::LeafMetadata
    void updateMetadata(const nano::LeafMetadata& leafMetadata) {
        std::stringstream returnString;

        returnString << "<b>Model:</b> " << leafMetadata.model().toStdString() << "<br>";
        returnString << "<b>Firmware:</b> " << leafMetadata.firmware().toStdString() << "<br>";
        returnString << "<b>Serial:</b> " << leafMetadata.serialNumber().toStdString()
                     << "<br><br>";

        returnString << "<b>IP:</b> " << leafMetadata.IP().toStdString() << "<br>";
        returnString << "<b>Port:</b> " << leafMetadata.port() << "<br><br>";
        returnString << "<b>Hardware Name:</b> " << leafMetadata.hardwareName().toStdString()
                     << "<br>";
        returnString << "<b>Hardware Version:</b> " << leafMetadata.hardwareVersion().toStdString()
                     << "<br>";
        if (leafMetadata.rhythmController().isConnected()) {
            auto rhythmController = leafMetadata.rhythmController();
            returnString << "<br><br><b>Rhythm Controller</b><br>";
            returnString << "<b>ID: </b>" << rhythmController.ID() << "<br>";
            returnString << "<b>Firmware: </b>" << rhythmController.firmwareVersion().toStdString()
                         << "<br>";
            returnString << "<b>Hardware Version: </b>"
                         << rhythmController.hardwareVersion().toStdString() << "<br>";
            returnString << "<b>Mode: </b>" << rhythmController.mode() << "<br>";

            if (rhythmController.auxAvailable()) {
                returnString << "<b>Aux Detected</b><br>";
            } else {
                returnString << "<b>No Aux Detected</b><br>";
            }
        } else if (leafMetadata.model() == "NL22") {
            // NL22 requires a separate rhtym controller, in most newer lights, this feature is
            // built in.
            returnString << "<b>No Rhythm Controller Detected</b><br>";
        }

        mMetadata->updateText(QString(returnString.str().c_str()));
    }

    /// pointer to comm data
    CommLayer* mComm;

    /// nanoleaf being displayed
    nano::LeafMetadata mLeaf;

    /// lights to display
    QLabel* mDisplayLights;

    /// name of the group
    QLabel* mName;

    /// show the state of the nanoleaf
    cor::Button* mStateButton;

    /// checkbox to select/deselect nanoleaf
    cor::CheckBox* mCheckBox;

    /// button to change name.
    QPushButton* mChangeName;

    /// button for deleting the currently selected nanoleaf
    QPushButton* mDeleteButton;

    /// widget for metadata
    cor::ExpandingTextScrollArea* mMetadata;

    /// label for schedules widget
    QLabel* mSchedulesLabel;

    /// widget for schedules.
    DisplayNanoleafSchedulesWidget* mSchedulesWidget;

    /// widget for greying out widgets in the background
    GreyOutOverlay* mGreyout;

    /// input to change the name of a light
    cor::TextInputWidget* mChangeNameInput;

    /// the height of a row in a scroll area
    int mRowHeight;
};


#endif // DISPLAYNANOLEAFCONTROLLERWIDGET_H
