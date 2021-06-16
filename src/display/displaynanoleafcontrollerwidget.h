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
#include "comm/nanoleaf/leafeffectpage.h"
#include "comm/nanoleaf/leafeffectwidget.h"
#include "comm/nanoleaf/leafmetadata.h"
#include "comm/nanoleaf/leafpanelimage.h"
#include "cor/widgets/button.h"
#include "cor/widgets/checkbox.h"
#include "cor/widgets/expandingtextscrollarea.h"
#include "cor/widgets/textinputwidget.h"
#include "displaynanoleafscheduleswidget.h"
#include "greyoutoverlay.h"
#include "menu/displaymoodmetadata.h"
#include "menu/groupstatelistmenu.h"
#include "menu/lightslistmenu.h"
#include "rotatelightwidget.h"
#include "speedwidget.h"
#include "syncwidget.h"

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
          mPanelImage{new nano::LeafPanelImage(this)},
          mDisplayLights{new QLabel(this)},
          mName{new QLabel(this)},
          mStateButton{new cor::Button(this, {})},
          mCheckBox{new cor::CheckBox(this)},
          mEffectsButton{new QPushButton("Effects", this)},
          mSpeedButton{new QPushButton("Change Speed", this)},
          mChangeName{new QPushButton("Change Name", this)},
          mChangeRotation{new QPushButton("Rotate", this)},
          mDeleteButton{new QPushButton("Delete", this)},
          mMetadata{new cor::ExpandingTextScrollArea(this)},
          mSchedulesLabel{new QLabel("<b>Schedules:</b>", this)},
          mCurrentEffectLabel{new QLabel("<b>Current Effect:</b>", this)},
          mSchedulesWidget{new DisplayNanoleafSchedulesWidget(this)},
          mGreyout{new GreyOutOverlay(true, parentWidget()->parentWidget())},
          mEffect{new nano::LeafEffectWidget({}, false, this)},
          mEffectsPage{new nano::LeafEffectPage(parentWidget()->parentWidget())},
          mSpeedWidget{new SpeedWidget(parentWidget()->parentWidget())},
          mChangeNameInput{new cor::TextInputWidget(parentWidget()->parentWidget())},
          mRotateLightWidget{new RotateLightWidget(parentWidget()->parentWidget())},
          mSyncWidget{new SyncWidget(this)},
          mRowHeight{10} {
        auto font = mName->font();
        font.setPointSize(20);
        mName->setFont(font);
        mName->setWordWrap(true);
        connect(mChangeNameInput, SIGNAL(textAdded(QString)), this, SLOT(nameChanged(QString)));
        connect(mChangeNameInput, SIGNAL(cancelClicked()), this, SLOT(closeNameWidget()));
        mChangeNameInput->setVisible(false);

        // TODO: turn on the speed button when the speed slider is fully implemented.
        mSpeedButton->setVisible(false);
        connect(mSpeedWidget, SIGNAL(cancelClicked()), this, SLOT(closeSpeedWidget()));
        mSpeedWidget->setVisible(false);

        connect(mEffectsButton, SIGNAL(clicked(bool)), this, SLOT(handleEffectsPressed()));
        connect(mSpeedButton, SIGNAL(clicked(bool)), this, SLOT(handleSpeedPressed()));
        connect(mChangeName, SIGNAL(clicked(bool)), this, SLOT(handleChangeNamePressed()));
        connect(mChangeRotation, SIGNAL(clicked(bool)), this, SLOT(rotateButtonPressed(bool)));
        connect(mDeleteButton, SIGNAL(clicked(bool)), this, SLOT(deleteButtonPressed(bool)));
        mDeleteButton->setStyleSheet(cor::kDeleteButtonBackground);

        mEffect->displayCheckbox(false);

        mDisplayLights->setAlignment(Qt::AlignHCenter | Qt::AlignVCenter);

        connect(mEffectsPage, SIGNAL(closePressed()), this, SLOT(closeEffectsPressed()));
        connect(mEffectsPage,
                SIGNAL(selectEffect(cor::LightID, QString)),
                this,
                SLOT(effectSelected(cor::LightID, QString)));
        mEffectsPage->setVisible(false);

        connect(mGreyout, SIGNAL(clicked()), this, SLOT(greyOutClicked()));
        mGreyout->greyOut(false);

        mStateButton->setIconPercent(0.85);

        mSchedulesLabel->setVisible(false);
        mSchedulesWidget->setVisible(false);

        connect(mCheckBox,
                SIGNAL(clicked(ECheckboxState)),
                this,
                SLOT(checkBoxClicked(ECheckboxState)));

        this->setStyleSheet(cor::kDarkerGreyBackground);

        connect(mRotateLightWidget, SIGNAL(cancelClicked()), this, SLOT(rotateWidgetClosed()));
        connect(mRotateLightWidget,
                SIGNAL(valueChanged(int)),
                this,
                SLOT(rotateWidgetChangedAngle(int)));

        mStateButton->setVisible(false);
    }

    /// getter for controller represented by the widget
    const nano::LeafMetadata& metadata() const noexcept { return mLeaf; }

    /// getter for discovery state.
    nano::ELeafDiscoveryState discoveryState() const noexcept { return mDiscoveryState; }

    /// updates the meatadata's UI elements.
    void updateLeafMetadata(const nano::LeafMetadata& leafMetadata,
                            nano::ELeafDiscoveryState discoveryState,
                            bool isSelected) {
        mLeaf = leafMetadata;
        mDiscoveryState = discoveryState;

        mEffect->update(mLeaf.currentEffect(), true);
        if (mEffectsPage->currentEffect() != leafMetadata.currentEffectName()) {
            mEffectsPage->updateEffects(mLeaf.currentEffect(), mLeaf.effects().items());
        }
        mEffectsPage->setNanoleaf(leafMetadata);

        if (isSelected) {
            mCheckBox->checkboxState(ECheckboxState::checked);
        } else {
            mCheckBox->checkboxState(ECheckboxState::unchecked);
        }

        if (mDiscoveryState != nano::ELeafDiscoveryState::connected) {
            mStateButton->setVisible(false);
            mEffectsButton->setVisible(false);
            mChangeRotation->setVisible(false);
            mChangeName->setVisible(false);
            mDisplayLights->setVisible(false);
            mCheckBox->setVisible(false);
            mSyncWidget->setVisible(true);
            mCurrentEffectLabel->setVisible(false);
            mEffect->setVisible(false);
            mSyncWidget->changeState(ESyncState::syncing);
        } else {
            mStateButton->setVisible(false); // currently hidden
            mEffectsButton->setVisible(true);
            mChangeRotation->setVisible(true);
            mChangeName->setVisible(true);
            mCheckBox->setVisible(true);
            mDisplayLights->setVisible(true);
            mSyncWidget->setVisible(false);
            mCurrentEffectLabel->setVisible(true);
            mEffect->setVisible(true);
            mSyncWidget->changeState(ESyncState::synced);
        }
        if (!leafMetadata.name().isEmpty()) {
            mName->setText(leafMetadata.name());
        } else {
            mName->setText(leafMetadata.hardwareName());
        }
        auto light = mComm->nanoleaf()->lightFromMetadata(leafMetadata);
        if (light.second) {
            mState = light.first.state();
            if (mState.isOn()) {
                mCurrentEffectLabel->setText("<b>Current Effect:</b>");
            } else {
                mCurrentEffectLabel->setText("Currently Off, Effect if On:");
            }
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
        int ySpacer = this->height() / 15;

        mGreyout->resize();

        // top of both
        auto nameWidth = this->width() - xSpacer - buttonHeight * 2;
        mName->setGeometry(xSpacer / 2, yPosColumn1, nameWidth, buttonHeight);
        headerX += mName->width() + mName->x();

        mStateButton->setGeometry(headerX, yPosColumn1, buttonHeight, buttonHeight);
        headerX += mStateButton->width() + xSpacer / 2;
        mCheckBox->setGeometry(headerX, yPosColumn1, buttonHeight, buttonHeight);
        headerX += mCheckBox->width();

        yPosColumn1 += mName->height();
        yPosColumn2 += mName->height();

        mDisplayLights->setGeometry(xSpacer / 2,
                                    yPosColumn1,
                                    this->width() - xSpacer / 2,
                                    buttonHeight * 4);
        mSyncWidget->setGeometry(xSpacer / 2,
                                 yPosColumn1,
                                 this->width() - xSpacer / 2,
                                 buttonHeight * 4);
        drawPanels();
        yPosColumn1 += mDisplayLights->height();
        yPosColumn2 += mDisplayLights->height();


        // column 1
        mCurrentEffectLabel->setGeometry(xSpacer, yPosColumn1, columnWidth, buttonHeight * 0.75);
        yPosColumn1 += mCurrentEffectLabel->height();
        mEffect->setGeometry(xSpacer, yPosColumn1, columnWidth, buttonHeight * 2);
        yPosColumn1 += mEffect->height();
        mMetadata->setGeometry(xSpacer, yPosColumn1, columnWidth, buttonHeight * 3 - ySpacer);
        yPosColumn1 += mMetadata->height();
        // mChangeName->setGeometry(xSpacer, yPosColumn1, columnWidth, buttonHeight);

        // column 2
        mEffectsButton->setGeometry(xSecondColumnStart, yPosColumn2, columnWidth, buttonHeight);
        yPosColumn2 += mEffectsButton->height();
        if (mSpeedButton->isVisible()) {
            mSpeedButton->setGeometry(xSecondColumnStart, yPosColumn2, columnWidth, buttonHeight);
            yPosColumn2 += mSpeedButton->height();
        }
        mChangeName->setGeometry(xSecondColumnStart, yPosColumn2, columnWidth, buttonHeight);
        yPosColumn2 += mChangeName->height();
        mChangeRotation->setGeometry(xSecondColumnStart, yPosColumn2, columnWidth, buttonHeight);
        yPosColumn2 += mChangeRotation->height();

        //        mSchedulesLabel->setGeometry(xSecondColumnStart,
        //                                     yPosColumn2,
        //                                     columnWidth,
        //                                     buttonHeight * 0.5);
        //        yPosColumn2 += mSchedulesLabel->height();
        //        mSchedulesWidget->setGeometry(xSecondColumnStart,
        //                                      yPosColumn2,
        //                                      columnWidth,
        //                                      buttonHeight * 3.5);
        //        yPosColumn2 += mSchedulesWidget->height();

        mDeleteButton->setGeometry(xSecondColumnStart, yPosColumn2, columnWidth, buttonHeight);
        yPosColumn2 += mDeleteButton->height();

        if (mLastSize != size()) {
            if (mChangeNameInput->isOpen()) {
                mGreyout->raise();
                mChangeNameInput->resize();
                mChangeNameInput->raise();
            }

            if (mSpeedWidget->isOpen()) {
                mGreyout->raise();
                mSpeedWidget->resize();
                mSpeedWidget->raise();
            }

            if (mEffectsPage->isOpen()) {
                mGreyout->raise();
                mEffectsPage->resize();
                mEffectsPage->raise();
            }

            if (mRotateLightWidget->isOpen()) {
                mGreyout->raise();
                mRotateLightWidget->resize();
                mRotateLightWidget->raise();
            }
        }
        mLastSize = size();
    }
signals:

    /// emit when a light is deleted
    void deleteNanoleaf(cor::LightID, QString);

    /// emits when a light should be selected
    void selectLight(cor::LightID);

    /// emits when a light should be deselected
    void deselectLight(cor::LightID);

    /// emits when an effect is selected. Signals the light's serial number and the desired effect.
    void selectEffect(cor::LightID, QString);

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

private slots:

    /// handle when the checkbox is clicked
    void checkBoxClicked(ECheckboxState state) {
        if (state == ECheckboxState::checked) {
            mCheckBox->checkboxState(ECheckboxState::unchecked);
            emit deselectLight(mLeaf.serialNumber());
        } else {
            mCheckBox->checkboxState(ECheckboxState::checked);
            emit selectLight(mLeaf.serialNumber());
        }
    }

    /// handle when the effects widget is closed.
    void closeEffectsPressed() {
        mGreyout->greyOut(false);
        mEffectsPage->pushOut();
    }

    /// handle when the name change is pressed
    void handleChangeNamePressed() {
        mGreyout->greyOut(true);
        mChangeNameInput->pushIn("Change Name of Nanoleaf", mLeaf.name());
        mChangeNameInput->setVisible(true);
        mChangeNameInput->raise();
    }

    /// handles when the effects button pressed
    void handleEffectsPressed() {
        mGreyout->greyOut(true);
        mEffectsPage->updateEffects(mLeaf.currentEffect(), mLeaf.effects().items());
        mEffectsPage->pushIn("View Effects");
        mEffectsPage->setVisible(true);
        mEffectsPage->raise();
    }

    void effectSelected(cor::LightID light, QString effect) { emit selectEffect(light, effect); }

    /// handles when the speed button is pressed
    void handleSpeedPressed() {
        mGreyout->greyOut(true);
        mSpeedWidget->raise();
        mSpeedWidget->pushIn(EProtocolType::nanoleaf, mState.speed());
        mSpeedWidget->setVisible(true);
    }

    /// change a name of a nanoleaf.
    void nameChanged(const QString& name) {
        if (!name.isEmpty()) {
            mComm->nanoleaf()->renameLight(mLeaf, name);
            mLeaf.name(name);
            updateLeafMetadata(mLeaf,
                               mDiscoveryState,
                               mCheckBox->checkboxState() == ECheckboxState::checked);
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

    /// close the speed widget
    void closeSpeedWidget() {
        mGreyout->greyOut(false);
        mSpeedWidget->pushOut();
    }

    /// grey out clicked
    void greyOutClicked() {
        mChangeNameInput->pushOut();
        mSpeedWidget->pushOut();
        mEffectsPage->pushOut();
        mGreyout->greyOut(false);

        if (mRotateLightWidget->isOpen()) {
            mRotateLightWidget->pushOut();
        }
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
            emit deleteNanoleaf(mLeaf.serialNumber(), mLeaf.IP());
        }
    }

    /// rotate button pressed
    void rotateButtonPressed(bool) {
        mGreyout->greyOut(true);
        setVisible(true);
        mRotateLightWidget->raise();
        mRotateLightWidget->pushIn();

        mRotateLightWidget->setNanoleaf(mLeaf,
                                        mLeaf.panels().orientationValue(),
                                        mState.palette(),
                                        mState.isOn());
    }

    /// rotate widget closed
    void rotateWidgetClosed() {
        mGreyout->greyOut(false);
        mRotateLightWidget->pushOut();
    }

    /// rotation changed angle
    void rotateWidgetChangedAngle(int angle) {
        mLeaf.changeOrientation(angle);
        mComm->nanoleaf()->globalOrientationChange(mLeaf, angle);
        drawPanels();
        mGreyout->greyOut(false);
        mRotateLightWidget->pushOut();
    }

private:
    /// update the metadata for the nano::LeafMetadata
    void updateMetadata(const nano::LeafMetadata& leafMetadata) {
        std::stringstream returnString;
        returnString << "<b>IP:</b> " << leafMetadata.IP().toStdString() << "<br>";
        returnString << "<b>Port:</b> " << leafMetadata.port() << "<br><br>";
        returnString << "<b>Hardware Name:</b> " << leafMetadata.hardwareName().toStdString()
                     << "<br>";

        if (mDiscoveryState == nano::ELeafDiscoveryState::connected) {
            returnString << "<b>Model:</b> " << leafMetadata.model().toStdString() << "<br>";
            returnString << "<b>Firmware:</b> " << leafMetadata.firmware().toStdString() << "<br>";
            returnString << "<b>Serial:</b> " << leafMetadata.serialNumber().toStdString()
                         << "<br><br>";
            returnString << "<b>Hardware Version:</b> "
                         << leafMetadata.hardwareVersion().toStdString() << "<br>";

            if (leafMetadata.rhythmController().isConnected()) {
                auto rhythmController = leafMetadata.rhythmController();
                returnString << "<br><br><b>Rhythm Controller</b><br>";
                returnString << "<b>ID: </b>" << rhythmController.ID() << "<br>";
                returnString << "<b>Firmware: </b>"
                             << rhythmController.firmwareVersion().toStdString() << "<br>";
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
        }
        mMetadata->updateText(QString(returnString.str().c_str()));
    }

    /// converts an effect name to a more user-friendly name
    QString effectToPrettyName(const QString& effect) {
        if (effect == nano::kTemporaryEffect) {
            return "Custom";
        }
        return effect;
    }

    /// draws the nanoleafs actual layout and rotation.
    void drawPanels() {
        // render the image for the panel
        mPanelImage->drawPanels(mLeaf.panels(),
                                mLeaf.panels().orientationValue(),
                                mState.palette(),
                                mState.isOn());
        // draw the image to the panel label
        mPanelPixmap.convertFromImage(mPanelImage->image());
        if (!mPanelPixmap.isNull()) {
            mPanelPixmap = mPanelPixmap.scaled(mDisplayLights->width(),
                                               mDisplayLights->height(),
                                               Qt::KeepAspectRatio,
                                               Qt::SmoothTransformation);
            mDisplayLights->setPixmap(mPanelPixmap);
        }
    }

    /// pointer to comm data
    CommLayer* mComm;

    /// nanoleaf being displayed
    nano::LeafMetadata mLeaf;

    /// state of the light
    cor::LightState mState;

    /// state of discovery for nanoleaf.
    nano::ELeafDiscoveryState mDiscoveryState;

    /// generates the panel image
    nano::LeafPanelImage* mPanelImage;

    /// pixmap that stores the panel image.
    QPixmap mPanelPixmap;

    /// lights to display
    QLabel* mDisplayLights;

    /// name of the group
    QLabel* mName;

    /// show the state of the nanoleaf
    cor::Button* mStateButton;

    /// checkbox to select/deselect nanoleaf
    cor::CheckBox* mCheckBox;

    /// button to view the effects stored on the light..
    QPushButton* mEffectsButton;

    /// button to change speed of lights.
    QPushButton* mSpeedButton;

    /// button to change name.
    QPushButton* mChangeName;

    /// button to change rotation of lights.
    QPushButton* mChangeRotation;

    /// button for deleting the currently selected nanoleaf
    QPushButton* mDeleteButton;

    /// widget for metadata
    cor::ExpandingTextScrollArea* mMetadata;

    /// label for schedules widget
    QLabel* mSchedulesLabel;

    /// label above the effect.
    QLabel* mCurrentEffectLabel;

    /// widget for schedules.
    DisplayNanoleafSchedulesWidget* mSchedulesWidget;

    /// widget for greying out widgets in the background
    GreyOutOverlay* mGreyout;

    /// effect to display.
    nano::LeafEffectWidget* mEffect;

    /// widget for effects.
    nano::LeafEffectPage* mEffectsPage;

    /// widget for changing the speed of the nanoleaf.
    SpeedWidget* mSpeedWidget;

    /// input to change the name of a light
    cor::TextInputWidget* mChangeNameInput;

    /// widget to handle rotating a light.
    RotateLightWidget* mRotateLightWidget;

    /// sync widget to display when searching for the light
    SyncWidget* mSyncWidget;

    /// the height of a row in a scroll area
    int mRowHeight;

    /// the last size of the widget.
    QSize mLastSize;
};


#endif // DISPLAYNANOLEAFCONTROLLERWIDGET_H
