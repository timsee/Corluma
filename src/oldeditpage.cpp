/*!
 * \copyright
 * Copyright (C) 2015 - 2020.
 * Released under the GNU General Public License.
 */

#include "oldeditpage.h"

#include <QMessageBox>
#include <QPainter>
#include <QScroller>

#include "utils/qt.h"

OldEditPage::OldEditPage(QWidget* parent, CommLayer* comm, GroupData* parser)
    : QWidget(parent),
      mComm(comm),
      mGroups(parser) {
    mTopMenu = new EditPageTopMenu(this);
    mTopMenu->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

    connect(mTopMenu->closeButton(), SIGNAL(clicked(bool)), this, SLOT(closePressed(bool)));
    connect(mTopMenu->resetButton(), SIGNAL(clicked(bool)), this, SLOT(resetPressed(bool)));
    connect(mTopMenu->deleteButton(), SIGNAL(clicked(bool)), this, SLOT(deletePressed(bool)));
    connect(mTopMenu->saveButton(), SIGNAL(clicked(bool)), this, SLOT(savePressed(bool)));
    connect(mTopMenu->nameEdit(),
            SIGNAL(textChanged(QString)),
            this,
            SLOT(lineEditChanged(QString)));

    mSimpleGroupWidget = new ListSimpleGroupWidget(this, cor::EListType::linear);
    mSimpleGroupWidget->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    connect(mSimpleGroupWidget, SIGNAL(deviceClicked(QString)), this, SLOT(clickedDevice(QString)));
    QScroller::grabGesture(mSimpleGroupWidget->viewport(), QScroller::LeftMouseButtonGesture);
    mSimpleGroupWidget->setStyleSheet("background-color:rgba(33,32,32,255);");
}

void OldEditPage::updateDevices(const std::vector<cor::Light>& checkedDevices,
                                const std::vector<cor::Light>& devices) {
    mSimpleGroupWidget->updateDevices(devices, cor::EWidgetType::full, true, false);
    mSimpleGroupWidget->setCheckedDevices(checkedDevices);
}

// ----------------------------
// Slots
// ----------------------------

void OldEditPage::closePressed(bool) {
    if (checkForChanges()) {
        QMessageBox::StandardButton reply;
        reply = QMessageBox::question(this,
                                      "Changes",
                                      "Changes were made, save the changes?",
                                      QMessageBox::Yes | QMessageBox::No);
        if (reply == QMessageBox::Yes) {
            auto saveSuccessful = saveChanges();
            if (!saveSuccessful) {
                QMessageBox msgBox;
                msgBox.setText("Saving unsuccessful, no changes will be made.");
                msgBox.exec();
            }
        }
    }
    emit pressedClose();
}

void OldEditPage::resetPressed(bool) {
    reset();
}

void OldEditPage::savePressed(bool) {
    if (checkForChanges()) {
        QMessageBox::StandardButton reply;
        reply = QMessageBox::question(this,
                                      "Changes",
                                      "Changes were made, save the changes?",
                                      QMessageBox::Yes | QMessageBox::No);
        if (reply == QMessageBox::Yes) {
            auto saveSuccessful = saveChanges();
            if (!saveSuccessful) {
                QMessageBox msgBox;
                msgBox.setText("Saving unsuccessful, no changes will be made.");
                msgBox.exec();
            }
            // close
            emit pressedClose();
        }
    }
}


// ----------------------------
// Protected
// ----------------------------


void OldEditPage::resize() {
    auto yPos = 0;
    mTopMenu->setGeometry(0, yPos, width(), height() / 5);
    yPos += mTopMenu->height();

    mSimpleGroupWidget->setGeometry(0, yPos, width(), height() * 4 / 5);

    mSimpleGroupWidget->resizeWidgets();
}

void OldEditPage::resizeEvent(QResizeEvent*) {
    resize();
}

void OldEditPage::paintEvent(QPaintEvent*) {
    QStyleOption opt;
    opt.init(this);
    QPainter painter(this);

    painter.setRenderHint(QPainter::Antialiasing);
    painter.fillRect(rect(), QBrush(QColor(48, 47, 47)));
}

void OldEditPage::lineEditChanged(const QString& newText) {
    changeName(newText);

    if (checkForChanges()) {
        mTopMenu->saveButton()->setEnabled(true);
    } else {
        mTopMenu->saveButton()->setEnabled(false);
    }
}

// ----------------------------
// Private
// ----------------------------

bool OldEditPage::shouldSetChecked(const cor::Light& device,
                                   const std::vector<cor::Light>& groupDevices) {
    for (const auto& collectionDevice : groupDevices) {
        if (device.uniqueID() == collectionDevice.uniqueID()) {
            return true;
        }
    }
    return false;
}

void OldEditPage::clickedDevice(const QString&) {
    // call the highlight button
    if (checkForChanges()) {
        mTopMenu->saveButton()->setEnabled(true);
    } else {
        mTopMenu->saveButton()->setEnabled(false);
    }
}

void OldEditPage::pushIn() {
    moveWidget(
        this,
        QPoint(int(parentWidget()->width() * 0.125f), int(-1 * parentWidget()->height())),
        QPoint(int(parentWidget()->width() * 0.125f), int(parentWidget()->height() * 0.125f)));
    raise();
    setVisible(true);
}

void OldEditPage::pushOut() {
    moveWidget(
        this,
        QPoint(int(parentWidget()->width() * 0.125f), int(parentWidget()->height() * 0.125f)),
        QPoint(int(parentWidget()->width() * 0.125f), int(-1 * parentWidget()->height())));
}
