#ifndef DISPLAYNANOLEAFCONTROLLERWIDGET_H
#define DISPLAYNANOLEAFCONTROLLERWIDGET_H

#include <QPainter>
#include <QPushButton>
#include <QStyleOption>
#include <QTextEdit>
#include <QWidget>
#include "comm/commlayer.h"
#include "comm/commnanoleaf.h"
#include "comm/nanoleaf/leafmetadata.h"
#include "cor/widgets/expandingtextscrollarea.h"
#include "cor/widgets/textinputwidget.h"
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
          mName{new QLabel(this)},
          mLightsLabel{new QLabel("<b>Lights:</b>", this)},
          mLights{new LightsListMenu(this, false)},
          mChangeName{new QPushButton("Change Name", this)},
          mMetadata{new cor::ExpandingTextScrollArea(this)},
          mGreyout{new GreyOutOverlay(true, parentWidget()->parentWidget())},
          mChangeNameInput{new cor::TextInputWidget(parentWidget()->parentWidget())} {
        auto font = mName->font();
        font.setPointSize(20);
        mName->setFont(font);
        connect(mChangeNameInput, SIGNAL(textAdded(QString)), this, SLOT(nameChanged(QString)));
        connect(mChangeNameInput, SIGNAL(cancelClicked()), this, SLOT(closeNameWidget()));
        mChangeNameInput->setVisible(false);

        connect(mGreyout, SIGNAL(clicked()), this, SLOT(greyOutClicked()));
        mGreyout->greyOut(false);

        connect(mChangeName, SIGNAL(clicked(bool)), this, SLOT(handleChangeNamePressed()));
        this->setStyleSheet("background-color:rgb(33,32,32);");
    }

    /// getter for controller represented by the widget
    const nano::LeafMetadata& metadata() const noexcept { return mLeaf; }

    /// updates the meatadata's UI elements.
    void updateLeafMetadata(const nano::LeafMetadata& leafMetadata) {
        mLeaf = leafMetadata;
        if (!leafMetadata.name().isEmpty()) {
            mName->setText(leafMetadata.name());
        } else {
            mName->setText(leafMetadata.hardwareName());
        }
        // auto light = mComm->arducor()->lightsFromNames(mController.names());
        //        mLights->updateLights();
        //        mLights->showLights(lights);
        updateMetadata(leafMetadata);
        resize();
    }

    /// changes the row height of rows in scroll areas.
    void changeRowHeight(int height) { mRowHeight = height; }

    /// reset the widget to showing no group
    void reset() {
        mLeaf = {};
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

        mGreyout->resize();

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
        mMetadata->setGeometry(xSecondColumnStart, yPosColumn2, columnWidth, buttonHeight * 4);
        yPosColumn2 += mMetadata->height();
        mChangeName->setGeometry(xSecondColumnStart, yPosColumn2, columnWidth, buttonHeight);
        yPosColumn2 += mChangeName->height();
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


private slots:
    void handleChangeNamePressed() {
        mGreyout->greyOut(true);
        mChangeNameInput->pushIn("Change Name of Nanoleaf", mLeaf.name());
        mChangeNameInput->raise();
        mChangeNameInput->setVisible(true);
    }

    void nameChanged(const QString& name) {
        if (!name.isEmpty()) {
            mComm->nanoleaf()->renameLight(mLeaf, name);
            mLeaf.name(name);
            updateLeafMetadata(mLeaf);
            mGreyout->greyOut(false);
            mChangeNameInput->pushOut();
        }
    }

    void closeNameWidget() {
        mGreyout->greyOut(false);
        mChangeNameInput->pushOut();
    }

    void greyOutClicked() {
        mChangeNameInput->pushOut();
        mGreyout->greyOut(false);
    }

private:
    /// update the metadata for the nano::LeafMetadata
    void updateMetadata(const nano::LeafMetadata& leafMetadata) {
        std::stringstream returnString;

        returnString << "<b>IP:</b> " << leafMetadata.IP().toStdString() << "<br>";
        returnString << "<b>Port:</b> " << leafMetadata.port() << "<br>";
        returnString << "<b>Serial:</b> " << leafMetadata.serialNumber().toStdString() << "<br>";
        returnString << "<b>Firmware:</b> " << leafMetadata.firmware().toStdString() << "<br>";
        returnString << "<b>Hardware Name:</b> " << leafMetadata.hardwareName().toStdString()
                     << "<br>";
        returnString << "<b>Model:</b> " << leafMetadata.model().toStdString() << "<br>";

        mMetadata->updateText(QString(returnString.str().c_str()));
    }

    /// pointer to comm data
    CommLayer* mComm;

    /// nanoleaf being displayed
    nano::LeafMetadata mLeaf;

    /// name of the group
    QLabel* mName;

    /// label for lights
    QLabel* mLightsLabel;

    /// displays the lights that are part of this group and their current states.
    LightsListMenu* mLights;

    /// button to change name.
    QPushButton* mChangeName;

    /// widget for metadata
    cor::ExpandingTextScrollArea* mMetadata;

    /// widget for greying out widgets in the background
    GreyOutOverlay* mGreyout;

    /// input to change the name of a light
    cor::TextInputWidget* mChangeNameInput;

    /// the height of a row in a scroll area
    int mRowHeight;
};


#endif // DISPLAYNANOLEAFCONTROLLERWIDGET_H
